<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
-->

---
title: csv-ls
section: 1
...

# NAME #

csv-ls - list files in CSV format

# SYNOPSIS #

**csv-ls** [OPTION]... [FILE]...

# DESCRIPTION #

List information about FILEs (current directory by default).
Sort entries alphabetically if -U is not specified.

-a, \--all
:   do not ignore entries starting with .

-c, \--columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-d, \--directory
:   list directories themselves, not their contents

-l
:   use a longer listing format (can be used up to 3 times)

-M, \--merge
:   merge output with a CSV stream in table form from standard input

-N, \--table-name *NAME*
:   produce output as table *NAME*

-R, \--recursive
:   list subdirectories recursively

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--as-table
:   produce output as table *file*

-U
:   do not sort; list entries in directory order

\--colors
:   add color columns

\--no-colors
:   don't add color columns

\--help
:   display this help and exit

\--version
:   output version information and exit

# COLUMNS #

| name          | type   | description                       | level |
|---------------|--------|-----------------------------------|-------|
| type_mode     | string | type and mode                     | 1     |
| nlink         | int    | number of hard links              | 1     |
| owner_name    | string | user name of owner                | 1     |
| group_name    | string | group name of owner               | 1     |
| size          | int    | total size, in bytes              | 1     |
| mtime         | string | time of last modification, fmt    | 1     |
| name          | string | file name                         | 0     |
| symlink       | string | symlink path (if a symlink)       | 1     |
| parent        | string | parent directory                  | 2     |
| type          | int    | type of file (S_IFMT)             | 2     |
| type_name     | string | type of file (as string)          | 2     |
| mode          | int    | file type and mode                | 2     |
| owner_read    | int    | owner has read permission         | 2     |
|               |        | 0/1, S_IRUSR                      |       |
| owner_write   | int    | owner has write permission        | 2     |
|               |        | 0/1, S_IWUSR                      |       |
| owner_execute | int    | owner has execute permission      | 2     |
|               |        | 0/1, S_IXUSR                      |       |
| group_read    | int    | group has read permission         | 2     |
|               |        | 0/1, S_IRGRP                      |       |
| group_write   | int    | group has write permission        | 2     |
|               |        | 0/1, S_IWGRP                      |       |
| group_execute | int    | group has execute permission      | 2     |
|               |        | 0/1, S_IXGRP                      |       |
| other_read    | int    | others has read permission        | 2     |
|               |        | 0/1, S_IROTH                      |       |
| other_write   | int    | others has write permission       | 2     |
|               |        | 0/1, S_IWOTH                      |       |
| other_execute | int    | others has execute permission     | 2     |
|               |        | 0/1, S_IXOTH                      |       |
| setuid        | int    | set-user-id set, 0/1, S_ISUID     | 2     |
| setgid        | int    | set-group-id set, 0/1, S_ISGID    | 2     |
| sticky        | int    | sticky, 0/1, S_ISVTX              | 2     |
| owner_id      | int    | user id of owner                  | 2     |
| group_id      | int    | group id of owner                 | 2     |
| ctime         | string | time of last status change, fmt   | 2     |
| atime         | string | time of last access, fmt          | 2     |
| full_path     | string | parent + name                     | 2     |
| mtime_sec     | int    | time of last modification (sec)   | 3     |
| mtime_nsec    | int    | time of last modification (nsec)  | 3     |
| ctime_sec     | int    | time of last status change (sec)  | 3     |
| ctime_nsec    | int    | time of last status change (nsec) | 3     |
| atime_sec     | int    | time of last access (sec)         | 3     |
| atime_nsec    | int    | time of last access (nsec)        | 3     |
| dev           | int    | id of device containing file      | 3     |
| ino           | int    | inode number                      | 3     |
| rdev          | int    | device id (if special file)       | 3     |
| blksize       | int    | block size for filesystem I/O     | 3     |
| blocks        | int    | number of 512B blocks allocated   | 3     |

# EXAMPLES #

`csv-ls`
:   print files (just names) in the current directory

`csv-ls -l -s`
:   print files in the current directory, in a format similar to ls -l

# SEE ALSO #

**[ls](http://man7.org/linux/man-pages/man1/ls.1.html)**(1),
**[inode](http://man7.org/linux/man-pages/man7/inode.7.html)**(7),
**csv-show**(1),
**csv-nix-tools**(7)
