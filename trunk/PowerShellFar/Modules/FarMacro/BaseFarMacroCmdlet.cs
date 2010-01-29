/*
PowerShellFar plugin for Far Manager
Copyright (c) 2006 Roman Kuzmin
*/

using System.Management.Automation;
using FarNet;

namespace FarMacro
{
	public class BaseFarMacroCmdlet : BaseCmdlet
	{
		/// <summary>
		/// See <see cref="Macro.Area"/>
		/// </summary>
		[Parameter(Position = 0, Mandatory = true, HelpMessage = "See Macro.Area", ParameterSetName = "Properties")]
		public MacroArea Area { get; set; }

		/// <summary>
		/// See <see cref="Macro.Name"/>
		/// </summary>
		[Parameter(Position = 1, Mandatory = true, HelpMessage = "See Macro.Name")]
		public string Name { get; set; }

		/// <summary>
		/// See <see cref="Macro.Sequence"/>
		/// </summary>
		[Parameter(Position = 2, HelpMessage = "See Macro.Sequence")]
		public string Sequence { get; set; }

		/// <summary>
		/// See <see cref="Macro.Description"/>
		/// </summary>
		[Parameter(Position = 3, HelpMessage = "See Macro.Description")]
		public string Description { get; set; }

		/// <summary>
		/// See <see cref="Macro.CommandLine"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.CommandLine")]
		public string CommandLine { get; set; }

		/// <summary>
		/// See <see cref="Macro.SelectedText"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.SelectedText")]
		public string SelectedText { get; set; }

		/// <summary>
		/// See <see cref="Macro.EnableOutput"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.EnableOutput")]
		public SwitchParameter EnableOutput { get; set; }

		/// <summary>
		/// See <see cref="Macro.DisablePlugins"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.DisablePlugins")]
		public SwitchParameter DisablePlugins { get; set; }

		/// <summary>
		/// See <see cref="Macro.RunAfterFarStart"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.RunAfterFarStart")]
		public SwitchParameter RunAfterFarStart { get; set; }

		/// <summary>
		/// See <see cref="Macro.SelectedItems"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.SelectedItems")]
		public string SelectedItems { get; set; }

		/// <summary>
		/// See <see cref="Macro.PanelIsPlugin"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.PanelIsPlugin")]
		public string PanelIsPlugin { get; set; }

		/// <summary>
		/// See <see cref="Macro.ItemIsDirectory"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.ItemIsDirectory")]
		public string ItemIsDirectory { get; set; }

		/// <summary>
		/// See <see cref="Macro.SelectedItems2"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.SelectedItems2")]
		public string SelectedItems2 { get; set; }

		/// <summary>
		/// See <see cref="Macro.PanelIsPlugin2"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.PanelIsPlugin2")]
		public string PanelIsPlugin2 { get; set; }

		/// <summary>
		/// See <see cref="Macro.ItemIsDirectory2"/>
		/// </summary>
		[Parameter(HelpMessage = "See Macro.ItemIsDirectory2")]
		public string ItemIsDirectory2 { get; set; }

		///
		protected Macro CreateMacro()
		{
			Macro macro = new Macro();

			macro.Area = Area;
			macro.Name = Name;
			macro.Sequence = Sequence;
			macro.Description = Description;
			macro.EnableOutput = EnableOutput;
			macro.DisablePlugins = DisablePlugins;
			macro.RunAfterFarStart = RunAfterFarStart;
			macro.CommandLine = CommandLine;
			macro.SelectedText = SelectedText;
			macro.SelectedItems = SelectedItems;
			macro.PanelIsPlugin = PanelIsPlugin;
			macro.ItemIsDirectory = ItemIsDirectory;
			macro.SelectedItems2 = SelectedItems2;
			macro.PanelIsPlugin2 = PanelIsPlugin2;
			macro.ItemIsDirectory2 = ItemIsDirectory2;

			return macro;
		}
	}
}