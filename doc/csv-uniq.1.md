---
title: csv-uniq
section: 1
...

# NAME #

csv-uniq - merge adjacent duplicate rows of CSV file

# SYNOPSIS #

**csv-uniq** [OPTION]...

# DESCRIPTION #

XXX

-c, --columns=*NAME1*[,*NAME2*...]
:   use these columns

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

# EXAMPLE #

csv-ls -c owner_name | csv-uniq -c owner_name | csv-sort -c owner_name | csv-uniq -c owner_name
:   list owners of all files in the current directory

# SEE ALSO #

**uniq**(1), **csv-show**(1), **csv-nix-tools**(7)
