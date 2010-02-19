
<#
.SYNOPSIS
	Test a filer handler.
	Author: Roman Kuzmin

.DESCRIPTION
	Steps:
	- invoke this script, it registers a *.test files handler;
	- [Enter] on a text file *.test to view its lines in a panel;
	- invoke this script again, the handler should be unregistered.
#>

if (!$TestFiler) {

	# install the handler
	$global:TestFiler = {&{
		# get and show all lines in a panel
		$p = New-Object PowerShellFar.UserPanel
		$p.Panel.Info.Title = $_.Name
		$p.Panel.Info.HostFile = $_.Name
		$p.Panel.Info.StartSortMode = 'Unsorted'
		$p.AddObjects([IO.File]::ReadAllLines($_.Name))
		$p.Show()
	}}

	# register the handler
	$attr = New-Object FarNet.ModuleFilerAttribute -Property @{ Name = "PSF test filer"; Mask = "*.test" }
	$Far.RegisterFiler($null, "d7fb89f3-b24b-40f1-b94b-83031d87bf52", $TestFiler, $attr)
	Show-FarMessage "Test filer is registered. [Enter] *.test files now."
}
else {
	# unregister and uninstall the handler
	$Far.UnregisterFiler($TestFiler)
	Remove-Variable TestFiler -Scope Global
	Show-FarMessage "Test filer is unregistered"
}