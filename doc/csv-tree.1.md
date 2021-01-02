<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-tree
section: 1
...

# NAME #

csv-tree - process hierarchical CSV data

# SYNOPSIS #

**csv-tree** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input, build hierarchical structure and print
back data with hierarchical information.

-i, \--indent=*NAME*[,*NEW-NAME*]
:   indent data from column *NAME* and put it in a new column *NEW-NAME*, if *NEW-NAME* is omitted column *NAME* is replaced

-k, \--key=*NAME*
:   use column *NAME* as a unique key identifying each row

-p, \--parent=*NAME*
:   use column *NAME* as a pointer to parent row

-m, \--sum=*NAME*[,*NEW-NAME*]
:   sum data from column *NAME* and put it in a new column *NEW-NAME*, if *NEW-NAME* is omitted column *NAME* is replaced

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -llR /usr/local | csv-tree -k full_path -p parent -i name -m size | csv-cut -c size,name -S`
:   recursively print files and directories from /usr/local in tree structure

`csv-ps | csv-tree -k pid -p ppid -i command -n vm_rss_KiB | csv-cut -c vm_rss_KiB,command -S`
:   print process tree with combined memory usage

# SEE ALSO #

**csv-lstree**(1), **csv-pstree**(1), **csv-nix-tools**(7)
