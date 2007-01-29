#pragma once

namespace FarManagerImpl
{;
public ref class VisibleEditorLineCollection : public EditorLineCollection
{
public:
	virtual property ILine^ Item[int] { ILine^ get(int index) override; void set(int index, ILine^ value) override; }
	virtual property int Count { int get() override; }
	virtual void Add(String^ item) override;
	virtual void Clear() override;
	virtual void Insert(int index, String^ item) override;
	virtual void RemoveAt(int index) override;
internal:
	VisibleEditorLineCollection();
protected:
	virtual property IEnumerator<ILine^>^ EnumeratorImpl { IEnumerator<ILine^>^ get() override; }
private:
	static void Go(int no);
	static void Go(int no, int pos);
	static void SetPos(const EditorInfo& ei);
	static void RemoveCurrent();
};
}
