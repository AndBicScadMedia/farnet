
<#
.Synopsis
	Build script (https://github.com/nightroman/Invoke-Build)
#>

param(
	$Platform = (property Platform Win32)
)

$FarHome = "C:\Bin\Far\$Platform"
$fromModule = "$FarHome\FarNet\Modules\RightWords"
$fromNHunspell = "$FarHome\FarNet\NHunspell"

task . Build, Clean

task Build {
	use 4.0 MSBuild
	exec { MSBuild RightWords.csproj /p:Configuration=Release /p:FarHome=$FarHome }
}

task Clean {
	Remove-Item -Force -Recurse -ErrorAction 0 `
	z, bin, obj, About-RightWords.htm, FarNet.RightWords.*.nupkg
}

task Help {
	exec { MarkdownToHtml From=About-RightWords.text To=About-RightWords.htm }
}

task Version {
	$dll = Get-Item -LiteralPath $fromModule\RightWords.dll
	assert ($dll.VersionInfo.FileVersion -match '^(\d+\.\d+\.\d+)\.0$')
	$script:Version = $matches[1]
	$Version
}

task Package Help, {
	$toModule = 'z\tools\FarHome\FarNet\Modules\RightWords'
	$toNHunspell = 'z\tools\FarHome\FarNet\NHunspell'

	Remove-Item -Force -Recurse -ErrorAction 0 -Path [z]
	$null = mkdir $toModule, $toNHunspell

	Copy-Item -Destination $toModule `
	About-RightWords.htm,
	History.txt,
	LICENSE.txt,
	RightWords.macro.lua,
	$fromModule\RightWords.dll,
	$fromModule\RightWords.resources,
	$fromModule\RightWords.ru.resources

	Copy-Item -Destination $toNHunspell `
	$fromNHunspell\Hunspellx64.dll,
	$fromNHunspell\Hunspellx86.dll,
	$fromNHunspell\NHunspell.dll,
	$fromNHunspell\NHunspell.xml
}

task NuGet Package, Version, {
	$text = @'
RightWords is the FarNet module for FarManager.

It provides the spell-checker and thesaurus based on NHunspell. The core
Hunspell is used in OpenOffice and it works with dictionaries published
on OpenOffice.org.

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
		<id>FarNet.RightWords</id>
		<version>$Version</version>
		<authors>Roman Kuzmin</authors>
		<owners>Roman Kuzmin</owners>
		<projectUrl>https://code.google.com/p/farnet</projectUrl>
		<iconUrl>https://farnet.googlecode.com/svn/trunk/FarNetLogo.png</iconUrl>
		<licenseUrl>https://farnet.googlecode.com/svn/trunk/RightWords/LICENSE.txt</licenseUrl>
		<requireLicenseAcceptance>false</requireLicenseAcceptance>
		<summary>$text</summary>
		<description>$text</description>
		<releaseNotes>https://farnet.googlecode.com/svn/trunk/RightWords/History.txt</releaseNotes>
		<tags>FarManager FarNet Module NHunspell</tags>
	</metadata>
</package>
"@
	# pack
	exec { NuGet pack z\Package.nuspec -NoPackageAnalysis }
}
