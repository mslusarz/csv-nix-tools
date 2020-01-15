---
title: csv-header
section: 1
...

# NAME #

csv-header - process header of a CSV file

# SYNOPSIS #

**csv-header** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print back to standard output with
the column header transformed.

-A, --add-types *NAME1*:*TYPE1*[,*NAME2*:*TYPE2*]
:   (NOT IMPLEMENTED YET) add type *TYPE1* to column *NAME1*, *TYPE2* to column *NAME2*, etc.

-C, --change-type *NAME*:*TYPE*
:   (NOT IMPLEMENTED YET) change type of column *NAME* to *TYPE*

-G, --guess-types
:   (NOT IMPLEMENTED YET) add types to columns by guessing based on their contents

-M, --remove-types
:   (NOT IMPLEMENTED YET) remove types from columns

-m, --remove
:   remove column header

-n, --rename *NAME*,*NEW-NAME*
:   (NOT IMPLEMENTED YET) rename column *NAME* to *NEW-NAME*

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLES #

csv-ls | csv-count -r | csv-header -m
:   print number of files in the current directory without csv header

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
