/*
FAR.NET plugin for Far Manager
Copyright (c) 2005-2009 FAR.NET Team
*/

#include "stdafx.h"
#include "Dialog.h"
#include "EditorHost.h"
#include "Far.h"
#include "Panel.h"
#include "ViewerHost.h"

PluginStartupInfo Info;
static FarStandardFunctions FSF;

namespace FarNet
{;
enum EMessage
{
	MTitle,
	MMessage1,
	MMessage2,
	MMessage3,
	MMessage4,
	MButton,
};

#define __START try {
#define __END } catch(Exception^ e) { Far::Instance->ShowError(nullptr, e); }

// SetStartupInfo is called once, after the plugin DLL is loaded.
void WINAPI SetStartupInfo(const PluginStartupInfo* psi)
{
#ifdef TEST1
	StartTest1();
#endif

	Info = *psi;
	FSF = *psi->FSF;
	Info.FSF = &FSF;

	__START;
	Far::StartFar();
	__END;
}

// Unloads sub-plugins and the main plugin
void WINAPI ExitFAR()
{
	// don't try/catch, FAR can't help
	Far::Instance->Stop();

#ifdef TEST1
	StopTest1();
#endif
}

// GetPluginInfo is called to get general plugin info.
// It passes in joined information about all plugins.
void WINAPI GetPluginInfo(PluginInfo* pi)
{
	__START;
	Far::Instance->AsGetPluginInfo(pi);
	__END;
}

// OpenPlugin is called to create a new plugin instance or do a job.
HANDLE WINAPI OpenPlugin(int from, INT_PTR item)
{
	__START;
	return Far::Instance->AsOpenPlugin(from, item);
	__END;
	return INVALID_HANDLE_VALUE;
}

int WINAPI Configure(int itemIndex)
{
	__START;
	return Far::Instance->AsConfigure(itemIndex);
	__END;
	return false;
}

void WINAPI ClosePlugin(HANDLE hPlugin)
{
	__START;
	PanelSet::AsClosePlugin(hPlugin);
	__END;
}

int WINAPI GetFiles(HANDLE hPlugin, PluginPanelItem* panelItem, int itemsNumber, int move, char* destPath, int opMode)
{
	__START;
	return PanelSet::AsGetFiles(hPlugin, panelItem, itemsNumber, move, destPath, opMode);
	__END;
	return 0;
}

int WINAPI PutFiles(HANDLE hPlugin, PluginPanelItem* panelItem, int itemsNumber, int move, int opMode)
{
	__START;
	return PanelSet::AsPutFiles(hPlugin, panelItem, itemsNumber, move, opMode);
	__END;
	return 0;
}

int WINAPI GetFindData(HANDLE hPlugin, PluginPanelItem** pPanelItem, int* pItemsNumber, int opMode)
{
	return PanelSet::AsGetFindData(hPlugin, pPanelItem, pItemsNumber, opMode);
}

void WINAPI FreeFindData(HANDLE /*hPlugin*/, PluginPanelItem* panelItem, int /*itemsNumber*/)
{
	__START;
	PanelSet::AsFreeFindData(panelItem);
	__END;
}

void WINAPI GetOpenPluginInfo(HANDLE hPlugin, OpenPluginInfo* info)
{
	__START;
	PanelSet::AsGetOpenPluginInfo(hPlugin, info);
	__END;
}

int WINAPI SetDirectory(HANDLE hPlugin, const char* dir, int opMode)
{
	__START;
	return PanelSet::AsSetDirectory(hPlugin, dir, opMode);
	__END;
	return false;
}

int WINAPI DeleteFiles(HANDLE hPlugin, PluginPanelItem* panelItem, int itemsNumber, int opMode)
{
	__START;
	return PanelSet::AsDeleteFiles(hPlugin, panelItem, itemsNumber, opMode);
	__END;
	return false;
}

int WINAPI MakeDirectory(HANDLE hPlugin, char* name, int opMode)
{
	__START;
	return PanelSet::AsMakeDirectory(hPlugin, name, opMode);
	__END;
	return 0;
}

HANDLE WINAPI OpenFilePlugin(char* name, const unsigned char* data, int dataSize)
{
	__START;
	return Far::Instance->AsOpenFilePlugin(name, data, dataSize);
	__END;
	return INVALID_HANDLE_VALUE;
}

int WINAPI ProcessDialogEvent(int id, void* param)
{
	__START;
	return FarDialog::AsProcessDialogEvent(id, param);
	__END;
	return true; // ignore, there was a problem
}

int WINAPI ProcessEditorEvent(int type, void* param)
{
	__START;
	return EditorHost::AsProcessEditorEvent(type, param);
	__END;
	return 0;
}

int WINAPI ProcessEditorInput(const INPUT_RECORD* rec)
{
	__START;
	return EditorHost::AsProcessEditorInput(rec);
	__END;
	return 0;
}

int WINAPI ProcessEvent(HANDLE hPlugin, int id, void* param)
{
	__START;
	return PanelSet::AsProcessEvent(hPlugin, id, param);
	__END;
	return false;
}

int WINAPI ProcessKey(HANDLE hPlugin, int key, unsigned int controlState)
{
	__START;
	return PanelSet::AsProcessKey(hPlugin, key, controlState);
	__END;
	return true; // ignore, there was a problem
}

int WINAPI ProcessViewerEvent(int type, void* param)
{
	__START;
	return ViewerHost::AsProcessViewerEvent(type, param);
	__END;
	return 0;
}

}