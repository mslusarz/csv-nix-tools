<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

-f
:   add parent directory prefix to file names

-s
:   print output in table format

-S
:   print output in table format with pager

# EXAMPLES #

`csv-lstree`

`csv-lstree -s /usr/local`

# SEE ALSO #

**csv-ls**(1), **csv-tree**(1), **csv-show**(1), **csv-nix-tools**(7)
