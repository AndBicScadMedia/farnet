
/*
PowerShellFar module for Far Manager
Copyright (c) 2006 Roman Kuzmin
*/

/*
_090929_061740 Far 2.0.1145 does not sync the current directory with the panel path.
	//! Empty provider path: open item panel, do 'cd \\DEV-RKUZ9400-SWK'
	//! PS shows an error and the location is set to invalid 'Microsoft.PowerShell.Core\FileSystem::'
	PathInfo pi = A.Psf.Engine.SessionState.Path.CurrentLocation;
	if (pi.ProviderPath.Length > 0 && !pi.ProviderPath.StartsWith(@"\\", StringComparison.Ordinal))
		Environment.CurrentDirectory = pi.ProviderPath;
*/

using System;
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
	/// Panel exploring PowerShell provider items.
	/// </summary>
	public sealed class ItemPanel : FormatPanel
	{
		PowerPath _location_;

		/// <summary>
		/// Creates a panel for provider items at the current location.
		/// </summary>
		public ItemPanel()
			: this(null)
		{ }

		/// <summary>
		/// Creates a panel for provider items at the given location.
		/// </summary>
		/// <param name="path">Path to start at.</param>
		public ItemPanel(string path)
		{
			// location and position
			if (string.IsNullOrEmpty(path))
			{
				// current location, post the current name
				FarFile f = Far.Net.Panel.CurrentFile;
				if (f != null)
					PostName(f.Name);
			}
			else
			{
				// resolve a share for a computer
				if (path.StartsWith("\\\\", StringComparison.Ordinal))
				{
					string computer = path.Substring(2);
					while (computer.EndsWith("\\", StringComparison.Ordinal))
						computer = computer.Substring(0, computer.Length - 1);
					if (computer.IndexOf('\\') < 0)
					{
						string share = SelectShare(computer);
						if (share == null)
							path = ".";
						else
							path = "\\\\" + computer + "\\" + share;
					}
				}

				// set location
				A.SetLocation(path);
			}
			SetLocation(new PowerPath(A.Psf.Engine.SessionState.Path.CurrentLocation), true);
		}

		/// <summary>
		/// It is fired when items are changed.
		/// </summary>
		public event EventHandler ItemsChanged;

		/// <summary>
		/// Fixed drive name.
		/// </summary>
		/// <remarks>
		/// When a drive is fixed the panel is used for some custom operations on this drive items.
		/// </remarks>
		public string Drive
		{
			get { return _Drive; }
			set
			{
				if (value == null)
					throw new ArgumentNullException("value");

				_Drive = value;
			}
		}
		string _Drive = string.Empty;

		// Current location
		internal PowerPath Location
		{
			get { return _location_; }
		}

		internal override void DeleteFiles2(IList<FarFile> files, bool shift)
		{
			// go
			A.SetLocation(Location.Path);

			// command; confirmation, recurse
			Command c = new Command("Remove-Item");
			FarConfirmations confirm = Far.Net.Confirmations;
			if ((confirm & FarConfirmations.Delete) != 0 && (confirm & FarConfirmations.DeleteNotEmptyFolders) != 0)
			{
				c.Parameters.Add(Prm.Confirm);
			}
			else if ((confirm & FarConfirmations.Delete) != 0)
			{
				c.Parameters.Add(Prm.Confirm);
				c.Parameters.Add(Prm.Recurse);
			}
			else if ((confirm & FarConfirmations.DeleteNotEmptyFolders) == 0)
			{
				c.Parameters.Add(Prm.Recurse);
			}

			// other params
			c.Parameters.Add(Prm.Force);
			c.Parameters.Add(Prm.ErrorAction, ActionPreference.Continue);

			// go
			try
			{
				using (PowerShell p = A.Psf.CreatePipeline())
				{
					p.Commands.AddCommand(c);
					p.Invoke(SelectedItems);

					if (A.ShowError(p))
						Update(true);
					else
						Update(false);

					Redraw();

					ItemPanel pp2 = AnotherPanel as ItemPanel;
					if (pp2 != null)
						pp2.UpdateRedraw(true);
				}
			}
			finally
			{
				// fire
				if (ItemsChanged != null)
					ItemsChanged(this, null);
			}
		}

		///
		internal override void BuildFiles(Collection<PSObject> values)
		{
			if (!My.ProviderInfoEx.IsNavigation(Location.Provider))
			{
				base.BuildFiles(values);
				return;
			}

			var files = new List<FarFile>(values.Count);
			Files = files;

			if (Location.Provider.ImplementingType == typeof(FileSystemProvider))
			{
				foreach (PSObject value in values)
					files.Add(new SystemMapFile(value, Map));
			}
			else
			{
				foreach (PSObject value in values)
					files.Add(new ItemMapFile(value, Map));
			}
		}

		internal override object GetData()
		{
			// get child items for the panel location
			return A.GetChildItems(Location.Path);
		}

		//! This is normally called by Far and then Far calls OnUpdateFiles().
		//! If you call this then you call UpdateRedraw(false, 0, 0) yourself.
		internal override void OnSetDirectory(SetDirectoryEventArgs e)
		{
			string newLocation = e.Name;
			if (newLocation == "..")
			{
				/*
				We might use 'cd ..' but we have to be sure that the current location is in sync
				with the panel path. It looks more reliable to build the parent path from the panel
				path. This way is good for some not standard cases. This way is bad if current
				location is problematic:
				??
				cd HKCU:\Software\Far2\PluginHotkeys
				cd Plugins/Brackets/Brackets.dll
				[..] --> error; it is rather PS issue though...
				*/
				newLocation = Location.Path;
				// 090814 use ":\\" instead of "\\", [_090814_130836]
				if (newLocation.EndsWith(":\\", StringComparison.Ordinal))
				{
					newLocation = null;
				}
				else
				{
					// 090814 [_090814_130836] PS V2 may get paths with extra '\' in the end
					newLocation = newLocation.TrimEnd(new char[] { '\\' });

					// find name
					int iSlash = newLocation.LastIndexOf('\\');
					if (iSlash < 0)
					{
						// no slashes, split by ::
						int iProvider = newLocation.IndexOf("::", StringComparison.Ordinal);
						if (iProvider > 0)
						{
							// FarMacro
							PostName(newLocation.Substring(iProvider + 2));
							newLocation = newLocation.Substring(0, iProvider + 2);
						}
						else
						{
							newLocation = null;
						}
					}
					else
					{
						//! Issue with names z:|z, Microsoft.PowerShell.Core\Registry::HKEY_CURRENT_USER - Far doesn't set cursor there
						if (newLocation.Length > iSlash + 2)
							PostName(newLocation.Substring(iSlash + 1));

						newLocation = newLocation.Substring(0, iSlash);
						if (newLocation.StartsWith("\\\\", StringComparison.Ordinal)) //HACK network path
						{
							iSlash = newLocation.LastIndexOf('\\');
							if (iSlash <= 1)
							{
								// show computer shares menu
								string computer = newLocation.Substring(2);
								string share = SelectShare(computer);
								if (share == null)
									newLocation = null;
								else
									newLocation += "\\" + share;
							}
						}

						// add \, else we can't step to the root from level 1
						if (newLocation != null)
							newLocation += "\\";
					}
				}
			}
			else if (newLocation == "\\")
			{
				if (Location.Drive == null || string.IsNullOrEmpty(Location.Drive.Name))
					newLocation = null;
				else
					newLocation = Location.Drive.Name + ":\\";
			}
			else
			{
				//! Make full path, else we can't step into e.g. Registry key z:|z
				if (UserWants == UserAction.Enter)
				{
					FarFile file = CurrentFile;
					newLocation = My.PathEx.Combine(Location.Path, file.Name);
				}
			}

			// no location, show drive menu
			if (newLocation == null)
			{
				// fixed drive?
				if (Drive.Length > 0)
					return;

				// menu
				newLocation = SelectDrivePrompt(Location.Drive == null ? null : Location.Drive.Name);

				// case: cancel
				if (newLocation == null)
				{
					e.Ignore = true;
					return;
				}

				// case: new object panel
				if (newLocation == "&Any objects")
				{
					ObjectPanel op = new ObjectPanel();
					op.OpenInstead(this);
					return;
				}
			}

			try
			{
				// 090409 was: A.Psf.InvokeCode("Set-Location -LiteralPath $args[0]", newLocation);
				A.Psf.Engine.SessionState.Path.SetLocation(Kit.EscapeWildcard(newLocation));

				// now get the current location and process it
				SetLocation(new PowerPath(A.Psf.Engine.SessionState.Path.CurrentLocation), false);
			}
			catch (RuntimeException exception)
			{
				// tell
				e.Ignore = true;

				// show
				if ((e.Mode & (OperationModes.Find | OperationModes.Silent)) == 0)
					A.Msg(exception.Message);
			}
		}

		bool UIAttributesCan()
		{
			return Drive.Length == 0 && My.ProviderInfoEx.HasProperty(Location.Provider);
		}

		internal override void UIAttributes()
		{
			if (Drive.Length > 0)
				return;

			if (!My.ProviderInfoEx.HasProperty(Location.Provider))
			{
				A.Msg(Res.NotSupportedByProvider);
				return;
			}

			// go
			A.SetLocation(Location.Path);

			// new mode
			FarFile f = CurrentFile;
			(new PropertyPanel(f == null ? Location.Path : My.PathEx.Combine(Location.Path, f.Name))).OpenChild(this);
		}

		internal override void UICreate()
		{
			// go
			A.SetLocation(Location.Path);

			UI.NewValueDialog ui = new UI.NewValueDialog("New " + Location.Provider.Name + " item");
			while (ui.Dialog.Show())
			{
				try
				{
					using (PowerShell p = A.Psf.CreatePipeline())
					{
						//! Don't use Value if it is empty (e.g. to avoid (default) property at new key in Registry).
						//! Don't use -Force or you silently kill existing item\property (with all children, properties, etc.)
						Command c = new Command("New-Item");
						c.Parameters.Add("Path", My.PathEx.Combine(Location.Path, ui.Name.Text));
						if (ui.Type.Text.Length > 0)
							c.Parameters.Add("ItemType", ui.Type.Text);

						if (ui.Value.Text.Length > 0)
							c.Parameters.Add("Value", ui.Value.Text);
						c.Parameters.Add(Prm.ErrorAction, ActionPreference.Continue);
						p.Commands.AddCommand(c);
						p.Invoke();
						if (A.ShowError(p))
							continue;
					}

					// fire
					if (ItemsChanged != null)
						ItemsChanged(this, null);

					// update this panel with name
					UpdateRedraw(false, ui.Name.Text);

					// update that panel if the path is the same
					ItemPanel pp2 = AnotherPanel as ItemPanel;
					if (pp2 != null && pp2.Location.Path == Location.Path)
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

		internal override void UICopyHere()
		{
			FarFile file = CurrentFile;
			if (file == null)
				return;
			string name = file.Name;

			IInputBox ib = Far.Net.CreateInputBox();
			ib.Title = "Copy";
			ib.Prompt = "New name";
			ib.History = "Copy";
			ib.Text = name;
			if (!ib.Show() || ib.Text == name)
				return;

			// go
			A.SetLocation(Location.Path);

			//! no idea what CopyContainers.CopyTargetContainer is
			string src = Kit.EscapeWildcard(My.PathEx.Combine(Location.Path, name));
			A.Psf.Engine.InvokeProvider.Item.Copy(src, ib.Text, false, CopyContainers.CopyTargetContainer);

			// fire
			if (ItemsChanged != null)
				ItemsChanged(this, null);

			UpdateRedraw(false, ib.Text);
		}

		internal override bool UICopyMoveCan(bool move)
		{
			if (base.UICopyMoveCan(move))
				return true;

			//! Actually e.g. functions can be copied, see UICopyHere
			return My.ProviderInfoEx.IsNavigation(Location.Provider);
		}

		internal override bool UICopyMove(bool move)
		{
			if (base.UICopyMove(move))
				return true;

			// check provider
			if (!My.ProviderInfoEx.IsNavigation(Location.Provider))
			{
				//! Actually e.g. functions can be copied, see UICopyHere
				A.Msg(Res.NotSupportedByProvider);
				return true;
			}

			// destination path
			ItemPanel ip2 = AnotherPanel as ItemPanel;
			string destination;
			if (ip2 == null)
			{
				// ignore plugin panel
				IPanel panel = Far.Net.Panel2;
				if (panel.IsPlugin)
					return true;

				// standard panel path
				destination = panel.CurrentDirectory;
			}
			else
			{
				// item panel path
				destination = ip2.CurrentDirectory;
			}

			// go
			A.SetLocation(Location.Path);

			Command c;
			if (move)
			{
				c = new Command("Move-Item");
				c.Parameters.Add(Prm.Force);
			}
			else
			{
				c = new Command("Copy-Item");
				c.Parameters.Add(Prm.Recurse);
			}
			c.Parameters.Add("Destination", destination);
			c.Parameters.Add(Prm.Confirm);
			c.Parameters.Add(Prm.ErrorAction, ActionPreference.Continue);

			using (PowerShell p = A.Psf.CreatePipeline())
			{
				p.Commands.AddCommand(c);
				p.Invoke(SelectedItems);
				A.ShowError(p);
			}

			// fire
			if (ItemsChanged != null)
				ItemsChanged(this, null);

			// update this
			UpdateRedraw(move);

			// update that
			if (ip2 != null)
				ip2.UpdateRedraw(true);

			return true;
		}

		internal override void UIRename()
		{
			FarFile f = CurrentFile;
			if (f == null)
				return;
			string name = f.Name;

			IInputBox ib = Far.Net.CreateInputBox();
			ib.Title = "Rename";
			ib.Prompt = "New name";
			ib.History = "Copy";
			ib.Text = name;
			if (!ib.Show() || ib.Text == name)
				return;

			// go
			A.SetLocation(Location.Path);

			// workaround; Rename-Item has no -LiteralPath; e.g. z`z[z.txt is a big problem
			string src = Kit.EscapeWildcard(My.PathEx.Combine(Location.Path, name));
			A.Psf.Engine.InvokeProvider.Item.Rename(src, ib.Text);

			// fire
			if (ItemsChanged != null)
				ItemsChanged(this, null);

			UpdateRedraw(false, ib.Text);
		}

		/// <summary>
		/// Sets new location
		/// </summary>
		internal void SetLocation(PowerPath location, bool update)
		{
			// fixed drive?
			if (Drive.Length > 0 && !Kit.Equals(Drive, location.Drive.Name))
				return;

			// customise if not yet
			if (Drive.Length == 0 && (Location == null || Location.Provider.ImplementingType != location.Provider.ImplementingType))
			{
				if (string.IsNullOrEmpty(Drive))
				{
					Files.Clear();
					Columns = null;
					ExcludeMemberPattern = null;

					System.Collections.IDictionary options = A.Psf.Providers[location.Provider.Name] as System.Collections.IDictionary;
					if (options != null)
					{
						try
						{
							Converter.SetProperties(this, options, true);
						}
						catch (ArgumentException ex)
						{
							throw new InvalidDataException("Invalid settings for '" + location.Provider.Name + "' provider: " + ex.Message);
						}
					}
				}
			}

			_location_ = location;

			//! path is used for Set-Location on Invoking()
			Title = "Items: " + Location.Path;
			PanelDirectory = Location.Path;

			if (Location.Provider.ImplementingType == typeof(FileSystemProvider))
			{
				UseFilter = true;
				UseSortGroups = true;
				Highlighting = PanelHighlighting.Full;

				// _090929_061740 Before Far 2.0.1145 we used to sync the current directory to
				// the PS location. Now it is not needed because Far does not do that any more.
			}
			else
			{
				UseFilter = true;
				UseSortGroups = false;
				Highlighting = PanelHighlighting.Default;
			}

			if (update && IsOpened)
				UpdateRedraw(false);
		}

		internal override void WriteFile(FarFile file, string path)
		{
			// *) V2 CTP3: this fails: Get-Item alias:\ls | Select-Object *
			// *) _091019_081503, Connect 498479:
			// -- V2 RC: -Property * may cause errors 'The URL cannot be empty.*'
			// -- V2 RC: -Property * may consume too much memory forever
			A.Psf.InvokeCode(@"
Format-List -InputObject $args[0] -Property * -Force -ErrorAction 0 |
Out-File -FilePath $args[1] -Width ([int]::MaxValue)
", file.Data, path);
		}

		internal override void EditFile(FarFile file, bool alternative)
		{
			if (!My.ProviderInfoEx.HasContent(Location.Provider))
			{
				A.Msg(Res.NotSupportedByProvider);
				return;
			}

			if (file.IsDirectory)
				return;

			FileInfo fi = Cast<FileInfo>.From(file.Data);
			if (fi != null)
			{
				base.EditFile(file, alternative);
				return;
			}

			try
			{
				string tmp = Far.Net.TempName();
				try
				{
					// item path
					string itemPath = My.PathEx.Combine(Location.Path, file.Name);

					// get content
					const string code = "Get-Content -LiteralPath $args[0] -ReadCount 0";
					Collection<PSObject> content = A.Psf.InvokeCode(code, itemPath);

					// write content
					using (StreamWriter sw = new StreamWriter(tmp, false, Encoding.Unicode))
					{
						foreach (PSObject p in content)
							sw.WriteLine(p.ToString());
					}

					if (!alternative)
					{
						ItemEditor edit = new ItemEditor();
						edit.Open(tmp, true, itemPath, this);
						tmp = null;
						return;
					}

					// notepad:
					for (; ; )
					{
						DateTime stamp1 = File.GetLastWriteTime(tmp);
						My.ProcessEx.StartNotepad(tmp).WaitForExit();

						// exit if it is not modified
						DateTime stamp2 = File.GetLastWriteTime(tmp);
						if (stamp2 <= stamp1)
							return;

						try
						{
							// read
							string text = File.ReadAllText(tmp, Encoding.Unicode).TrimEnd();

							// set
							if (!A.SetContentUI(itemPath, text))
								continue;

							// done
							UpdateRedraw(false);
							break;
						}
						catch (RuntimeException ex)
						{
							A.Msg(ex.Message);
						}
					}
				}
				finally
				{
					if (tmp != null)
						File.Delete(tmp);
				}
			}
			catch (RuntimeException ex)
			{
				Far.Net.ShowError("Edit", ex);
			}
		}

		/// <summary>
		/// Steps into a container or opens a leaf.
		/// </summary>
		public override void OpenFile(FarFile file)
		{
			if (file == null) { throw new ArgumentNullException("file"); }

			if (file.Data == null)
				return;

			if (file.IsDirectory)
			{
				OnSetDirectory(new SetDirectoryEventArgs() { Name = file.Name });
				UpdateRedraw(false, 0, 0);
				return;
			}

			base.OpenFile(file);
		}

		///
		internal override void HelpMenuInitItems(HelpMenuItems items, PanelMenuEventArgs e)
		{
			if (items.OpenFileAttributes == null && UIAttributesCan())
				items.OpenFileAttributes = new SetItem()
				{
					Text = "Item &properties",
					Click = delegate { UIAttributes(); }
				};

			if (items.OpenFile == null)
				items.OpenFile = new SetItem()
				{
					Text = "Child items",
					Click = delegate { UIOpenFile(CurrentFile); }
				};

			if (items.Copy == null && UICopyMoveCan(false))
				items.Copy = new SetItem()
				{
					Text = "&Copy item(s)",
					Click = delegate { UICopyMove(false); }
				};

			if (items.CopyHere == null && e.CurrentFile != null)
				items.CopyHere = new SetItem()
				{
					Text = "Copy &here",
					Click = delegate { UICopyHere(); }
				};

			if (items.Move == null && UICopyMoveCan(true))
				items.Move = new SetItem()
				{
					Text = "&Move item(s)",
					Click = delegate { UICopyMove(true); }
				};

			if (items.Rename == null)
				items.Rename = new SetItem()
				{
					Text = "&Rename item",
					Click = delegate { UIRename(); }
				};

			if (items.Create == null)
				items.Create = new SetItem()
				{
					Text = "&New item",
					Click = delegate { UICreate(); }
				};

			if (items.Delete == null)
				items.Delete = new SetItem()
				{
					Text = "&Delete item(s)",
					Click = delegate { UIDelete(false); }
				};

			base.HelpMenuInitItems(items, e);
		}

	}
}
