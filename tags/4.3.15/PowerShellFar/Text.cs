/*
PowerShellFar module for Far Manager
Copyright (c) 2006 Roman Kuzmin
*/

namespace PowerShellFar
{
	/// <summary>
	/// Resource strings.
	/// </summary>
	static class Res
	{
		public const string
			InvokeSelectedCode = "Invoke selected code",
			BackgroundJobs = "Background jobs",
			// main menu
			MenuInvokeInputCode = "&1. Invoke input code... ", // use right margin spaces
			MenuInvokeSelectedCode = "&2. " + InvokeSelectedCode,
			MenuBackgroundJobs = "&3. " + BackgroundJobs + "...",
			MenuCommandHistory = "&4. Command history...",
			MenuEditorConsole = "&5. Editor console",
			MenuPowerPanel = "&6. Power panel...",
			MenuTabExpansion = "&7. TabExpansion",
			MenuSnapin = "&8. Modules+...",
			MenuDebugger = "&9. Debugger...",
			MenuError = "&0. Errors...",
			MenuHelp = "&-. Help",
			// errors
			AskSaveModified = "Would you like to save modified data?",
			EditorConsoleCannotComplete = "Editor console can't complete the command\nbecause its window is not current at this moment.",
			LogError = "Cannot write to the log; ensure the path is valid and the file is not busy.",
			PropertyIsNotSettable = "Note: this property is not settable, changes will be lost.",
			NeedsEditor = "Editor is not opened or its window is not current.",
			NotSupportedByProvider = "Operation is not supported by the provider.",
			NoUserMenu = "You did not define your user menu $Psf.UserMenu.\nPlease, see help and example script Profile-.ps1",
			PropertyIsNotSettableNow = "The property is not settable at this moment.",
			CanNotClose = "Cannot close the session at this time.",
			MaximumPanelColumnCount = "Valid maximum column count should be from 3 to 13.", // _100202_113617
			InvalidColumnKind = "Invalid column kind: ",
			// others
			Cancel = "Cancel",
			Delete = "Delete",
			PromptCode = "Enter PowerShell code",
			Remove = "Remove",
			CtrlC = "Cancel key is pressed.",
			UIApply = "Apply command",
			
			// History of main input code, general top level code
			History = "PowerShellFar",
			// History of "Apply command", pieces of code based on $_
			HistoryApply = "PowerShellFarApply",
			// History of the host prompt box, any strings
			HistoryPrompt = "PowerShellFarPrompt",
			// History of BP action
			HistoryAction = "PowerShellFarAction",
			// History of BP command, command names
			HistoryCommand = "PowerShellFarCommand",
			// History of BP script, any script paths
			HistoryScript = "PowerShellFarScript",
			// History of BP variable, variable names
			HistoryVariable = "PowerShellFarVariable",
			
			// Main name
			Me = "PowerShellFar";
	}

	/// <summary>
	/// Invariand words and strings.
	/// </summary>
	static class Word
	{
		public const string
			Alignment = "Alignment",
			ConsoleExtension = ".psfconsole",
			Definition = "Definition",
			Description = "Description",
			ExecutionContext = "ExecutionContext",
			Expression = "Expression",
			FormatString = "FormatString",
			Id = "Id",
			Key = "Key",
			Kind = "Kind",
			Name = "Name",
			Label = "Label",
			PSModulePath = "PSModulePath",
			Status = "Status",
			Value = "Value",
			Width = "Width";
	}
}