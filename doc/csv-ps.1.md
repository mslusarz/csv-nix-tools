---
title: csv-ps
section: 1
...

# NAME #

csv-ps - list processes in CSV format

# SYNOPSIS #

**csv-ps** [OPTION]...

# DESCRIPTION #

Print to standard output the list of system processes in the CSV format.

-c, --columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-l
:   use a longer listing format (can be used up to 4 times)

-M, --merge
:   merge output with a CSV stream in table form from standard input

-N, --table-name *NAME*
:   produce output as table *NAME*

-p, --pid=PID1[,PID2...]
:   select processes from this list

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

-T, --as-table
:   produce output as table *proc*

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLES #

csv-ps -S
:   browse all processes with custom pager

# SEE ALSO #

**ps**(1), **csv-show**(1), **csv-nix-tools**(7)
