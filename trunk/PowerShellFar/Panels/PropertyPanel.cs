
/*
PowerShellFar module for Far Manager
Copyright (c) 2006 Roman Kuzmin
*/

using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Text;
using FarNet;
using Microsoft.PowerShell.Commands;

namespace PowerShellFar
{
	/// <summary>
	/// Panel exploring PowerShell provider item properties.
	/// </summary>
	public sealed class PropertyPanel : ListPanel
	{
		string _itemPath;

		PSObject _item;
		ProviderInfo _provider;

		/// <summary>
		/// Creates a property panel for an item.
		/// </summary>
		/// <param name="itemPath">Item path.</param>
		public PropertyPanel(string itemPath)
		{
			// get item; 090409
			_item = A.Psf.Engine.InvokeProvider.Item.Get(new string[] { itemPath }, true, true)[0];
			_itemPath = itemPath;

			// get its provider
			PSPropertyInfo pi = _item.Properties["PSProvider"];
			if (pi == null)
				throw new InvalidOperationException();

			_provider = pi.Value as ProviderInfo;
			if (_provider == null || !My.ProviderInfoEx.HasProperty(_provider))
				throw new InvalidOperationException(Res.NotSupportedByProvider);

			Title = "Properties: " + _itemPath;
			PanelDirectory = _itemPath + ".*"; //??
			SortMode = PanelSortMode.Name;
		}

		internal override PSObject Target
		{
			get { return _item; }
		}

		internal override void OnUpdateFiles(PanelEventArgs e)
		{
			try
			{
				// reset
				Files.Clear();

				//! get properties
				// - Using -LiteralPath is a problem, e.g. Registry: returns nothing.
				// - Script is used for PS conversion of property values to string.
				// - Script has to ignore a property with empty name (if any, can be in Registry).
				// - If PS* included then they can't be found by `gp <path> <name>`;
				// so, don't add, they are noisy anyway (even if marked system or hidden).

				// get property bag 090409
				Collection<PSObject> bag = A.Psf.Engine.InvokeProvider.Property.Get(Kit.EscapeWildcard(_itemPath), null);

				// filter
				var filter = new List<string>(5);
				filter.Add("PSChildName");
				filter.Add("PSDrive");
				filter.Add("PSParentPath");
				filter.Add("PSPath");
				filter.Add("PSProvider");

				// add
				foreach (PSObject o in bag)
				{
					foreach (PSPropertyInfo pi in o.Properties)
					{
						// skip empty ?? still needed?
						string name = pi.Name;
						if (string.IsNullOrEmpty(name))
							continue;

						// filter and shrink filter
						int i = filter.IndexOf(name);
						if (i >= 0)
						{
							filter.RemoveAt(i);
							continue;
						}

						// create file
						SetFile file = new SetFile()
						{
							Name = name,
							IsReadOnly = !pi.IsSettable,
							Data = pi,
							// set its value
							Description = Converter.FormatValue(pi.Value, A.Psf.Settings.FormatEnumerationLimit)
						};

						// add
						Files.Add(file);
					}
				}
			}
			catch (RuntimeException error)
			{
				if ((e.Mode & (OperationModes.Find | OperationModes.Silent)) == 0)
					A.Msg(error.Message);
			}
		}

		internal override void UICreate()
		{
			if (!My.ProviderInfoEx.HasDynamicProperty(_provider))
			{
				A.Msg(Res.NotSupportedByProvider);
				return;
			}

			UI.NewValueDialog ui = new UI.NewValueDialog("New property");
			while (ui.Dialog.Show())
			{
				try
				{
					using (PowerShell p = A.Psf.CreatePipeline())
					{
						//! Don't use Value if it is empty (e.g. to avoid (default) property at new key in Registry).
						//! Don't use -Force or you silently kill existing item\property (with all children, properties, etc.)
						Command c = new Command("New-ItemProperty");
						c.Parameters.Add("LiteralPath", _itemPath);
						c.Parameters.Add(Word.Name, ui.Name.Text);
						c.Parameters.Add("PropertyType", ui.Type.Text);

						if (ui.Value.Text.Length > 0)
							c.Parameters.Add("Value", ui.Value.Text);
						c.Parameters.Add(Prm.ErrorAction, ActionPreference.Continue);
						p.Commands.AddCommand(c);
						p.Invoke();
						if (A.ShowError(p))
							continue;
					}

					// update this panel with name
					UpdateRedraw(false, ui.Name.Text);

					// update that panel if the path is the same
					PropertyPanel pp2 = AnotherPanel as PropertyPanel;
					if (pp2 != null && pp2._itemPath == _itemPath)
						pp2.UpdateRedraw(true);

					// exit the loop
					return;
				}
				catch (RuntimeException exception)
				{
					A.Msg(exception.Message);
					continue;
				}
			}
		}

		internal override void SetUserValue(PSPropertyInfo info, string value)
		{
			try
			{
				A.SetPropertyValue(_itemPath, info.Name, Converter.Parse(info, value));
				WhenPropertyChanged(_itemPath);
			}
			catch (RuntimeException ex)
			{
				A.Msg(ex.Message);
			}
		}

		internal override void UICopyHere()
		{
			if (!My.ProviderInfoEx.HasDynamicProperty(_provider))
			{
				A.Msg(Res.NotSupportedByProvider);
				return;
			}

			FarFile f = CurrentFile;
			if (f == null)
				return;
			string name = f.Name;

			IInputBox ib = Far.Net.CreateInputBox();
			ib.Title = "Copy";
			ib.Prompt = "New name";
			ib.History = "Copy";
			ib.Text = name;
			if (!ib.Show() || ib.Text == name)
				return;

			string src = Kit.EscapeWildcard(_itemPath);
			A.Psf.Engine.InvokeProvider.Property.Copy(src, name, src, ib.Text);

			UpdateRedraw(false, ib.Text);
		}

		internal override bool UICopyMove(bool move)
		{
			if (base.UICopyMove(move))
				return true;

			if (!My.ProviderInfoEx.HasDynamicProperty(_provider))
			{
				A.Msg(Res.NotSupportedByProvider);
				return true;
			}

			PropertyPanel pp2 = AnotherPanel as PropertyPanel;
			if (pp2 == null)
			{
				A.Msg("You can Copy|Move properties only to another property panel.");
				return true;
			}

			// collect
			IList<string> names = CollectSelectedNames();
			if (names.Count == 0)
				return true;

			//! gotchas in {Copy|Move}-ItemProperty:
			//! *) -Name takes a single string only? (help: yes (copy), no (move) - odd!)
			//! *) Names can't be pipelined (help says they can)
			//! so, use provider directly (no confirmation)
			string src = Kit.EscapeWildcard(_itemPath);
			string dst = Kit.EscapeWildcard(pp2._itemPath);
			if (move)
			{
				foreach (string name in names)
					A.Psf.Engine.InvokeProvider.Property.Move(src, name, dst, name);
			}
			else
			{
				foreach (string name in names)
					A.Psf.Engine.InvokeProvider.Property.Copy(src, name, dst, name);
			}

			// update this
			UpdateRedraw(move);

			// update that
			if (pp2 != null)
				pp2.UpdateRedraw(true);
			return true;
		}

		internal override void UIRename()
		{
			FarFile f = CurrentFile;
			if (f == null)
				return;
			string name = f.Name;

			//! Registry: workaround: (default)
			if (Kit.Equals(name, "(default)") && _provider.ImplementingType == typeof(RegistryProvider))
			{
				A.Msg("Cannot rename this property.");
				return;
			}

			IInputBox ib = Far.Net.CreateInputBox();
			ib.Title = "Rename";
			ib.Prompt = "New name";
			ib.History = "Copy";
			ib.Text = name;
			if (!ib.Show() || ib.Text == name)
				return;

			using (PowerShell p = A.Psf.CreatePipeline())
			{
				Command c = new Command("Rename-ItemProperty");
				c.Parameters.Add(new CommandParameter("LiteralPath", _itemPath));
				c.Parameters.Add(new CommandParameter(Word.Name, name));
				c.Parameters.Add(new CommandParameter("NewName", ib.Text));
				c.Parameters.Add(Prm.Force);
				c.Parameters.Add(Prm.ErrorAction, ActionPreference.Continue);
				p.Commands.AddCommand(c);
				p.Invoke();
				if (!A.ShowError(p))
					UpdateRedraw(true, ib.Text);
			}
		}

		/// <summary>
		/// Should be called when an item property is changed.
		/// </summary>
		internal static void WhenPropertyChanged(string itemPath)
		{
			PropertyPanel p = Far.Net.FindPanel(typeof(PropertyPanel)) as PropertyPanel;
			if (p == null)
				return;

			if (p._itemPath == itemPath)
				p.UpdateRedraw(true);

			p = p.AnotherPanel as PropertyPanel;
			if (p == null)
				return;

			if (p._itemPath == itemPath)
				p.UpdateRedraw(true);
		}

		//! just a reminder: Get-ItemProperty: -Name takes wildcard, so that use EscapeWildcard for it
		internal override void WriteFile(FarFile file, string path)
		{
			foreach (PSObject o in A.Psf.Engine.InvokeProvider.Property.Get(Kit.EscapeWildcard(_itemPath), null))
			{
				PSPropertyInfo pi = o.Properties[file.Name];
				if (pi == null)
					return;

				A.Psf.InvokeCode(
					"$args[0] | Format-List Name, IsSettable, IsInstance, TypeNameOfValue, Value | Out-File $args[1] -Width ([int]::MaxValue)",
					pi, path);
				return;
			}
		}

		internal override void DoDeleteFiles(FilesEventArgs args)
		{
			// delete value = enter null
			if (args.Move)
			{
				args.Move = false;
				base.DoDeleteFiles(args);
				return;
			}

			// skip not suitable provider
			if (!My.ProviderInfoEx.HasDynamicProperty(_provider))
			{
				A.Msg(Res.NotSupportedByProvider);
				return;
			}

			IList<string> names = CollectNames(args.Files);

			// confirmation
			FarConfirmations conf = Far.Net.Confirmations;

			//! Registry: workaround: (default)
			if (_provider.ImplementingType == typeof(RegistryProvider))
			{
				for (int i = names.Count; --i >= 0; )
				{
					if (Kit.Equals(names[i], "(default)"))
					{
						if ((conf & (FarConfirmations.Delete)) == 0 ||
							Far.Net.Message("Are you sure you want to delete (default) property?", Res.Delete, MsgOptions.YesNo) == 0)
							A.Psf.Engine.InvokeProvider.Property.Remove(Kit.EscapeWildcard(_itemPath), string.Empty);
						names.RemoveAt(i);
						break;
					}
				}
				if (names.Count == 0)
				{
					UpdateRedraw(false);
					return;
				}
			}

			using (PowerShell p = A.Psf.CreatePipeline())
			{
				Command c = new Command("Remove-ItemProperty");
				c.Parameters.Add("LiteralPath", _itemPath);
				c.Parameters.Add(Word.Name, names);
				if ((conf & FarConfirmations.Delete) != 0)
					c.Parameters.Add(Prm.Confirm);
				c.Parameters.Add(Prm.Force);
				c.Parameters.Add(Prm.ErrorAction, ActionPreference.Continue);
				p.Commands.AddCommand(c);
				p.Invoke();

				// ?? V2 CTP3 bug: Registry: Remove-ItemProperty -Confirm fails on 'No':
				// Remove-ItemProperty : Property X does not exist at path HKEY_CURRENT_USER\Y
				// There is no workaround added yet. Submitted: MS Connect #484664.
				if (A.ShowError(p))
					Update(true);
				else
					Update(false);

				Redraw();

				PropertyPanel pp2 = AnotherPanel as PropertyPanel;
				if (pp2 != null)
					pp2.UpdateRedraw(true);
			}
		}

		internal override void DoEditFile(FarFile file)
		{
			if (file == null)
				return;

			PSPropertyInfo pi = file.Data as PSPropertyInfo;
			if (pi == null)
				return;

			if (!pi.IsSettable)
				A.Msg(Res.PropertyIsNotSettable);

			try
			{
				string temp = Far.Net.TempName();
				string line = Converter.InfoToLine(pi);
				if (line == null)
				{
					// write by PS
					//! use encoding name
					A.Psf.InvokeCode("$args[0] | Out-File -FilePath $args[1] -Width $args[2] -Encoding Unicode -ErrorAction Stop", pi.Value, temp, int.MaxValue);
				}
				else
				{
					File.WriteAllText(temp, line, Encoding.Unicode);
				}

				// editor
				PropertyEditor edit = new PropertyEditor();
				edit.Open(temp, true, _itemPath, pi);

			}
			catch (RuntimeException ex)
			{
				Far.Net.ShowError("Edit", ex);
			}
		}

		///
		internal override void HelpMenuInitItems(HelpMenuItems items, PanelMenuEventArgs e)
		{
			if (items.Copy == null)
				items.Copy = new SetItem()
				{
					Text = "&Copy property(s)",
					Click = delegate { UICopyMove(false); }
				};

			if (items.CopyHere == null)
				items.CopyHere = new SetItem()
				{
					Text = "Copy &here",
					Click = delegate { UICopyHere(); }
				};

			if (items.Move == null)
				items.Move = new SetItem()
				{
					Text = "&Move property(s)",
					Click = delegate { UICopyMove(true); }
				};

			if (items.Rename == null)
				items.Rename = new SetItem()
				{
					Text = "&Rename property",
					Click = delegate { UIRename(); }
				};

			if (items.Create == null)
				items.Create = new SetItem()
				{
					Text = "&New property",
					Click = delegate { UICreate(); }
				};

			if (items.Delete == null)
				items.Delete = new SetItem()
				{
					Text = "&Delete property(s)",
					Click = delegate { UIDelete(false); }
				};

			base.HelpMenuInitItems(items, e);
		}

		/// <summary>
		/// Collects names of files.
		/// </summary>
		static IList<string> CollectNames(IList<FarFile> files)
		{
			var r = new List<string>();
			r.Capacity = files.Count;
			foreach (FarFile f in files)
				r.Add(f.Name);
			return r;
		}

		/// <summary>
		/// Collects original names (selected if any or the current).
		/// </summary>
		IList<string> CollectSelectedNames()
		{
			var r = new List<string>();
			foreach (FarFile f in SelectedFiles)
				r.Add(f.Name);
			return r;
		}

	}
}
