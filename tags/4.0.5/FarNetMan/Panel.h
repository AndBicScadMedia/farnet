/*
FarNet plugin for Far Manager
Copyright (c) 2005-2009 FarNet Team
*/

#pragma once

namespace FarNet
{;
ref class FarFile : IFile
{
public:
	INL_PROP_FLAG(IsArchive, FILE_ATTRIBUTE_ARCHIVE);
	INL_PROP_FLAG(IsCompressed, FILE_ATTRIBUTE_COMPRESSED);
	INL_PROP_FLAG(IsDirectory, FILE_ATTRIBUTE_DIRECTORY);
	INL_PROP_FLAG(IsEncrypted, FILE_ATTRIBUTE_ENCRYPTED);
	INL_PROP_FLAG(IsHidden, FILE_ATTRIBUTE_HIDDEN);
	INL_PROP_FLAG(IsReadOnly, FILE_ATTRIBUTE_READONLY);
	INL_PROP_FLAG(IsReparsePoint, FILE_ATTRIBUTE_REPARSE_POINT);
	INL_PROP_FLAG(IsSystem, FILE_ATTRIBUTE_SYSTEM);
	virtual property DateTime CreationTime;
	virtual property DateTime LastAccessTime;
	virtual property DateTime LastWriteTime;
	virtual property FileAttributes Attributes { FileAttributes get(); void set(FileAttributes value); }
	virtual property Int64 Length;
	virtual property Object^ Data;
	virtual property String^ AlternateName;
	virtual property String^ Description;
	virtual property String^ Owner;
	virtual property String^ Name
	{
		String^ get() { return _Name; }
		void set(String^ value) { if (!value) throw gcnew ArgumentNullException("value"); _Name = value; }
	}
public:
	virtual String^ ToString() override
	{
		return Name;
	}
internal:
	FarFile() : _Name(String::Empty) {}
private:
	DWORD _flags;
	String^ _Name;
};

ref class FarPanel : public IPanel
{
public:
	virtual property bool Highlight { bool get(); }
	virtual property bool IsActive { bool get(); }
	virtual property bool IsLeft { bool get(); }
	virtual property bool IsPlugin { bool get(); }
	virtual property bool IsVisible { bool get(); void set(bool value); }
	virtual property bool NumericSort { bool get(); void set(bool value); }
	virtual property bool RealNames { bool get(); }
	virtual property bool ReverseSortOrder { bool get(); void set(bool value); }
	virtual property bool SelectedFirst { bool get(); }
	virtual property bool ShowHidden { bool get(); }
	virtual property bool UseSortGroups { bool get(); }
	virtual property IFile^ CurrentFile { IFile^ get(); }
	virtual property IList<IFile^>^ ShownFiles { IList<IFile^>^ get(); }
	virtual property IList<IFile^>^ SelectedFiles { IList<IFile^>^ get(); }
	virtual property int CurrentIndex { int get(); }
	virtual property int TopIndex { int get(); }
	virtual property PanelSortMode SortMode { PanelSortMode get(); void set(PanelSortMode value); }
	virtual property PanelType Type { PanelType get(); }
	virtual property PanelViewMode ViewMode { PanelViewMode get(); void set(PanelViewMode value); }
	virtual property Place Window { Place get(); }
	virtual property Point Frame { Point get(); }
	virtual property String^ Path { String^ get(); void set(String^ value); }
public:
	virtual void Close();
	virtual void Close(String^ path);
	virtual void GoToName(String^ name);
	virtual void GoToPath(String^ path);
	virtual void Redraw();
	virtual void Redraw(int current, int top);
	virtual void Update(bool keepSelection);
public:
	virtual String^ ToString() override;
internal:
	FarPanel(bool current);
	static FarFile^ ItemToFile(const PluginPanelItem& item);
protected:
	bool TryInfo(PanelInfo& pi);
internal:
	property HANDLE Handle { HANDLE get(); void set(HANDLE value); }
	property int Index { int get() { return (int)(INT_PTR)_handle; } void set(int value) { _handle = (HANDLE)(INT_PTR)value; } }
private:
	// For FAR panel it is PANEL_ACTIVE or PANEL_PASSIVE, otherwise it is a plugin handle
	HANDLE _handle;
};

#define FPPI_FLAG(Name)\
public: virtual property bool Name {\
	bool get() { return _##Name; }\
	void set(bool value) {\
	_##Name = value;\
	if (m) m->Flags = Flags();\
}}\
private: bool _##Name

#define FPPI_PROP(Type, Name, Set)\
public: virtual property Type Name {\
	Type get() { return _##Name; }\
	void set(Type value) {\
	_##Name = value;\
	if (m) { Set; }\
}}\
private: Type _##Name

#define FPPI_TEXT(Name, Data)\
public: virtual property String^ Name {\
	String^ get() { return _##Name; }\
	void set(String^ value) {\
	_##Name = value;\
	if (m) {\
	delete[] m->Data;\
	m->Data = NewChars(value);\
	}\
}}\
private: String^ _##Name

ref class FarPluginPanelInfo : IPluginPanelInfo
{
internal:
	FarPluginPanelInfo();
	void Free();
	OpenPluginInfo& Make();
public:
	FPPI_FLAG(CompareFatTime);
	FPPI_FLAG(ExternalDelete);
	FPPI_FLAG(ExternalGet);
	FPPI_FLAG(ExternalMakeDirectory);
	FPPI_FLAG(ExternalPut);
	FPPI_FLAG(PreserveCase);
	FPPI_FLAG(RawSelection);
	FPPI_FLAG(RealNames);
	FPPI_FLAG(RightAligned);
	FPPI_FLAG(ShowNamesOnly);
	FPPI_FLAG(UseAttrHighlighting);
	FPPI_FLAG(UseFilter);
	FPPI_FLAG(UseHighlighting);
	FPPI_FLAG(UseSortGroups);
	FPPI_PROP(bool, StartSortDesc, m->StartSortOrder = _StartSortDesc);
	FPPI_PROP(PanelSortMode, StartSortMode, m->StartSortMode = int(_StartSortMode));
	FPPI_PROP(PanelViewMode, StartViewMode, m->StartPanelMode = int(_StartViewMode) + 0x30);
	FPPI_TEXT(CurrentDirectory, CurDir);
	FPPI_TEXT(Format, Format);
	FPPI_TEXT(HostFile, HostFile);
	FPPI_TEXT(Title, PanelTitle);
public:
	virtual property array<DataItem^>^ InfoItems { array<DataItem^>^ get() { return _InfoItems; } void set(array<DataItem^>^ value); }
	virtual void SetKeyBarAlt(array<String^>^ labels);
	virtual void SetKeyBarAltShift(array<String^>^ labels);
	virtual void SetKeyBarCtrl(array<String^>^ labels);
	virtual void SetKeyBarCtrlAlt(array<String^>^ labels);
	virtual void SetKeyBarCtrlShift(array<String^>^ labels);
	virtual void SetKeyBarMain(array<String^>^ labels);
	virtual void SetKeyBarShift(array<String^>^ labels);
private:
	int Flags();
	void CreateInfoLines();
	void DeleteInfoLines();
	static void Free12Strings(wchar_t* const dst[12]);
	static void Make12Strings(wchar_t** dst, array<String^>^ src);
private:
	OpenPluginInfo* m;
	array<DataItem^>^ _InfoItems;
	array<String^>^ _keyBarAlt;
	array<String^>^ _keyBarAltShift;
	array<String^>^ _keyBarCtrl;
	array<String^>^ _keyBarCtrlAlt;
	array<String^>^ _keyBarCtrlShift;
	array<String^>^ _keyBarMain;
	array<String^>^ _keyBarShift;
};

ref class FarPluginPanel : public FarPanel, IPluginPanel
{
public: // FarPanel
	virtual property bool IsPlugin { bool get() override; }
	virtual property Guid Id { Guid get(); void set(Guid value); }
	virtual property IFile^ CurrentFile { IFile^ get() override; }
	virtual property IList<IFile^>^ ShownFiles { IList<IFile^>^ get() override; }
	virtual property IList<IFile^>^ SelectedFiles { IList<IFile^>^ get() override; }
	virtual property String^ Path { String^ get() override; void set(String^ value) override; }
	virtual property String^ StartDirectory { String^ get(); void set(String^ value); }
public: // IPluginPanel
	virtual property bool AddDots;
	virtual property bool IdleUpdate;
	virtual property bool IsOpened { bool get(); }
	virtual property bool IsPushed { bool get() { return _IsPushed; } }
	virtual property IList<IFile^>^ Files { IList<IFile^>^ get(); }
	virtual property Comparison<Object^>^ DataComparison;
	virtual property IPluginPanel^ AnotherPanel { IPluginPanel^ get(); }
	virtual property IPluginPanelInfo^ Info { IPluginPanelInfo^ get() { return %_info; } }
	virtual property Object^ Data;
	virtual property Object^ Host;
	virtual property String^ DotsDescription;
	virtual void Open();
	virtual void Open(IPluginPanel^ oldPanel);
	virtual void PostData(Object^ data) { _postData = data; }
	virtual void PostFile(IFile^ file) { _postFile = file; }
	virtual void PostName(String^ name) { _postName = name; }
	virtual void Push();
public: DEF_EVENT(Closed, _Closed);
public: DEF_EVENT(CtrlBreakPressed, _CtrlBreakPressed);
public: DEF_EVENT(GettingInfo, _GettingInfo);
public: DEF_EVENT(GotFocus, _GotFocus);
public: DEF_EVENT(Idled, _Idled);
public: DEF_EVENT(LosingFocus, _LosingFocus);
public: DEF_EVENT_ARGS(Closing, _Closing, PanelEventArgs);
public: DEF_EVENT_ARGS(DeletingFiles, _DeletingFiles, FilesEventArgs);
public: DEF_EVENT_ARGS(Executing, _Executing, ExecutingEventArgs);
public: DEF_EVENT_ARGS(GettingData, _GettingData, PanelEventArgs);
public: DEF_EVENT_ARGS(GettingFiles, _GettingFiles, GettingFilesEventArgs);
public: DEF_EVENT_ARGS(KeyPressed, _KeyPressed, PanelKeyEventArgs);
public: DEF_EVENT_ARGS(MakingDirectory, _MakingDirectory, MakingDirectoryEventArgs);
public: DEF_EVENT_ARGS(PuttingFiles, _PuttingFiles, FilesEventArgs);
public: DEF_EVENT_ARGS(Redrawing, _Redrawing, PanelEventArgs);
public: DEF_EVENT_ARGS(SettingDirectory, _SettingDirectory, SettingDirectoryEventArgs);
public: DEF_EVENT_ARGS(ViewModeChanged, _ViewModeChanged, ViewModeChangedEventArgs);
internal:
	FarPluginPanel();
	void AssertOpen();
	List<IFile^>^ ReplaceFiles(List<IFile^>^ files);
internal:
	bool _IsPushed;
	FarPluginPanelInfo _info;
	Object^ _postData;
	IFile^ _postFile;
	String^ _postName;
private:
	Guid _Id;
	List<IFile^>^ _files;
	String^ _StartDirectory;
};

const int cPanels = 4;
ref class PanelSet
{
internal:
	static property FarPluginPanel^ PostedPanel { FarPluginPanel^ get() { return _panels[0]; } }
	static void BeginOpenMode();
	static void EndOpenMode();
	static HANDLE AddPluginPanel(FarPluginPanel^ plugin);
	static int AsDeleteFiles(HANDLE hPlugin, PluginPanelItem* panelItem, int itemsNumber, int opMode);
	static int AsGetFiles(HANDLE hPlugin, PluginPanelItem* panelItem, int itemsNumber, int move, const wchar_t** destPath, int opMode);
	static int AsGetFindData(HANDLE hPlugin, PluginPanelItem** pPanelItem, int* pItemsNumber, int opMode);
	static int AsMakeDirectory(HANDLE hPlugin, const wchar_t** name, int opMode);
	static int AsProcessEvent(HANDLE hPlugin, int id, void* param);
	static int AsProcessKey(HANDLE hPlugin, int key, unsigned int controlState);
	static int AsPutFiles(HANDLE hPlugin, PluginPanelItem* panelItem, int itemsNumber, int move, int opMode);
	static int AsSetDirectory(HANDLE hPlugin, const wchar_t* dir, int opMode);
	static FarPanel^ GetPanel(bool active);
	static FarPluginPanel^ GetPluginPanel(Guid id);
	static FarPluginPanel^ GetPluginPanel(Type^ hostType);
	static FarPluginPanel^ GetPluginPanel2(FarPluginPanel^ plugin);
	static void AsClosePlugin(HANDLE hPlugin);
	static void AsFreeFindData(PluginPanelItem* panelItem);
	static void AsGetOpenPluginInfo(HANDLE hPlugin, OpenPluginInfo* info);
	static void OpenPluginPanel(FarPluginPanel^ plugin);
	static void PushPluginPanel(FarPluginPanel^ plugin);
	static void ReplacePluginPanel(FarPluginPanel^ oldPanel, FarPluginPanel^ newPanel);
internal:
	static List<FarPluginPanel^> _stack;
private:
	PanelSet() {}
private:
	// Posted [0] and opened [1..3] panels; i.e. size is 4, see AddPluginPanel().
	static array<FarPluginPanel^>^ _panels = gcnew array<FarPluginPanel^>(cPanels);
	static bool _inAsSetDirectory;
	static int _openMode;
};

}
