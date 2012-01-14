
/*
PowerShellFar module for Far Manager
Copyright (c) 2006-2012 Roman Kuzmin
*/

using System;
using FarNet;
using FarNet.Forms;

namespace PowerShellFar.UI
{
	class InputDialog
	{
		public IDialog UIDialog { get; private set; }
		public IText[] UIPrompt { get; private set; }
		public IEdit UIEdit { get; private set; }

		public InputDialog(string caption, string history, params string[] prompt)
		{
			int w = Far.Net.UI.WindowSize.X - 7;
			int h = 5 + prompt.Length;

			UIDialog = Far.Net.CreateDialog(-1, -1, w, h);
			UIDialog.AddBox(3, 1, w - 4, h - 2, caption);
			UIPrompt = new IText[prompt.Length];
			for (int i = 0; i < prompt.Length; ++i)
				UIPrompt[i] = UIDialog.AddText(5, -1, w - 6, prompt[i]);
			UIEdit = UIDialog.AddEdit(5, -1, w - 6, string.Empty);

			// history
			UIEdit.History = history;

			// hotkeys
			UIEdit.KeyPressed += delegate(object sender, KeyPressedEventArgs e)
			{
				switch (e.Key.VirtualKeyCode)
				{
					case KeyCode.Tab:
						// [Tab]
						e.Ignore = true;
						EditorKit.ExpandCode(UIEdit.Line);
						return;
					case KeyCode.F1:
						if (e.Key.IsShift())
						{
							// [ShiftF1]
							e.Ignore = true;
							Help.ShowHelpForContext();
							return;
						}
						break;
				}
			};
		}
	}
}
