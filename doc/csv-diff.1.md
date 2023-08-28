<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-diff
section: 1
...

# NAME #

csv-diff - compare CSV files row by row

# SYNOPSIS #

**csv-diff** [OPTION]... FILE1 FILE2 [FILE3] [FILE4]...

# DESCRIPTION #

Read CSV streams from 2 or more files, merge them with an indication of
differences in values and print resulting CSV file to standard output.

All files must have the same number of rows and the list of columns must be
a superset of the first file or the list specified by \--columns option.

-c, \--columns=*NAME1*[,*NAME2*...]
:   select only these columns

-C, \--colors=[0|1]
:   add color columns (auto enabled with \--show-full)

-d, \--diffs=[0|1]
:   add diff columns (auto enabled with \--show)

-q, \--brief
:   report only when files differ

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

\--all-rows
:   show all rows

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

## Exit status: ##

 - 0 if inputs are the same
 - 1 if inputs are different
 - 2 if there was any trouble

# EXAMPLES #

`csv-diff file1.csv file2.csv -S`
:   compare files, add _color columns and pipe everything to csv-show with colors enabled

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
