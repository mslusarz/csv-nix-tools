<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-cut
section: 1
...

# NAME #

csv-cut - remove and/or reorder columns in CSV stream

# SYNOPSIS #

**csv-cut** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input, remove and/or reorder columns and print
resulting file to standard output.

-c, \--columns=*NAME1*[,*NAME2*...]
:   select only these columns

-r, \--reverse
:   apply \--columns filter in reverse, removing only selected columns

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply the filter to columns only with "NAME." prefix

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -l | csv-cut -c name,size -s`
:   list files and their sizes

# SEE ALSO #

**[cut](http://man7.org/linux/man-pages/man1/cut.1.html)**(1),
**csv-show**(1), **csv-nix-tools**(7)
