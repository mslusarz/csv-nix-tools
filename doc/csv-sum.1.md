---
title: csv-sum
section: 1
...

# NAME #

csv-sum - take a sum of numerical or string column(s)

# SYNOPSIS #

**csv-sum** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print back to standard output the sum of
values of chosen columns.

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

csv-ls -l | csv-sum -c size
:   print the sum of sizes of all files in the current directory

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
