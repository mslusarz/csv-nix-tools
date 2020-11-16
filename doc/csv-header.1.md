<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-header
section: 1
...

# NAME #

csv-header - process header of a CSV file

# SYNOPSIS #

**csv-header** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print back to standard output with
the column header transformed.

-e, \--set-types *NAME1*:*TYPE1*[,*NAME2*:*TYPE2*...]
:   set type of column *NAME1* to *TYPE1*, type of column *NAME2* to *TYPE2*, etc.

-G, \--guess-types
:   add types to columns by guessing based on their contents

-M, \--remove-types
:   remove types from columns

-m, \--remove
:   remove column header

-n, \--rename *NAME*,*NEW-NAME*
:   rename column *NAME* to *NEW-NAME*

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls | csv-count -r | csv-header -m`
:   print number of files in the current directory without csv header

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
