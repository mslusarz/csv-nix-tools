<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-groups
section: 1
...

# NAME #

csv-groups - list system groups in CSV format

# SYNOPSIS #

**csv-groups** [OPTION]...

# DESCRIPTION #

Print to standard output the list of system groups in the CSV format.

-c, \--columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-l
:   use a longer listing format (can be used once)

-M, \--merge
:   merge output with a CSV stream in table form from standard input

-N, \--table-name *NAME*
:   produce output as table *NAME*

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--as-table
:   produce output as table *group*

-X, \--no-types
:   disable printing of type names in column names

\--help
:   display this help and exit

\--version
:   output version information and exit

# COLUMNS #

| name   | type   | description | level |
|--------|--------|-------------|-------|
| name   | string | name        | 0     |
| gid    | int    | group id    | 0     |
| passwd | string | password    | 1     |

passwd column comes from /etc/group, so it is useless on modern systems,
as a real password is stored somewhere else and is not visible to normal users.

# EXAMPLES #

`csv-groups -s`
:   print the list of groups

`csv-groups -T -N grp | csv-users -M | csv-sqlite -T 'select user.name as user_name, grp.name as group_name from user, grp where user.gid = grp.gid' -s`
:   print user names and the group name of their default group

# SEE ALSO #

**csv-users**(1), **csv-group-members**(1), **csv-show**(1), **csv-nix-tools**(7)
