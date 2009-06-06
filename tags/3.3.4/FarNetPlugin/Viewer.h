#pragma once

namespace FarManagerImpl
{;

public ref class Viewer : public IViewer
{
public:
	virtual property bool Async { bool get(); void set(bool value); }
	virtual property bool DeleteOnClose { bool get(); void set(bool value); }
	virtual property bool DeleteOnlyFileOnClose { bool get(); void set(bool value); }
	virtual property bool DisableHistory { bool get(); void set(bool value); }
	virtual property bool EnableSwitch { bool get(); void set(bool value); }
	virtual property bool IsModal { bool get(); void set(bool value); }
	virtual property bool IsOpened { bool get(); }
	virtual property Place Window { Place get(); void set(Place value); }
	virtual property String^ FileName { String^ get(); void set(String^ value); }
	virtual property String^ Title { String^ get(); void set(String^ value); }
public:
	virtual void Open();
internal:
	Viewer();
	void GetParams();
private:
	void EnsureClosed();
private:
	int _id;
	long _flags;
	Place _window;
	String^ _fileName;
	String^ _title;
};
}