#pragma once

#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers
#define NOTEXTMETRIC // fixes pack 2 linking problem

#pragma warning(push,3)
#include <vcclr.h>
#include "plugin.hpp"
#pragma warning(pop)

using namespace FarManager::Forms;
using namespace FarManager;
using namespace Microsoft::Win32;
using namespace System::Collections::Specialized;
using namespace System::Collections::Generic;
using namespace System::Diagnostics;
using namespace System::IO;
using namespace System::Reflection;
using namespace System::Text::RegularExpressions;
using namespace System::Text;
using namespace System;

extern PluginStartupInfo Info;

#include "Utils.h"
