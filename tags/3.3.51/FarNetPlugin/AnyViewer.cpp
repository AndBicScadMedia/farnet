/*
FAR.NET plugin for Far Manager
Copyright (c) 2005-2008 FAR.NET Team
*/

#include "StdAfx.h"
#include "AnyViewer.h"
#include "Far.h"
#include "Viewer.h"

namespace FarNet
{;
void AnyViewer::ViewText(String^ text, String^ title, OpenMode mode)
{
	String^ tmpfile = Far::Instance->TempName();
	File::WriteAllText(tmpfile, text, Encoding::Unicode);

	Viewer viewer;
	viewer.DeleteSource = FarManager::DeleteSource::UnusedFile;
	viewer.DisableHistory = true;
	viewer.FileName = tmpfile;
	viewer.Title = title;
	viewer.Open(mode);
}
}