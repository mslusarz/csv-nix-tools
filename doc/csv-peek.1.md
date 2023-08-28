<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-peek
section: 1
...

# NAME #

csv-peek - peek at the CSV stream and pass it unmodified

# SYNOPSIS #

**csv-peek** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input, write it to stdout and its copy to stderr.

-s, \--show
:   print to stderr in table format

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -l | csv-peek -s | csv-sum -c size -s`
:   prints sum of sizes of all files in current directory to stdout and list of files to stderr

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
