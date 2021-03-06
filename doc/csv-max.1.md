<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-max
section: 1
...

# NAME #

csv-max - take a maximum value of numerical or string column(s)

# SYNOPSIS #

**csv-max** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print back to standard output maximum
values of chosen columns.

-c, \--columns=*NAME1*[,*NAME2*...]
:   use these columns

-n *NEW-NAME1*[,*NEW-NAME2*]
:   create columns with these names, instead of default max(NAME)

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

`csv-ls -l | csv-max -c size`
:   print the biggest file in the current directory

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
