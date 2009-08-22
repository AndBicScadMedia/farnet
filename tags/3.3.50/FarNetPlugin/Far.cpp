/*
FAR.NET plugin for Far Manager
Copyright (c) 2005-2008 FAR.NET Team
*/

#include "StdAfx.h"
#include "Far.h"
#include "CommandLine.h"
#include "Dialog.h"
#include "Editor.h"
#include "EditorHost.h"
#include "InputBox.h"
#include "KeyMacroHost.h"
#include "ListMenu.h"
#include "Menu.h"
#include "Message.h"
#include "Panel.h"
#include "PluginInfo.h"
#include "PluginSet.h"
#include "Viewer.h"
#include "ViewerHost.h"

namespace FarNet
{;
Far::Far()
{}

void Far::StartFar()
{
	if (_instance) throw gcnew InvalidOperationException("Already started.");
	_instance = gcnew Far;
	_instance->Start();

	// versions
	DWORD vn; Info.AdvControl(Info.ModuleNumber, ACTL_GETFARVERSION, &vn);
	int v1 = (vn & 0x0000ff00)>>8, v2 = vn & 0x000000ff, v3 = (int)((long)vn&0xffff0000)>>16;
	if (v1 >= 1 && v2 >= 71 && v3 >= 2315)
		_version_1_71_2315 = true;
}

void Far::Start()
{
	_hotkey = GetFarValue("PluginHotkeys\\Plugins/FAR.NET/FarNetPlugin.dll", "Hotkey", String::Empty)->ToString();
	RegisterTool(nullptr, String::Empty, gcnew EventHandler<ToolEventArgs^>(this, &Far::OnNetF11Menus), ToolOptions::F11Menus);
	RegisterTool(nullptr, String::Empty, gcnew EventHandler<ToolEventArgs^>(this, &Far::OnNetConfig), ToolOptions::Config);
	RegisterTool(nullptr, String::Empty, gcnew EventHandler<ToolEventArgs^>(this, &Far::OnNetDisk), ToolOptions::Disk);
	PluginSet::LoadPlugins();
}

//! Don't use FAR UI
void Far::Stop()
{
	PluginSet::UnloadPlugins();
	_instance = nullptr;

	delete[] _pConfig;
	delete[] _pDisk;
	delete[] _pDialog;
	delete[] _pEditor;
	delete[] _pPanels;
	delete[] _pViewer;
	delete _prefixes;
}

String^ Far::PluginFolderPath::get()
{
	String^ pluginPath = OemToStr(Info.ModuleName);
	return (gcnew FileInfo(pluginPath))->DirectoryName;
}

String^ Far::RootFar::get()
{
	String^ key = RootKey;
	return key->Substring(0, key->LastIndexOf('\\'));
}

String^ Far::RootKey::get()
{
	return OemToStr(Info.RootKey);
}

void Far::Free(ToolOptions options)
{
	if (int(options & ToolOptions::Config))
	{
		delete[] _pConfig;
		_pConfig = 0;
	}
	if (int(options & ToolOptions::Disk))
	{
		delete[] _pDisk;
		_pDisk = 0;
	}
	if (int(options & ToolOptions::Dialog))
	{
		delete[] _pDialog;
		_pDialog = 0;
	}
	if (int(options & ToolOptions::Editor))
	{
		delete[] _pEditor;
		_pEditor = 0;
	}
	if (int(options & ToolOptions::Panels))
	{
		delete[] _pPanels;
		_pPanels = 0;
	}
	if (int(options & ToolOptions::Viewer))
	{
		delete[] _pViewer;
		_pViewer = 0;
	}
}

void Far::RegisterTool(BasePlugin^ plugin, String^ name, EventHandler<ToolEventArgs^>^ handler, ToolOptions options)
{
	if (plugin && ES(name))
		throw gcnew ArgumentException("'name' must not be empty.");

	RegisterTool(gcnew ToolPluginInfo(plugin, name, handler, options));
}

void Far::RegisterTool(ToolPluginInfo^ tool)
{
	ToolOptions options = tool->Options;
	if (int(options & ToolOptions::Config))
	{
		delete[] _pConfig;
		_pConfig = 0;
		_toolConfig.Add(tool);
	}
	if (int(options & ToolOptions::Disk))
	{
		delete[] _pDisk;
		_pDisk = 0;
		_toolDisk.Add(tool);
	}
	if (int(options & ToolOptions::Dialog))
	{
		delete[] _pDialog;
		_pDialog = 0;
		_toolDialog.Add(tool);
	}
	if (int(options & ToolOptions::Editor))
	{
		delete[] _pEditor;
		_pEditor = 0;
		_toolEditor.Add(tool);
	}
	if (int(options & ToolOptions::Panels))
	{
		delete[] _pPanels;
		_pPanels = 0;
		_toolPanels.Add(tool);
	}
	if (int(options & ToolOptions::Viewer))
	{
		delete[] _pViewer;
		_pViewer = 0;
		_toolViewer.Add(tool);
	}
}

void Far::RegisterTools(IEnumerable<ToolPluginInfo^>^ tools)
{
	for each(ToolPluginInfo^ tool in tools)
		RegisterTool(tool);
}

static int RemoveByHandler(List<ToolPluginInfo^>^ list, EventHandler<ToolEventArgs^>^ handler)
{
	int r = 0;
	for(int i = list->Count; --i >= 0;)
	{
		if (list[i]->Handler == handler)
		{
			++r;
			list->RemoveAt(i);
		}
	}
	return r;
}

void Far::UnregisterTool(EventHandler<ToolEventArgs^>^ handler)
{
	if (RemoveByHandler(%_toolConfig, handler))
	{
		delete[] _pConfig;
		_pConfig = 0;
	}
	if (RemoveByHandler(%_toolDisk, handler))
	{
		delete[] _pDisk;
		_pDisk = 0;
	}
	if (RemoveByHandler(%_toolDialog, handler))
	{
		delete[] _pDialog;
		_pDialog = 0;
	}
	if (RemoveByHandler(%_toolEditor, handler))
	{
		delete[] _pEditor;
		_pEditor = 0;
	}
	if (RemoveByHandler(%_toolPanels, handler))
	{
		delete[] _pPanels;
		_pPanels = 0;
	}
	if (RemoveByHandler(%_toolViewer, handler))
	{
		delete[] _pViewer;
		_pViewer = 0;
	}
}

String^ Far::RegisterCommand(BasePlugin^ plugin, String^ name, String^ prefix, EventHandler<CommandEventArgs^>^ handler)
{
	delete _prefixes;
	_prefixes = 0;
	CommandPluginInfo^ it = gcnew CommandPluginInfo(plugin, name, prefix, handler);
	_registeredCommand.Add(it);
	return it->Prefix;
}

void Far::RegisterCommands(IEnumerable<CommandPluginInfo^>^ commands)
{
	delete _prefixes;
	_prefixes = 0;
	_registeredCommand.AddRange(commands);
}

void Far::UnregisterCommand(EventHandler<CommandEventArgs^>^ handler)
{
	for(int i = _registeredCommand.Count; --i >= 0;)
	{
		if (_registeredCommand[i]->Handler == handler)
		{
			delete _prefixes;
			_prefixes = 0;
			_registeredCommand.RemoveAt(i);
		}
	}
}

void Far::RegisterFiler(BasePlugin^ plugin, String^ name, EventHandler<FilerEventArgs^>^ handler, String^ mask, bool creates)
{
	_registeredFiler.Add(gcnew FilerPluginInfo(plugin, name, handler, mask, creates));
}

void Far::RegisterEditors(IEnumerable<EditorPluginInfo^>^ editors)
{
	_registeredEditor.AddRange(editors);
}

void Far::RegisterFilers(IEnumerable<FilerPluginInfo^>^ filers)
{
	_registeredFiler.AddRange(filers);
}

void Far::UnregisterFiler(EventHandler<FilerEventArgs^>^ handler)
{
	for(int i = _registeredFiler.Count; --i >= 0;)
		if (_registeredFiler[i]->Handler == handler)
			_registeredFiler.RemoveAt(i);
}

void Far::Msg(String^ body)
{
	Message::Show(body, nullptr, MessageOptions::Ok, nullptr, nullptr);
}

void Far::Msg(String^ body, String^ header)
{
	Message::Show(body, header, MessageOptions::Ok, nullptr, nullptr);
}

int Far::Msg(String^ body, String^ header, MessageOptions options)
{
	return Message::Show(body, header, options, nullptr, nullptr);
}

int Far::Msg(String^ body, String^ header, MessageOptions options, array<String^>^ buttons)
{
	return Message::Show(body, header, options, buttons, nullptr);
}

int Far::Msg(String^ body, String^ header, MessageOptions options, array<String^>^ buttons, String^ helpTopic)
{
	return Message::Show(body, header, options, buttons, helpTopic);
}

IMessage^ Far::CreateMessage()
{
	return gcnew Message;
}

void Far::Run(String^ command)
{
	int colon = command->IndexOf(':', 1);
	if (colon < 0)
		return;

	for each(CommandPluginInfo^ it in _registeredCommand)
	{
		String^ pref = it->Prefix;
		if (colon != pref->Length || !command->StartsWith(pref, StringComparison::OrdinalIgnoreCase))
			continue;

		//! Notify before each command, because a plugin may have to set a command environment,
		//! e.g. PowerShellFar sets the default runspace once and location always.
		//! If there is a plugin, call it directly, else it has to be done by its handler.
		if (it->Plugin)
			it->Plugin->Invoking();

		// invoke
		CommandEventArgs e(command->Substring(colon + 1));
		it->Handler(this, %e);
		break;
	}
}

IntPtr Far::HWnd::get()
{
	return (IntPtr)Info.AdvControl(Info.ModuleNumber, ACTL_GETFARHWND, nullptr);
}

System::Version^ Far::Version::get()
{
	DWORD vn;
	Info.AdvControl(Info.ModuleNumber, ACTL_GETFARVERSION, &vn);
	return gcnew System::Version((vn&0x0000ff00)>>8, vn&0x000000ff, (int)((long)vn&0xffff0000)>>16);
}

IMenu^ Far::CreateMenu()
{
	return gcnew Menu;
}

IMenuItem^ Far::CreateMenuItem()
{
	return gcnew MenuItem;
}

IListMenu^ Far::CreateListMenu()
{
	return gcnew ListMenu;
}

FarConfirmations Far::Confirmations::get()
{
	return (FarConfirmations)Info.AdvControl(Info.ModuleNumber, ACTL_GETCONFIRMATIONS, 0);
}

FarMacroState Far::MacroState::get()
{
	ActlKeyMacro command;
	command.Command = MCMD_GETSTATE;
	return (FarMacroState)Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &command);
}

array<IEditor^>^ Far::Editors()
{
	return EditorHost::Editors();
}

array<IViewer^>^ Far::Viewers()
{
	return ViewerHost::Viewers();
}

IAnyEditor^ Far::AnyEditor::get()
{
	return %EditorHost::_anyEditor;
}

IAnyViewer^ Far::AnyViewer::get()
{
	return %ViewerHost::_anyViewer;
}

String^ Far::PasteFromClipboard()
{
	char* buffer = Info.FSF->PasteFromClipboard();
	String^ r = OemToStr(buffer);
	Info.FSF->DeleteBuffer(buffer);
	return r;
}

void Far::CopyToClipboard(String^ text)
{
	CBox sText(text);
	Info.FSF->CopyToClipboard(sText);
}

IEditor^ Far::CreateEditor()
{
	return gcnew FarNet::Editor;
}

IViewer^ Far::CreateViewer()
{
	return gcnew FarNet::Viewer;
}

array<int>^ Far::CreateKeySequence(String^ keys)
{
	if (!keys) throw gcnew ArgumentNullException("keys");
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

void Far::PostKeySequence(array<int>^ sequence)
{
	PostKeySequence(sequence, true);
}

void Far::PostKeySequence(array<int>^ sequence, bool disableOutput)
{
	if (sequence == nullptr) throw gcnew ArgumentNullException("sequence");
	if (sequence->Length == 0)
		return;

	// local buffer for a small sequence
	const int smallCount = 50;
	DWORD keys[smallCount];

	KeySequence keySequence;
	keySequence.Count = sequence->Length;
	keySequence.Flags = disableOutput ? KSFLAGS_DISABLEOUTPUT : 0;
	keySequence.Sequence = keySequence.Count <= smallCount ? keys : new DWORD[keySequence.Count];
	DWORD* cur = keySequence.Sequence;
	for each(int i in sequence)
	{
		*cur = i;
		cur++;
	}
	try
	{
		if (!Info.AdvControl(Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, &keySequence))
			throw gcnew OperationCanceledException;
	}
	finally
	{
		if (keySequence.Sequence != keys)
			delete keySequence.Sequence;
	}
}

// don't throw on a wrong key, it is used for validation
int Far::NameToKey(String^ key)
{
	if (!key) throw gcnew ArgumentNullException("key");
	char buf[33];
	StrToOem(key, buf, sizeof(buf));
	return Info.FSF->FarNameToKey(buf);
}

String^ Far::KeyToName(int key)
{
	char name[33];
	if (!Info.FSF->FarKeyToName(key, name, sizeof(name) - 1))
		return nullptr;
	return OemToStr(name);
}

void Far::PostKeys(String^ keys)
{
	PostKeys(keys, true);
}

void Far::PostKeys(String^ keys, bool disableOutput)
{
	if (keys == nullptr)
		throw gcnew ArgumentNullException("keys");

	keys = keys->Trim();
	PostKeySequence(CreateKeySequence(keys), disableOutput);
}

void Far::PostText(String^ text)
{
	PostText(text, true);
}

void Far::PostText(String^ text, bool disableOutput)
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
	PostKeys(keys.ToString(), disableOutput);
}

int Far::SaveScreen(int x1, int y1, int x2, int y2)
{
	return (int)(INT_PTR)Info.SaveScreen(x1, y1, x2, y2);
}

void Far::RestoreScreen(int screen)
{
	Info.RestoreScreen((HANDLE)(INT_PTR)screen);
}

IKeyMacroHost^ Far::KeyMacro::get()
{
	return %KeyMacroHost::_instance;
}

ILine^ Far::CommandLine::get()
{
	return gcnew FarCommandLine;
}

ILine^ Far::Line::get()
{
	switch (GetWindowType(-1))
	{
	case WindowType::Editor:
		{
			IEditor^ editor = Editor;
			return editor->CurrentLine;
		}
	case WindowType::Panels:
		{
			return CommandLine;
		}
	case WindowType::Dialog:
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

IEditor^ Far::Editor::get()
{
	return EditorHost::GetCurrentEditor();
}

IViewer^ Far::Viewer::get()
{
	return ViewerHost::GetCurrentViewer();
}

IPanel^ Far::Panel::get()
{
	return PanelSet::GetPanel(true);
}

IPanel^ Far::Panel2::get()
{
	return PanelSet::GetPanel(false);
}

IInputBox^ Far::CreateInputBox()
{
	return gcnew InputBox;
}

//! frequently called
void Far::AsGetPluginInfo(PluginInfo* pi)
{
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = PF_DIALOG | PF_EDITOR | PF_VIEWER | PF_FULLCMDLINE | PF_PRELOAD;

	WindowInfo wi;
	wi.Pos = -1;
	if (!Info.AdvControl(Info.ModuleNumber, ACTL_GETSHORTWINDOWINFO, &wi))
		wi.Type = -1;

	if (_toolConfig.Count)
	{
		if (!_pConfig)
		{
			_pConfig = new CStr[_toolConfig.Count];
			for(int i = _toolConfig.Count; --i >= 0;)
				_pConfig[i].Set(Res::MenuPrefix + _toolConfig[i]->Alias(ToolOptions::Config));
		}
		pi->PluginConfigStringsNumber = _toolConfig.Count;
		pi->PluginConfigStrings = (const char**)_pConfig;
	}

	if (_toolDisk.Count)
	{
		if (!_pDisk)
		{
			_pDisk = new CStr[_toolDisk.Count];
			for(int i = _toolDisk.Count; --i >= 0;)
				_pDisk[i].Set(Res::MenuPrefix + _toolDisk[i]->Alias(ToolOptions::Disk));
		}
		pi->DiskMenuStringsNumber = _toolDisk.Count;
		pi->DiskMenuStrings = (const char**)_pDisk;
	}

	switch(wi.Type)
	{
	case WTYPE_EDITOR:
		if (_toolEditor.Count)
		{
			if (!_pEditor)
			{
				_pEditor = new CStr[_toolEditor.Count];
				for(int i = _toolEditor.Count; --i >= 0;)
					_pEditor[i].Set(Res::MenuPrefix + _toolEditor[i]->Alias(ToolOptions::Editor));
			}
			pi->PluginMenuStringsNumber = _toolEditor.Count;
			pi->PluginMenuStrings = (const char**)_pEditor;
		}
		break;
	case WTYPE_PANELS:
		if (_toolPanels.Count)
		{
			if (!_pPanels)
			{
				_pPanels = new CStr[_toolPanels.Count];
				for(int i = _toolPanels.Count; --i >= 0;)
					_pPanels[i].Set(Res::MenuPrefix + _toolPanels[i]->Alias(ToolOptions::Panels));
			}
			pi->PluginMenuStringsNumber = _toolPanels.Count;
			pi->PluginMenuStrings = (const char**)_pPanels;
		}
		break;
	case WTYPE_VIEWER:
		if (_toolViewer.Count)
		{
			if (!_pViewer)
			{
				_pViewer = new CStr[_toolViewer.Count];
				for(int i = _toolViewer.Count; --i >= 0;)
					_pViewer[i].Set(Res::MenuPrefix + _toolViewer[i]->Alias(ToolOptions::Viewer));
			}
			pi->PluginMenuStringsNumber = _toolViewer.Count;
			pi->PluginMenuStrings = (const char**)_pViewer;
		}
		break;
	case WTYPE_DIALOG:
		if (_toolDialog.Count)
		{
			if (!_pDialog)
			{
				_pDialog = new CStr[_toolDialog.Count];
				for(int i = _toolDialog.Count; --i >= 0;)
					_pDialog[i].Set(Res::MenuPrefix + _toolDialog[i]->Alias(ToolOptions::Dialog));
			}
			pi->PluginMenuStringsNumber = _toolDialog.Count;
			pi->PluginMenuStrings = (const char**)_pDialog;
		}
		break;
	}

	if (_registeredCommand.Count)
	{
		if (_prefixes == 0)
		{
			String^ PrefString = String::Empty;
			for each(CommandPluginInfo^ it in _registeredCommand)
			{
				if (PrefString->Length > 0)
					PrefString = String::Concat(PrefString, ":");
				PrefString = String::Concat(PrefString, it->Prefix);
			}
			_prefixes = new CStr(PrefString);
		}
		pi->CommandPrefix = *_prefixes;
	}
}

void Far::ProcessPrefixes(INT_PTR item)
{
	char* command = (char*)item;
	Run(OemToStr(command));
}

void Far::GetUserScreen()
{
	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN, 0);
}

void Far::SetUserScreen()
{
	Info.Control(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0);
}

// No exceptions, return what we can get
ICollection<String^>^ Far::GetDialogHistory(String^ name)
{
	List<String^>^ r = gcnew List<String^>;

	String^ keyName = RootFar + "\\SavedDialogHistory\\" + name;
	CBox sKeyName(keyName);

	HKEY hk;
	LONG lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER, sKeyName, 0, KEY_READ, &hk);
	if (lResult != ERROR_SUCCESS)
		return r;

	try
	{
		char lineName[99];
		int index = 0;
		for(;; ++index)
		{
			Info.FSF->sprintf(lineName, "Line%i", index);

			DWORD dwType = 0, dwCount = 0;
			lResult = ::RegQueryValueEx(hk, lineName, NULL, &dwType, NULL, &dwCount);
			if (lResult != ERROR_SUCCESS || dwType != REG_SZ)
				break;

			char* buf = new char[dwCount];
			lResult = ::RegQueryValueEx(hk, lineName, NULL, &dwType, (LPBYTE)buf, &dwCount);
			if (lResult != ERROR_SUCCESS)
				break;

			String^ s = OemToStr(buf);
			delete[] buf;
			r->Add(s);
		}
		return r;
	}
	finally
	{
		::RegCloseKey(hk);
	}
}

// No exceptions, return what we can get
ICollection<String^>^ Far::GetHistory(String^ name)
{
	List<String^>^ r = gcnew List<String^>;

	String^ keyName = RootFar + "\\" + name;
	CBox sKeyName(keyName);

	HKEY hk;
	LONG lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER, sKeyName, 0, KEY_READ, &hk);
	if (lResult != ERROR_SUCCESS)
		return r;

	char* buf = NULL;
	try
	{
		DWORD dwType = 0, dwCount = 0;
		lResult = ::RegQueryValueEx(hk, "Lines", NULL, &dwType, NULL, &dwCount);
		if (lResult != ERROR_SUCCESS || dwType != REG_BINARY)
			return r;

		buf = new char[dwCount];
		lResult = ::RegQueryValueEx(hk, "Lines", NULL, &dwType, (LPBYTE)buf, &dwCount);
		if (lResult != ERROR_SUCCESS)
			return r;

		for(int i = 0; i < (int)dwCount;)
		{
			String^ s = OemToStr(buf + i);
			r->Add(s);
			i += s->Length + 1;
		}
		return r;
	}
	finally
	{
		delete[] buf;
		::RegCloseKey(hk);
	}
}

void Far::ShowError(String^ title, Exception^ error)
{
	if (!error) throw gcnew ArgumentNullException("error");

	switch(Msg(
		error->Message,
		String::IsNullOrEmpty(title) ? error->GetType()->FullName : title,
		MessageOptions::LeftAligned | MessageOptions::Warning,
		gcnew array<String^>{"Ok", "View Info", "Copy Info"}))
	{
	case 1:
		Far::Instance->AnyViewer->ViewText(
			ExceptionInfo(error, false) + "\n" + error->ToString(),
			error->GetType()->FullName,
			OpenMode::Modal);
		return;
	case 2:
		CopyToClipboard(ExceptionInfo(error, false) + "\r\n" + error->ToString());
		return;
	}
}

IDialog^ Far::CreateDialog(int left, int top, int right, int bottom)
{
	return gcnew FarDialog(left, top, right, bottom);
}

void Far::WriteText(int left, int top, ConsoleColor foregroundColor, ConsoleColor backgroundColor, String^ text)
{
	CBox sText(text);
	Info.Text(left, top, int(foregroundColor)|(int(backgroundColor)<<4), sText);
}

void Far::ShowHelp(String^ path, String^ topic, HelpOptions options)
{
	CBox sPath; sPath.Reset(path);
	CBox sTopic; sTopic.Reset(topic);
	Info.ShowHelp(sPath, sTopic, (int)options);
}

void Far::Write(String^ text)
{
	if (!ValueUserScreen::Get())
	{
		ValueUserScreen::Set(true);
		GetUserScreen();
	}
	Console::Write(text);
	SetUserScreen();
}

void Far::Write(String^ text, ConsoleColor foregroundColor)
{
	ConsoleColor fc = Console::ForegroundColor;
	Console::ForegroundColor = foregroundColor;
	Write(text);
	Console::ForegroundColor = fc;
}

void Far::Write(String^ text, ConsoleColor foregroundColor, ConsoleColor backgroundColor)
{
	ConsoleColor fc = Console::ForegroundColor;
	ConsoleColor bc = Console::BackgroundColor;
	Console::ForegroundColor = foregroundColor;
	Console::BackgroundColor = backgroundColor;
	Write(text);
	Console::ForegroundColor = fc;
	Console::BackgroundColor = bc;
}

IPanelPlugin^ Far::CreatePanelPlugin()
{
	return gcnew FarPanelPlugin;
}

IPanelPlugin^ Far::GetPanelPlugin(Type^ hostType)
{
	return PanelSet::GetPanelPlugin(hostType);
}

IFile^ Far::CreatePanelItem()
{
	return gcnew FarFile;
}

IFile^ Far::CreatePanelItem(FileSystemInfo^ info, bool fullName)
{
	FarFile^ r = gcnew FarFile;
	if (info)
	{
		r->Name = fullName ? info->FullName : info->Name;
		r->CreationTime = info->CreationTime;
		r->LastAccessTime = info->LastAccessTime;
		r->LastWriteTime = info->LastWriteTime;
		r->SetAttributes(info->Attributes);
		if (!r->IsDirectory)
			r->Length = ((FileInfo^)info)->Length;
	}
	return r;
}

String^ Far::Input(String^ prompt)
{
	return Input(prompt, nullptr, nullptr, String::Empty);
}

String^ Far::Input(String^ prompt, String^ history)
{
	return Input(prompt, history, nullptr, String::Empty);
}

String^ Far::Input(String^ prompt, String^ history, String^ title)
{
	return Input(prompt, history, title, String::Empty);
}

String^ Far::Input(String^ prompt, String^ history, String^ title, String^ text)
{
	InputBox ib;
	ib.Prompt = prompt;
	ib.Title = title;
	ib.Text = text;
	ib.History = history;
	ib.EmptyEnabled = true;
	return ib.Show() ? ib.Text : nullptr;
}

//::FAR Window managenent

ref class FarWindowInfo : public IWindowInfo
{
public:
	FarWindowInfo(const WindowInfo& wi, bool full)
		: _Current(wi.Current != 0), _Modified(wi.Modified != 0), _Type((WindowType)wi.Type)
	{
		if (full)
		{
			_Name = OemToStr(wi.Name);
			_TypeName = OemToStr(wi.TypeName);
		}
	}
	virtual property bool Current { bool get() { return _Current; } }
	virtual property bool Modified { bool get() { return _Modified; } }
	virtual property String^ Name { String^ get() { return _Name; } }
	virtual property String^ TypeName { String^ get() { return _TypeName; } }
	virtual property WindowType Type { WindowType get() { return _Type; } }
private:
	bool _Current;
	bool _Modified;
	String^ _Name;
	String^ _TypeName;
	WindowType _Type;
};

int Far::WindowCount::get()
{
	return (int)Info.AdvControl(Info.ModuleNumber, ACTL_GETWINDOWCOUNT, 0);
}

IWindowInfo^ Far::GetWindowInfo(int index, bool full)
{
	WindowInfo wi;
	wi.Pos = index;
	if (!Info.AdvControl(Info.ModuleNumber, full ? ACTL_GETWINDOWINFO : ACTL_GETSHORTWINDOWINFO, &wi))
		throw gcnew InvalidOperationException("GetWindowInfo:" + index + " failed.");
	return gcnew FarWindowInfo(wi, full);
}

WindowType Far::GetWindowType(int index)
{
	WindowInfo wi;
	wi.Pos = index;
	if (!Info.AdvControl(Info.ModuleNumber, ACTL_GETSHORTWINDOWINFO, &wi))
		throw gcnew InvalidOperationException("GetWindowType:" + index + " failed.");
	return (WindowType)wi.Type;
}

void Far::SetCurrentWindow(int index)
{
	if (!Info.AdvControl(Info.ModuleNumber, ACTL_SETCURRENTWINDOW, (void*)(INT_PTR)index))
		throw gcnew InvalidOperationException("SetCurrentWindow:" + index + " failed.");
}

bool Far::Commit()
{
	return Info.AdvControl(Info.ModuleNumber, ACTL_COMMIT, 0) != 0;
}

Char Far::CodeToChar(int code)
{
	code &= ~(KeyCode::Alt | KeyCode::Ctrl);
	return code < 0 || code > 255 ? 0 : OemToChar(char(code));
}

Object^ Far::GetFarValue(String^ keyPath, String^ valueName, Object^ defaultValue)
{
	RegistryKey^ key;
	try
	{
		key = Registry::CurrentUser->OpenSubKey(RootFar + "\\" + keyPath);
		return key ? key->GetValue(valueName, defaultValue) : defaultValue;
	}
	finally
	{
		if (key)
			key->Close();
	}
}

Object^ Far::GetPluginValue(String^ pluginName, String^ valueName, Object^ defaultValue)
{
	RegistryKey^ key;
	try
	{
		key = Registry::CurrentUser->OpenSubKey(RootKey + "\\" + pluginName);
		return key ? key->GetValue(valueName, defaultValue) : defaultValue;
	}
	finally
	{
		if (key)
			key->Close();
	}
}

void Far::SetPluginValue(String^ pluginName, String^ valueName, Object^ newValue)
{
	RegistryKey^ key;
	try
	{
		key = Registry::CurrentUser->CreateSubKey(RootKey + "\\" + pluginName);
		key->SetValue(valueName, newValue);
	}
	finally
	{
		if (key)
			key->Close();
	}
}

//::FAR callbacks

bool Far::AsConfigure(int itemIndex)
{
	ToolPluginInfo^ tool = _toolConfig[itemIndex];
	ToolEventArgs e(ToolOptions::Config);
	tool->Handler(this, %e);
	return e.Ignore ? false : true;
}

HANDLE Far::AsOpenFilePlugin(char* name, const unsigned char* data, int dataSize)
{
	if (_registeredFiler.Count == 0)
		return INVALID_HANDLE_VALUE;

	ValueCanOpenPanel canopenpanel(true);
	ValueUserScreen userscreen;

	try
	{
		FilerEventArgs^ e;
		for each(FilerPluginInfo^ it in _registeredFiler)
		{
			// create?
			if (!name && !it->Creates)
				continue;

			// mask?
			if (SS(it->Mask) && !CompareNameEx(it->Mask, name, true))
				continue;

			// arguments
			if (!e)
				e = gcnew FilerEventArgs(OemToStr(name), gcnew UnmanagedMemoryStream((unsigned char*)data, dataSize, dataSize, FileAccess::Read));
			else
				e->Data->Seek(0, SeekOrigin::Begin);

			// invoke
			it->Handler(this, e);

			// open a posted panel
			if (PanelSet::PostedPanel)
			{
				HANDLE h = PanelSet::AddPanelPlugin(PanelSet::PostedPanel);
				return h;
			}
		}

		return INVALID_HANDLE_VALUE;
	}
	finally
	{
		// drop a posted panel
		PanelSet::PostedPanel = nullptr;
	}
}

HANDLE Far::AsOpenPlugin(int from, INT_PTR item)
{
	ValueCanOpenPanel canopenpanel(true);
	ValueUserScreen userscreen;

	try
	{
		// call, plugin may create a panel waiting for opening
		switch(from)
		{
		case OPEN_COMMANDLINE:
			{
				ProcessPrefixes(item);
			}
			break;
		case OPEN_DISKMENU:
			{
				ToolPluginInfo^ tool = _toolDisk[(int)item];
				ToolEventArgs e(ToolOptions::Disk);
				tool->Handler(this, %e);
			}
			break;
		case OPEN_PLUGINSMENU:
			{
				ToolPluginInfo^ tool = _toolPanels[(int)item];
				ToolEventArgs e(ToolOptions::Panels);
				tool->Handler(this, %e);
			}
			break;
		case OPEN_EDITOR:
			{
				ValueCanOpenPanel::Set(false);
				ToolPluginInfo^ tool = _toolEditor[(int)item];
				ToolEventArgs e(ToolOptions::Editor);
				tool->Handler(this, %e);
			}
			break;
		case OPEN_VIEWER:
			{
				ValueCanOpenPanel::Set(false);
				ToolPluginInfo^ tool = _toolViewer[(int)item];
				ToolEventArgs e(ToolOptions::Viewer);
				tool->Handler(this, %e);
			}
			break;
		case OPEN_DIALOG:
			{
				ValueCanOpenPanel::Set(false);
				const OpenDlgPluginData* dd = (const OpenDlgPluginData*)item;
				ToolPluginInfo^ tool = _toolDialog[dd->ItemNumber];
				ToolEventArgs e(ToolOptions::Dialog);
				FarDialog::_hDlgLast = dd->hDlg;
				tool->Handler(this, %e);
			}
			break;
		}

		// open a posted panel
		if (PanelSet::PostedPanel)
		{
			HANDLE h = PanelSet::AddPanelPlugin(PanelSet::PostedPanel);
			return h;
		}

		// don't open a panel
		return INVALID_HANDLE_VALUE;
	}
	finally
	{
		// drop a posted panel
		PanelSet::PostedPanel = nullptr;
	}
}

array<IPanelPlugin^>^ Far::PushedPanels()
{
	array<IPanelPlugin^>^ r = gcnew array<IPanelPlugin^>(PanelSet::_stack.Count);
	for(int i = PanelSet::_stack.Count; --i >= 0;)
		r[i] = PanelSet::_stack[i];
	return r;
}

void Far::ShowPanelMenu(bool showPushCommand)
{
	Menu m;
	m.AutoAssignHotkeys = true;
	m.HelpTopic = "MenuPanels";
	m.ShowAmpersands = true;
	m.Title = showPushCommand ? "Push/show panels" : "Show panels";

	// "Push" command
	if (showPushCommand)
	{
		FarPanelPlugin^ pp = dynamic_cast<FarPanelPlugin^>(Panel);
		if (pp)
		{
			IMenuItem^ mi = m.Add("Push the panel");
			mi->Data = pp;
		}
		else
		{
			showPushCommand = false;
		}
	}

	// pushed panels
	if (PanelSet::_stack.Count)
	{
		if (showPushCommand)
			m.Add(String::Empty)->IsSeparator = true;
		for(int i = PanelSet::_stack.Count; --i >= 0;)
		{
			FarPanelPlugin^ pp = PanelSet::_stack[i];
			IMenuItem^ mi = m.Add(JoinText(pp->_info.Title, pp->_info.CurrentDirectory));
			mi->Data = pp;
		}
	}

	// go
	if (!m.Show())
		return;

	// push
	if (showPushCommand && m.Selected == 0)
	{
		FarPanelPlugin^ pp = (FarPanelPlugin^)m.SelectedData;
		pp->Push();
		return;
	}

	// pop
	FarPanelPlugin^ pp = (FarPanelPlugin^)m.SelectedData;
	pp->Open();
}

void Far::PostStep(EventHandler^ step)
{
	// make keys once
	if (!_hotkeys)
	{
		if (ES(_hotkey))
			throw gcnew OperationCanceledException(Res::ErrorNoHotKey);

		array<int>^ keys = gcnew array<int>(2);
		keys[1] = NameToKey(_hotkey);
		keys[0] = NameToKey("F11");
		_hotkeys = keys;
	}

	// post handler and keys
	_handler = step;
	PostKeySequence(_hotkeys);
}

void Far::OnNetF11Menus(Object^ /*sender*/, ToolEventArgs^ e)
{
	// process and drop a posted step handler
	if (_handler)
	{
		EventHandler^ handler = _handler;
		_handler = nullptr;
		handler(nullptr, nullptr);
		return;
	}

	// show panels menu
	if (e->From == ToolOptions::Panels)
		ShowPanelMenu(true);
}

void Far::OnNetDisk(Object^ /*sender*/, ToolEventArgs^ /*e*/)
{
	ShowPanelMenu(false);
}

void Far::OnNetConfig(Object^ /*sender*/, ToolEventArgs^ /*e*/)
{
	Menu m;
	m.AutoAssignHotkeys = true;
	m.HelpTopic = "MenuConfig";
	m.Title = "FAR.NET plugins";

	m.Add(Res::CommandPlugins + " : " + (_registeredCommand.Count));
	m.Add(Res::EditorPlugins + "  : " + (_registeredEditor.Count));
	m.Add(Res::FilerPlugins + "   : " + (_registeredFiler.Count));
	m.Add(String::Empty)->IsSeparator = true;
	m.Add(Res::PanelsTools + "    : " + (_toolPanels.Count - 1));
	m.Add(Res::EditorTools + "    : " + (_toolEditor.Count - 1));
	m.Add(Res::ViewerTools + "    : " + (_toolViewer.Count - 1));
	m.Add(Res::DialogTools + "    : " + (_toolDialog.Count - 1));
	m.Add(Res::ConfigTools + "    : " + (_toolConfig.Count - 1));
	m.Add(Res::DiskTools + "      : " + (_toolDisk.Count - 1));

	while(m.Show())
	{
		switch(m.Selected)
		{
		case 0:
			if (_registeredCommand.Count)
				OnConfigCommand();
			break;
		case 1:
			if (_registeredEditor.Count)
				OnConfigEditor();
			break;
		case 2:
			if (_registeredFiler.Count)
				OnConfigFiler();
			break;
			// mind separator
		case 4:
			if (_toolPanels.Count > 1)
				OnConfigTool(Res::PanelsTools, ToolOptions::Panels, %_toolPanels);
			break;
		case 5:
			if (_toolEditor.Count > 1)
				OnConfigTool(Res::EditorTools, ToolOptions::Editor, %_toolEditor);
			break;
		case 6:
			if (_toolViewer.Count > 1)
				OnConfigTool(Res::ViewerTools, ToolOptions::Viewer, %_toolViewer);
			break;
		case 7:
			if (_toolDialog.Count > 1)
				OnConfigTool(Res::DialogTools, ToolOptions::Dialog, %_toolDialog);
			break;
		case 8:
			if (_toolConfig.Count > 1)
				OnConfigTool(Res::ConfigTools, ToolOptions::Config, %_toolConfig);
			break;
		case 9:
			if (_toolDisk.Count > 1)
				OnConfigTool(Res::DiskTools, ToolOptions::Disk, %_toolDisk);
			break;
		}
	}
}

void Far::OnConfigTool(String^ title, ToolOptions option, List<ToolPluginInfo^>^ list)
{
	Menu m;
	m.Title = title;
	m.HelpTopic = option == ToolOptions::Disk ? "ConfigDisk" : "ConfigTool";

	ToolPluginInfo^ selected;
	List<ToolPluginInfo^> sorted(list);
	for(;;)
	{
		m.Items->Clear();
		sorted.Sort(gcnew ToolPluginAliasComparer(option));
		for each(ToolPluginInfo^ it in sorted)
		{
			if (ES(it->Name))
				continue;
			if (it == selected)
				m.Selected = m.Items->Count;
			IMenuItem^ mi = m.Add(Res::MenuPrefix + it->Alias(option) + " : " + it->Key);
			mi->Data = it;
		}

		// case: disk
		if (option == ToolOptions::Disk)
		{
			while(m.Show()) {}
			return;
		}

		// show others
		if (!m.Show())
			return;

		IMenuItem^ mi = m.Items[m.Selected];
		selected = (ToolPluginInfo^)mi->Data;

		InputBox ib;
		ib.EmptyEnabled = true;
		ib.HelpTopic = _helpTopic + "ConfigTool";
		ib.Prompt = "New string (ampersand ~ hotkey)";
		ib.Text = selected->Alias(option);
		ib.Title = "Original: " + selected->Name;
		if (!ib.Show())
			continue;

		// restore the name on empty alias
		String^ alias = ib.Text->TrimEnd();
		if (alias->Length == 0)
			alias = selected->Name;

		// reset the alias
		Free(option);
		selected->Alias(option, alias);
	}
}

void Far::OnConfigCommand()
{
	Menu m;
	m.AutoAssignHotkeys = true;
	m.HelpTopic = "ConfigCommand";
	m.Title = Res::CommandPlugins;

	for each(CommandPluginInfo^ it in _registeredCommand)
	{
		IMenuItem^ mi = m.Add(it->Prefix->PadRight(4) + " " + it->Key);
		mi->Data = it;
	}

	while(m.Show())
	{
		IMenuItem^ mi = m.Items[m.Selected];
		CommandPluginInfo^ it = (CommandPluginInfo^)mi->Data;

		InputBox ib;
		ib.EmptyEnabled = true;
		ib.HelpTopic = _helpTopic + "ConfigCommand";
		ib.Prompt = "New prefix for " + it->Name;
		ib.Text = it->Prefix;
		ib.Title = "Original prefix: " + it->DefaultPrefix;

		String^ alias = nullptr;
		while(ib.Show())
		{
			alias = ib.Text->Trim();
			if (alias->IndexOf(" ") >= 0 || alias->IndexOf(":") >= 0)
			{
				Msg("Prefix must not contain ' ' or ':'.");
				alias = nullptr;
				continue;
			}
			break;
		}
		if (!alias)
			continue;

		// restore original on empty
		if (alias->Length == 0)
			alias = it->DefaultPrefix;

		// reset
		delete _prefixes;
		_prefixes = 0;
		it->Prefix = alias;
		mi->Text = alias->PadRight(4) + " " + it->Key;
		CommandPlugin^ command = dynamic_cast<CommandPlugin^>(it->Plugin);
		if (command)
			command->Prefix = alias;
	}
}

void Far::OnConfigEditor()
{
	Menu m;
	m.AutoAssignHotkeys = true;
	m.HelpTopic = "ConfigEditor";
	m.Title = Res::EditorPlugins;

	for each(EditorPluginInfo^ it in _registeredEditor)
	{
		IMenuItem^ mi = m.Add(it->Key);
		mi->Data = it;
	}

	while(m.Show())
	{
		IMenuItem^ mi = m.Items[m.Selected];
		EditorPluginInfo^ it = (EditorPluginInfo^)mi->Data;

		InputBox ib;
		ib.EmptyEnabled = true;
		ib.HelpTopic = _helpTopic + "ConfigEditor";
		ib.History = "Masks";
		ib.Prompt = "New mask for " + it->Name;
		ib.Text = it->Mask;
		ib.Title = "Original mask: " + it->DefaultMask;

		if (!ib.Show())
			return;
		String^ mask = ib.Text->Trim();

		// restore original on empty
		if (mask->Length == 0)
			mask = it->DefaultMask;

		// set
		it->Mask = mask;
		EditorPlugin^ filer = dynamic_cast<EditorPlugin^>(it->Plugin);
		if (filer)
			filer->Mask = mask;
	}
}

void Far::OnConfigFiler()
{
	Menu m;
	m.AutoAssignHotkeys = true;
	m.HelpTopic = "ConfigFiler";
	m.Title = Res::FilerPlugins;

	for each(FilerPluginInfo^ it in _registeredFiler)
	{
		IMenuItem^ mi = m.Add(it->Key);
		mi->Data = it;
	}

	while(m.Show())
	{
		IMenuItem^ mi = m.Items[m.Selected];
		FilerPluginInfo^ it = (FilerPluginInfo^)mi->Data;

		InputBox ib;
		ib.EmptyEnabled = true;
		ib.HelpTopic = _helpTopic + "ConfigFiler";
		ib.History = "Masks";
		ib.Prompt = "New mask for " + it->Name;
		ib.Text = it->Mask;
		ib.Title = "Original mask: " + it->DefaultMask;

		if (!ib.Show())
			return;
		String^ mask = ib.Text->Trim();

		// restore original on empty
		if (mask->Length == 0)
			mask = it->DefaultMask;

		// set
		it->Mask = mask;
		FilerPlugin^ filer = dynamic_cast<FilerPlugin^>(it->Plugin);
		if (filer)
			filer->Mask = mask;
	}
}

bool Far::CompareName(String^ mask, const char* name, bool skipPath)
{
	for each(String^ s in mask->Split(gcnew array<Char>{',', ';'}, StringSplitOptions::RemoveEmptyEntries))
	{
		CBox buf(s);
		if (Info.CmpName(buf, name, skipPath))
			return true;
	}
	return false;
}

bool Far::CompareNameEx(String^ mask, const char* name, bool skipPath)
{
	int i = mask->IndexOf('|');
	if (i < 0)
		return CompareName(mask, name, skipPath);
	return  CompareName(mask->Substring(0, i), name, skipPath) && !CompareName(mask->Substring(i + 1), name, skipPath);
}

void Far::OnEditorOpened(FarNet::Editor^ editor)
{
	if (_registeredEditor.Count == 0)
		return;

	EditorInfo ei;
	EditorControl_ECTL_GETINFO(ei);
	for each(EditorPluginInfo^ it in _registeredEditor)
	{
		// mask?
		if (SS(it->Mask) && !CompareNameEx(it->Mask, ei.FileName, true))
			continue;

		//! tradeoff: catch all to call other plugins, too
		try
		{
			it->Handler(editor, EventArgs::Empty);
		}
		catch(Exception^ error)
		{
			//! show plugin info, too
			ShowError(it->Key, error);
		}
	}
}

void Far::Redraw()
{
	if (_version_1_71_2315)
		Info.AdvControl(Info.ModuleNumber, ACTL_REDRAWALL, 0);
}

String^ Far::TempName(String^ prefix)
{
	char dest[MAX_PATH];
	char pref[5];
	StrToOem(prefix, pref, 5);
	if (!Info.FSF->MkTemp(dest, pref))
		throw gcnew OperationCanceledException(__FUNCTION__);
	return gcnew String(dest);
}

String^ Far::TempFolder(String^ prefix)
{
	String^ r = TempName(prefix);
	Directory::CreateDirectory(r);
	return r;
}

IDialog^ Far::Dialog::get()
{
	return FarDialog::GetDialog();
}

}