---
title: csv-grep-rpn
section: 1
...

# NAME #

csv-grep-rpn - filter rows of CSV stream for which RPN expression returns true

# SYNOPSIS #

**csv-grep-rpn** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input, compute RPN expression for each row
and print back to standard output rows for which expression is non-zero.

For full specification of RPN syntax accepted by this tool see **csv-add-rpn**(1).

-e *RPN-EXPR*
:   use expression *RPN-EXPR* to filter; RPN expressions use space as
a separator, so this needs to be quoted

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply the filter to rows only with _table column equal *NAME*

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #
csv-ls -c size,name | csv-grep-rpn -e `"`%size 2000 > %size 3000 < and`"` -s
:   print files whose size is between 2000 and 3000 bytes

csv-ls | csv-grep-rpn -e `"`%name 2 1 substr 'o' ==`"`
:   print files whose second character is 'o'

# SEE ALSO #

**csv-grep**(1), **csv-grep-sql**(1), **csv-add-rpn**(1), **csv-show**(1),
**csv-nix-tools**(7)
