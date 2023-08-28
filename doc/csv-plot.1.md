<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-plot
section: 1
...

# NAME #

csv-plot - output 2D or 3D graph from CSV data read from standard input

# SYNOPSIS #

**csv-plot** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and output 2D or 3D gnuplot script to
standard output.


-g, \--gnuplot
:   pipe to gnuplot

-G, \--grid
:   draw a grid

-t, \--terminal *TERMINAL*
:   use *TERMINAL* as gnuplot's output (e.g. png, gif, dumb)

-x *COLNAME*
:   use *COLNAME* as x axis

-y *COLNAME*
:   use *COLNAME* as y axis

-z *COLNAME*
:   use *COLNAME* as z axis

-T, \--table=*NAME*
:   apply to rows only with _table column equal *NAME*

-X, \--no-types
:   disable parsing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-plot -x xcol -y ycol < file1.csv`
:   generate 2D graph gnuplot script from data in file1.csv

`csv-plot -x xcol -y ycol -z zcol -g < file1.csv`
:   generate 3D graph gnuplot script from data in file1.csv and pipe it to gnuplot

# SEE ALSO #

**[gnuplot](https://linux.die.net/man/1/gnuplot)**(1),
**csv-nix-tools**(7)
