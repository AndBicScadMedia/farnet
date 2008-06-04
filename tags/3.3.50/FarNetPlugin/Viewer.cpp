/*
FAR.NET plugin for Far Manager
Copyright (c) 2005-2008 FAR.NET Team
*/

#include "StdAfx.h"
#include "Viewer.h"
#include "Far.h"
#include "Utils.h"
#include "ViewerHost.h"

namespace FarNet
{;
Viewer::Viewer()
: _id(-1)
, _Title(String::Empty)
{}

void Viewer::Open()
{
	Open(OpenMode::None);
}

void Viewer::Open(OpenMode mode)
{
	AssertClosed();

	// flags
	int flags = 0;
	if (_EnableSwitch)
		flags |= VF_ENABLE_F6;
	if (_DisableHistory)
		flags |= VF_DISABLEHISTORY;
	switch(mode)
	{
	case OpenMode::None:
		flags |= (VF_NONMODAL | VF_IMMEDIATERETURN); break;
	case OpenMode::Wait:
		flags |= VF_NONMODAL; break;
	}
	switch(_DeleteSource)
	{
	case FarManager::DeleteSource::UnusedFile:
		flags |= VF_DELETEONLYFILEONCLOSE; break;
	case FarManager::DeleteSource::UnusedFolder:
		flags |= VF_DELETEONCLOSE; break;
	}

	CBox sFileName(_FileName);
	CBox sTitle(_Title);

	// from dialog? set modal
	WindowType wt = Far::Instance->GetWindowType(-1);
	if (wt == WindowType::Dialog)
		flags &= ~VF_NONMODAL;

	// open: see editor
	_id = -1;
	ViewerHost::_viewerWaiting = this;
	Info.Viewer(sFileName, sTitle, _Window.Left, _Window.Top, _Window.Right, _Window.Bottom, flags); //?? test window values

	// redraw FAR
	if (wt == WindowType::Dialog)
		Far::Instance->Redraw();

	// errors: see editor
	if (_id == -1)
		throw gcnew OperationCanceledException("Cannot open the file '" + FileName + "'");
}

int Viewer::Id::get()
{
	return _id;
}

DeleteSource Viewer::DeleteSource::get()
{
	return _DeleteSource;
}

void Viewer::DeleteSource::set(FarManager::DeleteSource value)
{
	_DeleteSource = value;
}

bool Viewer::EnableSwitch::get()
{
	return _EnableSwitch;
}

void Viewer::EnableSwitch::set(bool value)
{
	AssertClosed();
	_EnableSwitch = value;
}

bool Viewer::DisableHistory::get()
{
	return _DisableHistory;
}

void Viewer::DisableHistory::set(bool value)
{
	AssertClosed();
	_DisableHistory = value;
}

bool Viewer::IsOpened::get()
{
	return _id >= 0;
}

String^ Viewer::FileName::get()
{
	return _FileName;
}

void Viewer::FileName::set(String^ value)
{
	AssertClosed();
	_FileName = value;
}

String^ Viewer::Title::get()
{
	return _Title;
}

Int64 Viewer::FileSize::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID >= 0 && vi.ViewerID == _id)
		return vi.FileSize.i64;
	else
		return -1;
}

Point Viewer::WindowSize::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	Point r;
	if (vi.ViewerID >= 0 && vi.ViewerID == _id)
	{
		r.X = vi.WindowSizeX;
		r.Y = vi.WindowSizeY;
	}
	return r;
}

void Viewer::Title::set(String^ value)
{
	AssertClosed();
	_Title = value;
}

Place Viewer::Window::get()
{
	return _Window;
}

void Viewer::Window::set(Place value)
{
	AssertClosed();
	_Window = value;
}

ViewFrame Viewer::Frame::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	ViewFrame r;
	if (vi.ViewerID >= 0 && vi.ViewerID == _id)
	{
		r.Pos = vi.FilePos.i64;
		r.LeftPos = vi.LeftPos;
	}
	return r;
}

void Viewer::Frame::set(ViewFrame value)
{
	AssertCurrentViewer();
	ViewerSetPosition vsp;
	vsp.Flags = VSP_NORETNEWPOS;
	vsp.LeftPos = value.LeftPos;
	vsp.StartPos.i64 = value.Pos;
	Info.ViewerControl(VCTL_SETPOSITION, &vsp);
}

Int64 Viewer::SetFrame(Int64 pos, int left, ViewFrameOptions options)
{
	AssertCurrentViewer();
	ViewerSetPosition vsp;
	vsp.Flags = (DWORD)options;
	vsp.LeftPos = left;
	vsp.StartPos.i64 = pos;
	Info.ViewerControl(VCTL_SETPOSITION, &vsp);
	return vsp.StartPos.i64;
}

void Viewer::Close()
{
	AssertCurrentViewer();
	Info.ViewerControl(VCTL_QUIT, 0);
}

void Viewer::Redraw()
{
	AssertCurrentViewer();
	Info.ViewerControl(VCTL_REDRAW, 0);
}

void Viewer::Select(Int64 symbolStart, int symbolCount)
{
	AssertCurrentViewer();
	if (symbolCount <= 0)
	{
		Info.ViewerControl(VCTL_SELECT, 0);
	}
	else
	{
		ViewerSelect vs;
		vs.BlockLen = symbolCount;
		vs.BlockStartPos.i64 = symbolStart;
		Info.ViewerControl(VCTL_SELECT, &vs);
	}
}

void Viewer::AssertClosed()
{
	if (IsOpened) throw gcnew InvalidOperationException("Viewer must not be open for this operation.");
}

bool Viewer::HexMode::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID < 0 || vi.ViewerID != _id)
		return false;
	else
		return vi.CurMode.Hex != 0;
}

void Viewer::HexMode::set(bool value)
{
	AssertCurrentViewer();
	ViewerSetMode vsm;
	vsm.Flags = vsm.Reserved = 0;
	vsm.Type = VSMT_HEX;
	vsm.Param.iParam = value;
	Info.ViewerControl(VCTL_SETMODE, &vsm);
}

bool Viewer::WrapMode::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID < 0 || vi.ViewerID != _id)
		return false;
	else
		return vi.CurMode.Wrap != 0;
}

void Viewer::WrapMode::set(bool value)
{
	AssertCurrentViewer();
	ViewerSetMode vsm;
	vsm.Flags = vsm.Reserved = 0;
	vsm.Type = VSMT_WRAP;
	vsm.Param.iParam = value;
	Info.ViewerControl(VCTL_SETMODE, &vsm);
}

bool Viewer::WordWrapMode::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID < 0 || vi.ViewerID != _id)
		return false;
	else
		return vi.CurMode.WordWrap != 0;
}

void Viewer::WordWrapMode::set(bool value)
{
	AssertCurrentViewer();
	ViewerSetMode vsm;
	vsm.Flags = vsm.Reserved = 0;
	vsm.Type = VSMT_WORDWRAP;
	vsm.Param.iParam = value;
	Info.ViewerControl(VCTL_SETMODE, &vsm);
}

}