
// Escape and unescape \ and " in editor selection with \
// (e.g. string values in .reg files)

using System;
using System.Text.RegularExpressions;
using FarNet;

public class Escape : ModuleTool
{
	public override string Name
	{
		get { return "Escape selected text"; }
	}

	public override ToolOptions Options
	{
		get { return ToolOptions.Editor; }
	}

	public override void Invoke(object sender, ToolEventArgs e)
	{
		IEditor editor = Far.Host.Editor;
		ISelection select = editor.Selection;
		if (select.Exists)
			select.SetText(Regex.Replace(select.GetText("\r"), @"([\\""])", @"\$1"));
	}
}

public class Unescape : ModuleTool
{
	public override string Name
	{
		get { return "Unescape selected text"; }
	}

	public override ToolOptions Options
	{
		get { return ToolOptions.Editor; }
	}

	public override void Invoke(object sender, ToolEventArgs e)
	{
		IEditor editor = Far.Host.Editor;
		ISelection select = editor.Selection;
		if (select.Exists)
			select.SetText(Regex.Replace(select.GetText("\r"), @"\\([\\""])", "$1"));
	}
}