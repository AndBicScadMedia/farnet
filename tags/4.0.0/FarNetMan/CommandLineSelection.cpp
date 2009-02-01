/*
FarNet plugin for Far Manager
Copyright (c) 2005-2009 FarNet Team
*/

#include "StdAfx.h"
#include "CommandLineSelection.h"
#include "Far.h"

namespace FarNet
{;
CommandLineSelection::CommandLineSelection()
{
}

String^ CommandLineSelection::Text::get()
{
	//??? tweak size and empty
	int size = Info.Control(INVALID_HANDLE_VALUE, FCTL_GETCMDLINESELECTEDTEXT, 0, 0);
	CBox buf(size);
	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETCMDLINESELECTEDTEXT, size, (LONG_PTR)(wchar_t*)buf);
	return gcnew String(buf);
}

void CommandLineSelection::Text::set(String^ value)
{
	CmdLineSelect cls;
	if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_GETCMDLINESELECTION, 0, (LONG_PTR)&cls))
		throw gcnew OperationCanceledException;
	if (cls.SelStart < 0)
		throw gcnew InvalidOperationException("Can't set text: there is no selection.");
	String^ text = Far::Instance->CommandLine->Text;
	String^ text1 = text->Substring(0, cls.SelStart);
	String^ text2 = text->Substring(cls.SelEnd);
	text = text1 + value + text2;
	CBox sb(text);
	if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_SETCMDLINE, 0, (LONG_PTR)(wchar_t*)sb))
		throw gcnew OperationCanceledException;
	cls.SelEnd = cls.SelStart + value->Length;
	if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_SETCMDLINESELECTION, 0, (LONG_PTR)&cls))
		throw gcnew OperationCanceledException;
}

int CommandLineSelection::End::get()
{
	CmdLineSelect cls;
	if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_GETCMDLINESELECTION, 0, (LONG_PTR)&cls))
		throw gcnew OperationCanceledException;
	return cls.SelEnd;
}

int CommandLineSelection::Length::get()
{
	CmdLineSelect cls;
	if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_GETCMDLINESELECTION, 0, (LONG_PTR)&cls))
		throw gcnew OperationCanceledException;
	return cls.SelStart >= 0 ? cls.SelEnd - cls.SelStart : 0;
}

int CommandLineSelection::Start::get()
{
	CmdLineSelect cls;
	if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_GETCMDLINESELECTION, 0, (LONG_PTR)&cls))
		throw gcnew OperationCanceledException;
	return cls.SelStart;
}

String^ CommandLineSelection::ToString()
{
	return Text;
}
}
