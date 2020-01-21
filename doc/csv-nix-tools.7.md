---
title: csv-nix-tools
section: 7
...

# NAME #

csv-nix-tools - collection of tools for gathering and processing system information

# DESCRIPTION #

**csv-nix-tools** is a collection of tools for gathering and processing system
information using CSV (with minor extensions) as a intermediate format.

The extensions are:

- each column has a type, encoded in the name using name:type syntax
- optional "_table" column having special meaning

Currently used types are: string, int, string[], int[], although array types
are not well supported at the moment (they are produced by some tools, but
there's no easy way to process them).

Column "_table" resolves a problem of storing multiple sets of data in one
CSV stream. It allows:

- merging of data from multiple sources,
- processing of data from one table at the time,
- combining data from multiple tables, based on relation between them

See **csv-tables-tut**(7) for introduction into tables support in csv-nix-tools.

# SOURCE TOOLS #

- **csv-env**(1) - list environment variables
- **csv-group-members**(1) - list system groups and users that belong to them
- **csv-groups**(1) - list system groups
- **csv-ls**(1) - list files
- **csv-netstat**(1) - list network connections
- **csv-ps**(1) - list processes
- **csv-users**(1) - list system users

# PROCESSING TOOLS #

- **csv-add-concat**(1) - add a new column by concatenation of columns and fixed strings
- **csv-add-exec**(1) - add a new column by executing an external command
- **csv-add-replace**(1) - add a new column by performing a string substitution on another column
- **csv-add-rev**(1) - add a new column by reversing another column characterwise
- **csv-add-rpn**(1) - add a new column from RPN expression
- **csv-add-split**(1) - add two new columns by splitting another one using a separator
- **csv-add-sql**(1) - add a new column from SQL expression
- **csv-add-substring**(1) - add a new column by extracting a substring of another column
- **csv-avg**(1) - take an average of numerical column(s)
- **csv-cat**(1) - concatenate CSV files and print on the standard output
- **csv-count**(1) - count the number of columns and/or rows
- **csv-cut**(1) - remove and/or reorder columns
- **csv-grep**(1) - filter rows matching a pattern
- **csv-grep-rpn**(1) - filter rows for which RPN expression returns true
- **csv-grep-sql**(1) - filter rows for which SQL expression returns true
- **csv-head**(1) - print on the standard output the beginning of a CSV file from standard input
- **csv-header**(1) - process header of a CSV file
- **csv-max**(1) - take a maximum value of numerical or string column(s)
- **csv-merge**(1) - merge multiple CSV streams
- **csv-min**(1) - take a minimum value of numerical or string column(s)
- **csv-sort**(1) - sort CSV file by column(s)
- **csv-sql**(1) - process CSV input data using simplified SQL syntax
- **csv-sqlite**(1) - process CSV input data using SQLite
- **csv-sum**(1) - take a sum of numerical or string column(s)
- **csv-tac**(1) - concatenate CSV files and print on the standard output in reverse
- **csv-tail**(1) - print on the standard output the end of a CSV file from standard input
- **csv-uniq**(1) - merge adjacent duplicate rows

# SINK TOOLS #

- **csv-exec**(1) - execute an external command for each row
- **csv-show**(1) - print in human-readable format

# COMMON OPTIONS FOR ALL SOURCE TOOLS #

-c, --columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-l
:   use a longer listing format (can be used up multiple times)

-M, --merge
:   merge output with a CSV stream in table form from standard input

-N, --table-name=*NAME*
:   produce output as table *NAME*

-T, --as-table
:   produce output as table

Options -M/-N/-T can be used to merge output of multiple source tools, to
be ultimately consumed by **csv-sqlite**(1). See **csv-tables-tut**(7) for
one example.

# COMMON OPTIONS FOR ALL SOURCE AND PROCESSING TOOLS #

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

# COMMON OPTIONS FOR PROCESSING TOOLS #

-t, --table=*NAME*
:   apply to rows only with _table column equal *NAME*

# COMMON OPTIONS FOR ALL TOOLS #

--help
:   display this help and exit

--version
:   output version information and exit

# SEE ALSO #

**<https://github.com/mslusarz/csv-nix-tools>**
