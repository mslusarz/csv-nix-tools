<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-add-replace
section: 1
...

# NAME #

csv-add-replace - add a new column by performing a string substitution on another column

# SYNOPSIS #

**csv-add-replace** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by performing string substitution either using fixed
strings or regular expression.

-c *NAME*
:   use column *NAME* as an input data

-e *REGEX*
:   use *REGEX* as a basic regular expression

-E *EREGEX*
:   use *EREGEX* as an extended regular expression

-F *PATTERN*
:   use *PATTERN* as a fixed string pattern

-i, \--ignore-case
:   perform matching ignoring case distinction

-n *NEW-NAME*
:   create column *NEW-NAME* as an output

-r *REPLACEMENT*
:   use *REPLACEMENT* as an replacement for pattern; for fixed string pattern
it is not interpreted, but for regular expression %1 to %9 are replaced
by corresponding matching sub-expression, and %0 is the whole matching
expression

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply to rows only with _table column equal *NAME*

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -c full_path | csv-add-replace -c full_path -E '(.*)\.c$' -r '%1.o' -n new -s`
:   list files; if file has .c extension it will be replaced by .o extension

# SEE ALSO #

**[regex](http://man7.org/linux/man-pages/man7/regex.7.html)**(7),
**[sed](http://man7.org/linux/man-pages/man1/sed.1.html)**(1),
**csv-show**(1),
**csv-nix-tools**(7)
