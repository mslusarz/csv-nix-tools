---
title: csv-merge
section: 1
...

# NAME #

csv-merge - merge multiple CSV streams

# SYNOPSIS #

**csv-merge** [OPTION]...

# DESCRIPTION #

Read multiple CSV streams (from standard input or files)
and print back to standard output a merged CSV with tables.

-N, \--table-name *NAME*
:   set *NAME* as a table name for future table-less file (-p)

-p, \--path-without-table *FILE*
:   read CSV stream without table from *FILE* and use name set by -N as its name; '-' means standard input

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

\--path-with-table *FILE*
:   read CSV stream with table from *FILE*; '-' means standard input

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

csv-ls -c size,name | csv-merge -N file -p -
:   equivalent of "csv-ls -c size,name -T"

csv-users -T | csv-merge \--path-with-table - -N file -p files.csv
:   produces CSV stream with tables from 2 streams, one with a table
(csv-users -T), and one without (files.csv)

# SEE ALSO #

**csv-tables-tut**(7), **csv-sqlite**(1), **csv-show**(1), **csv-nix-tools**(7)
