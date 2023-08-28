<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-pstree
section: 1
...

# NAME #

csv-pstree - list processes in hierarchical manner

# SYNOPSIS #

**csv-pstree** [OPTION]... [FILE]...

# DESCRIPTION #

List processes in hierarchical manner.

-c *NAME1*[,*NAME2*...]
:   choose the list of columns

-f *PID*
:   print only process *PID* and its descendants

-L
:   add 'level' column

-s
:   print output in table format

-S
:   print output in table format with pager

-X
:   disable printing of type names in column names

# EXAMPLES #

`csv-pstree -S`

`csv-pstree -f 2 -S`

`csv-pstree -f 1 | csv-exec kill %pid`

`csv-pstree -c pid,command -S`

# SEE ALSO #

**csv-ps**(1), **csv-tree**(1), **csv-show**(1), **csv-nix-tools**(7)
