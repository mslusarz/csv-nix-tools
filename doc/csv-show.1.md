<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2019-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

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
**[less](http://man7.org/linux/man-pages/man1/less.1.html)**(1)-based, and no-backend.
By default backend is guessed based on whether standard output is a terminal.

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

- supports changing foreground and background color of each cell

- doesn't support searching (yet)

**less** backend supports everything the command **less**(1) supports.

**none** backend simply prints everything to standard output.

-p, \--spacing *NUM*
:   use *NUM* spaces between columns instead of 3

-u, \--ui *curses*/*less*/*none*
:   use UI based on **[ncurses](http://man7.org/linux/man-pages/man3/ncurses.3x.html)**(3),
**[less](http://man7.org/linux/man-pages/man1/less.1.html)**(1), or nothing(tm)

-s
:   short for \--ui none

-S
:   short for \--ui curses

\--no-header
:   remove column headers

\--set-color *COLNAME*:[fg=]*COLOR1*[,bg=*COLOR2*]
:   set *COLOR1* as foreground and *COLOR2* as background of column *COLNAME*

    colors can be specified in 3 ways:

    - as a predefined name (black, red, green, yellow, blue, magenta, cyan, white, default)

    - as a decimal number for terminal-specific color

    - in RRGGBB format (FF0000 is red, 00FF00 is green, 0000FF is blue, etc.),
      note: this requires changing definition of a color slot, but
      some terminals lie about supporting this capability, and there's
      no way of detecting this

-C, \--use-color-columns
:   use columns with _color suffix to render each cell

-i, \--interactive
:   line-selection mode (ncurses backend only)

\--on-key *key*,*text*,*action*[,*return*]
:   in interactive mode, show *text* as *key*'s description, on *key* pipe current row to *action* (either a command or special text 'stdout') and quit unless *return* suboption was used, *key* can be one of: *enter*, *f1*, *f2*, *f3*, *f4*, *f5*, *f6*, *f7*, *f8*, *f9*, *f10*, *f11*, *f12*

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
