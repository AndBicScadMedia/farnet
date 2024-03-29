
/*
FarNet plugin for Far Manager
Copyright (c) 2006-2015 Roman Kuzmin
*/

#include "StdAfx.h"
#include "Viewer.h"
#include "Viewer0.h"

namespace FarNet
{;
void AnyViewer::ViewText(String^ text, String^ title, OpenMode mode)
{
	Works::EditorTools::ViewText(text, title, mode);
}

Viewer::Viewer()
: _id(-1)
, _Title(String::Empty)
, _CodePage(CP_DEFAULT)
{}

void Viewer::Open(OpenMode mode)
{
	AssertClosed();

	if (ValueUserScreen::Get()) //????
	{
		ValueUserScreen::Set(false);
		Far::Api->UI->SaveUserScreen();
	}

	// flags
	int flags = 0;

	if (_DisableHistory)
		flags |= VF_DISABLEHISTORY;

	switch(_Switching)
	{
	case FarNet::Switching::Enabled:
		flags |= VF_ENABLE_F6;
		break;
	case FarNet::Switching::Auto:
		if (_DeleteSource == FarNet::DeleteSource::None &&
			_Closed == nullptr &&
			_GotFocus == nullptr &&
			_LosingFocus == nullptr &&
			_Opened == nullptr)
			flags |= VF_ENABLE_F6;
		break;
	}

	switch(mode)
	{
	case OpenMode::None:
		flags |= (VF_NONMODAL | VF_IMMEDIATERETURN); break;
	case OpenMode::Wait:
		flags |= VF_NONMODAL; break;
	}

	switch(_DeleteSource)
	{
	case FarNet::DeleteSource::UnusedFile:
		flags |= VF_DELETEONLYFILEONCLOSE; break;
	case FarNet::DeleteSource::UnusedFolder:
		flags |= VF_DELETEONCLOSE; break;
	}

	PIN_ES(pinFileName, _FileName);
	PIN_ES(pinTitle, _Title);

	// from dialog? set modal
	WindowKind wt = Far::Api->Window->Kind;
	if (wt == WindowKind::Dialog || Far::Api->UI->IsCommandMode)
		flags &= ~VF_NONMODAL;

	// open: see editor
	_id = -1;
	Viewer0::_viewerWaiting = this;
	Info.Viewer(
		pinFileName,
		pinTitle,
		_Window.Left,
		_Window.Top,
		_Window.Right,
		_Window.Bottom,
		flags,
		_CodePage); //?? test window values

	// redraw Far
	if (wt == WindowKind::Dialog)
		Far::Api->UI->Redraw();

	// errors: see editor
	if (_id == -1)
		throw gcnew InvalidOperationException("Cannot open the file '" + FileName + "'");
}

IntPtr Viewer::Id::get()
{
	return (IntPtr)_id;
}

DeleteSource Viewer::DeleteSource::get()
{
	return _DeleteSource;
}

void Viewer::DeleteSource::set(FarNet::DeleteSource value)
{
	_DeleteSource = value;
}

Switching Viewer::Switching::get()
{
	return _Switching;
}

void Viewer::Switching::set(FarNet::Switching value)
{
	AssertClosed();
	_Switching = value;
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
		return vi.FileSize;
	else
		return -1;
}

Point Viewer::WindowSize::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	Point r;
	if (vi.ViewerID >= 0 && vi.ViewerID == _id)
	{
		r.X = (int)vi.WindowSizeX;
		r.Y = (int)vi.WindowSizeY;
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
		r.Offset = vi.FilePos;
		r.Column = vi.LeftPos;
	}
	return r;
}

void Viewer::Frame::set(ViewFrame value)
{
	AssertCurrentViewer();
	ViewerSetPosition vsp = {sizeof(vsp)};
	vsp.Flags = VSP_NORETNEWPOS;
	vsp.LeftPos = value.Column;
	vsp.StartPos = value.Offset;
	Info.ViewerControl(_id, VCTL_SETPOSITION, 0, &vsp);
}

Int64 Viewer::SetFrame(Int64 pos, int left, ViewFrameOptions options)
{
	AssertCurrentViewer();
	ViewerSetPosition vsp = {sizeof(vsp)};
	vsp.Flags = (DWORD)options;
	vsp.LeftPos = left;
	vsp.StartPos = pos;
	Info.ViewerControl(_id, VCTL_SETPOSITION, 0, &vsp);
	return vsp.StartPos;
}

void Viewer::Close()
{
	AssertCurrentViewer();
	Info.ViewerControl(_id, VCTL_QUIT, 0, 0);
}

void Viewer::Redraw()
{
	AssertCurrentViewer();
	Info.ViewerControl(_id, VCTL_REDRAW, 0, 0);
}

void Viewer::SelectText(Int64 symbolStart, int symbolCount)
{
	AssertCurrentViewer();
	if (symbolCount <= 0)
	{
		Info.ViewerControl(_id, VCTL_SELECT, 0, 0);
	}
	else
	{
		ViewerSelect vs = {sizeof(vs)};
		vs.BlockLen = symbolCount;
		vs.BlockStartPos = symbolStart;
		Info.ViewerControl(_id, VCTL_SELECT, 0, &vs);
	}
}

void Viewer::AssertClosed()
{
	if (IsOpened) throw gcnew InvalidOperationException("Viewer must not be open for this operation.");
}

ViewerViewMode Viewer::ViewMode::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID < 0 || vi.ViewerID != _id)
		return ViewerViewMode::Text;
	else
		return (ViewerViewMode)vi.CurMode.ViewMode;
}
void Viewer::ViewMode::set(ViewerViewMode value)
{
	AssertCurrentViewer();
	ViewerSetMode vsm = {sizeof(vsm)};
	vsm.Flags = 0;
	vsm.Type = VSMT_VIEWMODE;
	vsm.iParam = (intptr_t)value;
	Info.ViewerControl(_id, VCTL_SETMODE, 0, &vsm);
}

bool Viewer::WrapMode::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID < 0 || vi.ViewerID != _id)
		return false;
	else
		return (vi.CurMode.Flags & VMF_WRAP) != 0;
}
void Viewer::WrapMode::set(bool value)
{
	AssertCurrentViewer();
	ViewerSetMode vsm = {sizeof(vsm)};
	vsm.Flags = 0;
	vsm.Type = VSMT_WRAP;
	vsm.iParam = value;
	Info.ViewerControl(_id, VCTL_SETMODE, 0, &vsm);
}

bool Viewer::WordWrapMode::get()
{
	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID < 0 || vi.ViewerID != _id)
		return false;
	else
		return (vi.CurMode.Flags & VMF_WORDWRAP) != 0;
}
void Viewer::WordWrapMode::set(bool value)
{
	AssertCurrentViewer();
	ViewerSetMode vsm = {sizeof(vsm)};
	vsm.Flags = 0;
	vsm.Type = VSMT_WORDWRAP;
	vsm.iParam = value;
	Info.ViewerControl(_id, VCTL_SETMODE, 0, &vsm);
}

int Viewer::CodePage::get()
{
	if (!IsOpened)
		return (int)_CodePage;

	ViewerInfo vi; ViewerControl_VCTL_GETINFO(vi, true);
	if (vi.ViewerID < 0 || vi.ViewerID != _id)
		return 0;

	return (int)vi.CurMode.CodePage;
}

void Viewer::CodePage::set(int value)
{
	AssertClosed();

	_CodePage = value;
}

DateTime Viewer::TimeOfOpen::get()
{
	return _TimeOfOpen;
}

void Viewer::Activate()
{
	int nWindow = Far::Api->Window->Count;
	for(int i = 0; i < nWindow; ++i)
	{
		WindowKind kind = Far::Api->Window->GetKindAt(i);
		if (kind != WindowKind::Viewer)
			continue;
		
		String^ name = Far::Api->Window->GetNameAt(i);
		if (name == _FileName)
		{
			Far::Api->Window->SetCurrentAt(i);
			return;
		}
	}
	throw gcnew InvalidOperationException("Cannot find the window by name.");
}

}
