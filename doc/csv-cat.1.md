---
title: csv-cat
section: 1
...

# NAME #

csv-cat - concatenate CSV files and print on the standard output

# SYNOPSIS #

**csv-cat** [OPTION]... [FILE]...

# DESCRIPTION #

Concatenate CSV FILE(s) to standard output. With no FILE, or when FILE is -,
read standard input.

All files must have the same columns (number, names, and types), but their
order can be different between files. csv-cat chooses the order of the first
input file.

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

csv-cat file1.csv - file2.csv
:   output file1.csv contents, the standard input, then file2.contents

# SEE ALSO #

**[cat](http://man7.org/linux/man-pages/man1/cat.1.html)**(1),
**csv-tac**(1), **csv-show**(1), **csv-nix-tools**(7)
