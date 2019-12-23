---
title: csv-show
section: 1
...

# NAME #

csv-show - print on the standard output a CSV file from standard input in human-readable format

# SYNOPSIS #

**csv-show** [OPTION]...

# DESCRIPTION #

Print CSV file from standard input in human-readable format, using tables, without types.

There are 3 backends: **ncurses**(3)-based, **less**(1)-based and no-backend.

**ncurses** backend:

- keeps column names at the top of the screen at all times

- supports scrolling by:
    - one column ([ and ] keys)
    - half screen (left and right keys)
    - one line (up and down keys)
    - one screen (page up, backspace, page down and space keys)
    - to the beginning of the line (home key)
    - to the end of the line (end key)

- doesn't support searching (yet)

**less** backend supports everything the command **less**(1) supports.

**none** backend simply prints everything to standard output.

-u, --ui curses/less/none
:   use UI based on **ncurses**(3), **less**(1) or nothing(tm)

-s, --spacing NUM
:   use NUM spaces between columns instead of 3

--with-types
:   print types in column headers

--no-header
:   remove column headers

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLE #

csv-show < file1.csv
:   print file1.csv in human-readable format

csv-show --ui none < file1.csv
:   print file1.csv in human-readable format without any pager

# SEE ALSO #

**ncurses**(3), **less**(1), **csv-nix-tools**(7)
