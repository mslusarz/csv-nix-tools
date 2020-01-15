---
title: csv-add-concat
section: 1
...

# NAME #

csv-add-concat - add a new column by concatenation of columns and fixed strings

# SYNOPSIS #

**csv-add-concat** [OPTION]... -- new_name = [%name|str]...

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by concatenation of columns and fixed strings.

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

csv-ls -R -c parent,name . | csv-add-concat -s -- full_path = %parent / %name
:    list files with their full path

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
