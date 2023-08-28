<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-avg
section: 1
...

# NAME #

csv-avg - take an average of numerical column(s)

# SYNOPSIS #

**csv-avg** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print back to standard output averages
of chosen columns.

-c, \--columns=*NAME1*[,*NAME2*...]
:   use these columns

-n *NEW-NAME1*[,*NEW-NAME2*]
:   create columns with these names, instead of default avg(NAME)

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

`csv-ls -l | csv-avg -c size`
:   print average size of file in the current directory

# LIMITATIONS #

Partial sums of each column must be within -2^63-1 .. 2^63 range.

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
