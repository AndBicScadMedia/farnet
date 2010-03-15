/*
FarNet plugin for Far Manager
Copyright (c) 2005 FarNet Team
*/

#include "StdAfx.h"
#include "Far0.h"
#include "Dialog.h"
#include "ModuleLoader.h"
#include "ModuleManager.h"
#include "ModuleProxy.h"
#include "Panel0.h"
#include "Panel2.h"
#include "Registry.h"
#include "Shelve.h"
#include "Wrappers.h"

namespace FarNet
{;
void Far0::Start()
{
	_hMutex = CreateMutex(NULL, FALSE, NULL);
	_hotkey = FarRegistryKey::GetFarValue("PluginHotkeys\\Plugins/FarNet/FarNetMan.dll", "Hotkey", String::Empty)->ToString();
	ModuleLoader::LoadModules();
}

//! Don't use Far UI
void Far0::Stop()
{
	CloseHandle(_hMutex);
	ModuleLoader::UnloadModules();

	delete[] _pConfig;
	delete[] _pDisk;
	delete[] _pDialog;
	delete[] _pEditor;
	delete[] _pPanels;
	delete[] _pViewer;
	delete _prefixes;
}

void Far0::UnregisterProxyAction(ProxyAction^ action)
{
	// case: tool
	ProxyTool^ tool = dynamic_cast<ProxyTool^>(action);
	if (tool)
	{
		UnregisterProxyTool(tool);
		return;
	}

	LOG_3("Unregister " + action);

	ModuleLoader::Actions->Remove(action->Id);

	ProxyCommand^ command = dynamic_cast<ProxyCommand^>(action);
	if (command)
	{
		_registeredCommand.Remove(command);
		delete _prefixes;
		_prefixes = 0;
		return;
	}

	ProxyEditor^ editor = dynamic_cast<ProxyEditor^>(action);
	if (editor)
	{
		_registeredEditor.Remove(editor);
		return;
	}

	ProxyFiler^ filer = dynamic_cast<ProxyFiler^>(action);
	if (filer)
	{
		_registeredFiler.Remove(filer);
		return;
	}
}

void Far0::UnregisterProxyTool(ProxyTool^ tool)
{
	LOG_3("Unregister " + tool);

	ModuleLoader::Actions->Remove(tool->Id);

	InvalidateProxyTool(tool->Options);
}

void Far0::InvalidateProxyTool(ModuleToolOptions options)
{
	if (int(options & ModuleToolOptions::Config))
	{
		_toolConfig = nullptr;
		delete[] _pConfig;
		_pConfig = 0;
	}

	if (int(options & ModuleToolOptions::Dialog))
	{
		_toolDialog = nullptr;
		delete[] _pDialog;
		_pDialog = 0;
	}

	if (int(options & ModuleToolOptions::Disk))
	{
		_toolDisk = nullptr;
		delete[] _pDisk;
		_pDisk = 0;
	}

	if (int(options & ModuleToolOptions::Editor))
	{
		_toolEditor = nullptr;
		delete[] _pEditor;
		_pEditor = 0;
	}

	if (int(options & ModuleToolOptions::Panels))
	{
		_toolPanels = nullptr;
		delete[] _pPanels;
		_pPanels = 0;
	}

	if (int(options & ModuleToolOptions::Viewer))
	{
		_toolViewer = nullptr;
		delete[] _pViewer;
		_pViewer = 0;
	}
}

void Far0::RegisterProxyCommand(ProxyCommand^ info)
{
	LOG_3("Register " + info);
	ModuleLoader::Actions->Add(info->Id, info);

	_registeredCommand.Add(info);
	delete _prefixes;
	_prefixes = 0;
}

void Far0::RegisterProxyEditor(ProxyEditor^ info)
{
	LOG_3("Register " + info);
	ModuleLoader::Actions->Add(info->Id, info);

	_registeredEditor.Add(info);
}

void Far0::RegisterProxyFiler(ProxyFiler^ info)
{
	LOG_3("Register " + info);
	ModuleLoader::Actions->Add(info->Id, info);

	_registeredFiler.Add(info);
}

void Far0::RegisterProxyTool(ProxyTool^ info)
{
	LOG_3("Register " + info);

	ModuleLoader::Actions->Add(info->Id, info);

	InvalidateProxyTool(info->Options);
}

void Far0::Run(String^ command)
{
	int colon = command->IndexOf(':', 1);
	if (colon < 0)
		return;

	for each(ProxyCommand^ it in _registeredCommand)
	{
		String^ pref = it->Prefix;
		if (colon != pref->Length || !command->StartsWith(pref, StringComparison::OrdinalIgnoreCase))
			continue;

		// invoke
		ModuleCommandEventArgs e;
		e.Command = command->Substring(colon + 1);
		it->Invoke(nullptr, %e);

		break;
	}
}

/*
-- It is called frequently to get information about menu and disk commands.
-- It is not called when FarNet is unloaded.

// http://forum.farmanager.com/viewtopic.php?f=7&t=3890
// (?? it would be nice to have ACTL_POSTCALLBACK)
*/
void Far0::AsGetPluginInfo(PluginInfo* pi)
{
	//! STOP
	// Do not ignore these methods even in stepping mode:
	// *) plugins can change this during stepping and Far has to be informed;
	// *) there is no more or less noticeable performance gain at all, really.
	// We still can do that with some global flags telling that something was changed, but with
	// not at all performance gain it would be more complexity for nothing. The code is disabled.

	// get window type, this is the only known way to get the current area
	// (?? wish to have 'from' parameter)
	WindowInfo wi;
	wi.Pos = -1;
	if (!Info.AdvControl(Info.ModuleNumber, ACTL_GETSHORTWINDOWINFO, &wi))
		wi.Type = -1;

	//! Do not forget to add FarNet menus first -> alloc one more and use shifted index.

	// config
	{
		if (!_toolConfig)
		{
			_toolConfig = ModuleLoader::GetTools(ModuleToolOptions::Config);
			_pConfig = new CStr[_toolConfig->Length + 1];
			_pConfig[0].Set(Res::MenuPrefix);

			for(int i = _toolConfig->Length; --i >= 0;)
				_pConfig[i + 1].Set(Res::MenuPrefix + _toolConfig[i]->GetMenuText());
		}

		pi->PluginConfigStringsNumber = _toolConfig->Length + 1;
		pi->PluginConfigStrings = (const wchar_t**)_pConfig;
	}

	// disk (do not add .NET item!)
	{
		if (!_toolDisk)
		{
			_toolDisk = ModuleLoader::GetTools(ModuleToolOptions::Disk);
			if (_toolDisk->Length > 0)
			{
				_pDisk = new CStr[_toolDisk->Length];

				//! Use just Name, not menu text, and no prefix.
				for(int i = _toolDisk->Length; --i >= 0;)
					_pDisk[i].Set(_toolDisk[i]->Name);
			}
		}

		pi->DiskMenuStringsNumber = _toolDisk->Length;
		pi->DiskMenuStrings = (const wchar_t**)_pDisk;
	}

	// type
	switch(wi.Type)
	{
	case WTYPE_DIALOG:
		{
			if (!_toolDialog)
			{
				_toolDialog = ModuleLoader::GetTools(ModuleToolOptions::Dialog);
				_pDialog = new CStr[_toolDialog->Length + 1];
				_pDialog[0].Set(Res::MenuPrefix);

				for(int i = _toolDialog->Length; --i >= 0;)
					_pDialog[i + 1].Set(Res::MenuPrefix + _toolDialog[i]->GetMenuText());
			}

			pi->PluginMenuStringsNumber = _toolDialog->Length + 1;
			pi->PluginMenuStrings = (const wchar_t**)_pDialog;
		}
		break;
	case WTYPE_EDITOR:
		{
			if (!_toolEditor)
			{
				_toolEditor = ModuleLoader::GetTools(ModuleToolOptions::Editor);
				_pEditor = new CStr[_toolEditor->Length + 1];
				_pEditor[0].Set(Res::MenuPrefix);

				for(int i = _toolEditor->Length; --i >= 0;)
					_pEditor[i + 1].Set(Res::MenuPrefix + _toolEditor[i]->GetMenuText());
			}

			pi->PluginMenuStringsNumber = _toolEditor->Length + 1;
			pi->PluginMenuStrings = (const wchar_t**)_pEditor;
		}
		break;
	case WTYPE_PANELS:
		{
			if (!_toolPanels)
			{
				_toolPanels = ModuleLoader::GetTools(ModuleToolOptions::Panels);
				_pPanels = new CStr[_toolPanels->Length + 1];
				_pPanels[0].Set(Res::MenuPrefix);

				for(int i = _toolPanels->Length; --i >= 0;)
					_pPanels[i + 1].Set(Res::MenuPrefix + _toolPanels[i]->GetMenuText());
			}

			pi->PluginMenuStringsNumber = _toolPanels->Length + 1;
			pi->PluginMenuStrings = (const wchar_t**)_pPanels;
		}
		break;
	case WTYPE_VIEWER:
		{
			if (!_toolViewer)
			{
				_toolViewer = ModuleLoader::GetTools(ModuleToolOptions::Viewer);
				_pViewer = new CStr[_toolViewer->Length + 1];
				_pViewer[0].Set(Res::MenuPrefix);

				for(int i = _toolViewer->Length; --i >= 0;)
					_pViewer[i + 1].Set(Res::MenuPrefix + _toolViewer[i]->GetMenuText());
			}

			pi->PluginMenuStringsNumber = _toolViewer->Length + 1;
			pi->PluginMenuStrings = (const wchar_t**)_pViewer;
		}
		break;
	}

	if (_registeredCommand.Count)
	{
		if (_prefixes == 0)
		{
			String^ PrefString = String::Empty;
			for each(ProxyCommand^ it in _registeredCommand)
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

void Far0::ProcessPrefixes(INT_PTR item)
{
	wchar_t* command = (wchar_t*)item;
	Run(gcnew String(command));
}

int Far0::GetPaletteColor(PaletteColor paletteColor)
{
	INT_PTR index = (INT_PTR)paletteColor;
	if (index < 0 || index >= COL_LASTPALETTECOLOR)
		throw gcnew ArgumentOutOfRangeException("paletteColor");
	return (int)Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void*)index);
}

//::Far callbacks

bool Far0::AsConfigure(int itemIndex)
{
	if (itemIndex == 0)
	{
		OpenConfig();
		return true;
	}

	// STOP: If it is called by [ShiftF9] from a F11-menu then Far sends the
	// index from that menu, not from our config items. There is nothing we can
	// do about it: the same method is called from the config menu. All we can
	// do is to check sanity of the index and ignore invalid values.
	if (--itemIndex >= _toolConfig->Length)
		return false;

	ProxyTool^ tool = _toolConfig[itemIndex];
	ModuleToolEventArgs e;
	e.From = ModuleToolOptions::Config;
	tool->Invoke(nullptr, %e);
	return e.Ignore ? false : true;
}

HANDLE Far0::AsOpenFilePlugin(wchar_t* name, const unsigned char* data, int dataSize, int opMode)
{
	if (_registeredFiler.Count == 0)
		return INVALID_HANDLE_VALUE;

	Panel0::BeginOpenMode();
	ValueUserScreen userscreen;

	try
	{
		ModuleFilerEventArgs^ e;
		for each(ProxyFiler^ it in _registeredFiler)
		{
			// create?
			if (!name && !it->Creates)
				continue;

			// mask?
			if (SS(it->Mask) && !CompareNameEx(it->Mask, name, true))
				continue;

			// arguments
			if (!e)
			{
				e = gcnew ModuleFilerEventArgs;
				e->Name = gcnew String(name);
				e->Mode = (OperationModes)opMode;
				e->Data = gcnew UnmanagedMemoryStream((unsigned char*)data, dataSize, dataSize, FileAccess::Read);
			}
			else
			{
				e->Data->Seek(0, SeekOrigin::Begin);
			}

			// invoke
			it->Invoke(nullptr, e);

			// open a posted panel
			if (Panel0::PostedPanel)
			{
				HANDLE h = Panel0::AddPluginPanel(Panel0::PostedPanel);
				return h;
			}
		}

		return INVALID_HANDLE_VALUE;
	}
	finally
	{
		Panel0::EndOpenMode();
	}
}

HANDLE Far0::AsOpenPlugin(int from, INT_PTR item)
{
	Panel0::BeginOpenMode();
	ValueUserScreen userscreen;

	// call a plugin; it may create a panel waiting for opening
	try
	{
		switch(from)
		{
		case OPEN_COMMANDLINE:
			{
				LOG_AUTO(3, "OPEN_COMMANDLINE");

				ProcessPrefixes(item);
			}
			break;
		case OPEN_DISKMENU:
			{
				LOG_AUTO(3, "OPEN_DISKMENU");

				ProxyTool^ tool = _toolDisk[(int)item];
				ModuleToolEventArgs e;
				e.From = ModuleToolOptions::Disk;
				tool->Invoke(nullptr, %e);
			}
			break;
		case OPEN_PLUGINSMENU:
			{
				if (item == 0)
				{
					OpenMenu(ModuleToolOptions::Panels);
					break;
				}

				LOG_AUTO(3, "OPEN_PLUGINSMENU");

				ProxyTool^ tool = _toolPanels[(int)item - 1];
				ModuleToolEventArgs e;
				e.From = ModuleToolOptions::Panels;
				tool->Invoke(nullptr, %e);
			}
			break;
		case OPEN_EDITOR:
			{
				if (item == 0)
				{
					OpenMenu(ModuleToolOptions::Editor);
					break;
				}

				LOG_AUTO(3, "OPEN_EDITOR");

				ProxyTool^ tool = _toolEditor[(int)item - 1];
				ModuleToolEventArgs e;
				e.From = ModuleToolOptions::Editor;
				tool->Invoke(nullptr, %e);
			}
			break;
		case OPEN_VIEWER:
			{
				if (item == 0)
				{
					OpenMenu(ModuleToolOptions::Viewer);
					break;
				}

				LOG_AUTO(3, "OPEN_VIEWER");

				ProxyTool^ tool = _toolViewer[(int)item - 1];
				ModuleToolEventArgs e;
				e.From = ModuleToolOptions::Viewer;
				tool->Invoke(nullptr, %e);
			}
			break;
		//! STOP: dialog case is different
		case OPEN_DIALOG:
			{
				const OpenDlgPluginData* dd = (const OpenDlgPluginData*)item;

				// just to be sure, see also _091127_112807
				FarDialog::_hDlgTop = dd->hDlg;

				int index = dd->ItemNumber;
				if (index == 0)
				{
					OpenMenu(ModuleToolOptions::Dialog);
					break;
				}

				LOG_AUTO(3, "OPEN_DIALOG");

				ProxyTool^ tool = _toolDialog[index - 1];
				ModuleToolEventArgs e;
				e.From = ModuleToolOptions::Dialog;
				tool->Invoke(nullptr, %e);
			}
			break;
		}

		// open a posted panel
		if (Panel0::PostedPanel)
		{
			HANDLE h = Panel0::AddPluginPanel(Panel0::PostedPanel);
			return h;
		}

		// don't open a panel
		return INVALID_HANDLE_VALUE;
	}
	finally
	{
		Panel0::EndOpenMode();
	}
}

void Far0::AssertHotkeys()
{
	if (!_hotkeys)
	{
		if (ES(_hotkey))
			throw gcnew OperationCanceledException(Res::ErrorNoHotKey);

		array<int>^ keys = gcnew array<int>(2);
		keys[1] = Far::Net->NameToKey(_hotkey);
		keys[0] = Far::Net->NameToKey("F11");
		_hotkeys = keys;
	}
}

void Far0::PostStep(EventHandler^ handler)
{
	// ensure keys
	AssertHotkeys();

	// post handler and keys
	_handler = handler;
	Far::Net->PostKeySequence(_hotkeys);
}

void Far0::PostStepAfterKeys(String^ keys, EventHandler^ handler)
{
	// ensure keys
	AssertHotkeys();

	// post the handler, keys and hotkeys
	_handler = handler;
	Far::Net->PostKeys(keys);
	Far::Net->PostKeySequence(_hotkeys);
}

void Far0::PostStepAfterStep(EventHandler^ handler1, EventHandler^ handler2)
{
	// ensure keys
	AssertHotkeys();

	// post the second handler, keys and invoke the first handler
	_handler = handler2;
	Far::Net->PostKeySequence(_hotkeys);
	try
	{
		handler1->Invoke(nullptr, nullptr);
	}
	catch(...)
	{
		//! 'F11 <hotkey>' is already posted and will trigger the menu; so, let's use a fake step
		_handler = gcnew EventHandler(&VoidStep);
		throw;
	}
}

void Far0::OpenMenu(ModuleToolOptions from)
{
	// process and drop a posted step handler
	if (_handler)
	{
		EventHandler^ handler = _handler;
		_handler = nullptr;
		handler(nullptr, nullptr);
		return;
	}

	// show the panels menu or the message
	if (from == ModuleToolOptions::Panels)
		ShowPanelMenu(true);
	else
		Far::Net->Message("This menu is empty but it is used internally.", "FarNet");
}

void Far0::OpenConfig()
{
	IMenu^ menu = Far::Net->CreateMenu();
	menu->AutoAssignHotkeys = true;
	menu->HelpTopic = "MenuConfig";
	menu->Title = "Modules configuration";

	List<IModuleTool^> tools = ModuleLoader::GetTools();

	String^ format = "{0,-10} : {1,2}";
	menu->Add(String::Format(format, Res::ModuleMenuTools, tools.Count));
	menu->Add(String::Format(format, Res::ModuleCommands, _registeredCommand.Count));
	menu->Add(String::Format(format, Res::ModuleEditors, _registeredEditor.Count));
	menu->Add(String::Format(format, Res::ModuleFilers, _registeredFiler.Count));
	menu->Add("Settings")->IsSeparator = true;
	menu->Add("UI culture");

	while(menu->Show())
	{
		switch(menu->Selected)
		{
		case 0:
			if (tools.Count)
				Works::ConfigTool::Show(%tools, Far0::_helpTopic + "ConfigTool", Res::MenuPrefix, gcnew ModuleToolComparer);
			break;
		case 1:
			if (_registeredCommand.Count)
				Works::ConfigCommand::Show(%_registeredCommand, Far0::_helpTopic + "ConfigCommand");
			break;
		case 2:
			if (_registeredEditor.Count)
				Works::ConfigEditor::Show(%_registeredEditor, Far0::_helpTopic + "ConfigEditor");
			break;
		case 3:
			if (_registeredFiler.Count)
				Works::ConfigFiler::Show(%_registeredFiler, Far0::_helpTopic + "ConfigFiler");
			break;
		case 5: // mind separator
			Works::ConfigUICulture::Show(ModuleLoader::GetModuleManagers(), Far0::_helpTopic + "ConfigUICulture");
			break;
		}
	}
}

bool Far0::CompareName(String^ mask, const wchar_t* name, bool skipPath)
{
	for each(String^ s in mask->Split(gcnew array<Char>{',', ';'}, StringSplitOptions::RemoveEmptyEntries))
	{
		PIN_NE(pin, s);
		if (Info.CmpName(pin, name, skipPath))
			return true;
	}
	return false;
}

bool Far0::CompareNameEx(String^ mask, const wchar_t* name, bool skipPath)
{
	int i = mask->IndexOf('|');
	if (i < 0)
		return CompareName(mask, name, skipPath);
	return  CompareName(mask->Substring(0, i), name, skipPath) && !CompareName(mask->Substring(i + 1), name, skipPath);
}

void Far0::InvokeModuleEditors(IEditor^ editor, const wchar_t* fileName)
{
	if (_registeredEditor.Count == 0)
		return;

	AutoEditorInfo ei;

	for each(ProxyEditor^ it in _registeredEditor)
	{
		// mask?
		if (SS(it->Mask) && !CompareNameEx(it->Mask, fileName, true))
			continue;

		//! tradeoff: catch all to call the others, too
		try
		{
			it->Invoke(editor, nullptr);
		}
		catch(Exception^ e)
		{
			//! show the address, too
			Far::Net->ShowError(it->Key, e);
		}
	}
}

void Far0::AsProcessSynchroEvent(int type, void* /*param*/)
{
	if (type != SE_COMMONSYNCHRO)
		return;

	WaitForSingleObject(_hMutex, INFINITE);
	try
	{
		//! handlers can be added during calls, don't use 'for each'
		while(_syncHandlers.Count)
		{
			EventHandler^ handler = _syncHandlers[0];
			_syncHandlers.RemoveAt(0);

			LOG_AUTO(3, String::Format("AsProcessSynchroEvent: {0}", Log::Format(handler->Method)));

			handler(nullptr, nullptr);
		}
	}
	finally
	{
		_syncHandlers.Clear();

		ReleaseMutex(_hMutex);
	}
}

void Far0::PostJob(EventHandler^ handler)
{
	if (!handler)
		throw gcnew ArgumentNullException("handler");

	WaitForSingleObject(_hMutex, INFINITE);
	try
	{
		if (_syncHandlers.IndexOf(handler) >= 0)
		{
			LOG_3(String::Format("PostJob: skip already posted {0}", Log::Format(handler->Method)));
			return;
		}

		LOG_3(String::Format("PostJob: call ACTL_SYNCHRO and post {0}", Log::Format(handler->Method)));

		_syncHandlers.Add(handler);
		if (_syncHandlers.Count == 1)
			Info.AdvControl(Info.ModuleNumber, ACTL_SYNCHRO, 0);
	}
	finally
	{
		ReleaseMutex(_hMutex);
	}
}

#undef GetEnvironmentVariable
CultureInfo^ Far0::GetCurrentUICulture(bool update)
{
	// get cached value
	if (_currentUICulture && !update)
		return _currentUICulture;

	// FARLANG
	String^ lang = Environment::GetEnvironmentVariable("FARLANG");

	// a few known cases
	if (lang == "English")
		return _currentUICulture = CultureInfo::GetCultureInfo("en");
	if (lang == "Russian")
		return _currentUICulture = CultureInfo::GetCultureInfo("ru");
	if (lang == "Czech")
		return _currentUICulture = CultureInfo::GetCultureInfo("cs");
	if (lang == "German")
		return _currentUICulture = CultureInfo::GetCultureInfo("de");
	if (lang == "Hungarian")
		return _currentUICulture = CultureInfo::GetCultureInfo("hu");
	if (lang == "Polish")
		return _currentUICulture = CultureInfo::GetCultureInfo("pl");

	// find by name
	for each(CultureInfo^ ci in CultureInfo::GetCultures(CultureTypes::NeutralCultures))
	{
		if (ci->EnglishName == lang)
			return _currentUICulture = ci;
	}

	// fallback
	return _currentUICulture = CultureInfo::InvariantCulture;
}

void Far0::ShowPanelMenu(bool showPushCommand)
{
	String^ sPushShelveThePanel = "Push/Shelve the panel";
	String^ sSwitchFullScreen = "Switch full screen";
	String^ sClose = "Close the panel";

	IMenu^ menu = Far::Net->CreateMenu();
	menu->AutoAssignHotkeys = true;
	menu->HelpTopic = "MenuPanels";
	menu->ShowAmpersands = true;
	menu->Title = ".NET panel tools";
	menu->BreakKeys->Add(VKeyCode::Delete);

	FarItem^ mi;
	for(;; menu->Items->Clear())
	{
		// Push/Shelve
		if (showPushCommand)
		{
			IAnyPanel^ panel = Far::Net->Panel;
			if (panel->IsPlugin)
			{
				IPanel^ plugin = dynamic_cast<IPanel^>(panel);
				if (plugin)
				{
					mi = menu->Add(sPushShelveThePanel);
					mi->Data = plugin;

					mi = menu->Add(sSwitchFullScreen);
					mi->Data = plugin;
				}
				else
				{
					showPushCommand = false;
				}

				mi = menu->Add(sClose);
				mi->Data = panel;
			}
			else if (panel->Kind == PanelKind::File)
			{
				FarItem^ mi = menu->Add(sPushShelveThePanel);
				mi->Data = panel;
			}
		}

		// Pop/Unshelve
		if (ShelveInfo::_stack.Count)
		{
			menu->Add("Pop/Unshelve")->IsSeparator = true;

			for each(ShelveInfo^ si in ShelveInfo::_stack)
			{
				FarItem^ mi = menu->Add(si->Title);
				mi->Data = si;
			}
		}

		// go
		if (!menu->Show())
			return;

		FarItem^ item = menu->Items[menu->Selected];
		Object^ data = item->Data;

		// [Delete]:
		if (menu->BreakKey == VKeyCode::Delete)
		{
			// case: remove shelved file panel;
			// do not remove plugin panels because of their shutdown bypassed
			ShelveInfoPanel^ shelve = dynamic_cast<ShelveInfoPanel^>(data);
			if (shelve)
				ShelveInfo::_stack.Remove(shelve);

			continue;
		}

		// Push/Shelve
		if ((Object^)item->Text == (Object^)sPushShelveThePanel)
		{
			((IAnyPanel^)data)->Push();
			return;
		}

		// Full screen:
		if ((Object^)item->Text == (Object^)sSwitchFullScreen)
		{
			FarNet::Panel2^ pp = (FarNet::Panel2^)data;
			pp->SwitchFullScreen();
			return;
		}

		// Close panel:
		if ((Object^)item->Text == (Object^)sClose)
		{
			Panel1^ panel = (Panel1^)data;

			//?? native plugin panel: go to the first item to work around "Far does not restore panel state",
			// this does not restore either but is still better than unexpected current item after exit.
			if (nullptr == dynamic_cast<FarNet::Panel2^>(panel))
				panel->Redraw(0, 0);

			((Panel1^)data)->Close();
			return;
		}

		// Pop/Unshelve
		ShelveInfo^ shelve = (ShelveInfo^)data;
		shelve->Unshelve();

		return;
	}
}

void Far0::InvalidateProxyCommand()
{
	delete _prefixes;
	_prefixes = 0;
}

}