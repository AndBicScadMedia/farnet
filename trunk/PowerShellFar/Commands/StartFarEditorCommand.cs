/*
PowerShellFar plugin for Far Manager
Copyright (C) 2006-2009 Roman Kuzmin
*/

using System.ComponentModel;
using System.Management.Automation;
using FarNet;

namespace PowerShellFar.Commands
{
	/// <summary>
	/// Start-FarEditor command.
	/// Creates and opens an editor.
	/// </summary>
	/// <seealso cref="NewFarEditorCommand"/>
	[Description("Creates and opens an editor.")]
	public sealed class StartFarEditorCommand : NewFarEditorCommand
	{
		///
		[Parameter(HelpMessage = _helpModal)]
		public SwitchParameter Modal
		{
			get;
			set;
		}

		///
		protected override void ProcessRecord()
		{
			if (Stop())
				return;
			IEditor editor = CreateEditor();
			if (Modal)
				editor.Open(OpenMode.Modal);
			else
				editor.Open();
		}
	}
}
