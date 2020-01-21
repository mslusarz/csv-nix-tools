---
title: csv-tables-tut
section: 7
...

# NAME #

csv-tables-tut - tutorial for csv-nix-tools' tables

# TUTORIAL #

Let's say we want to see the default group of some system users.

Start with the list of users:

```
$ csv-users -c name,uid,gid |
    csv-grep-sql -e "uid >= 110 and uid < 115" -s
name                uid     gid
dnsmasq             110   65534
colord              111     121
speech-dispatcher   112      29
hplip               113       7
kernoops            114   65534
```

We want to add another table to this stream, so we have to produce it in table form first:

```
$ csv-users -T -c name,uid,gid |
    csv-grep-sql -T user -e "uid >= 110 and uid < 115" -s
_table   user.name           user.uid   user.gid
user     dnsmasq                  110      65534
user     colord                   111        121
user     speech-dispatcher        112         29
user     hplip                    113          7
user     kernoops                 114      65534
```

Notice that the csv-grep-sql command doesn't use "user." prefix in SQL expression thanks to "-T user".

Now add groups using -M option of csv-groups:

```
$ csv-users -T -c name,uid,gid |
    csv-grep-sql -T user -e "uid >= 110 and uid < 115" |
    csv-groups -M -s
_table   user.name           user.uid   user.gid   group.name   group.gid
user     dnsmasq                  110      65534                                  
user     colord                   111        121                                  
user     speech-dispatcher        112         29                                  
user     hplip                    113          7                                  
user     kernoops                 114      65534                                  
group                                              root                 0
group                                              daemon               1
group                                              bin                  2
group                                              sys                  3
group                                              adm                  4
group                                              tty                  5
... output cut here ...
```

We can filter each table independently. For example the command below will
output exactly the same information as the one above, as csv-grep-sql
will look only at rows with _table = 'user'.

```
$ csv-users -T -c name,uid,gid |
    csv-groups -M |
    csv-grep-sql -T user -e "uid >= 110 and uid < 115" -s
```

Now join both tables together:

```
$ csv-users -T -c name,uid,gid |
    csv-grep-sql -T user -e "uid >= 110 and uid < 115" |
    csv-groups -M |
    csv-sqlite -T "select user.name as user_name, group.name as group_name from user, group where user.gid = group.gid" -s
sqlite3_prepare_v2(select='select user.name as user_name, group.name as group_name from user, group where user.gid = group.gid'): near "group": syntax error
...
```

Oops, "group" is an SQL keyword, let's rename the table with groups to "grp":

```
$ csv-users -T -c name,uid,gid |
    csv-grep-sql -T user -e "uid >= 110 and uid < 115" |
    csv-groups -M -N grp |
    csv-sqlite -T "select user.name as user_name, grp.name as group_name from user, grp where user.gid = grp.gid" -s
user_name           group_name
dnsmasq             nogroup
colord              colord
speech-dispatcher   audio
hplip               lp
kernoops            nogroup
```

That's it.

All source tools in **csv-nix-tools**(7) support -M (**m**erge with input), -T (output as **t**able) and -N (re**n**ame output table) options.
Most processing tools support -T (filtering by **t**able) option.

# SEE ALSO #

**csv-nix-tools**(7), **csv-merge**(1)
