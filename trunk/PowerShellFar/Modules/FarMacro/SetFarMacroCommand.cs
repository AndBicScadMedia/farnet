/*
PowerShellFar plugin for Far Manager
Copyright (c) 2006 Roman Kuzmin
*/

using System.Collections.Generic;
using System.ComponentModel;
using System.Management.Automation;
using FarNet;

namespace FarMacro
{
	/// <summary>
	/// Set-FarMacro command.
	/// Installs one or more macros.
	/// </summary>
	[Cmdlet(VerbsCommon.Set, BaseCmdlet.Noun)]
	[Description("Installs one or more macros.")]
	public sealed class SetFarMacroCommand : BaseFarMacroCmdlet
	{
		[Parameter(Position = 0, Mandatory = true, ValueFromPipeline = true, HelpMessage = "Input macro instances.", ParameterSetName = "Macros")]
		public Macro InputObject { get; set; }

		///
		protected override void ProcessRecord()
		{
			Macro macro = InputObject == null ? CreateMacro() : InputObject;
			Far.Macro.Install(macro);
		}
	}
}