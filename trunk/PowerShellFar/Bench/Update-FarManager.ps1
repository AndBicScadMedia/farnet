
<#
.SYNOPSIS
	Updates Far Manager 2.x and standard plugins.
	Author: Roman Kuzmin

.DESCRIPTION
	If Far Manager is running the script prompts you to exit running instances
	and waits until this is done. That is why you should not run the script in
	Far Manager. On the other hand it is still useful to start the script from
	Far Manager (using 'start' command or [ShiftEnter] in the command line), in
	this case you do not have to set the parameter FARHOME.

	%USERPROFILE% directory is the destination for downloaded archives. Old
	files are not deleted. It is recommended to keep at the last downloaded
	archive there, the script downloads only new archives, if any.

	Command 7z has to be available, e.g. 7z.exe in the system path.

	The script gets the latest web archive name. If the file already exists the
	script exits. Otherwise it downloads the archive and starts extraction. It
	extracts everything to the home directory and then removes plugin folders
	that did not exist before.

	Finally the script checks and shows extra user items which do not exist in
	the archive, e.g. user plugins and files and standard files excluded now.

.EXAMPLE
	# This command is suitable for the Far command line or the user menu. Also,
	# it keeps the console opened, so that you can view the output.
	start powershell -noexit Update-FarManager
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
	# Already downloaded archive where Far should be updated from.
	$Archive
	,
	[switch]
	# Download and update only stable builds.
	$Stable
)

if ($Host.Name -ne 'ConsoleHost') { throw "Please, invoke by the console host." }
if (![IO.Directory]::Exists($FARHOME)) { throw "Directory not found: '$FARHOME'." }
if (@('x86', 'x64') -notcontains $Platform) { throw "Invalid platform value: '$Platform'." }
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

### download if not yet
<#
DownloadFile() may depend on IE Tools > Internet Options > Connections > LAN settings
My machines:
- machine 1: nothing is checked: OK
- machine 2: "Use automatic configuration script" + some script: OK
- machine 3:
"Automatically detect settings": ERROR: (407) Proxy Authentication Required.
nothing is checked: OK
#>
if (!$Archive) {
	### get URL: stable, platform
	$URL = $(if ($Stable) {"http://www.farmanager.com/files/update2.php"} else {"http://www.farmanager.com/nightly/update2.php"})
	$URL += $(if ('x86' -eq $Platform) {'?p=32'} else {'?p=64'})

	### look for updates (request the archive name)
	Write-Host -ForegroundColor Cyan "Looking for updates at '$URL'..."
	$wc = New-Object Net.WebClient
	$initext = $wc.DownloadString($URL)
	if ($initext -notmatch 'arc="(Far[^"]+\.7z)"') { throw "Cannot get an archive name from '$ini'." }

	### exit if the archive exists
	$Name = $matches[1]
	$Archive = "$env:USERPROFILE\$Name"
	if ([IO.File]::Exists($Archive)) {
		Write-Host -ForegroundColor Cyan "The archive '$Archive' already exists; use the parameter -Archive to update from it."
		return
	}

	### download the archive
	$URL = $(if ($Stable) {"http://www.farmanager.com/files/$Name"} else {"http://www.farmanager.com/nightly/$Name"})
	Write-Host -ForegroundColor Cyan "Downloading '$Archive' from $URL..."
	$wc.DownloadFile($URL, $Archive)
}

### exit running
Write-Host -ForegroundColor Cyan "Waiting for Far Manager exit..."
Wait-Process Far -ErrorAction 0

### extract all
Write-Host -ForegroundColor Cyan "Extracting from '$Archive'..."
$plugins1 = [System.IO.Directory]::GetDirectories("$FARHOME\Plugins")
$files1 = foreach($_ in '*.hlf', '*.lng') { [System.IO.Directory]::GetFiles($FARHOME, $_) }
& '7z' 'x' $Archive "-o$FARHOME" '-aoa'
if ($lastexitcode) { throw "7z failed." }

### remove not used plugins
Write-Host -ForegroundColor Cyan "Removing not used plugins..."
$plugins2 = [System.IO.Directory]::GetDirectories("$FARHOME\Plugins")
foreach($plugin in $plugins2) {
	if ($plugins1 -notcontains $plugin) {
		Write-Host "Removing $plugin"
		[System.IO.Directory]::Delete($plugin, $true)
	}
}

### remove not used files
Write-Host -ForegroundColor Cyan "Removing not used files..."
$files2 = foreach($_ in '*.hlf', '*.lng') { [System.IO.Directory]::GetFiles($FARHOME, $_) }
foreach($file in $files2) {
	if ($files1 -notcontains $file) {
		Write-Host "Removing $file"
		[System.IO.File]::Delete($file)
	}
}

### check extra items
Write-Host -ForegroundColor Cyan "Checking extra items..."
$nExtra = 0
$inArchive = @{}
.{ & '7z' 'l' $Archive '-slt' | .{process{ if ($_ -match '^Path = (.+)') { $inArchive.Add($matches[1], $null) } }} }
.{
	Get-ChildItem $FarHome -Force -Name -ErrorAction 0
	Get-ChildItem "$FarHome\Plugins" -Force -Name -ErrorAction 0 | .{process{ "Plugins\$_" }}
	foreach($key in $inArchive.Keys) {
		if ($key -match '^Plugins\\\w+$|^[^\\]+$' -and $key -ne 'Plugins' -and [IO.Directory]::Exists("$FARHOME\$key")) {
			Get-ChildItem "$FARHOME\$key" -Recurse -Force -Name -ErrorAction 0 | .{process{ "$key\$_" }}
		}
	}
} | .{process{
	if (!$inArchive.ContainsKey($_)) {
		$_
		++$nExtra
	}
}}
Write-Host -ForegroundColor Cyan "$nExtra extra items."

Write-Host -ForegroundColor Green "Update succeeded."
