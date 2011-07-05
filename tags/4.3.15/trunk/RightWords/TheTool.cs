﻿
/*
FarNet module RightWords
Copyright (c) 2011 Roman Kuzmin
*/

namespace FarNet.RightWords
{
	[System.Runtime.InteropServices.Guid("ca7ecdc0-f446-4bff-a99d-06c90fe0a3a9")]
	[ModuleTool(Name = Settings.Name, Options = ModuleToolOptions.Dialog | ModuleToolOptions.Editor | ModuleToolOptions.Panels)]
	public class TheTool : ModuleTool
	{
		public override void Invoke(object sender, ModuleToolEventArgs e)
		{
			if (e == null) return;

			var menu = Far.Net.CreateMenu();
			menu.Title = Settings.Name;

			menu.Add(UI.DoCorrectWord).Click += delegate { Actor.CorrectWord(); };

			if (e.From == ModuleToolOptions.Editor)
			{
				var editor = Far.Net.Editor;

				menu.Add(UI.DoCorrectText).Click += delegate { Actor.CorrectText(); };

				var itemHighlighting = menu.Add(UI.DoHighlighting);
				itemHighlighting.Click += delegate { Actor.Highlight(editor); };
				if (editor.Data[Settings.EditorDataId] != null)
					itemHighlighting.Checked = true;
			}

			menu.Add(UI.DoThesaurus).Click += delegate { Actor.ShowThesaurus(); };

			menu.Show();
		}
	}
}
