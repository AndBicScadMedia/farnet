
<#
.Synopsis
	Build script (https://github.com/nightroman/Invoke-Build)
#>

param
(
	$FarHome = (property FarHome),
	$FarNetModules = (property FarNetModules "$FarHome\FarNet\Modules")
)

$dir = "$FarNetModules\Backslash"
$src = 'Backslash.cs'
$dll = 'Backslash.dll'

use Framework\v3.5 csc

task Build -Inputs $src -Outputs $dll {
	exec { csc /target:library /optimize "/reference:$FarHome\FarNet\FarNet.dll" *.cs }
}

task Clean {
	Remove-Item $dll -ErrorAction 0
}

task Install {
	$null = mkdir $dir -Force
	Copy-Item $dll $dir
}
