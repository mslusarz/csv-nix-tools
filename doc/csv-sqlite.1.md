<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-sqlite
section: 1
...

# NAME #

csv-sqlite - process CSV input data using SQLite

# SYNOPSIS #

**csv-sqlite** [OPTION]... sql-query

# DESCRIPTION #

Read CSV stream from standard input, load it into memory-backed sqlite database,
execute an SQL query and print back to standard output its result.

-i *FILE*
:   ignore standard input and read from *FILE* instead; can be used multiple times; '-' means standard input

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--use-tables
:   interpret input as "table" stream (as _table column and columns with
"table." prefixes) and import each csv table into its own sql table

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -c size,name | csv-sqlite "select size, name from input where size > 2000 and size < 3000" -s`
:    print files whose size is between 2000 and 3000 bytes

`csv-users -T | csv-groups -M -N grp | csv-sqlite -T "select user.name as user_name, grp.name as group_name from user, grp where user.gid = grp.gid"`
:   print all system users and the name of the default group they belong to

# SEE ALSO #

**[sqlite3](https://linux.die.net/man/1/sqlite3)**(1),
**<https://www.sqlite.org/lang.html>**,
**csv-sql**(1), **csv-add-sql**(1), **csv-grep-sql**(1),
**csv-show**(1), **csv-nix-tools**(7)
