
/*
FarNet plugin for Far Manager
Copyright (c) 2005 FarNet Team
*/

#include "StdAfx.h"
#include "Far1.h"
#include "CommandLine.h"
#include "Dialog.h"
#include "Editor0.h"
#include "Far0.h"
#include "InputBox.h"
#include "ListMenu.h"
#include "Menu.h"
#include "Message.h"
#include "Panel0.h"
#include "Panel2.h"
#include "UI.h"
#include "Viewer0.h"
#include "Window.h"

namespace FarNet
{;
void Far1::Connect()
{
	// the instance
	Far::Net = %Far;

	// the data paths
	_LocalData = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData), "Far Manager");
	_RoamingData = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData), "Far Manager");
}

String^ Far1::CurrentDirectory::get()
{
	DWORD size = Info.FSF->GetCurrentDirectory(0, 0);
	CBox buf(size);
	Info.FSF->GetCurrentDirectory(size, buf);
	return gcnew String(buf);
}

IModuleCommand^ Far1::GetModuleCommand(Guid id)
{
	IModuleAction^ action;
	if (!Works::Host::Actions->TryGetValue(id, action))
		return nullptr;

	return (IModuleCommand^)action;
}

IModuleFiler^ Far1::GetModuleFiler(Guid id)
{
	IModuleAction^ action;
	if (!Works::Host::Actions->TryGetValue(id, action))
		return nullptr;

	return (IModuleFiler^)action;
}

IModuleTool^ Far1::GetModuleTool(Guid id)
{
	IModuleAction^ action;
	if (!Works::Host::Actions->TryGetValue(id, action))
		return nullptr;

	return (IModuleTool^)action;
}

int Far1::Message(String^ body, String^ header, MsgOptions options, array<String^>^ buttons, String^ helpTopic)
{
	return Message::Show(body, header, options, buttons, helpTopic);
}

System::Version^ Far1::FarVersion::get()
{
	DWORD vn;
	Info.AdvControl(Info.ModuleNumber, ACTL_GETFARVERSION, &vn);
	return gcnew System::Version((vn&0x0000ff00)>>8, vn&0x000000ff, (int)((long)vn&0xffff0000)>>16);
}

System::Version^ Far1::FarNetVersion::get()
{
	return Assembly::GetExecutingAssembly()->GetName()->Version;
}

IMenu^ Far1::CreateMenu()
{
	return gcnew Menu;
}

IListMenu^ Far1::CreateListMenu()
{
	return gcnew ListMenu;
}

FarConfirmations Far1::Confirmations::get()
{
	return (FarConfirmations)Info.AdvControl(Info.ModuleNumber, ACTL_GETCONFIRMATIONS, 0);
}

FarNet::MacroArea Far1::MacroArea::get()
{
	ActlKeyMacro command;
	command.Command = MCMD_GETAREA;
	return (FarNet::MacroArea)(1 + Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &command));
}

FarNet::MacroState Far1::MacroState::get()
{
	ActlKeyMacro command;
	command.Command = MCMD_GETSTATE;
	return (FarNet::MacroState)Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &command);
}

array<IEditor^>^ Far1::Editors()
{
	return Editor0::Editors();
}

array<IViewer^>^ Far1::Viewers()
{
	return Viewer0::Viewers();
}

IAnyEditor^ Far1::AnyEditor::get()
{
	return %Editor0::_anyEditor;
}

IAnyViewer^ Far1::AnyViewer::get()
{
	return %Viewer0::_anyViewer;
}

String^ Far1::PasteFromClipboard()
{
	wchar_t* buffer = Info.FSF->PasteFromClipboard();
	String^ r = gcnew String(buffer);
	Info.FSF->DeleteBuffer(buffer);
	return r;
}

void Far1::CopyToClipboard(String^ text)
{
	PIN_NE(pin, text);
	Info.FSF->CopyToClipboard(pin);
}

IEditor^ Far1::CreateEditor()
{
	return gcnew FarNet::Editor;
}

IViewer^ Far1::CreateViewer()
{
	return gcnew FarNet::Viewer;
}

array<int>^ Far1::CreateKeySequence(String^ keys)
{
	if (!keys)
		throw gcnew ArgumentNullException("keys");
	
	array<wchar_t>^ space = {' ', '\t', '\r', '\n'};
	array<String^>^ a = keys->Split(space, StringSplitOptions::RemoveEmptyEntries);
	array<int>^ r = gcnew array<int>(a->Length);

	for(int i = 0; i < a->Length; ++i)
	{
		int k = NameToKey(a[i]);
		if (k == -1)
			throw gcnew ArgumentException("Argument 'keys' contains invalid key: '" + a[i] + "'.");

		r[i] = k;
	}
	
	return r;
}

//! [_090328_170110] KSFLAGS_NOSENDKEYSTOPLUGINS is not set,
//! but Tab for TabExpansion is not working in .ps1 editor, why?
void Far1::PostKeySequence(array<int>^ sequence, bool enableOutput)
{
	if (sequence == nullptr) throw gcnew ArgumentNullException("sequence");
	if (sequence->Length == 0)
		return;

	// local buffer for a small sequence
	const int smallCount = 256;
	DWORD keys[smallCount];

	KeySequence keySequence;
	keySequence.Count = sequence->Length;
	keySequence.Flags = enableOutput ? 0 : KSFLAGS_DISABLEOUTPUT;

	DWORD* cur = keySequence.Count <= smallCount ? keys : new DWORD[keySequence.Count];
	keySequence.Sequence = cur;
	for each(int i in sequence)
	{
		*cur = i;
		++cur;
	}

	try
	{
		if (!Info.AdvControl(Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, &keySequence))
			throw gcnew InvalidOperationException;
	}
	finally
	{
		if (keySequence.Sequence != keys)
			delete keySequence.Sequence;
	}
}

// Don't throw on a wrong key, it is used for validation.
// See also:
// About AltXXXXX and etc.: http://forum.farmanager.com/viewtopic.php?f=8&t=5058
int Far1::NameToKey(String^ key)
{
	if (!key)
		throw gcnew ArgumentNullException("key");

	PIN_NE(pin, key);
	return Info.FSF->FarNameToKey(pin);
}

String^ Far1::KeyToName(int key)
{
	wchar_t name[33];
	if (!Info.FSF->FarKeyToName(key, name, countof(name) - 1))
		return nullptr;

	return gcnew String(name);
}

void Far1::PostKeys(String^ keys, bool enableOutput)
{
	if (keys == nullptr)
		throw gcnew ArgumentNullException("keys");

	keys = keys->Trim();
	PostKeySequence(CreateKeySequence(keys), enableOutput);
}

void Far1::PostText(String^ text, bool enableOutput)
{
	if (text == nullptr)
		throw gcnew ArgumentNullException("text");

	StringBuilder keys;
	text = text->Replace(CV::CRLF, CV::LF)->Replace('\r', '\n');
	for each(Char c in text)
	{
		switch(c)
		{
		case ' ':
			keys.Append("Space ");
			break;
		case '\n':
			keys.Append("Enter ");
			break;
		case '\t':
			keys.Append("Tab ");
			break;
		default:
			keys.Append(c);
			keys.Append(" ");
			break;
		}
	}
	PostKeys(keys.ToString(), enableOutput);
}

ILine^ Far1::Line::get()
{
	switch (Window->Kind)
	{
	case FarNet::WindowKind::Editor:
		{
			IEditor^ editor = Editor;
			return editor[-1];
		}
	case FarNet::WindowKind::Panels:
		{
			return CommandLine;
		}
	case FarNet::WindowKind::Dialog:
		{
			IDialog^ dialog = Dialog;
			if (dialog) //?? need?
			{
				IControl^ control = dialog->Focused;
				IEdit^ edit = dynamic_cast<IEdit^>(control);
				if (edit)
					return edit->Line;
				IComboBox^ combo = dynamic_cast<IComboBox^>(control);
				if (combo)
					return combo->Line;
			}
			break;
		}
	}
	return nullptr;
}

IEditor^ Far1::Editor::get()
{
	return Editor0::GetCurrentEditor();
}

IViewer^ Far1::Viewer::get()
{
	return Viewer0::GetCurrentViewer();
}

IPanel^ Far1::Panel::get()
{
	return Panel0::GetPanel(true);
}

IPanel^ Far1::Panel2::get()
{
	return Panel0::GetPanel(false);
}

IInputBox^ Far1::CreateInputBox()
{
	return gcnew InputBox;
}

ICollection<String^>^ Far1::GetDialogHistory(String^ name)
{
	return GetHistory("SavedDialogHistory\\" + name, nullptr);
}

//! Hack, not API.
// Avoid exceptions, return what we can get.
ICollection<String^>^ Far1::GetHistory(String^ name, String^ filter)
{
	List<String^>^ r = gcnew List<String^>;

	IRegistryKey^ key = nullptr;
	try
	{
		key = Far::Net->OpenRegistryKey(name, false);
		if (!key)
			return r;

		array<String^>^ lines = reinterpret_cast<array<String^>^>(key->GetValue(L"Lines", nullptr));
		if (lines && lines->Length)
		{
			// capacity
			r->Capacity = lines->Length;

			String^ types = nullptr;
			if (SS(filter))
			{
				Object^ o = key->GetValue(L"Types", nullptr);
				if (o)
					types = o->ToString();
			}

			for(int i = lines->Length; --i >= 0;)
			{
				// filter
				if (types && i < types->Length)
				{
					if (filter->IndexOf(types[i]) < 0)
						continue;
				}

				// add
				r->Add(lines[i]);
			}
		}
	}
	finally
	{
		delete key;
	}

	return r;
}

void Far1::ShowError(String^ title, Exception^ error)
{
	// 091028 Do not throw on null, just ignore.
	if (!error)
		return;

	// stop a running macro
	String^ msgMacro = nullptr;
	FarNet::MacroState macro = MacroState;
	if (macro == FarNet::MacroState::Executing || macro == FarNet::MacroState::ExecutingCommon)
	{
		// log
		msgMacro = "A macro has been stopped.";
		Log::Source->TraceEvent(TraceEventType::Warning, 0, msgMacro);

		// stop
		UI->Break();
	}

	// log
	String^ info = Log::TraceException(error);

	// case: not loaded
	//! non-stop loading
	//! no UI on unloading
	if (Works::Host::State != Works::HostState::Loaded)
	{
		//! trace the full string, so that a user can report this
		Log::TraceError(error->ToString());

		// info to show
		if (!info)
			info = Log::FormatException(error);

		// with title
		info += title + Environment::NewLine;

		if (Works::Host::State == Works::HostState::Loading)
			Far::Net->UI->Write(info, ConsoleColor::Red);
		else
			Far::Net->Message(info + Environment::NewLine + error->ToString(), title, (MsgOptions::Gui | MsgOptions::Warning));

		return;
	}

	// quiet: CtrlBreak in a dialog
	if (error->GetType()->FullName == "System.Management.Automation.PipelineStoppedException") //_110128_075844
		return;

	// ask
	int res = Message(
		error->Message,
		String::IsNullOrEmpty(title) ? error->GetType()->FullName : title,
		MsgOptions::LeftAligned | MsgOptions::Warning,
		gcnew array<String^>{"Ok", "More"});
	if (res < 1)
		return;

	// info to show
	if (!info)
		info = Log::FormatException(error);

	// add macro info
	if (msgMacro)
		info += Environment::NewLine + msgMacro + Environment::NewLine;

	// add verbose information
	info += Environment::NewLine + error->ToString();

	// locked editor
	Works::EditorTools::EditText(info, error->GetType()->FullName, true);
}

IDialog^ Far1::CreateDialog(int left, int top, int right, int bottom)
{
	return gcnew FarDialog(left, top, right, bottom);
}

Works::IPanelWorks^ Far1::WorksPanel(FarNet::Panel^ panel, Explorer^ explorer)
{
	return gcnew FarNet::Panel2(panel, explorer);
}

array<Panel^>^ Far1::Panels(Guid typeId)
{
	return Panel0::PanelsByGuid(typeId);
}

array<Panel^>^ Far1::Panels(Type^ type)
{
	return Panel0::PanelsByType(type);
}

String^ Far1::Input(String^ prompt, String^ history, String^ title, String^ text)
{
	InputBox ib;
	ib.Prompt = prompt;
	ib.Title = title;
	ib.Text = text;
	ib.History = history;
	ib.EmptyEnabled = true;
	return ib.Show() ? ib.Text : nullptr;
}

Char Far1::CodeToChar(int code)
{
	// get just the code
	code &= KeyMode::CodeMask;

	// not char
	if (code > 0xFFFF)
		return 0;

	// convert
	return Char(code);
}

void Far1::PostStep(Action^ handler)
{
	Far0::PostStep(handler);
}

void Far1::PostStepAfterKeys(String^ keys, Action^ handler)
{
	Far0::PostStepAfterKeys(keys, handler);
}

void Far1::PostStepAfterStep(Action^ handler1, Action^ handler2)
{
	Far0::PostStepAfterStep(handler1, handler2);
}

String^ Far1::TempName(String^ prefix)
{
	// reasonable buffer
	PIN_NE(pin, prefix);
	wchar_t buf[CBox::eBuf];
	int size = Info.FSF->MkTemp(buf, countof(buf), pin);
	if (size <= countof(buf))
		return gcnew String(buf);

	// larger buffer
	CBox box(size);
	Info.FSF->MkTemp(box, size, pin);
	return gcnew String(box);
}

String^ Far1::TempFolder(String^ prefix)
{
	String^ dir = TempName(prefix);
	if (!Directory::Exists(dir))
		Directory::CreateDirectory(dir);
	return dir;
}

IDialog^ Far1::Dialog::get()
{
	return FarDialog::GetDialog();
}

void Far1::PostJob(Action^ handler)
{
	Far0::PostJob(handler);
}

CultureInfo^ Far1::GetCurrentUICulture(bool update)
{
	return Far0::GetCurrentUICulture(update);
}

void Far1::PostMacro(String^ macro, bool enableOutput, bool disablePlugins)
{
	if (!macro)
		throw gcnew ArgumentNullException("macro");

	PIN_NE(pin, macro);
	ActlKeyMacro command;
	command.Command = MCMD_POSTMACROSTRING;
	command.Param.PlainText.SequenceText = (wchar_t*)pin;
	command.Param.PlainText.Flags = 0;
	if (!enableOutput)
		command.Param.PlainText.Flags |= KSFLAGS_DISABLEOUTPUT;
	if (disablePlugins)
		command.Param.PlainText.Flags |= KSFLAGS_NOSENDKEYSTOPLUGINS;
	if (!Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &command))
		throw gcnew InvalidOperationException(__FUNCTION__ " failed.");
}

void Far1::Quit()
{
	if (!Works::ModuleLoader::CanExit())
		return;
	
	Info.AdvControl(Info.ModuleNumber, ACTL_QUIT, 0);
}

ILine^ Far1::CommandLine::get()
{
	return gcnew FarNet::CommandLine;
}

IWindow^ Far1::Window::get()
{
	return %FarNet::Window::Instance;
}

IRegistryKey^ Far1::OpenRegistryKey(String^ name, bool writable)
{
	return Works::WinRegistry::OpenKey(name, writable);
}

// Implementation of Far methods.
ref class FarMacro : Works::FarMacro
{
public:
	virtual MacroParseError^ Check(String^ sequence, bool silent) override
	{
		PIN_ES(pin, sequence);

		ActlKeyMacro args;
		args.Command = MCMD_CHECKMACRO;
		args.Param.PlainText.SequenceText = pin;
		args.Param.PlainText.Flags = silent ? KSFLAGS_SILENTCHECK : 0;

		//! it always gets ErrCode
		Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &args);
		if (args.Param.MacroResult.ErrCode == MPEC_SUCCESS)
			return nullptr;

		MacroParseError^ r = gcnew MacroParseError;
		r->ErrorCode = (MacroParseStatus)args.Param.MacroResult.ErrCode;
		r->Token = gcnew String(args.Param.MacroResult.ErrSrc);
		r->Line = args.Param.MacroResult.ErrPos.Y;
		r->Pos = args.Param.MacroResult.ErrPos.X;
		return r;
	}
	virtual void Load() override
	{
		ActlKeyMacro args;
		args.Command = MCMD_LOADALL;
		if (!Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &args))
			throw gcnew InvalidOperationException(__FUNCTION__ " failed.");
	}
	virtual void Save() override
	{
		ActlKeyMacro args;
		args.Command = MCMD_SAVEALL;
		if (!Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &args))
			throw gcnew InvalidOperationException(__FUNCTION__ " failed.");
	}
};

IMacro^ Far1::Macro::get()
{
	return Works::FarMacro::Instance ? Works::FarMacro::Instance : gcnew FarMacro();
}

IUserInterface^ Far1::UI::get()
{
	return %FarUI::Instance;
}

bool Far1::MatchPattern(String^ input, String^ pattern)
{
	if (!input) throw gcnew ArgumentNullException("input");
	
	// empty
	if (ES(pattern))
		return true;

	// regex
	if (pattern->StartsWith("/") && pattern->EndsWith("/"))
		return Regex::IsMatch(input, pattern->Substring(1, pattern->Length - 2), RegexOptions::IgnoreCase);

	// wildcard
	PIN_NE(pin, input);
	return Far0::CompareNameExclude(pattern, pin, false);
}

String^ Far1::GetFolderPath(SpecialFolder folder)
{
	switch(folder)
	{
	case SpecialFolder::LocalData: return _LocalData;
	case SpecialFolder::RoamingData: return _RoamingData;
	default: throw gcnew ArgumentException("folder");
	}
}

IModuleManager^ Far1::GetModuleManager(Type^ type)
{
	return Works::ModuleLoader::GetModuleManager(type);
}

void Far1::ShowHelp(String^ path, String^ topic, HelpOptions options)
{
	PIN_NE(pinPath, path);
	PIN_NS(pinTopic, topic);

	Info.ShowHelp(pinPath, pinTopic, (int)options);
}

void Far1::ShowHelpTopic(String^ topic)
{
	String^ path = Path::GetDirectoryName(Assembly::GetCallingAssembly()->Location);

	PIN_NE(pinPath, path);
	PIN_NS(pinTopic, topic);

	Info.ShowHelp(pinPath, pinTopic, FHELP_CUSTOMPATH);
}

String^ Far1::GetHelpTopic(String^ topic)
{
	return "<" + Path::GetDirectoryName(Assembly::GetCallingAssembly()->Location) + "\\>" + topic;
}

}
