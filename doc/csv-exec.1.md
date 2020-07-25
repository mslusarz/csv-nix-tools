<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-exec
section: 1
...

# NAME #

csv-exec - execute an external command for each row

# SYNOPSIS #

**csv-exec** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and execute an external command for each row.

\--help
:   display this help and exit

\--version
:   output version information and exit

# EXAMPLES #

`csv-ls -R -c full_path . | csv-grep -c full_path -e '~$' | csv-exec -- rm -f %full_path`
:   remove all backup files

# SEE ALSO #

**csv-nix-tools**(7)
