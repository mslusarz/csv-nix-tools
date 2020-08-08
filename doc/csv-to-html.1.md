<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-to-html
section: 1
...

# NAME #

csv-to-html - convert CSV file from standard input to HTML

# SYNOPSIS #

**csv-to-html** [OPTION]...

# DESCRIPTION #

Convert CSV file from standard input to HTML.

-C, \--use-color-columns
:   use columns with _color suffix to render each cell

\--datatables
:   use datatables.net JavaScript library

\--no-header
:   remove column headers

\--set-color *COLNAME*:[fg=]*COLOR1*[,bg=*COLOR2*]
:   set *COLOR1* as foreground and *COLOR2* as background of column *COLNAME*

\--tabulator
:   use tabulator.info JavaScript library

\--with-types
:   print types in column headers

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-to-html < file1.csv > file1.html && xdg-open file1.html`
:   generate file1.html from file1.csv and open it in the default browser

# SEE ALSO #

**csv-show**(1), **csv-to-json**(1), **csv-to-xml**(1), **csv-nix-tools**(7)
