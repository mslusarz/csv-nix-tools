---
title: csv-uniq
section: 1
...

# NAME #

csv-uniq - merge adjacent duplicate rows of CSV file

# SYNOPSIS #

**csv-uniq** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input, remove adjacent duplicate rows and
print back to standard output the remaining rows.

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

# EXAMPLES #

csv-ls -c owner_name | csv-uniq -c owner_name | csv-sort -c owner_name | csv-uniq -c owner_name
:   list owners of all files in the current directory

# SEE ALSO #

**[uniq](http://man7.org/linux/man-pages/man1/uniq.1.html)**(1),
**csv-show**(1), **csv-nix-tools**(7)
