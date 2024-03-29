
# ColorerPack - syntax and color schemes for Colorer

- markdown.hrc
- [powershell.hrc](#powershell)
- [r.hrc](#r)
- [visual.hrd](#visual)
- [History](#History)

*********************************************************************
## powershell.hrc {#powershell}

Windows PowerShell syntax scheme. It is designed for PowerShell 2.0 and above.
In addition to standard language it recognizes some useful external features.

Together with *visual.hrd* it adds some colors to PowerShellFar console, the
Far Manager editor which works somewhat similar to PowerShell console.

### Outlined regions

- Functions and filters;
- Triple-hash line comments `###`;
- `task` entries (DSL of *Invoke-Build*, *psake*).

### Regex syntax

Regex syntax is automatically colored in string literals following the regex
type shortcut `[regex]` and regex operators (`-match`, `-replace`, ...):

<pre>
<span style='color:#008080'>[regex]</span><span style='color:#ff0000'>'(</span><span style='color:#ff0000'>?</span><span style='color:#0000ff'>i</span><span style='color:#ff0000'>)</span><span style='color:#0000ff'>^</span><span style='color:#800000'>text</span><span style='color:#0000ff'>$</span><span style='color:#ff0000'>'</span>
<span style='color:#0000ff'>-match</span> <span style='color:#ff0000'>'(</span><span style='color:#ff0000'>?</span><span style='color:#0000ff'>i</span><span style='color:#ff0000'>)</span><span style='color:#0000ff'>^</span><span style='color:#800000'>text</span><span style='color:#0000ff'>$</span><span style='color:#ff0000'>'</span>
<span style='color:#0000ff'>-replace</span> <span style='color:#ff0000'>'(</span><span style='color:#ff0000'>?</span><span style='color:#0000ff'>i</span><span style='color:#ff0000'>)</span><span style='color:#0000ff'>^</span><span style='color:#800000'>text</span><span style='color:#0000ff'>$</span><span style='color:#ff0000'>'</span>
</pre>

In addition to natural recognition regex syntax is colored in strings after the
conventional comment `<#regex#>`. The example also shows use of a here-string:

<pre>
<span style='color:#008000'>&lt;#regex#></span><span style='color:#ff0000'>@'</span>
<span style='color:#ff0000'>(?</span><span style='color:#0000ff'>ix</span><span style='color:#ff0000'>)</span>
<span style='color:#0000ff'>^</span><span style='color:#800000'> text1 </span><span style='color:#008000'>  # comment1</span>
<span style='color:#800000'> text2 </span><span style='color:#0000ff'>$</span><span style='color:#008000'>  # comment2</span>
<span style='color:#ff0000'>'@</span>
</pre>

Note useful inline regex options (see .NET manuals for the others):

- `(?i)` - *IgnoreCase* (remember, .NET regex is case sensitive by default);
- `(?x)` - *IgnorePatternWhitespace* (for multiline regex, inline comments, ...).

### SQL syntax

SQL syntax is colored in here-strings (`@'...'@`, `@"..."@`) after the
conventional comment `<#sql#>`:

<pre>
<span style='color:#008000'>&lt;#sql#></span><span style='color:#ff0000'>@'</span>
<span style='color:#0000ff'>SELECT</span> <span style='color:#0000ff'>*</span>
<span style='color:#0000ff'>FROM</span> table1
<span style='color:#0000ff'>WHERE</span> data1 <span style='color:#0000ff'>=</span> <span style='color:#ff0000'>@param1</span>
<span style='color:#ff0000'>'@</span>
</pre>

### PSF console output

- Output numbers, dates, times, and paths are colored.
- Output area markers `<=` and `=>` are invisible.
- Output areas have light gray background.

*********************************************************************
## r.hrc {#r}

R syntax scheme. R is a programming language and software environment for
statistical computing and graphics.
See [R project home page](http://www.r-project.org/).

### Features

- Outlined functions.
- Outlined triple-hash comments `###`.
- TODO, BUG, FIX, web-addresses, etc. in other comments.

The scheme covers most of R syntax quite well.
This code snippet highlights some typical features:

<pre>
<span style='color:#008000'>### Draws simple quantiles/ECDF</span>
<span style='color:#008000'># R home page: </span><span style='color:#0000ff'>http://www.r-project.org/</span>
<span style='color:#008000'># </span><span style='color:#800000; background:#c0c0c0; '>!!</span><span style='color:#008000'> see ecdf() {library(stats)} for a better example</span>
<span style='color:#000000'>plot</span><span style='color:#0000ff'>(</span><span style='color:#000000'>x</span> <span style='color:#0000ff'>&lt;-</span> <span style='color:#000000'>sort</span><span style='color:#0000ff'>(</span><span style='color:#000000'>rnorm</span><span style='color:#0000ff'>(</span><span style='color:#800080'>47</span><span style='color:#0000ff'>))</span><span style='color:#0000ff'>,</span> <span style='color:#000000'>type</span> <span style='color:#0000ff'>=</span> <span style='color:#ff0000'>"</span><span style='color:#800000'>s</span><span style='color:#ff0000'>"</span><span style='color:#0000ff'>,</span> <span style='color:#000000'>main</span> <span style='color:#0000ff'>=</span> <span style='color:#ff0000'>"</span><span style='color:#800000'>plot(x, type = </span><span style='color:#800000; background:#ffff00; '>\"</span><span style='color:#800000'>s</span><span style='color:#800000; background:#ffff00; '>\"</span><span style='color:#800000'>)</span><span style='color:#ff0000'>"</span><span style='color:#0000ff'>)</span>
<span style='color:#000000'>points</span><span style='color:#0000ff'>(</span><span style='color:#000000'>x</span><span style='color:#0000ff'>,</span> <span style='color:#000000'>cex</span> <span style='color:#0000ff'>=</span> <span style='color:#800080'>.5</span><span style='color:#0000ff'>,</span> <span style='color:#000000'>col</span> <span style='color:#0000ff'>=</span> <span style='color:#ff0000'>"</span><span style='color:#800000'>dark red</span><span style='color:#ff0000'>"</span><span style='color:#0000ff'>)</span>
</pre>

*********************************************************************
## visual.hrd {#visual.hrd}

***visual.hrd***

Console color scheme with white background. It is initially called *visual* for
similarity to Visual Studio default colors used for some file types. It is also
designed to support *powershell.hrc* features and to customize appearance of
other schemes.

Another visual.hrd feature is colored console color codes in HRD files.

As far as color schemes are all about personal preferences, one may use this
scheme as the example in order to build an own scheme with similar features.

***visual-rgb.hrd***

RGB color scheme generated from *visual.hrd* with RGB values for standard
console colors. This scheme can be used with the *colorer.exe* in order to
create HTML files with the same colors as they are in the Far Manager editor
with *visual.hrd*.

*********************************************************************
## History {#History}

### 2012-05-06

*powershell.hrc*

- Corrected use of the operator `@`.
- Fixed `|` in the end of line comments.
- Minor tweaks, improved performance (~5%).

### 2012-04-28

*powershell.hrc*

- Reviewed list of known cmdlets (V3 included).

### 2012-04-22

*powershell.hrc*

- Fixed the region of `.` on invocation of script blocks and expressions.
- Amended syntax after `break continue throw return exit`.
- Amended syntax of redirection including V3 new features.
- Dropped outlined global/script variable assignment.
- Amended parameter list, outlined parameters.
- Added operators `-in -notin` (V3).
- Refactoring and optimization.

### 2012-04-20

*powershell.hrc*

- Many major improvements, PowerShell syntax parsing is almost fair now. At the
  same time the scheme is simpler and it works about 25% faster.
- Resolved most of known issues, both recently added and very old. Other known
  issues are rather exotic, some of them will be fixed later, some never.
- "Breaking" change: no spaces are allowed between conventional comments
  `<#sql#> <#regex#>` and following string edges.

### 2012-04-19

*powershell.hrc*

- Colored optional elements of `switch` and `data`.
- Fixed many of recently added known issues, not all yet.

### 2012-04-18

*powershell.hrc*

- All commands are fully parsed. This is the major improvement and the code
  looks more informative, at least with *visual.hrd*. There are a few known
  cases of scheme confusion, nothing serious. To be continued.
- New colors and areas: Argument, HashKey, StringEdge.
- Amended function parameter list, hash tables, etc.

*visual.hrd*

- def:TODO colors are inverted comment colors (look more comfortable).

### 2012-04-17

*powershell.hrc*

- Commands invoked by `& .` are fully parsed and colored (command name,
  parameters, and arguments are different things now). True parsing of all
  commands is coming soon.
- Improved parsing and colors of script parameter attributes.
- Amended parsing of hash literals.

### 2012-04-15

*powershell.hrc*

- Added member access and invocation operators `. ::`.
- Corrected some ambiguous operator-or-literal cases.
- Colored loop labels in `break continue`.

### 2012-04-14

*powershell.hrc*

- Amended `switch` block. In particular:
    - fixed use of `default` as a keyword
    - fixed context of inner blocks
- Simplified `data` block.

### 2012-04-12

*powershell.hrc*

- Optimized by eating spaces (about 20% faster).

### 2012-04-12

*powershell.hrc*

- Improved syntax of `do, for, foreach, while, function, filter, workflow, switch` blocks.
- Corrected and improved syntax of `data` sections (still simplified).
- Added `dynamicparam` block and auto-variable `$PSItem`.
- Removed not really useful reserved keywords.
- Simplified code related to improved blocks.
- Fixed a minor defect in loop labels.

*r.hrc*

- Amended outlined comments `###`.
- Optimized by eating spaces.

### 2012-04-11

*powershell.hrc*

- Fixed case sensitivity of `in` in `foreach`.
- Resolved false keyword colors in many cases.
- Improved `param, begin, process, end, if, else, elseif, try, catch, finally,
  trap` blocks: multiline syntax including comments, dual start/end pairs, etc.

### 2012-04-09

*powershell.hrc*

- Added `, ; & ::` operators and V3 redirection operators.
- Colored attributes, some loop labels, variable prefixes.
- Fixed `if`, `for`, ... after `param()`.
- Improved:
    - `switch`
    - `foreach( in )`
    - here-string error areas
    - type syntax, including generics and arrays
    - name patterns of function, filter, data, task

### 2012-04-01

*r.hrc*

- A new scheme of the programming language R.
<http://en.wikipedia.org/wiki/R_(programming_language)>

*powershell.hrc*

- Added yet missing operator `!`.
- Added reserved operators `&&`, `||` (as errors).

### 2012-03-29

*About-ColorerPack*

- Explained *powershell.hrc* features, including new.

*powershell.hrc*

Regex

- Regex syntax in regex operands: also with here-strings.
- Regex syntax in strings after `[regex]` and `<#regex#>`.
- Removed the old trick with `#REGEX` in here-strings.

SQL

- SQL syntax in here-strings after `<#sql#>`
- Removed the old trick with `--SQL`.

*visual.hrd*

- `def:StringContent` (commonly used for escaping). Background is highlighted,
  foreground is inherited from the context, this seems to be more informative.
- `xml:CData` color changed from `def:default`.
- `c:PreprocInclude` color is VS color.

### 2012-12-01

*powershell.hrc*

- Added workflow specific keywords and basic features. There is no special
  context, they also work outside workflows.
- Parameters are not outlined anymore. Latest PowerShellFar provides
  tab-expansion of script variables including parameters.

### 2013-01-07

Included *markdown.hrc*.

### 2013-01-18

*powershell.hrc*. Fixed `try, catch, finally` after `=`.

### 2013-01-26

*powershell.hrc*. Switch case conditions are arguments.

### 2013-01-27

*powershell.hrc*. cmdlet-binding fixes `[CmdletBinding()] param()`.

### 2013-11-14

*powershell.hrc*. Added operators `-shl`, `-shr`.
