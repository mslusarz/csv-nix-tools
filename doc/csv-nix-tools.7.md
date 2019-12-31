---
title: csv-nix-tools
section: 7
...

# NAME #

csv-nix-tools - collection of tools for gathering and processing system information

# DESCRIPTION #

- XXX format description
- XXX types
- XXX streams with tables

# SOURCE TOOLS #

- **csv-group-members**(1)
- **csv-groups**(1)
- **csv-ls**(1)
- **csv-netstat**(1)
- **csv-ps**(1)
- **csv-users**(1)

# PROCESSING TOOLS #

- **csv-add-concat**(1)
- **csv-add-exec**(1)
- **csv-add-replace**(1)
- **csv-add-rpn**(1)
- **csv-add-split**(1)
- **csv-add-substring**(1)
- **csv-avg**(1)
- **csv-cat**(1) - concatenate CSV files and print on the standard output
- **csv-count**(1)
- **csv-cut**(1)
- **csv-grep**(1)
- **csv-grep-rpn**(1)
- **csv-head**(1) - print on the standard output the beginning of a CSV file from standard input
- **csv-header**(1)
- **csv-max**(1)
- **csv-merge**(1)
- **csv-min**(1)
- **csv-sort**(1)
- **csv-sql**(1)
- **csv-sqlite**(1)
- **csv-sum**(1)
- **csv-tac**(1) - concatenate CSV files and print on the standard output in reverse
- **csv-tail**(1) - print on the standard output the end of a CSV file from standard input
- **csv-uniq**(1)

# SINK TOOLS #

- **csv-exec**(1)
- **csv-show**(1) - print on the standard output a CSV file from standard input in human-readable format

# COMMON OPTIONS FOR ALL SOURCE TOOLS #

-c, --columns=name1[,name2...]
:   choose the list of columns

-M, --merge
:   XXX

-N, --table-name=name
:   XXX

-T, --as-table
:   XXX

# COMMON OPTIONS FOR ALL SOURCE AND PROCESSING TOOLS #

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

# COMMON OPTIONS FOR ALL TOOLS #

--help
:   display this help and exit

--version
:   output version information and exit

# SEE ALSO #

**<https://github.com/mslusarz/csv-nix-tools>**
