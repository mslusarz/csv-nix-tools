<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2019-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-tail
section: 1
...

# NAME #

csv-tail - print on the standard output the end of a CSV file from standard input

# SYNOPSIS #

**csv-tail** [OPTION]...

# DESCRIPTION #

Print the last 10 rows of CSV file from standard input to standard output.

-n, \--lines=*NUM*
:   print the last *NUM* rows instead of the last 10, *NUM* must be >= 0

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-tail < file1.csv`
:   output the last 10 rows of file1.csv

`csv-tail -n 5 < file1.csv`
:   output the last 5 rows of file1.csv

# SEE ALSO #

**[tail](http://man7.org/linux/man-pages/man1/tail.1.html)**(1),
**csv-head**(1), **csv-show**(1), **csv-nix-tools**(7)
