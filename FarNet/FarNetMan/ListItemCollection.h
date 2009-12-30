/*
FarNet plugin for Far Manager
Copyright (c) 2005 FarNet Team
*/

#pragma once
#include "Menu.h"

namespace FarNet
{;
ref class FarBaseList;

ref class ListItemCollection : public Collection<FarItem^>
{
public:
	ListItemCollection(FarBaseList^ box);
protected:
	virtual void ClearItems() override;
	virtual void InsertItem(int index, FarItem^ item) override;
	virtual void RemoveItem(int index) override;
	virtual void SetItem(int index, FarItem^ item) override;
internal:
	void SetBox(FarBaseList^ value);
private:
	FarBaseList^ _box;
};

}
