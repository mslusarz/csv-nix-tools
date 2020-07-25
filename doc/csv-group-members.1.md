<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-group-members
section: 1
...

# NAME #

csv-group-members - list system groups and users that belong to them in CSV format

# SYNOPSIS #

**csv-group-members** [OPTION]...

# DESCRIPTION #

Print to standard output the list of system groups and users that belong to them
in the CSV format.

-c, \--columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-M, \--merge
:   merge output with a CSV stream in table form from standard input

-N, \--table-name *NAME*
:   produce output as table *NAME*

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--as-table
:   produce output as table *group_member*

\--help
:   display this help and exit

\--version
:   output version information and exit

# COLUMNS #

| name       | type   |
|------------|--------|
| group_name | string |
| user_name  | string |

# EXAMPLES #

`csv-group-members -s`
:   print groups and users

`csv-group-members -T | csv-grep -T group_member -c group_name -x -F audio | csv-users -M | csv-sqlite -T 'select user.uid from user, group_member where user.name = group_member.user_name' -s`
:   print user ids of users belonging to group 'audio'

# SEE ALSO #

**csv-groups**(1), **csv-users**(1), **csv-show**(1), **csv-nix-tools**(7)
