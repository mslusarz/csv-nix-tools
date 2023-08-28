<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-sort
section: 1
...

# NAME #

csv-sort - sort CSV file by column(s)

# SYNOPSIS #

**csv-sort** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input, sort it by chosen column and print
resulting file to standard output.

-c, \--columns=*NAME1*[,*NAME2*...]
:   sort first by column *NAME1*, then *NAME2*, etc.

-r, \--reverse
:   sort in descending order

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply to rows only with _table column equal *NAME*

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -c name,size | csv-sort -c size -r -s`
:   print files, sorted by size, in descending order

# SEE ALSO #

**[sort](http://man7.org/linux/man-pages/man1/sort.1.html)**(1),
**csv-show**(1), **csv-nix-tools**(7)
