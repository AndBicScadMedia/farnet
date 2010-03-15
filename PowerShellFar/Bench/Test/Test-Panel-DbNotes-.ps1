
<#
.SYNOPSIS
	Test panel with joined table TestNotes with lookup
	Author: Roman Kuzmin

.DESCRIPTION
	This script demonstrates some important techniques:

	1) Panel uses data adapter with manually configured INSERT, DELETE and
	UPDATE commands. Why?
	- when SELECT joins data from several table automatic command generation
	may not work;
	- automatically generated commands are not optimal and they don't use not
	standard data.

	2) When you <Enter> on the field Category the panel opens another (lookup)
	table TestCategories. You have to select a category by name and press
	[Enter], CategoryId is also taken internally.

	3) SELECT gets more data than shown: e.g. NoteId and CategoryId. A user
	never changes or see them but they are used internally for SQL commands and
	lookup - that is why they are still selected.
#>

param
(
	[switch]$GenericLookup
)

Assert-Far ($DbConnection -and $DbProviderFactory) "No connection, run Initialize-Test-.ps1" "Assert"

# data adapter and command to select data
$a = $DbProviderFactory.CreateDataAdapter()
$a.SelectCommand = $c = $DbConnection.CreateCommand()
$c.CommandText = @"
SELECT n.Note, c.Category, n.Created, n.NoteId, n.CategoryId
FROM TestNotes n JOIN TestCategories c ON n.CategoryId = c.CategoryId
"@

# reusable script blocks to add command parameters
$NoteId = { $null = $c.Parameters.Add("NoteId", [Data.SqlDbType]::Int, 0, "NoteId") }
$CategoryId = { $null = $c.Parameters.Add("CategoryId", [Data.SqlDbType]::Int, 0, "CategoryId") }
$Note = { $null = $c.Parameters.Add("Note", [Data.SqlDbType]::NText, 0, "Note") }
$Created = { $null = $c.Parameters.Add("Created", [Data.SqlDbType]::DateTime, 0, "Created") }

# command to insert data
$a.InsertCommand = $c = $DbConnection.CreateCommand()
$c.CommandText = @"
--SQL
INSERT TestNotes (Note, CategoryId, Created)
VALUES (@Note, @CategoryId, @Created)
"@
. $CategoryId
. $Note
. $Created

# command to delete data
$a.DeleteCommand = $c = $DbConnection.CreateCommand()
$c.CommandText = @"
--SQL
DELETE FROM TestNotes
WHERE NoteId = @NoteId
"@
. $NoteId

# command to update data
$a.UpdateCommand = $c = $DbConnection.CreateCommand()
$c.CommandText = @"
--SQL
UPDATE TestNotes
SET Note = @Note, CategoryId = @CategoryId, Created = @Created
WHERE NoteId = @NoteId
"@
. $NoteId
. $CategoryId
. $Note
. $Created

# create a panel, set adapter
$p = New-Object PowerShellFar.DataPanel
$p.Adapter = $a

# data appearance
$p.Panel.Info.Title = 'TestNotes'
$p.Columns = @(
	@{ Type = 'N'; Expression = 'Note'; Width = -80 }
	@{ Type = 'Z'; Expression = 'Category' }
	@{ Type = 'DC'; Expression = 'Created' }
)
$p.ExcludeMembers = 'NoteId', 'CategoryId'

# Setup lookup taking selected CategoryId (to use) and Category (to show);
# there are two alternative examples below:
#
# 1) GENERIC LOOKUP (NOT SIMPLE BUT MORE UNIVERSAL)
# $this - event sender, it is a lookup panel created by Test-Panel-DbCategories-.ps1
# $this.Parent - its parent panel, it is a member panel with TestNotes row fields
# $this.Parent.Value - destination row with CategoryId and Category
# $_ - event arguments (PowerShellFar.FileEventArgs)
# $_.File.Data - lookup row from TestCategories
#
# 2) DATA LOOKUP (SIMPLE BUT ONLY FOR DATA PANELS)
# Actually use this simple way always when possible.
# $this.CreateDataLookup() returns a helper handler that makes all the generic job.

if ($GenericLookup) {
	$p.AddLookup('Category', {
		Test-Panel-DbCategories- -Lookup {&{
			$r1 = $this.Parent.Value
			$r2 = $_.File.Data
			$r1.CategoryId = $r2.CategoryId
			$r1.Category = $r2.Category
		}}
	})
}
else {
	$p.AddLookup('Category', {
		Test-Panel-DbCategories- -Lookup $this.CreateDataLookup(@('Category', 'Category', 'CategoryId', 'CategoryId'))
	})
}

# go!
$p.Show()