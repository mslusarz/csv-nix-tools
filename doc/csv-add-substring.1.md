<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-add-substring
section: 1
...

# NAME #

csv-add-substring - add a new column by extracting a substring of another column

# SYNOPSIS #

**csv-add-substring** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by extracting substring of another column.

-c *NAME*
:   use column *NAME* as an input data

-n *NEW-NAME*
:   create column *NEW-NAME* as an output

-p *START-POS*
:   start from position *START-POS*; first character has position 1; negative
value mean starting from the end of string

-l *LENGTH*
:   take *LENGTH* characters from string; must not be negative

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply to rows only with _table column equal *NAME*

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -c name,mtime | csv-add-substring -c mtime -n myear -p 1 -l 4 -s`
:   list files and year they were last modified

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
