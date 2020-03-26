---
title: csv-head
section: 1
...

# NAME #

csv-head - print on the standard output the beginning of a CSV file from standard input

# SYNOPSIS #

**csv-head** [OPTION]...

# DESCRIPTION #

Print the first 10 rows of CSV file from standard input to standard output.

-n, \--lines=*NUM*
:   print the first *NUM* rows instead of the first 10, *NUM* must be >= 0

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-head < file1.csv`
:   output the first 10 rows of file1.csv

`csv-head -n 5 < file1.csv`
:   output the first 5 rows of file1.csv

# SEE ALSO #

**[head](http://man7.org/linux/man-pages/man1/head.1.html)**(1),
**csv-tail**(1), **csv-show**(1), **csv-nix-tools**(7)
