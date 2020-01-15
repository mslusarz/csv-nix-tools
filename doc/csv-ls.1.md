---
title: csv-ls
section: 1
...

# NAME #

csv-ls - list files in CSV format

# SYNOPSIS #

**csv-ls** [OPTION]... [FILE]...

# DESCRIPTION #

List information about the FILEs (the current directory by default).
Sort entries alphabetically if -U is not specified.

-a, --all
:   do not ignore entries starting with .

-c, --columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-d, --directory
:   list directories themselves, not their contents

-l
:   use a longer listing format (can be used up to 3 times)

-M, --merge
:   merge output with a CSV stream in table form from standard input

-N, --table-name *NAME*
:   produce output as table *NAME*

-R, --recursive
:   list subdirectories recursively

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

-T, --as-table
:   produce output as table *file*

-U
:   do not sort; list entries in directory order

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLES #

csv-ls
:   print files (just names) in the current directory

csv-ls -l -s
:   print files in the current directory, in a format similar to ls -l

# SEE ALSO #

**ls**(1), **csv-show**(1), **csv-nix-tools**(7)
