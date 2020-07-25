<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-grep
section: 1
...

# NAME #

csv-grep - filter rows of CSV stream matching a pattern

# SYNOPSIS #

**csv-grep** [OPTION]...

# DESCRIPTION #

Searches for PATTERN in column of CSV file read from standard input and print
to standard output only rows matching that pattern.

-c *NAME*
:   apply the filter to column *NAME*

-e *PATTERN*
:   use *PATTERN* as a basic regular expression pattern

-E *PATTERN*
:   use *PATTERN* as an extended regular expression pattern

-F *STRING*
:   use *STRING* as a fixed string pattern

-i, \--ignore-case
:   ignore case distinction

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply the filter to rows only with _table column equal *NAME*

-v, \--invert
:   invert the sense of matching, selecting non-matching rows

-x, \--whole
:   the pattern used by -e, -E or -F options must match exactly (no preceding
or succeeding characters before/after pattern)

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -l | csv-grep -c name -e '.*\.c$' -s`
:   list files with .c extension

# SEE ALSO #

**[grep](http://man7.org/linux/man-pages/man1/grep.1.html)**(1),
**[regex](http://man7.org/linux/man-pages/man7/regex.7.html)**(7),
**csv-grep-rpn**(1), **csv-grep-sql**(1), **csv-show**(1), **csv-nix-tools**(7)
