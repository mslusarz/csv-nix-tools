---
title: csv-add-split
section: 1
...

# NAME #

csv-add-split - add two new columns by splitting another one using a separator

# SYNOPSIS #

**csv-add-split** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by splitting another one using a separator.

-c *NAME*
:   use column *NAME* as an input data

-e *SEPARATOR*
:   use all characters from *SEPARATOR* as separators

-n *NAME1*,*NAME2*
:   create columns *NAME1* and *NAME2* as an output

-r, --reverse
:   start looking for separator from the end of input string

-p  --print-separator=*yes*/*no*/*auto*
:   include separator in 2nd output column (*auto* means *yes* if there's more
than 1 separator and *no* if there's only one), defaults to *auto*

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

csv-ls | csv-add-split -c name -e . -n base,ext -s
:   list files with base and extension in columns base and ext

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
