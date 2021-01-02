<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-lstree
section: 1
...

# NAME #

csv-lstree - list files in hierachical manner

# SYNOPSIS #

**csv-lstree** [OPTION]... [FILE]...

# DESCRIPTION #

Recursively list FILEs (starting from current directory by default).

-f, \--full-path
:   add parent directory prefix to file names

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

# EXAMPLES #

`csv-lstree`

`csv-lstree -s /usr/local`

# SEE ALSO #

**csv-ls**(1), **csv-tree**(1), **csv-show**(1), **csv-nix-tools**(7)
