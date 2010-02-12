
<#
.SYNOPSIS
	Test a tool handler.
	Author: Roman Kuzmin

.DESCRIPTION
	Steps:
	- invoke this script to register the tool;
	- try it in F11, disk and config menus;
	- invoke this script again to unregister the tool.
#>

if (!$TestTool) {

	# install the handler
	$global:TestTool = {&{
		Show-FarMsg ("Hello from " + $_.From)
	}}

	# register the handler
	$null = $Far.RegisterTool($null, "PSF test tool", $TestTool, "AllAreas")
	Show-FarMsg "Test tool is registered. Try it in [F11] menus, for example, right now."
}
else {
	# unregister and uninstall the handler
	$Far.UnregisterTool($TestTool)
	Remove-Variable TestTool -Scope Global
	Show-FarMsg "Test tool is unregistered"
}