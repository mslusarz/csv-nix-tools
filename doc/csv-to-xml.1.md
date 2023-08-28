<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-to-xml
section: 1
...

# NAME #

csv-to-xml - convert CSV file from standard input to XML

# SYNOPSIS #

**csv-to-xml** [OPTION]...

# DESCRIPTION #

Convert CSV file from standard input to XML.

\--generic-names
:   don't use column names for nodes

\--no-header
:   remove column headers

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-to-xml < file1.csv > file1.xml`
:   generate file1.xml from file1.csv

# SEE ALSO #

**csv-show**(1), **csv-to-html**(1), **csv-to-json**(1), **csv-nix-tools**(7)
