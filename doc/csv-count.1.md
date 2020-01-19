---
title: csv-count
section: 1
...

# NAME #

csv-count - count the number of columns and/or rows

# SYNOPSIS #

**csv-count** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print back to standard output
the number of columns or rows.

-c, --columns
:   print number of columns

-r, --rows
:   print number of rows

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLES #

csv-ls | csv-count -r
:   print number of files in the current directory

# SEE ALSO #

**[wc](http://man7.org/linux/man-pages/man1/wc.1.html)**(1),
**csv-show**(1), **csv-nix-tools**(7)
