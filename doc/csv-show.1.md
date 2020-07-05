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

There are 3 backends: **[ncurses](http://man7.org/linux/man-pages/man3/ncurses.3x.html)**(3)-based,
**[less](http://man7.org/linux/man-pages/man1/less.1.html)**(1)-based and no-backend.

**ncurses** backend:

- keeps column names at the top of the screen at all times

- supports scrolling by:
    - one column ([ and ] keys)
    - half screen (left and right keys)
    - one line (up and down keys)
    - one screen (page up, backspace, page down and space keys)
    - to the beginning of the file (home key)
    - to the beginning of the line (shift + home key)
    - to the end of the file (end key)
    - to the end of the line (shift + end key)

- doesn't support searching (yet)

**less** backend supports everything the command **less**(1) supports.

**none** backend simply prints everything to standard output.

-s, \--spacing *NUM*
:   use *NUM* spaces between columns instead of 3

-u, \--ui *curses*/*less*/*none*
:   use UI based on **[ncurses](http://man7.org/linux/man-pages/man3/ncurses.3x.html)**(3),
**[less](http://man7.org/linux/man-pages/man1/less.1.html)**(1) or nothing(tm)

\--no-header
:   remove column headers

\--set-color *COLNAME*:[fg=]*COLOR1*[,bg=*COLOR2*]
:   set *COLOR1* as foreground and *COLOR2* as background of column *COLNAME*

\--use-color-columns
:   use columns with _color suffix

\--with-types
:   print types in column headers

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-show < file1.csv`
:   print file1.csv in human-readable format

`csv-show --ui none < file1.csv`
:   print file1.csv in human-readable format without any pager

# SEE ALSO #

**[ncurses](http://man7.org/linux/man-pages/man3/ncurses.3x.html)**(3),
**[less](http://man7.org/linux/man-pages/man1/less.1.html)**(1),
**csv-nix-tools**(7)
