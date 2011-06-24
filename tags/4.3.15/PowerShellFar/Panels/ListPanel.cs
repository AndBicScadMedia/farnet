/*
PowerShellFar module for Far Manager
Copyright (c) 2006 Roman Kuzmin
*/

using System;
using System.Collections;
using System.Collections.Generic;
using System.Management.Automation;
using FarNet;

namespace PowerShellFar
{
	/// <summary>
	/// Base panel for property list, member list and etc.
	/// </summary>
	public abstract class ListPanel : AnyPanel
	{
		static string _lastCurrentName;

		internal ListPanel()
		{
			Panel.PostName(_lastCurrentName);
			Panel.Info.UseFilter = true;

			// for hidden members
			Panel.Info.UseAttributeHighlighting = true;
			Panel.Info.UseHighlighting = false;

			// 090411 Use custom Descriptions mode
			PanelModeInfo mode = new PanelModeInfo();
			mode.Columns = new FarColumn[]
			{
				new SetColumn() { Kind = "N", Name = "Name" },
				new SetColumn() { Kind = "Z", Name = "Value" }
			};
			Panel.Info.SetMode(PanelViewMode.AlternativeFull, mode);
		}

		/// <summary>
		/// The target object.
		/// </summary>
		internal abstract PSObject Target { get; }

		/// <summary>
		/// Puts a value into the command line or opens a lookup panel or member panel.
		/// </summary>
		public override void OpenFile(FarFile file)
		{
			PSPropertyInfo pi = file.Data as PSPropertyInfo;

			// e.g. visible mode: sender is MemberDefinition
			if (pi == null)
				return;

			// lookup opener?
			if (_LookupOpeners != null)
			{
				EventHandler<FileEventArgs> handler;
				if (_LookupOpeners.TryGetValue(file.Name, out handler))
				{
					handler(this, new FileEventArgs(file));
					return;
				}
			}

			// case: can show value in the command line
			string s = Converter.InfoToLine(pi);
			if (s != null)
			{
				// set command line
				ILine cl = Far.Net.CommandLine;
				cl.Text = "=" + s;
				cl.SelectText(1, s.Length + 1);
				return;
			}

			// case: enumerable
			IEnumerable ie = Cast<IEnumerable>.From(pi.Value);
			if (ie != null)
			{
				ObjectPanel op = new ObjectPanel();
				op.AddObjects(ie);
				op.ShowAsChild(this);
				return;
			}

			// open members
			OpenFileMembers(file);
		}

		internal override MemberPanel OpenFileMembers(FarFile file)
		{
			PSPropertyInfo pi = file.Data as PSPropertyInfo;
			if (pi == null)
				return null;
			if (pi.Value == null)
				return null;
			MemberPanel r = new MemberPanel(pi.Value);
			r.ShowAsChild(this);
			return r;
		}

		/// <summary>
		/// Sets new value.
		/// </summary>
		/// <param name="info">Property info.</param>
		/// <param name="value">New value.</param>
		internal abstract void SetUserValue(PSPropertyInfo info, string value);

		/// <summary>
		/// Calls base or assigns a value to the current property.
		/// </summary>
		internal override bool UICommand(string code)
		{
			// base
			code = code.TrimStart();
			if (!code.StartsWith("=", StringComparison.Ordinal))
				return base.UICommand(code);

			// skip empty
			FarFile f = Panel.CurrentFile;
			if (f == null)
				return true;
			PSPropertyInfo pi = f.Data as PSPropertyInfo;
			if (pi == null)
				return true;

			try
			{
				SetUserValue(pi, code.Substring(1));
				UpdateRedraw(true);
			}
			catch (RuntimeException ex)
			{
				A.Msg(ex.Message);
			}

			return true;
		}

		// Must be called last
		internal override bool CanClose()
		{
			if (Child != null)
				return true;

			FarFile f = Panel.CurrentFile;
			if (f == null)
				_lastCurrentName = null;
			else
				_lastCurrentName = f.Name;

			return true;
		}

		internal override void ShowHelp()
		{
			Far.Net.ShowHelp(A.Psf.AppHome, "ListPanel", HelpOptions.Path);
		}

		/// <summary>
		/// It "deletes" property values = assigns null values.
		/// </summary>
		internal override void DeleteFiles(IList<FarFile> files, bool shift)
		{
			foreach (FarFile file in files)
			{
				PSPropertyInfo pi = file.Data as PSPropertyInfo;
				if (pi == null)
					continue;
				try
				{
					SetUserValue(pi, null);
					UpdateRedraw(true);
				}
				catch (RuntimeException ex)
				{
					A.Msg(ex.Message);
				}
			}
		}

		internal override void UIApply()
		{
			A.InvokePipelineForEach(new PSObject[] { Target });
		}

		internal override void HelpMenuInitItems(HelpMenuItems items, PanelMenuEventArgs e)
		{
			if (items.ApplyCommand == null)
			{
				items.ApplyCommand = new SetItem()
				{
					Text = Res.UIApply,
					Click = delegate { UIApply(); }
				};
			}

			base.HelpMenuInitItems(items, e);
		}

	}
}