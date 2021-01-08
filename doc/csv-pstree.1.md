<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

-f *PID*
:   print only process *PID* and its descendants

-s
:   print output in table format

-S
:   print output in table format with pager

# EXAMPLES #

`csv-pstree -s`

`csv-pstree -f 2 -s`

`csv-pstree -f 1 | csv-exec kill %pid`

# SEE ALSO #

**csv-ps**(1), **csv-tree**(1), **csv-show**(1), **csv-nix-tools**(7)
