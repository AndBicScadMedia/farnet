
<#
.SYNOPSIS
	Updates FarNet, PowerShellFar and API help.
	Author: Roman Kuzmin

.DESCRIPTION
	Command 7z has to be available, e.g. 7z.exe in the system path.

	If Far Manager is running the script prompts you to exit running instances
	and waits until this is done. That is why you should not run the script in
	Far Manager. On the other hand it is still useful to start the script from
	Far Manager (using 'start' command or [ShiftEnter] in the command line), in
	this case you do not have to set the parameter -FARHOME. If -FARHOME is UNC
	then that machine has to be configured for PS remoting.

	-Archive directory is the destination for downloaded archives. Old files
	are not deleted. Keep the last downloaded archives there, the script
	downloads only new archives.

	On updating from the archives the script simply extracts files and replace
	the same existing with no warnings. Existing extra files are not deleted:
	thus, read History.txt on updates, you may want to remove some files.

	Updated items in %FARHOME%:
	-- Far.exe.config
	-- FarNet\FarNet.dll
	-- FarNet\FarNet.xml
	-- FarNet\PowerShellFar.chm
	-- FarNet\Modules\PowerShellFar (+ Bench, Extras, Modules)
	-- Plugins\FarNet

.EXAMPLE
	# This command starts update in a new console and keeps it opened to view
	# the output. Then it tells Far to exit because update will wait for this.
	>: Start-Process powershell.exe "-noexit Update-PowerShellFar"; $Far.Quit()
#>

param
(
	[Parameter()][string]
	# Far directory; needed if %FARHOME% is not defined and its location is not standard.
	$FARHOME = $(if ($env:FARHOME) {$env:FARHOME} else {"C:\Program Files\Far"})
	,
	[string]
	# Target platform: x86 or x64. Default: depends on the current process.
	$Platform = $(if ([intptr]::Size -eq 4) {'x86'} else {'x64'})
	,
	[string]
	# Downloaded archives directory. Default: $HOME.
	$Archive = $HOME
	,
	[string]
	# Version (X.Y.Z). Default: requested from the FarNet site.
	$Version
	,
	[switch]
	# Tells to update from already downloaded archives.
	$Force
)

if ($Host.Name -ne 'ConsoleHost') { throw "Please, invoke by the console host." }
if (![IO.Directory]::Exists($FARHOME)) { throw "Directory not found: '$FARHOME'." }
if (![IO.Directory]::Exists($Archive)) { throw "Directory not found: '$Archive'." }
if (@('x86', 'x64') -notcontains $Platform) { throw "Invalid platform value: '$Platform'." }
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

### to download if not yet
# see notes in Update-FarManager.ps1
$wc = New-Object Net.WebClient

### request version
if (!$Version) {
	# download Readme.txt, get version
	$URL = "http://farnet.googlecode.com/svn/trunk/PowerShellFar/Readme.txt"
	Write-Host -ForegroundColor Cyan "Getting version from '$URL'..."
	$initext = $wc.DownloadString($URL)
	if ($initext -notmatch 'Version\s+:\s+(\d+\.\d+\.\d+)') { throw "Cannot get version from '$ini'." }
	$Version = $matches[1]
}

### download missed archives
$Names = @("FarNet.$Version.7z", "PowerShellFar.$Version.7z", "PowerShellFar.doc.$Version.7z")
$Archives = @("$Archive\$($Names[0])", "$Archive\$($Names[1])", "$Archive\$($Names[2])")
$done = 0
for($$ = 0; $$ -lt 3; ++$$) {
	if ([IO.File]::Exists($Archives[$$])) {
		Write-Host -ForegroundColor Cyan "The archive '$($Archives[$$])' already exists."
	}
	else {
		$URL = "http://farnet.googlecode.com/files/$($Names[$$])"
		Write-Host -ForegroundColor Cyan "Downloading '$($Archives[$$])' from $URL..."
		$wc.DownloadFile($URL, $Archives[$$])
		++$done
	}
}
if (!$Force -and $done -eq 0) {
	Write-Host -ForegroundColor Cyan "All the archives already exist, use -Force to update from them."
	return
}

### exit running; use remoting for UNC
if ($FARHOME -match '^\\\\([^\\/]+)') {
	Write-Host -ForegroundColor Cyan "Waiting for Far Manager exit: $($matches[1])..."
	Invoke-Command -ComputerName ($matches[1]) { Wait-Process Far -ErrorAction 0 }
}
else {
	Write-Host -ForegroundColor Cyan "Waiting for Far Manager exit..."
	Wait-Process Far -ErrorAction 0
}

### extract FarNet
Write-Host -ForegroundColor Cyan "Extracting from '$($Archives[0])'..."
# x86
& '7z' 'x' ($Archives[0]) "-o$FARHOME" '-aoa' 'Far.exe.config' 'FarNet\*.*' 'Plugins\FarNet'
if ($lastexitcode) { throw "7z failed." }
# x64
if ($Platform -eq 'x64') {
	& '7z' 'e' ($Archives[0]) "-o$FARHOME\Plugins\FarNet" '-aoa' 'Plugins.x64\FarNet\FarNetMan.dll'
	if ($lastexitcode) { throw "7z failed." }
}

### extract PowerShellFar
Write-Host -ForegroundColor Cyan "Extracting from '$($Archives[1])'..."
& '7z' 'x' ($Archives[1]) "-o$FARHOME" '-aoa' 'FarNet\Modules\PowerShellFar'
if ($lastexitcode) { throw "7z failed." }

### extract PowerShellFar.chm
Write-Host -ForegroundColor Cyan "Extracting from '$($Archives[2])'..."
& '7z' 'x' ($Archives[2]) "-o$FARHOME" '-aoa'
if ($lastexitcode) { throw "7z failed." }

### done
Write-Host -ForegroundColor Green "Update succeeded."
