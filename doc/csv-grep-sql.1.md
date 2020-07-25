<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-grep-sql
section: 1
...

# NAME #

csv-grep-sql - filter rows of CSV stream for which SQL expression returns true

# SYNOPSIS #

**csv-grep-sql** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input, compute SQL expression for each row
and print back to standard output rows for which expression is non-zero.

For full specification of SQL syntax accepted by this tool see **csv-sql**(1).

-e *SQL-EXPR*
:   use expression *SQL-EXPR* to filter; SQL expressions use space as
a separator, so this needs to be quoted

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply the filter to rows only with _table column equal *NAME*

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -c size,name | csv-grep-sql -e "size > 2000 and size < 3000" -s`
:   print files whose size is between 2000 and 3000 bytes

`csv-ls | csv-grep-sql -e "substr(name, 2, 1) == 'o'"`
:   print files whose second character is 'o'

# SEE ALSO #

**csv-sql**(1), **csv-grep**(1), **csv-grep-rpn**(1), **csv-show**(1),
**csv-nix-tools**(7)
