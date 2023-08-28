<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-add-rev
section: 1
...

# NAME #

csv-add-rev - add a new column by reversing another column characterwise

# SYNOPSIS #

**csv-add-rev** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by reversing another column characterwise.

-c *NAME*
:   use column *NAME* as an input

-n *NEW-NAME*
:   create column *NEW-NAME* as an output

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply the filter to rows only with _table column equal *NAME*

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls | csv-add-rev -c name -n reversed_name -s`
:   list files with names reversed

# SEE ALSO #

**[rev](http://man7.org/linux/man-pages/man1/rev.1.html)**(1),
**csv-show**(1), **csv-nix-tools**(7)
