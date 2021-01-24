<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-printf
section: 1
...

# NAME #

csv-printf - formats data using printf

# SYNOPSIS #

**csv-printf** [OPTION]... FORMAT [COLUMNS]...

# DESCRIPTION #

Read CSV stream from standard input and print back data using **printf**(3).

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -c dev,ino | csv-printf "0x%x/%d\n" dev ino`
:   prints device and inode number in specified format

# SEE ALSO #

**[printf](http://www.cplusplus.com/reference/cstdio/printf/)**(3),
**csv-nix-tools**(7)
