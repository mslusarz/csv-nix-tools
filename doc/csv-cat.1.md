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

-s, --show
:   pipe output to **csv-show**(1)

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLE #

csv-cat file1.csv - file2.csv
:   output file1.csv contents, the standard input, then file2.contents

# SEE ALSO #

**cat**(1), **csv-tac**(1), **csv-show**(1)
