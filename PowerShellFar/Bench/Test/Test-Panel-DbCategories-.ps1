
<#
.SYNOPSIS
	Test panel with simple table TestCategories.
	Author: Roman Kuzmin

.DESCRIPTION
	The easy way to browse table data is to use the script Panel-DbData-.ps1.
	The code here is actually more complex than it should be because the panel
	is also used for lookup by Test-Panel-DbNotes-.ps1.
#>

param
(
	$Lookup
)

if (!$DbConnection -or !$DbProviderFactory) { throw "No connection, run Initialize-Test-.ps1" }

Panel-DbData- `
-Lookup $Lookup `
-Title 'TestCategories' `
-TableName 'TestCategories' `
-Columns 'Category', 'Remarks' `
-ExcludeMembers 'CategoryId'