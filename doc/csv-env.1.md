<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-env
section: 1
...

# NAME #

csv-env - list environment variables in CSV format

# SYNOPSIS #

**csv-env** [OPTION]...

# DESCRIPTION #

Print to standard output the list of environment variables in the CSV format.

-c, \--columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-M, \--merge
:   merge output with a CSV stream in table form from standard input

-N, \--table-name *NAME*
:   produce output as table *NAME*

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--as-table
:   produce output as table *env*

-X, \--no-types
:   disable printing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# COLUMNS #

| name  | type   |
|-------|--------|
| name  | string |
| value | string |

# EXAMPLES #

`csv-env -s`
:   print environment variables

# SEE ALSO #

**[printenv](http://man7.org/linux/man-pages/man1/printenv.1.html)**(1),
**csv-show**(1), **csv-nix-tools**(7)
