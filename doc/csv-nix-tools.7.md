---
title: csv-nix-tools
section: 7
...

# NAME #

csv-nix-tools - collection of tools for gathering and processing system information

# DESCRIPTION #

- XXX format description
- XXX types
- XXX labeled streams

# SOURCE TOOLS #

- **csv-group-members**(1)
- **csv-groups**(1)
- **csv-ls**(1)
- **csv-netstat**(1)
- **csv-ps**(1)
- **csv-users**(1)

# PROCESSING TOOLS #

- **csv-avg**(1)
- **csv-cat**(1) - concatenate CSV files and print on the standard output
- **csv-concat**(1)
- **csv-count**(1)
- **csv-cut**(1)
- **csv-exec-add**(1)
- **csv-grep**(1)
- **csv-head**(1) - print on the standard output the beginning of CSV file from standard input
- **csv-header**(1)
- **csv-max**(1)
- **csv-merge**(1)
- **csv-min**(1)
- **csv-replace**(1)
- **csv-rpn-add**(1)
- **csv-rpn-filter**(1)
- **csv-sort**(1)
- **csv-split**(1)
- **csv-sql**(1)
- **csv-sqlite**(1)
- **csv-substring**(1)
- **csv-sum**(1)
- **csv-tac**(1) - concatenate CSV files and print on the standard output in reverse
- **csv-tail**(1) - print on the standard output the end of CSV file from standard input
- **csv-uniq**(1)

# SINK TOOLS #

- **csv-exec**(1)
- **csv-show**(1)

# COMMON OPTIONS FOR ALL SOURCE TOOLS #

-f, --fields=name1[,name2...]
:   XXX

-L, --label=label
:   XXX

-M, --merge-with-stdin
:   XXX

# COMMON OPTIONS FOR ALL SOURCE AND PROCESSING TOOLS #

-s, --show
:   pipe output to **csv-show**(1)

# COMMON OPTIONS FOR ALL TOOLS #

--help
:   display this help and exit

--version
:   output version information and exit

# SEE ALSO #

**<https://github.com/mslusarz/csv-nix-tools>**
