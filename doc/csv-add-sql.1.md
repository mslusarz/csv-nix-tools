---
title: csv-add-sql
section: 1
...

# NAME #

csv-add-sql - add a new column from SQL expression

# SYNOPSIS #

**csv-add-sql** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by evaluation of SQL expression.

-n *NEW-NAME*
:   create column *NEW-NAME* as an output

-e *SQL-EXPR*
:   use expression *SQL-EXPR* to create new column; SQL expressions use space
as a separator, so this needs to be quoted

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

-T, --table=*NAME*
:   apply to rows only with _table column equal *NAME*

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLES #

csv-ls -c name,size,blocks | csv-add-sql -e "blocks * 512 AS space_used" -s
:   list files and real space they use

# SEE ALSO #

**csv-sql**(1), **csv-grep-sql**(1), **csv-show**(1), **csv-nix-tools**(7)
