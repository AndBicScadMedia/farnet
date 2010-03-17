﻿/*
FarNet plugin for Far Manager
Copyright (c) 2005 FarNet Team
*/

using System.Globalization;
using System.IO;
using System.Text;

namespace FarNet.Works
{
	public sealed class EditorTextWriter : TextWriter
	{
		readonly IEditor _Editor;

		public EditorTextWriter(IEditor editor)
			: base(CultureInfo.InvariantCulture)
		{
			_Editor = editor;
			NewLine = "\r";
		}

		public override void Write(char value)
		{
			_Editor.InsertChar(value);
		}

		public override void Write(string value)
		{
			_Editor.Insert(value);
		}

		public override Encoding Encoding
		{
			get { return Encoding.Unicode; }
		}
	}
}