
--[[
	This file contains example macros which invoke PowerShellFar features.
	DO NOT USE THIS AS IT IS, USE YOUR OWN MACROS, THIS IS ONLY AN EXAMPLE
]]

Macro {
area="Shell"; key="Space"; flags="EmptyCommandLine|DisableOutput"; description="PSF: Easy prefix"; action=function()
Keys("> : Space")
end;
}

Macro {
area="Shell"; key="ShiftSpace"; flags="NotEmptyCommandLine"; description="PSF: Easy invoke"; action=function()
Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", "::>: $Psf.InvokeSelectedCode()")
end;
}

Macro {
area="Common"; key="CtrlSpace"; description="PSF: Complete-Word-.ps1"; action=function()
if Area.DialogAutoCompletion or Area.ShellAutoCompletion then Keys("Esc") end
if Area.Editor or Area.Dialog or ((Area.Shell or Area.Info or Area.QView or Area.Tree) and not CmdLine.Empty) then
	Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", ":>:Complete-Word-.ps1")
else
	Keys("AKey")
end
end;
}

Macro {
area="Common"; key="CtrlShiftL"; description="PSF: Favorites menu"; action=function()
Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", ":>: Menu-Favorites-.ps1")
end;
}

Macro {
area="Common"; key="AltF10"; description="PSF: Command history"; action=function()
Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", ":>: $Psf.ShowHistory()")
end;
}

Macro {
area="Common"; key="CtrlShiftP"; description="PSF: Command box"; action=function()
Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", ":>: $Psf.InvokeInputCode()")
end;
}

Macro {
area="Editor"; key="Ctrl="; description="PSF: Show editor bookmarks"; action=function()
Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", ":>: Select-Bookmark-")
end;
}

Macro {
area="Shell"; key="F10"; description="PSF: Quit Far"; action=function()
if not Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", ":>: $Far.Quit()") then Keys("F10") end
end;
}

Macro {
area="Shell"; key="AltF12"; description="PSF: Folder history"; action=function()
Plugin.Call("10435532-9BB3-487B-A045-B0E6ECAAB6BC", ":>: Show-History-.ps1 -Folder")
end;
}

Macro {
area="Shell"; key="CtrlShiftF12"; description="PSF: Standard folder history"; action=function()
Keys("AltF12")
end;
}
