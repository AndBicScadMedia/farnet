/*
FarNet plugin for Far Manager
Copyright (c) 2005 FarNet Team
*/

#pragma once

namespace FarNet
{;
ref class Far1 sealed : IFar
{
public:
	virtual property FarConfirmations Confirmations { FarConfirmations get() override; }
	virtual property FarNet::MacroArea MacroArea { FarNet::MacroArea get() override; }
	virtual property FarNet::MacroState MacroState { FarNet::MacroState get() override; }
	virtual property IAnyEditor^ AnyEditor { IAnyEditor^ get() override; }
	virtual property IAnyPanel^ Panel { IAnyPanel^ get() override; }
	virtual property IAnyPanel^ Panel2 { IAnyPanel^ get() override; }
	virtual property IAnyViewer^ AnyViewer { IAnyViewer^ get() override; }
	virtual property IDialog^ Dialog { IDialog^ get() override; }
	virtual property IEditor^ Editor { IEditor^ get() override; }
	virtual property ILine^ CommandLine { ILine^ get() override; }
	virtual property ILine^ Line { ILine^ get() override; }
	virtual property IMacro^ Macro { IMacro^ get() override; }
	virtual property IUserInterface^ UI { IUserInterface^ get() override; }
	virtual property IViewer^ Viewer { IViewer^ get() override; }
	virtual property IWindow^ Window { IWindow^ get() override; }
	virtual property String^ ActivePath { String^ get() override; }
	virtual property Version^ FarNetVersion { System::Version^ get() override; }
	virtual property Version^ FarVersion { System::Version^ get() override; }
public:
	virtual array<int>^ CreateKeySequence(String^ keys) override;
	virtual IDialog^ CreateDialog(int left, int top, int right, int bottom) override;
	virtual IEditor^ CreateEditor() override;
	virtual IInputBox^ CreateInputBox() override;
	virtual IListMenu^ CreateListMenu() override;
	virtual IMenu^ CreateMenu() override;
	virtual IPanel^ CreatePanel() override;
	virtual IViewer^ CreateViewer() override;
public:
	virtual array<IEditor^>^ Editors() override;
	virtual array<IViewer^>^ Viewers() override;
	virtual Char CodeToChar(int code) override;
	virtual CultureInfo^ GetCurrentUICulture(bool update) override;
	virtual ICollection<String^>^ GetDialogHistory(String^ name) override;
	virtual ICollection<String^>^ GetHistory(String^ name, String^ filter) override;
	virtual IModuleCommand^ GetModuleCommand(Guid id) override;
	virtual IModuleFiler^ GetModuleFiler(Guid id) override;
	virtual IModuleTool^ GetModuleTool(Guid id) override;
	virtual int Message(String^ body, String^ header, MsgOptions options, array<String^>^ buttons, String^ helpTopic) override;
	virtual int NameToKey(String^ key) override;
	virtual IPanel^ FindPanel(Guid typeId) override;
	virtual IPanel^ FindPanel(Type^ hostType) override;
	virtual IRegistryKey^ OpenRegistryKey(String^ name, bool writable) override;
	virtual String^ Input(String^ prompt, String^ history, String^ title, String^ text) override;
	virtual String^ KeyToName(int key) override;
	virtual String^ PasteFromClipboard() override;
	virtual String^ TempFolder(String^ prefix) override;
	virtual String^ TempName(String^ prefix) override;
	virtual void CopyToClipboard(String^ text) override;
	virtual void PostJob(EventHandler^ handler) override;
	virtual void PostKeys(String^ keys, bool enableOutput) override;
	virtual void PostKeySequence(array<int>^ sequence, bool enableOutput) override;
	virtual void PostMacro(String^ macro, bool enableOutput, bool disablePlugins) override;
	virtual void PostStep(EventHandler^ handler) override;
	virtual void PostStepAfterKeys(String^ keys, EventHandler^ handler) override;
	virtual void PostStepAfterStep(EventHandler^ handler1, EventHandler^ handler2) override;
	virtual void PostText(String^ text, bool enableOutput) override;
	virtual void Quit() override;
	virtual void ShowError(String^ title, Exception^ error) override;
	virtual void ShowHelp(String^ path, String^ topic, HelpOptions options) override;
internal:
	static void Connect();
private:
	Far1() {}
	static Far1 Far;
};

}
