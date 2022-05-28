<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021-2022, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-lstree
section: 1
...

# NAME #

csv-lstree - list files in hierarchical manner

# SYNOPSIS #

**csv-lstree** [OPTION]... [FILE]...

# DESCRIPTION #

Recursively list FILEs (starting from current directory by default).

-c *NAME1*[,*NAME2*...]
:   choose the list of columns

-f
:   add parent directory prefix to file names

-L
:   add 'level' column

-s
:   print output in table format

-S
:   print output in table format with pager

# EXAMPLES #

`csv-lstree`

`csv-lstree -s /usr/local`

`csv-lstree -L ~ | csv-grep-sql -e 'level < 2' | csv-cut -c size,name -S`

# SEE ALSO #

**csv-ls**(1), **csv-tree**(1), **csv-show**(1), **csv-nix-tools**(7)
