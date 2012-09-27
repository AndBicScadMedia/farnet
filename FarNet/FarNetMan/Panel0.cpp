
/*
FarNet plugin for Far Manager
Copyright (c) 2005-2012 FarNet Team
*/

#include "StdAfx.h"
#include "Panel0.h"
#include "Panel2.h"
#include "Shelve.h"
#include "Wrappers.h"

namespace FarNet
{;
static List<FarFile^>^ ItemsToFiles(bool pureFiles, IList<FarFile^>^ files, IList<String^>^ names, PluginPanelItem* panelItem, int itemsNumber)
{
	List<FarFile^>^ r = gcnew List<FarFile^>(itemsNumber);

	//? Far bug: alone dots has UserData = 0 no matter what was written there; so check the dots name
	if (itemsNumber == 1 && panelItem[0].UserData.Data == 0 && wcscmp(panelItem[0].FileName, L"..") == 0)
		return r;

	// pure case
	if (pureFiles)
	{
		for(int i = 0; i < itemsNumber; ++i)
		{
			r->Add(Panel1::ItemToFile(panelItem[i]));
			if (names)
				names->Add(gcnew String(panelItem[i].AlternateFileName));
		}
		return r;
	}

	// data case
	for(int i = 0; i < itemsNumber; ++i)
	{
		int fi = (int)(INT_PTR)panelItem[i].UserData.Data;
		if (fi >= 0)
		{
			r->Add(files[fi]);
			if (names)
				names->Add(gcnew String(panelItem[i].AlternateFileName));
		}
	}

	return r;
}

// This is the very first called method
//! 090712. Allocation by chunks was originally used. But it turns out it does not improve
//! performance much (tested for 200000+ files). On the other hand allocation of large chunks
//! may fail due to memory fragmentation more frequently.
int Panel0::AsGetFindData(GetFindDataInfo* info)
{
	info->StructSize = sizeof(*info);

	Panel2^ pp = _panels[(int)info->hPanel];
	ExplorerModes mode = (ExplorerModes)info->OpMode;
	const bool canExploreLocation = pp->Host->Explorer->CanExploreLocation;

	Log::Source->TraceInformation("GetFindDataW Mode='{0}' Location='{1}'", mode, pp->CurrentLocation);

	try
	{
		// fake empty panel needed on switching modes, for example
		if (pp->_voidUpdateFiles)
		{
			Log::Source->TraceInformation("GetFindDataW fake empty panel");
			info->ItemsNumber = 0;
			info->PanelItem = 0;
			return 1;
		}

		// the Find mode
		const bool isFind = 0 != (info->OpMode & OPM_FIND);
		if (isFind && !canExploreLocation)
			return 0;

		// get the files
		if (!pp->_skipUpdateFiles)
		{
			GetFilesEventArgs args(mode, pp->Host->PageOffset, pp->Host->PageLimit, pp->Host->NeedsNewFiles);
			pp->Files = pp->Host->UIGetFiles(%args);
			if (args.Result != JobResult::Done)
				return 0;

			pp->Host->NeedsNewFiles = false;
		}

		// all item number
		int nItem = pp->Files->Count;
		if (pp->HasDots)
			++nItem;
		info->ItemsNumber = nItem;
		if (nItem == 0)
		{
			info->PanelItem = 0;
			return true;
		}

		// alloc all
		info->PanelItem = new PluginPanelItem[nItem];
		memset(info->PanelItem, 0, nItem * sizeof(PluginPanelItem));
		Log::Source->TraceInformation("GetFindDataW Address='{0:x}'", (long)info->PanelItem);

		// add dots
		int itemIndex = -1, fileIndex = -1;
		if (pp->HasDots)
		{
			++itemIndex;
			wchar_t* dots = new wchar_t[3];
			dots[0] = dots[1] = '.'; dots[2] = '\0';
			PluginPanelItem& p = info->PanelItem[0];
			p.UserData.Data = (void*)(-1);
			p.FileName = dots;
			p.Description = NewChars(pp->Host->DotsDescription);
		}

		// add files
		for each(FarFile^ file in pp->Files)
		{
			++itemIndex;
			++fileIndex;

			PluginPanelItem& p = info->PanelItem[itemIndex];

			// names
			p.FileName = NewChars(file->Name);
			p.Description = NewChars(file->Description);
			p.Owner = NewChars(file->Owner);

			// alternate names are for QView to work with any names,
			// even ExploreLocation explorers may have problem names
			if (info->OpMode == 0)
			{
				wchar_t buf[12]; // 12: 10=len(0xffffffff=4294967295) + 1=sign + 1=\0
				Info.FSF->itoa(fileIndex, buf, 10);
				int size = (int)wcslen(buf) + 1;
				wchar_t* alternate = new wchar_t[size];
				memcpy(alternate, buf, size * sizeof(wchar_t));
				p.AlternateFileName = alternate;
			}
			else
			{
				p.AlternateFileName = 0;
			}

			// other
			p.UserData.Data = (void*)(canExploreLocation ? -1 : fileIndex);
			p.FileAttributes = (DWORD)file->Attributes;
			p.FileSize = file->Length;
			p.CreationTime = DateTimeToFileTime(file->CreationTime);
			p.LastWriteTime = DateTimeToFileTime(file->LastWriteTime);
			p.LastAccessTime = DateTimeToFileTime(file->LastAccessTime);

			// columns
			System::Collections::ICollection^ columns = file->Columns;
			if (columns)
			{
				int nb = columns->Count;
				if (nb)
				{
					wchar_t** custom = new wchar_t*[nb];
					p.CustomColumnNumber = nb;
					p.CustomColumnData = custom;
					int iColumn = 0;
					for each(Object^ it in columns)
					{
						if (it)
							custom[iColumn] = NewChars(it->ToString());
						else
							custom[iColumn] = 0;
						++iColumn;
					}
				}
			}
		}

		// drop pure files
		if (canExploreLocation)
			pp->Files = nullptr;

		return true;
	}
	catch(Exception^ e)
	{
		if ((info->OpMode & (OPM_FIND | OPM_SILENT)) == 0)
			Far::Net->ShowError("Getting panel files", e);
		else
			Log::TraceException(e);

		return false;
	}
}

void Panel0::AsFreeFindData(const FreeFindDataInfo* info)
{
	Panel2^ pp = _panels[(int)info->hPanel]; //???? need? can it be static managed by Address -> Data map including Panel2^
	Log::Source->TraceInformation("FreeFindDataW Address='{0:x}' Location='{1}'", (long)info->PanelItem, pp->CurrentLocation);

	for(int i = (int)info->ItemsNumber; --i >= 0;)
	{
		PluginPanelItem& item = info->PanelItem[i];

		delete[] item.Owner;
		delete[] item.Description;
		delete[] item.AlternateFileName;
		delete[] item.FileName;

		if (item.CustomColumnData)
		{
			for(int j = (int)item.CustomColumnNumber; --j >= 0;)
				delete[] item.CustomColumnData[j];

			delete[] item.CustomColumnData;
		}
	}

	delete[] info->PanelItem;
}

int Panel0::AsSetDirectory(const SetDirectoryInfo* info)
{
	Panel2^ pp = _panels[(int)info->hPanel];
	ExplorerModes mode = (ExplorerModes)info->OpMode;
	String^ directory = gcnew String(info->Dir);

	Log::Source->TraceInformation("SetDirectoryW Mode='{0}' Name='{1}'", mode, directory);

	const bool canExploreLocation = pp->Host->Explorer->CanExploreLocation;

	//! Silent but not Find is possible on CtrlQ scan
	if (!canExploreLocation && 0 != (info->OpMode & (OPM_FIND | OPM_SILENT)))
		return 0;

	_inAsSetDirectory = true;
	try
	{
		Explorer^ explorer2;
		ExploreEventArgs^ args2;
		if (directory == "\\")
		{
			ExploreRootEventArgs^ args = gcnew ExploreRootEventArgs(mode);
			explorer2 = pp->Host->UIExploreRoot(args);
			if (!explorer2)
			{
				Panel^ mp = pp->Host;
				if (!mp->Parent)
					return 0;

				while(mp->Parent)
				{
					Panel^ parent = mp->Parent;
					mp->CloseChild();
					mp = parent;
				}

				return 1;
			}
			args2 = args;
		}
		else if (directory == "..")
		{
			ExploreParentEventArgs^ args = gcnew ExploreParentEventArgs(mode);
			explorer2 = pp->Host->UIExploreParent(args);
			if (!explorer2)
			{
				if (!pp->Host->Parent)
					return 0;

				pp->Host->CloseChild();
				return 1;
			}
			args2 = args;
		}
		else if (canExploreLocation)
		{
			ExploreLocationEventArgs^ args = gcnew ExploreLocationEventArgs(mode, directory);
			explorer2 = pp->Host->UIExploreLocation(args);
			args2 = args;
		}
		else
		{
			ExploreDirectoryEventArgs^ args = gcnew ExploreDirectoryEventArgs(mode, pp->Host->CurrentFile);
			explorer2 = pp->Host->UIExploreDirectory(args);
			args2 = args;
		}

		if (!explorer2)
			return 0;

		// open
		OpenExplorer(pp, explorer2, args2);
		return 1;
	}
	finally
	{
		_inAsSetDirectory = false;
	}
}

/*
?? NYI: Parameter destPath can be changed, i.e. (*destPath) replaced.
?? NYI: Not used return value -1 (stopped by a user).

It is called on F5/F6 when a target panel is a native plugin or an explorer
cannot export files but can get content and a target is a native file panel
(FarNet targets accept files themselves).
*/
int Panel0::AsGetFiles(GetFilesInfo* info)
{
	info->StructSize = sizeof(*info);

	Panel2^ pp = _panels[(int)info->hPanel];
	ExplorerModes mode = (ExplorerModes)info->OpMode;

	Log::Source->TraceInformation("GetFilesW Mode='{0}'", mode);

	Explorer^ explorer = pp->Host->Explorer;
	if (!explorer->CanGetContent)
		return 0;

	// modes
	const bool canExploreLocation = explorer->CanExploreLocation;
	const bool qview = 0 != int(mode & ExplorerModes::QuickView);
	const bool silent = int(mode & ExplorerModes::Silent);

	// process bad names? - do not if silent or the target is plugin
	const bool processBadNames = qview || (!silent && !Far::Net->Panel2->IsPlugin);

	// delete files on move?
	const bool deleteFiles = info->Move && explorer->CanDeleteFiles;
	List<FarFile^>^ filesToDelete;
	if (deleteFiles)
		filesToDelete = gcnew List<FarFile^>;

	// collect files
	List<String^>^ names = qview ? gcnew List<String^> : nullptr;
	List<FarFile^>^ files = ItemsToFiles(canExploreLocation, pp->Files, names, info->PanelItem, (int)info->ItemsNumber);

	// copy files
	String^ destination = gcnew String(info->DestPath);
	for(int i = 0; i < files->Count; ++i)
	{
		String^ fileName;
		if (qview)
		{
			// use the alias
			fileName = names[i];
		}
		else
		{
			// use the name, check/fix invalid
			fileName = files[i]->Name;
			if (Works::Kit::IsInvalidFileName(fileName))
			{
				if (!processBadNames)
					continue;

				fileName = Works::Kit::FixInvalidFileName(fileName);
				if (!fileName)
					continue;
			}
		}

		// export file
		fileName = Path::Combine(destination, fileName);
		GetContentEventArgs^ argsJob = Panel::WorksExportExplorerFile(explorer, pp->Host, mode, files[i], fileName);
		if (!argsJob || argsJob->Result != JobResult::Done)
			continue;

		// copy existing file
		if (SS(argsJob->UseFileName))
			File::Copy(argsJob->UseFileName, fileName, true);

		// collect to delete
		if (deleteFiles)
			filesToDelete->Add(files[i]);
	}

	// delete collected files
	if (deleteFiles && filesToDelete->Count > 0)
	{
		DeleteFilesEventArgs args(mode, filesToDelete, false);
		pp->Host->UIDeleteFiles(%args);
	}

	return 1;
}

//! It is called on [F5], [F6] only from a native/plugin panel to a module panel.
int Panel0::AsPutFiles(PutFilesInfo* info)
{
	info->StructSize = sizeof(*info);

	Panel2^ pp = _panels[(int)info->hPanel];
	ExplorerModes mode = (ExplorerModes)info->OpMode;

	Log::Source->TraceInformation("PutFilesW Mode='{0}'", mode);

	if (!pp->Host->Explorer->CanImportFiles)
		return 0;

	List<FarFile^>^ files = gcnew List<FarFile^>((int)info->ItemsNumber);
	for(int i = 0; i < (int)info->ItemsNumber; ++i)
		files->Add(Panel1::ItemToFile(info->PanelItem[i]));

	ImportFilesEventArgs args(
		mode,
		files,
		info->Move != 0,
		info->SrcPath ? gcnew String(info->SrcPath) : String::Empty);

	pp->Host->UIImportFiles(%args);

	// done:
	if (args.Result == JobResult::Done)
		return 1;

	// failed:
	if (args.Result != JobResult::Incomplete || args.FilesToStay->Count == 0)
		return 0;

	// incomplete:

	// drop selection flags
	for(int i = (int)info->ItemsNumber; --i >= 0;)
		info->PanelItem[i].Flags &= ~PPIF_SELECTED;

	// restore selection flags
	for each(FarFile^ file in args.FilesToStay)
	{
		int index = args.Files->IndexOf(file);
		if (index >= 0 && index < (int)info->ItemsNumber)
			info->PanelItem[index].Flags |= PPIF_SELECTED;
	}

	return -1;
}

//! It is called on move, too? I.e. is Move = Copy + Delete?
int Panel0::AsDeleteFiles(const DeleteFilesInfo* info)
{
	Panel2^ pp = _panels[(int)info->hPanel];
	ExplorerModes mode = (ExplorerModes)info->OpMode;

	Log::Source->TraceInformation("DeleteFilesW Mode='{0}'", mode);

	Explorer^ explorer = pp->Host->Explorer;
	if (!explorer->CanDeleteFiles)
		return 0;

	DeleteFilesEventArgs args(mode, ItemsToFiles(pp->Host->Explorer->CanExploreLocation, pp->Files, nullptr, info->PanelItem, (int)info->ItemsNumber), false);
	pp->Host->UIDeleteFiles(%args);

	return args.Result == JobResult::Ignore ? 0 : 1;
}

void Panel0::AsClosePanel(const ClosePanelInfo* info)
{
	Log::Source->TraceInformation("ClosePluginW");

	// disconnect
	Panel2^ pp = _panels[(int)info->hPanel];
	pp->Free();
	_panels[(int)info->hPanel] = nullptr;

	// done for pushed
	if (pp->_Pushed)
		return;

	// clean the whole panel stack
	for(Panel^ panel = pp->Host; panel; panel = panel->Parent)
	{
		try
		{
			panel->UIClosed();
		}
		catch(Exception^ ex)
		{
			Far::Net->ShowError("UIClosed", ex);
		}
	}
}

//! It is called too often, log Verbose
void Panel0::AsGetOpenPanelInfo(OpenPanelInfo* info)
{
	Log::Source->TraceEvent(TraceEventType::Verbose, 0, "GetOpenPluginInfoW");
	info->StructSize = sizeof(*info);

	// plugin panel
	Panel2^ pp = _panels[(int)info->hPanel];

	//! pushed case
	//?? _091015_190130 Far calls this after Close(), perhaps a bug. How: push folder tree panel.
	if (pp->IsPushed)
		return;

	// trigger - to update info before making it for Far
	if (!State::GetPanelInfo)
	{
		Log::Source->TraceEvent(TraceEventType::Verbose, 0, "UpdateInfo");
		pp->Host->UIUpdateInfo();
	}

	// make info
	*info = pp->Make();
}

static bool _reenterOnRedrawing;
int Panel0::AsProcessPanelEvent(const ProcessPanelEventInfo* info)
{
	Panel2^ pp = _panels[(int)info->hPanel];
	switch(info->Event)
	{
	case FE_BREAK:
		{
			Log::Source->TraceInformation("FE_BREAK");

			pp->Host->UICtrlBreak();
		}
		break;
	case FE_CLOSE:
		{
			Log::Source->TraceInformation("FE_CLOSE");

			//_090321_165608 FE_CLOSE issues
			if (!pp->_Pushed)
			{
				PanelEventArgs e;
				pp->Host->UIClosing(%e);
				return e.Ignore;
			}
		}
		break;
	case FE_COMMAND:
		{
			Log::Source->TraceInformation("FE_COMMAND");

			if (pp->Host->WorksInvokingCommand(nullptr))
			{
				CommandLineEventArgs e(gcnew String((const wchar_t*)info->Param));
				Log::Source->TraceInformation("InvokingCommand: {0}", e.Command);

				//! Try\catch in order to return exactly what the module returns.
				try
				{
					// call
					pp->Host->WorksInvokingCommand(%e);

					// clear the command line
					if (e.Ignore)
						Far::Net->CommandLine->Text = String::Empty;
				}
				catch(Exception^ exception)
				{
					Far::Net->ShowError("Event: Executing", exception);
				}

				return e.Ignore ? 1 : 0;
			}
		}
		break;
	case FE_CHANGEVIEWMODE:
		{
			Log::Source->TraceInformation("FE_CHANGEVIEWMODE");

			ViewChangedEventArgs e(gcnew String((const wchar_t*)info->Param));
			pp->Host->UIViewChanged(%e);
		}
		break;
	case FE_IDLE:
		{
			Log::Source->TraceEvent(TraceEventType::Verbose, 0, "FE_IDLE");

			// 1) call
			pp->Host->UIIdle();

			// 2) update after the handler: if the panel has set both IdleUpdate and Idled
			// then in Idled it should not care of data updates, it is done after that.
			if (pp->Host->IdleUpdate)
			{
				pp->Update(true);
				pp->Redraw();
			}
		}
		break;
	case FE_GOTFOCUS:
		{
			Log::Source->TraceEvent(TraceEventType::Verbose, 0, "FE_GOTFOCUS");

			pp->Host->UIGotFocus();
		}
		break;
	case FE_KILLFOCUS:
		{
			Log::Source->TraceEvent(TraceEventType::Verbose, 0, "FE_KILLFOCUS");

			pp->Host->UILosingFocus();
		}
		break;
	case FE_REDRAW:
		{
			Log::Source->TraceEvent(TraceEventType::Verbose, 0, "FE_REDRAW");

			// 090411 Data are shown now. Drop this flag to allow normal processing.
			pp->_skipUpdateFiles = false;

			// 090811 Internal work is in progress, do nothing
			if (pp->_voidUpdateFiles)
				return 0;

			if (_reenterOnRedrawing)
			{
				_reenterOnRedrawing = false;
				return 0;
			}

			PanelEventArgs e;
			pp->Host->UIRedrawing(%e);
			if (e.Ignore)
				return 1;

			// post selection
			if (pp->_postSelected)
			{
				array<int>^ selected = pp->_postSelected;
				pp->_postSelected = nullptr;
				pp->SelectAt(selected);
			}

			// case 1: find posted data
			if (pp->_postData)
			{
				Object^ data = pp->_postData;

				pp->_postData = nullptr;
				pp->_postFile = nullptr;
				pp->_postName = nullptr;

				IList<FarFile^>^ files = pp->ShownList;
				for(int n = files->Count, i = pp->HasDots ? 1 : 0; i < n; ++i)
				{
					if (data == files[i]->Data)
					{
						_reenterOnRedrawing = true;
						pp->Redraw(i, -1);
						return 1;
					}
				}

				return 0;
			}

			// case 2: find posted name
			if (pp->_postName)
			{
				String^ name = pp->_postName;

				pp->_postFile = nullptr;
				pp->_postName = nullptr;

				IList<FarFile^>^ files = pp->ShownList;
				for(int n = files->Count, i = pp->HasDots ? 1 : 0; i < n; ++i)
				{
					if (name == files[i]->Name)
					{
						_reenterOnRedrawing = true;
						pp->Redraw(i, -1);
						return 1;
					}
				}

				return 0;
			}

			// else: find posted file
			if (!pp->_postFile)
				return 0;

			IEqualityComparer<FarFile^>^ comparer = pp->Host->Explorer->FileComparer;
			FarFile^ file = pp->_postFile;
			pp->_postFile = nullptr;

			IList<FarFile^>^ files = pp->ShownList;
			for(int n = files->Count, i = pp->HasDots ? 1 : 0; i < n; ++i)
			{
				if (comparer->Equals(file, files[i]))
				{
					_reenterOnRedrawing = true;
					pp->Redraw(i, -1);
					return 1;
				}
			}

			return 0;
		}
		break;
	}
	return 0;
}

int Panel0::AsProcessPanelInput(const ProcessPanelInputInfo* info)
{
	// ignore not key events
	const INPUT_RECORD& ir = info->Rec;
	if (ir.EventType != KEY_EVENT)
		return 0;

	//! mind rare case: panel is null: closed by [AltF12] + select folder
	Panel2^ pp = _panels[(int)info->hPanel];
	if (!pp)
		return 0;

	// args, log
	KeyEventArgs e(KeyInfoFromInputRecord(ir));
	Log::Source->TraceEvent(TraceEventType::Information, 0, "KeyPressed {0}", %e);

	// 1. event; handlers work first of all
	pp->Host->WorksKeyPressed(%e);
	if (e.Ignore)
		return 1;

	// 2. method; default or custom virtual methods
	if (pp->Host->UIKeyPressed(e.Key))
		return 1;

	// 3. escape; special not yet handled case
	if ((e.Key->Is(KeyCode::Escape) || e.Key->IsShift(KeyCode::Escape)) && Far::Net->CommandLine->Length == 0)
	{
		pp->Host->WorksEscaping(%e);
		if (e.Ignore)
			return 1;

		pp->Host->UIEscape(e.Key->IsShift());
		return 1;
	}

	// CtrlR
	if (e.Key->IsCtrl(KeyCode::R))
		pp->Host->NeedsNewFiles = true;

	return 0;
}

IPanel^ Panel0::GetPanel(bool active)
{
	// get info and return null (e.g. Far started with /e or /v)
	PanelInfo pi;
	if (!TryPanelInfo((active ? PANEL_ACTIVE : PANEL_PASSIVE), pi))
		return nullptr;

	if (0 == (pi.Flags & PFLAGS_PLUGIN))
		return gcnew Panel1(active);

	for (int i = 1; i < cPanels; ++i)
	{
		Panel2^ p = _panels[i];
		if (p && p->IsActive == active)
			return p->Host;
	}

	return gcnew Panel1(true);
}

array<Panel^>^ Panel0::PanelsByGuid(Guid typeId)
{
	List<FarNet::Panel^> list;
	for (int i = 1; i < cPanels; ++i)
	{
		Panel2^ core = _panels[i];
		if (core)
		{
			Panel^ panel = core->Host;
			if (panel->TypeId == typeId)
				list.Add(panel);
		}
	}
	return list.ToArray();
}

array<Panel^>^ Panel0::PanelsByType(Type^ type)
{
	List<FarNet::Panel^> list;
	for (int i = 1; i < cPanels; ++i)
	{
		Panel2^ core = _panels[i];
		if (core)
		{
			Panel^ panel = core->Host;
			Type^ panelType;
			if (!type || type == (panelType = panel->GetType()) || type->IsSubclassOf(panelType))
				list.Add(panel);
		}
	}
	return list.ToArray();
}

Panel2^ Panel0::GetPanel2(Panel2^ plugin)
{
	for (int i = 1; i < cPanels; ++i)
	{
		Panel2^ p = _panels[i];
		if (p && p != plugin)
			return p;
	}
	return nullptr;
}

// [0] is for a waiting panel;
// [1-3] are 2 for already opened and 1 for being added
//? create a test case: open 2 panels and try to open 1 more
HANDLE Panel0::AddPluginPanel(Panel2^ plugin)
{
	for(int i = 1; i < cPanels; ++i)
	{
		if (_panels[i] == nullptr)
		{
			_panels[i] = plugin;
			plugin->Index = i;
			return plugin->Handle;
		}
	}
	throw gcnew InvalidOperationException("Cannot add a panel.");
}

// EndOpenMode() must be called after
void Panel0::BeginOpenMode()
{
	if (_openMode < 0)
		throw gcnew InvalidOperationException("Negative open mode.");

	++_openMode;
}

// BeginOpenMode() must be called before
void Panel0::EndOpenMode()
{
	if (_openMode <= 0)
		throw gcnew InvalidOperationException("Not positive open mode.");

	if (--_openMode == 0)
		_panels[0] = nullptr;
}

void Panel0::OpenPanel(Panel2^ plugin)
{
	// plugin must be called for opening
	if (_openMode == 0)
		throw gcnew InvalidOperationException("Cannot open a panel because a module is not called for opening.");

	// only one panel can be opened at a time
	if (_panels[0] && _panels[0] != plugin)
		throw gcnew InvalidOperationException("Cannot open a panel because another panel is already waiting.");

	// panels window should be current
	try
	{
		Far::Net->Window->SetCurrentAt(0);
	}
	catch(InvalidOperationException^ e)
	{
		throw gcnew InvalidOperationException("Cannot open a panel because panels window cannot be set current.", e);
	}

	_panels[0] = plugin;
}

//! it call Update/Redraw in some cases
void Panel0::ReplacePanel(Panel2^ oldPanel, Panel2^ newPanel)
{
	// check
	if (!oldPanel) throw gcnew ArgumentNullException("oldPanel");
	if (!newPanel) throw gcnew ArgumentNullException("newPanel");

	int id1 = oldPanel->Index;
	if (id1 < 1)
		throw gcnew InvalidOperationException("Old panel must be opened.");

	if (newPanel->Index >= 1)
		throw gcnew InvalidOperationException("New panel must be not opened.");

	//_110210_081347 active info
	newPanel->_ActiveInfo = oldPanel->_ActiveInfo; //???? is it done twice?

	// save old modes
	oldPanel->StartSortMode = oldPanel->SortMode;
	oldPanel->StartViewMode = oldPanel->ViewMode;

	// disconnect old panel
	oldPanel->Handle = 0;
	oldPanel->Free();

	// connect new panel
	_panels[id1] = newPanel;
	newPanel->Index = id1;

	// change panel modes
	if (newPanel->StartViewMode != PanelViewMode::Undefined &&
		newPanel->StartViewMode != oldPanel->StartViewMode ||
		newPanel->StartSortMode != PanelSortMode::Default &&
		newPanel->StartSortMode != oldPanel->StartSortMode)
	{
		// set void mode for switching panel modes
		newPanel->_voidUpdateFiles = true;
		newPanel->Update(false);

		// set only new modes
		if (newPanel->StartViewMode != PanelViewMode::Undefined && newPanel->StartViewMode != oldPanel->StartViewMode)
			newPanel->ViewMode = newPanel->StartViewMode;
		if (newPanel->StartSortMode != PanelSortMode::Default)
		{
			if (newPanel->StartSortMode != oldPanel->StartSortMode)
				newPanel->SortMode = newPanel->StartSortMode;
		}

		// drop void mode
		newPanel->_voidUpdateFiles = false;
	}

	//! switch to new data and redraw, but not always: in some cases it will be done anyway, e.g. by Far
	if (!_inAsSetDirectory)
	{
		newPanel->Update(false);
		newPanel->Redraw(0, 0);
	}
}

void Panel0::PushPanel(Panel2^ plugin)
{
	if (plugin->_Pushed)
		throw gcnew InvalidOperationException("Cannot push the panel because it is already pushed.");

	//! save current state effectively by Far API, not FarNet
	PanelInfo pi;
	GetPanelInfo(plugin->Handle, pi);

	// save modes
	bool reversed = (pi.Flags & PFLAGS_REVERSESORTORDER) != 0;
	plugin->StartSortMode = (PanelSortMode)(reversed ? -pi.SortMode : pi.SortMode);
	plugin->StartViewMode = (PanelViewMode)pi.ViewMode;

	// current
	FarFile^ file = nullptr;
	if (pi.ItemsNumber > 0)
	{
		AutoPluginPanelItem item(plugin->Handle, (int)pi.CurrentItem, ShownFile);
		int index = (int)item.Get().UserData.Data;
		if (index >= 0 && index < plugin->Files->Count)
			file = plugin->Files[index];
	}

	// push
	plugin->_Pushed = gcnew ShelveInfoModule(plugin);
	Works::ShelveInfo::Stack->Add(plugin->_Pushed);

	// reset position, close
	// 090411 Was: Redraw(0, 0) + Close(). New way looks more effective and perhaps avoids some Far->FarNet calls.
	plugin->Close(".");
	if (file)
		plugin->PostFile(file);

	// drop handle
	plugin->Handle = 0;
}

void Panel0::ShelvePanel(Panel1^ panel, bool modes)
{
	Works::ShelveInfo::Stack->Add(gcnew ShelveInfoNative(panel, modes));
}

// Explorer enters to the panel
void Panel0::OpenExplorer(Panel2^ core, Explorer^ explorer, ExploreEventArgs^ args)
{
	Panel^ oldPanel = core->Host;

	// explorers must get new explorers
	if ((Object^)explorer == (Object^)oldPanel->Explorer)
		throw gcnew InvalidOperationException("The same explorer object is not expected.");

	// make the panel
	Panel^ newPanel = nullptr;

	// make or reuse
	if (args->NewPanel || explorer->TypeId != oldPanel->Explorer->TypeId)
	{
		// make a new panel
		newPanel = explorer->CreatePanel();
	}
	else
	{
		// reuse, update is called there
		core->ReplaceExplorer(explorer);
		newPanel = oldPanel;
	}

	// post
	if (args)
	{
		newPanel->PostData(args->PostData);
		newPanel->PostFile(args->PostFile);
		newPanel->PostName(args->PostName);
	}

	// location
	String^ location = explorer->Location;
	if (location->Length)
		newPanel->CurrentLocation = location;
	else
		newPanel->CurrentLocation = "*";

	// same panel? update, reuse
	if (newPanel == oldPanel)
		return;

	// open new as child
	newPanel->OpenChild(oldPanel);
}

}
