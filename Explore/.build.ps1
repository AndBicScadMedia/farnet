
<#
.Synopsis
	Build script (https://github.com/nightroman/Invoke-Build)
#>

param(
	$Platform = (property Platform Win32)
)

$FarHome = "C:\Bin\Far\$Platform"
$ModuleHome = "$FarHome\FarNet\Modules\Explore"

task . Build, Clean

# Build and install
task Build {
	use 4.0 MSBuild
	exec { MSBuild Explore.csproj /p:Configuration=Release /p:FarHome=$FarHome }
}

# New About-Explore.htm
task Help {
	exec { MarkdownToHtml "From = About-Explore.text; To = About-Explore.htm" }
}

task Clean {
	Remove-Item -Force -Recurse -ErrorAction 0 -Path `
	z, bin, obj, About-Explore.htm, FarNet.Explore.*.nupkg
}

task Version {
	$dll = Get-Item -LiteralPath $ModuleHome\Explore.dll
	assert ($dll.VersionInfo.FileVersion -match '^(\d+\.\d+\.\d+)\.0$')
	($script:Version = $matches[1])
}

task Package Help, {
	$toModule = 'z\tools\FarHome\FarNet\Modules\Explore'

	Remove-Item -Force -Recurse -ErrorAction 0 -Path [z]
	$null = mkdir $toModule

	Copy-Item -Destination $toModule `
	About-Explore.htm,
	History.txt,
	LICENSE.txt,
	$ModuleHome\Explore.dll
}

task NuGet Package, Version, {
	$text = @'
Explore is the FarNet module for Far Manager.

It searches in FarNet module panels and opens the result panel.
It is invoked from the command line with the prefix "Explore:".

---

To install FarNet packages, follow these steps:

https://farnet.googlecode.com/svn/trunk/Install-FarNet.en.txt

---
'@
	# nuspec
	Set-Content z\Package.nuspec @"
<?xml version="1.0"?>
<package xmlns="http://schemas.microsoft.com/packaging/2010/07/nuspec.xsd">
	<metadata>
		<id>FarNet.Explore</id>
		<version>$Version</version>
		<owners>Roman Kuzmin</owners>
		<authors>Roman Kuzmin</authors>
		<projectUrl>https://code.google.com/p/farnet</projectUrl>
		<iconUrl>https://farnet.googlecode.com/svn/trunk/FarNetLogo.png</iconUrl>
		<licenseUrl>https://farnet.googlecode.com/svn/trunk/Explore/LICENSE.txt</licenseUrl>
		<requireLicenseAcceptance>false</requireLicenseAcceptance>
		<summary>$text</summary>
		<description>$text</description>
		<releaseNotes>https://farnet.googlecode.com/svn/trunk/Explore/History.txt</releaseNotes>
		<tags>FarManager FarNet Module</tags>
	</metadata>
</package>
"@
	# pack
	exec { NuGet pack z\Package.nuspec -NoPackageAnalysis }
}
