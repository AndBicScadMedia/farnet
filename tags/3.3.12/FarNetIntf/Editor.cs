/*
Far.NET plugin for Far Manager
Copyright (c) 2005-2007 Far.NET Team
*/

using FarManager.Forms;
using System.Collections.Generic;
using System;

namespace FarManager
{
	/// <summary>
	/// Interface of any editor. See <see cref="IFar.AnyEditor"/>.
	/// </summary>
	public interface IAnyEditor
	{
		/// <summary>
		/// Redraw event.
		/// </summary>
		event EventHandler<RedrawEventArgs> OnRedraw;
		/// <summary>
		/// Editor is opened event.
		/// </summary>
		event EventHandler AfterOpen;
		/// <summary>
		/// Event is fired before editor contents is saved.
		/// </summary>
		event EventHandler BeforeSave;
		/// <summary>
		/// Event is fired when the editor is closed.
		/// </summary>
		event EventHandler AfterClose;
		/// <summary>
		/// Key pressed in editor.
		/// </summary>
		event EventHandler<KeyEventArgs> OnKey;
		/// <summary>
		/// Mouse state is changed.
		/// </summary>
		event EventHandler<MouseEventArgs> OnMouse;
	}

	/// <summary>
	/// Far Editor interface. See <see cref="IFar.Editor"/>, <see cref="IFar.CreateEditor"/>.
	/// </summary>
	public interface IEditor : IAnyEditor
	{
		/// <summary>
		/// Internal editor identifier.
		/// </summary>
		int Id { get; }
		/// <summary>
		/// Size of tab symbol in spaces.
		/// Only for opened and current editor.
		/// </summary>
		int TabSize { get; set; }
		/// <summary>
		/// Expand tabs mode.
		/// Only for opened and current editor.
		/// </summary>
		ExpandTabsMode ExpandTabs { get; set; }
		/// <summary>
		/// Returns control to the calling function immediately after <see cref="Open"/>.
		/// If false continues when a user has closed the editor.
		/// It is read only when the editor is opened.
		/// </summary>
		bool Async { get; set; }
		/// <summary>
		/// Delete a directory with a file when it is closed and it is the only file there.
		/// It is read only when an editor is opened.
		/// </summary>
		/// <seealso cref="DeleteOnlyFileOnClose"/>
		bool DeleteOnClose { get; set; }
		/// <summary>
		/// Delete a file when it is closed.
		/// It is read only when an editor is opened.
		/// </summary>
		/// <seealso cref="DeleteOnClose"/>
		bool DeleteOnlyFileOnClose { get; set; }
		/// <summary>
		/// Enable switching to viewer.
		/// It is read only when an editor is opened.
		/// </summary>
		bool EnableSwitch { get; set; }
		/// <summary>
		/// Do not use editor history.
		/// It is read only when an editor is opened.
		/// </summary>
		bool DisableHistory { get; set; }
		/// <summary>
		/// Word delimiters specific to the current editor.
		/// Only for opened and current editor.
		/// </summary>
		/// <seealso cref="IFar.WordDiv"/>
		string WordDiv { get; set; }
		/// <summary>
		/// The current line. It is not a 'copy', when you move cursor to another line this object is changed respectively.
		/// Only for opened and current editor.
		/// </summary>
		ILine CurrentLine { get; }
		/// <summary>
		/// Editor lines.
		/// Only for opened and current editor.
		/// </summary>
		ILines Lines { get; }
		/// <summary>
		/// Name of a file being edited.
		/// It is read only when an editor is opened.
		/// </summary>
		string FileName { get; set; }
		/// <summary>
		/// Editor window position.
		/// </summary>
		Place Window { get; }
		/// <summary>
		/// Current selection. It is a collection <see cref="ILines"/> of selected line parts and a few extra members.
		/// Only for opened and current editor.
		/// </summary>
		ISelection Selection { get; }
		/// <summary>
		/// Disable switching to other windows.
		/// It is read only when an editor is opened.
		/// </summary>
		bool IsModal { get; set; }
		/// <summary>
		/// Open a new (non-existing) file in the editor, similar to pressing Shift-F4 in Far. 
		/// It is read only when an editor is opened.
		/// </summary>
		bool IsNew { get; set; }
		/// <summary>
		/// Insert text.
		/// The text is processed in the same way as it it had been entered from the keyboard.
		/// Only for opened and current editor.
		/// </summary>
		/// <param name="text">The text. Supported EOL: CR, LF, CR+LF.</param>
		void Insert(string text);
		/// <summary>
		/// Redraw editor window. Normally it should be called when changes are completed.
		/// Only for opened and current editor.
		/// </summary>
		void Redraw();
		/// <summary>
		/// Delete a character under <see cref="Cursor"/>.
		/// Only for opened and current editor.
		/// </summary>
		void DeleteChar();
		/// <summary>
		/// Delete a line under <see cref="Cursor"/>.
		/// Only for opened and current editor.
		/// </summary>
		void DeleteLine();
		/// <summary>
		/// Close the editor.
		/// Only for opened and current editor.
		/// </summary>
		void Close();
		/// <summary>
		/// Save the file being edited. Exception on failure.
		/// Only for opened and current editor.
		/// </summary>
		void Save();
		/// <summary>
		/// Save the file being edited to <paramref name="fileName"/>. Exception on failure.
		/// Only for opened and current editor.
		/// </summary>
		/// <param name="fileName">File name to save to.</param>
		void Save(string fileName);
		/// <summary>
		/// Inserts a new line at the current <see cref="Cursor"/> position
		/// and moves the cursor to the first position in the new line.
		/// Only for opened and current editor.
		/// </summary>
		void InsertLine();
		/// <summary>
		/// Inserts a new line at the current <see cref="Cursor"/> position
		/// and moves the cursor to the first position in the new line or to the indented position.
		/// The indent behaviour is the same as on pressing Enter in the editor.
		/// Only for opened and current editor.
		/// </summary>
		/// <param name="indent">Insert a line with indent.</param>
		void InsertLine(bool indent);
		/// <summary>
		/// Opens an editor using properties:
		/// <see cref="FileName"/>,
		/// <see cref="Title"/>,
		/// <see cref="Async"/>,
		/// <see cref="DeleteOnClose"/>,
		/// <see cref="DeleteOnlyFileOnClose"/>,
		/// <see cref="DisableHistory"/>,
		/// <see cref="EnableSwitch"/>,
		/// <see cref="IsModal"/>,
		/// <see cref="IsNew"/>.
		/// </summary>
		void Open();
		/// <summary>
		/// Is this editor opened?
		/// </summary>
		bool IsOpened { get; }
		/// <summary>
		/// Editor window title. Set it before opening (standard title) or after opening (temporary title).
		/// When an editor is opened the standard title will be automatically restored after the plugin has finished processing.
		/// </summary>
		string Title { get; set; }
		/// <summary>
		/// Overtype mode.
		/// Only for opened and current editor.
		/// </summary>
		bool Overtype { get; set; }
		/// <summary>
		/// Is the file modified?
		/// Only for opened and current editor.
		/// </summary>
		bool IsModified { get; }
		/// <summary>
		/// Is the file saved?
		/// Only for opened and current editor.
		/// </summary>
		bool IsSaved { get; }
		/// <summary>
		/// Is the file locked (Ctrl-L)?
		/// Only for opened and current editor.
		/// </summary>
		bool IsLocked { get; }
		/// <summary>
		/// Converts Char position to Tab position for a given line.
		/// </summary>
		/// <param name="line">Line index, -1 for current.</param>
		/// <param name="pos">Char posistion.</param>
		int ConvertPosToTab(int line, int pos);
		/// <summary>
		/// Converts Tab position to Char position for a given line.
		/// </summary>
		/// <param name="line">Line index, -1 for current.</param>
		/// <param name="tab">Tab posistion.</param>
		int ConvertTabToPos(int line, int tab);
		/// <summary>
		/// Converts a screen point to editor cursor point.
		/// </summary>
		Point ConvertScreenToCursor(Point screen);
		/// <summary>
		/// Current text frame.
		/// </summary>
		TextFrame Frame { get; set; }
		/// <summary>
		/// Call this method before processing large amount of lines, performance can be drastically improved.
		/// It is strongly recommended to call <see cref="End"/> after processing.
		/// Nested calls of <see cref="Begin"/> .. <see cref="End"/> are allowed.
		/// </summary>
		/// <remarks>
		/// Avoid using this method together with getting <see cref="Frame"/> or <see cref="Cursor"/>,
		/// their values can be invalid. But you can set them directly or using <see cref="GoTo"/> methods.
		/// </remarks>
		void Begin();
		/// <summary>
		/// If you have called <see cref="Begin"/> you have to call this method in the end of processing.
		/// </summary>
		void End();
		/// <summary>
		/// Current cursor position.
		/// Only for opened and current editor.
		/// </summary>
		Point Cursor { get; }
		/// <summary>
		/// Gets bookmarks in the current editor.
		/// Bookmarks are defined as <see cref="TextFrame"/>.
		/// Negative <c>Line</c> means undefined bookmark.
		/// To go to a bookmark set <see cref="Frame"/>.
		/// </summary>
		ICollection<TextFrame> Bookmarks();
		/// <summary>
		/// Go to a new cursor position.
		/// </summary>
		/// <param name="pos">Position.</param>
		/// <param name="line">Line.</param>
		/// <seealso cref="Frame"/>
		void GoTo(int pos, int line);
		/// <summary>
		/// Go to a line.
		/// </summary>
		/// <param name="line">Line.</param>
		/// <seealso cref="Frame"/>
		void GoToLine(int line);
		/// <summary>
		/// Go to a position in the current line.
		/// </summary>
		/// <param name="pos">Position.</param>
		/// <seealso cref="Frame"/>
		void GoToPos(int pos);
		/// <summary>
		/// Go to the end of text.
		/// </summary>
		/// <param name="addLine">Add an empty line if the last is not empty.</param>
		void GoEnd(bool addLine);
		/// <summary>
		/// User data.
		/// </summary>
		object Data { get; set; }
		/// <summary>
		/// Is the end line the current one?
		/// Only for opened and current editor.
		/// </summary>
		bool IsEnd { get; }
	}

	/// <summary>
	/// Arguments of <see cref="IAnyEditor.OnRedraw"/> event.
	/// </summary>
	public sealed class RedrawEventArgs : EventArgs
	{
		int _mode;
		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="mode">See <see cref="Mode"/>.</param>
		public RedrawEventArgs(int mode)
		{
			_mode = mode;
		}
		/// <summary>
		/// Parameter of Far EE_REDRAW event, see Far API, ProcessEditorEvent.
		/// </summary>
		public int Mode
		{
			get { return _mode; }
		}
	}

	/// <summary>
	/// List of strings.
	/// </summary>
	/// <remarks>
	/// Usually the list is internally connected to <see cref="ILines"/>,
	/// thus, strings in the list are dependent on this source; i.e.
	/// *) if it is <see cref="ILines.Strings"/> from <see cref="IEditor.Lines"/> then strings are lines of editor
	/// and standard list operations affect editor text;
	/// *) if it is <see cref="ILines.Strings"/> from <see cref="IEditor.Selection"/> then strings are selected line parts
	/// and list operations affect only selected text.
	/// </remarks>
	public interface IStrings : IList<string>
	{
		/// <summary>
		/// Strings concatenated with CRLF.
		/// </summary>
		string Text { get; set; }
	}

	/// <summary>
	/// Selection in <see cref="ILine"/>.
	/// </summary>
	public interface ILineSelection
	{
		/// <summary>
		/// Text of selection. If line doesn't contain selection it is null.
		/// </summary>
		string Text { get; set; }
		/// <summary>
		/// Start position of selection in the line. If line doesn't contain selection it is -1.
		/// </summary>
		int Start { get; }
		/// <summary>
		/// End position of selection in the line. If selection includes the EOL sequence this field has a value of -1.
		/// </summary>
		int End { get; }
		/// <summary>
		/// Selection length. If line doesn't contain selection it is -1.
		/// </summary>
		int Length { get; }
	}

	/// <summary>
	/// Interface of a line in various text and line editors:
	/// *) item of <see cref="IEditor.Lines"/> or <see cref="IEditor.Selection"/> in <see cref="IEditor"/>;
	/// *) command line <see cref="IFar.CommandLine"/>;
	/// *) edit box (<see cref="IEdit.Line"/>) and combo box (<see cref="IComboBox.Line"/>) in <see cref="IDialog"/>.
	/// </summary>
	public interface ILine
	{
		/// <summary>
		/// Line number in source <see cref="IEditor"/>.
		/// -1 for <see cref="IEditor.CurrentLine"/> and <see cref="IFar.CommandLine"/>.
		/// </summary>
		int No { get; }
		/// <summary>
		/// Line text (<see cref="IEditor.Lines"/>, <see cref="IEditor.CurrentLine"/>, <see cref="IFar.CommandLine"/>)
		/// or text of selected line part (<see cref="IEditor.Selection"/>).
		/// </summary>
		string Text { get; set; }
		/// <summary>
		/// End-of-line characters.
		/// </summary>
		string Eol { get; set; }
		/// <summary>
		/// Selected line parts.
		/// </summary>
		ILineSelection Selection { get; }
		/// <summary>
		/// Cursor position.
		/// Returns -1 if the line is not current.
		/// Setting of negative value moves cursor to the line end.
		/// </summary>
		int Pos { get; set; }
		/// <summary>
		/// Inserts text into the current or command line beginning from the current cursor position.
		/// Don't use if it is not the current line or the command line.
		/// </summary>
		/// <param name="text">String to insert to the line.</param>
		void Insert(string text);
		/// <summary>
		/// Selects a text fragment in the current or command line.
		/// </summary>
		/// <param name="start">Start position.</param>
		/// <param name="end">End position.</param>
		void Select(int start, int end);
		/// <summary>
		/// Clears selection in the current or command line.
		/// </summary>
		void Unselect();
		/// <summary>
		/// Gets an instance of a full line if this line represents only a part,
		/// (e.g. the line is from <see cref="IEditor.Selection"/>),
		/// or returns this instance itself.
		/// </summary>
		ILine FullLine { get; }
		/// <summary>
		/// The text length.
		/// </summary>
		int Length { get; }
	}

	/// <summary>
	/// List of lines. See <see cref="IEditor.Lines"/> (all editor lines), <see cref="IEditor.Selection"/> (editor selected lines\parts).
	/// </summary>
	public interface ILines : IList<ILine>
	{
		/// <summary>
		/// First line. Note that it always exists in editor.
		/// </summary>
		ILine First { get; }
		/// <summary>
		/// Last line. Note that it always exists in editor.
		/// </summary>
		ILine Last { get; }
		/// <summary>
		/// All text as string.
		/// </summary>
		string Text { get; set; }
		/// <summary>
		/// Text as string list, see <see cref="IStrings"/>.
		/// </summary>
		IStrings Strings { get; }
		/// <summary>
		/// Add a string to the end of the list.
		/// </summary>
		void Add(string item);
		/// <summary>
		/// Insert a string as a new line with specified line index.
		/// </summary>
		void Insert(int index, string item);
	}

	/// <summary>
	/// Text event arguments
	/// </summary>
	public class TextEventArgs : EventArgs
	{
		string _text;
		/// <summary>
		/// Text to be processed.
		/// </summary>
		public string Text
		{
			get { return _text; }
			set { _text = value; }
		}
	}

	/// <summary>
	/// Arguments of <see cref="IAnyEditor.OnKey"/> event.
	/// </summary>
	public sealed class KeyEventArgs : EventArgs
	{
		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="key">Key data.</param>
		public KeyEventArgs(KeyInfo key)
		{
			_key = key;
		}
		/// <summary>
		/// Key data.
		/// </summary>
		public KeyInfo Key
		{
			get { return _key; }
		}
		KeyInfo _key;
		/// <summary>
		/// Ignore event.
		/// </summary>
		public bool Ignore
		{
			get { return _ignore; }
			set { _ignore = value; }
		}
		bool _ignore;
	}

	/// <summary>
	/// Arguments of <see cref="IAnyEditor.OnMouse"/> event.
	/// </summary>
	public sealed class MouseEventArgs : EventArgs
	{
		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="mouse">Mouse data.</param>
		public MouseEventArgs(MouseInfo mouse)
		{
			_mouse = mouse;
		}
		/// <summary>
		/// Mouse data.
		/// </summary>
		public MouseInfo Mouse
		{
			get { return _mouse; }
		}
		MouseInfo _mouse;
		/// <summary>
		/// Ignore event.
		/// </summary>
		public bool Ignore
		{
			get { return _ignore; }
			set { _ignore = value; }
		}
		bool _ignore;
	}

	/// <summary>
	/// Editor expand tabs mode.
	/// </summary>
	public enum ExpandTabsMode
	{
		/// <summary>
		/// Tabs are not replaced with spaces.
		/// </summary>
		None,
		/// <summary>
		/// All tabs are replaced with spaces.
		/// </summary>
		All,
		/// <summary>
		/// Only new tabs are replaced with spaces.
		/// </summary>
		New
	}

	/// <summary>
	/// Complete information about text frame and cursor position in an editor.
	/// </summary>
	public struct TextFrame
	{
		/// <summary>
		/// Sets the same value x  for all members.
		/// </summary>
		/// <param name="x">Value.</param>
		public TextFrame(int x)
		{
			_line = _pos = _tabPos = _topLine = _leftPos = x;
		}
		/// <summary>
		/// Line index (starting with 0).
		/// </summary>
		public int Line { get { return _line; } set { _line = value; } }
		int _line;
		/// <summary>
		/// Position in the string (i.e. in chars).
		/// </summary>
		public int Pos { get { return _pos; } set { _pos = value; } }
		int _pos;
		/// <summary>
		/// Position on the screen (i.e. in columns).
		/// </summary>
		public int TabPos { get { return _tabPos; } set { _tabPos = value; } }
		int _tabPos;
		/// <summary>
		/// First visible line index.
		/// </summary>
		public int TopLine { get { return _topLine; } set { _topLine = value; } }
		int _topLine;
		/// <summary>
		/// Leftmost visible position of the text on the screen.
		/// </summary>
		public int LeftPos { get { return _leftPos; } set { _leftPos = value; } }
		int _leftPos;
		/// <summary>
		/// Compares two objects.
		/// </summary>
		public static bool operator ==(TextFrame left, TextFrame right)
		{
			return
				left._line == right._line &&
				left._pos == right._pos &&
				left._tabPos == right._tabPos &&
				left._topLine == right._topLine &&
				left._leftPos == right._leftPos;
		}
		/// <summary>
		/// Compares two objects.
		/// </summary>
		public static bool operator !=(TextFrame left, TextFrame right)
		{
			return !(left == right);
		}
		/// <summary>
		/// Equals()
		/// </summary>
		public override bool Equals(Object obj)
		{
			if (obj == null || GetType() != obj.GetType())
				return false;
			TextFrame that = (TextFrame)obj;
			return this == that;
		}
		/// <summary>
		/// ToString()
		/// </summary>
		public override string ToString()
		{
			return "((" + _pos + "/" + _tabPos + ", " + _line + ")(" + _leftPos + ", " + _topLine + "))";
		}
		/// <summary>
		/// GetHashCode()
		/// </summary>
		public override int GetHashCode()
		{
			return (_line << 16) ^ (_pos << 16) ^ _tabPos ^ _topLine ^ _leftPos;
		}
	}
}