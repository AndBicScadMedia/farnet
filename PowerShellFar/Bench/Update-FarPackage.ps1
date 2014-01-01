
<#
.Synopsis
	Gets, unpacks, and updates or installs Far Manager NuGet packages.
	Author: Roman Kuzmin

.Description
	Recommendations, not always requirements:
	- Close all running Far Manager instances before updating.
	- Invoke the script by the console host, i.e. PowerShell.exe.

	On updating from the package the script simply extracts new files and
	replaces existing files with same names. Existing extra files are not
	deleted. Read release notes and remove extra files manually.

	Use the switch Verbose in order to get verbose messages.

.Parameter Id
		The package ID, e.g. 'FarNet', 'FarNet.PowerShellFar'.
.Parameter Version
		The package version, e.g. '5.0.40'.
		If it is omitted then the latest version is requested from NuGet.
		If it is "?" then the script returns the latest from NuGet and stops.
.Parameter Path
		Specifies the package file path to be unpacked to the directory
		<package name without extension> in the output directory.
.Parameter OutputDirectory
		The parent of the unpacked directory. If it is omitted then the
		unpacked directory is created in the current location.
.Parameter CacheDirectory
		The directory for downloaded packages.
		Default: "$env:LOCALAPPDATA\NuGet\Cache".
.Parameter FarHome
		The Far Manager directory to be updated. If it is omitted then the
		script only downloads and unpacks the package without copying files.
.Parameter Platform
		Platform: x64 or x86|Win32. The default is extracted from Far.exe info.
		It is needed only for packages with FarHome.x64|x86 folders if Far.exe
		is not in FarHome or its info cannot be extracted for some reason.

.Example
	> Update-FarPackage FarNet ?

	This command just requests and returns the latest FarNet version.

.Example
	> Update-FarPackage FarNet -Verbose

	This command downloads the latest FarNet and unpacks it to the current
	directory. It does not install or update because FarHome is omitted.
	Verbose messages are enabled.

.Example
	> Update-FarPackage FarNet -FarHome <path> [-Platform <x64|x86>]

	This command updates FarNet in FarHome. The Platform is needed only if
	Far.exe is not in FarHome (normally it is). After updating look at extra
	files at the output directory FarNet.<Version>, e.g. About-FarNet.htm,
	FarNetAPI.chm, History.txt.

.Example
	> Update-FarPackage FarNet.PowerShellFar -FarHome <path>

	This command updates PowerShellFar. The Platform is not needed. After
	updating look at extra files at FarNet.PowerShellFar.<Version>, e.g.
	About-PowerShellFar.htm, History.txt.
#>

[CmdletBinding(DefaultParameterSetName='Id')]
param(
	[Parameter(ParameterSetName='Id', Position=0, Mandatory=1)][string]
	$Id,
	[Parameter(ParameterSetName='Id', Position=1)][string]
	$Version,
	[Parameter(ParameterSetName='Id')][string]
	$CacheDirectory = "$env:LOCALAPPDATA\NuGet\Cache",
	[Parameter(ParameterSetName='Path', Mandatory=1)][string]
	$Path,
	[string]
	$OutputDirectory = '.',
	[string]
	$FarHome,
	[string][ValidateSet('x64', 'x86', 'Win32')]
	$Platform
)

if ($MyInvocation.InvocationName -eq '.') { Write-Error 'Do not dot-source this script.' -ErrorAction Stop }
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

### use or download package
if ($PSCmdlet.ParameterSetName -eq 'Path') {
	$Path = $PSCmdlet.GetUnresolvedProviderPathFromPSPath($Path)
	if (![IO.File]::Exists($Path)) { Write-Error "Missing file '$Path'." }
}
else {
	# web client
	$web = New-Object -TypeName System.Net.WebClient
	$web.UseDefaultCredentials = $true

	# get version
	if (!$Version -or $Version -eq '?') {
		Write-Verbose "Getting the latest version..."
		$Uri = "http://www.nuget.org/api/v2/Packages()?`$filter=Id eq '$Id' and IsLatestVersion eq true"
		$xml = [xml]$web.DownloadString($Uri)
		$latest = ''
		try {
			foreach($_ in $xml.feed.entry) {
				if ($_.id -match "Id='([^']+)'") { $Id = $Matches[1] }
				$latest = $_.properties.Version
				break
			}
		}
		catch {}
		if (!$latest) { Write-Error "Cannot get the latest version for '$Id'. Check the package ID." }
		Write-Verbose "The latest version is '$latest'."
		if ($Version -eq '?') {
			return $latest
		}
		$Version = $latest
	}

	# exists?
	$Path = "$CacheDirectory\$Id.$Version.nupkg"
	if ([System.IO.File]::Exists($Path)) {
		# use existing
		Write-Verbose "Found '$Path'."
	}
	else {
		# ensure cache
		$CacheDirectory = $PSCmdlet.GetUnresolvedProviderPathFromPSPath($CacheDirectory)
		$null = [System.IO.Directory]::CreateDirectory($CacheDirectory)

		# download
		$Uri = "http://nuget.org/api/v2/package/$Id/$Version"
		Write-Verbose "Downloading '$Path'..."
		$web.DownloadFile($Uri, $Path)
	}
}

### destination directory
$destination = Join-Path $PSCmdlet.GetUnresolvedProviderPathFromPSPath($OutputDirectory) ([System.IO.Path]::GetFileNameWithoutExtension($Path))
Write-Verbose "Unpacking to '$destination'..."

### unpack to destination
Add-Type -AssemblyName WindowsBase
$v3 = $PSVersionTable.CLRVersion.Major -lt 4
$package = [System.IO.Packaging.Package]::Open($Path, 'Open', 'Read')
try {
	foreach($part in $package.GetParts()) {
		if ($part.Uri -notmatch '^/tools/(.*)') {continue}
		$uri = [System.Uri]::UnescapeDataString($Matches[1])
		$to = "$destination/$uri"
		$null = [System.IO.Directory]::CreateDirectory([System.IO.Path]::GetDirectoryName($to))
		$stream2 = New-Object System.IO.FileStream $to, 'Create'
		try {
			$stream1 = $part.GetStream('Open', 'Read')
			if ($v3) {
				$buffer = New-Object byte[] ($n = $stream1.Length)
				$null = $stream1.Read($buffer, 0, $n)
				$stream2.Write($buffer, 0, $n)
			}
			else {
				$stream1.CopyTo($stream2)
			}
		}
		finally {
			$stream2.Close()
		}
	}
}
finally {
	$package.Close()
}

### FarHome?
if (!$FarHome) {return}
$FarHome = $PSCmdlet.GetUnresolvedProviderPathFromPSPath($FarHome)
if (![IO.Directory]::Exists($FarHome)) { Write-Error "Parameter FarHome: missing directory '$FarHome'." }

### update from destination
Write-Verbose "Updating '$FarHome'..."

# updates FarHome
function UpdateFarHome($from) {
	foreach($name in Get-ChildItem -Name -LiteralPath $from -Force -Recurse) {
		if ([System.IO.Directory]::Exists("$from\$name")) {continue}
		$null = [System.IO.Directory]::CreateDirectory("$FarHome\$([System.IO.Path]::GetDirectoryName($name))")
		Copy-Item -LiteralPath "$from\$name" -Destination "$FarHome\$name" -Force
	}
}

# gets platform
function GetPlatform {
	if ($Platform) { return $Platform }
	if (!($exe = Get-Item -LiteralPath "$FarHome\Far.exe" -ErrorAction 0) -or ($exe.VersionInfo.FileVersion -notmatch '\b(x86|x64)\b')) {
		Write-Error "Cannot get info from Far.exe. Specify the Platform."
	}
	return ($Platform = $Matches[1])
}

# FarHome.x64
if ([System.IO.Directory]::Exists(($from = "$destination\FarHome.x64"))) {
	if ((GetPlatform) -eq 'x64') {
		UpdateFarHome $from
	}
}

# FarHome.x86
if ([System.IO.Directory]::Exists(($from = "$destination\FarHome.x86"))) {
	if ((GetPlatform) -ne 'x64') {
		UpdateFarHome $from
	}
}

# FarHome
if ([System.IO.Directory]::Exists(($from = "$destination\FarHome"))) {
	UpdateFarHome $from
}
