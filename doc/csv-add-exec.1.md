<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-add-exec
section: 1
...

# NAME #

csv-add-exec - add a new column by executing an external command

# SYNOPSIS #

**csv-add-exec** [OPTION]... \-- command

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by reading standard output of an external command whose
standard input is fed with input column.

-c, \--column=*NAME*
:   use column *NAME* as an input

-n, \--new-name=*NAME*
:   create column *NAME* as an output

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

`csv-ls -c full_path | csv-add-exec -s -c full_path -n new -- sed 's/.c$/.o/'`
:   list files; if file has .c extension it will be replaced by .o extension

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
