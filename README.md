**csv-nix-tools**
=================

[![Build Status](https://github.com/mslusarz/csv-nix-tools/workflows/Full/badge.svg)](https://github.com/mslusarz/csv-nix-tools/actions?query=workflow%3AFull+branch%3Amaster)
[![Coverage Status](https://codecov.io/github/mslusarz/csv-nix-tools/coverage.svg?branch=master)](https://codecov.io/gh/mslusarz/csv-nix-tools/branch/master)

csv-nix-tools is a collection of tools for gathering and processing system
information using CSV as an intermediate format.

# Status
This project is in pre-alpha stage. Everything listed below is functional,
but in the future anything can change (IOW don't run this in production yet).
Currently it builds and runs on Linux with glibc only, but portability patches
are welcomed.

# Requirements
- cmake >= 3.3
- glibc-devel
- sqlite >= 3 (optional, required by csv-sqlite)
- flex (optional, required by csv-sql)
- bison (optional, required by csv-sql)
- libprocps (optional, required by csv-ps)
- ncurses (optional, used by csv-show)
- libmnl (optional, required by csv-netstat)

# Installation
```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

# Available tools

Source:
- csv-groups - lists system groups
- csv-group-members - lists system groups and users that belong to them
- csv-ls - lists files
- csv-netstat - lists network connections
- csv-ps - lists processes (WIP)
- csv-users - lists system users

Filtering/processing:
- csv-avg - takes an average of numerical column(s)
- csv-cat - concatenates multiple csv files
- csv-concat - concatenates columns and user-defined strings
- csv-count - counts the number of columns and/or rows
- csv-cut - removes columns and reorders them
- csv-exec-add - pipes data to standard input of an external command and creates new column from its standard output
- csv-grep - filters rows matching a pattern
- csv-head - outputs the first N rows
- csv-header - processes data header
- csv-max - takes a maximum value of numerical or string column(s)
- csv-merge - merges multiple input streams
- csv-min - takes a minimum value of numerical or string column(s)
- csv-replace - performs string substitution on column(s) (similar to sed s/$str/$str/)
- csv-rpn-add - creates new column from RPN expression
- csv-rpn-filter - filters rows using RPN expression
- csv-sort - sorts input by column(s)
- csv-split - splits one column into two using a delimiter
- csv-sql - processes input data using simplified (but very fast) SQL-based syntax (WIP)
- csv-sqlite - processes input data using SQLite (requires loading the whole input before processing)
- csv-substring - extracts part of a column
- csv-sum - takes a sum of numerical or string column(s)
- csv-tac - concatenates files in reverse
- csv-tail - outputs the last N rows
- csv-uniq - merges adjacent duplicate rows

Sink:
- csv-exec - executes an external command for each row
- csv-show - formats data in human-readable form (also available as a "-s" option in all source and processing tools)

# Examples

List of files in directory:

```
$ csv-ls doc
name:string
csv-cat.1.md
csv-head.1.md
csv-nix-tools.7.md
csv-show.1.md
csv-tac.1.md
csv-tail.1.md
```

Extended information about files in one directory:

```
$ csv-ls -l doc
type_mode:string,nlink:int,owner_name:string,group_name:string,size:int,mtime:string,name:string,symlink:string
-rwxr-xr-x ,1,marcin,marcin,773,2019-12-23 01:24:58.478029642,csv-cat.1.md,
-rwxr-xr-x ,1,marcin,marcin,721,2019-12-23 02:15:21.074174728,csv-head.1.md,
-rwxr-xr-x ,1,marcin,marcin,1673,2019-12-23 02:16:07.494064188,csv-nix-tools.7.md,
-rwxr-xr-x ,1,marcin,marcin,1476,2019-12-23 02:43:08.227018848,csv-show.1.md,
-rwxr-xr-x ,1,marcin,marcin,825,2019-12-23 01:25:09.873956973,csv-tac.1.md,
-rwxr-xr-x ,1,marcin,marcin,710,2019-12-23 02:15:38.110135517,csv-tail.1.md,
```

Extended^3 information about files in one directory:

```
$ csv-ls -lll doc
type_mode:string,nlink:int,owner_name:string,group_name:string,size:int,mtime:string,name:string,symlink:string,parent:string,type:int,type_name:string,mode:int,owner_read:int,owner_write:int,owner_execute:int,group_read:int,group_write:int,group_execute:int,other_read:int,other_write:int,other_execute:int,setuid:int,setgid:int,sticky:int,owner_id:int,group_id:int,ctime:string,atime:string,full_path:string,mtime_sec:int,mtime_nsec:int,ctime_sec:int,ctime_nsec:int,atime_sec:int,atime_nsec:int,dev:int,ino:int,rdev:int,blksize:int,blocks:int
-rwxr-xr-x ,1,marcin,marcin,773,2019-12-23 01:24:58.478029642,csv-cat.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 01:24:58.478029642,2019-12-29 20:13:34.710830508,doc/csv-cat.1.md,1577060698,478029642,1577060698,478029642,1577646814,710830508,0x801,16122673,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,721,2019-12-23 02:15:21.074174728,csv-head.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:15:21.074174728,2019-12-29 20:13:34.710830508,doc/csv-head.1.md,1577063721,74174728,1577063721,74174728,1577646814,710830508,0x801,16122671,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,1673,2019-12-23 02:16:07.494064188,csv-nix-tools.7.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:16:07.494064188,2019-12-29 20:13:34.710830508,doc/csv-nix-tools.7.md,1577063767,494064188,1577063767,494064188,1577646814,710830508,0x801,16125292,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,1476,2019-12-23 02:43:08.227018848,csv-show.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:43:08.227018848,2019-12-29 20:13:34.710830508,doc/csv-show.1.md,1577065388,227018848,1577065388,227018848,1577646814,710830508,0x801,16125313,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,825,2019-12-23 01:25:09.873956973,csv-tac.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 01:25:09.873956973,2019-12-29 20:13:34.710830508,doc/csv-tac.1.md,1577060709,873956973,1577060709,873956973,1577646814,710830508,0x801,16124245,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,710,2019-12-23 02:15:38.110135517,csv-tail.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:15:38.110135517,2019-12-29 20:13:34.710830508,doc/csv-tail.1.md,1577063738,110135517,1577063738,110135517,1577646814,710830508,0x801,16125317,0,4096,8
```

All Makefiles in the current directory and below:

```
$ csv-ls -f size,parent,name -R . | csv-grep -f name -x -F Makefile
size:int,parent:string,name:string
55358,./build,Makefile
6345,./build/tests,Makefile
```

Files and their sizes sorted by size and name, in descending order:

```
$ csv-ls -f size,name | csv-sort -r -f size,name
size:int,name:string
14660,ls.c
7238,sort.c
6297,parse.c
6189,grep.c
4903,sum.c
4691,tail.c
4302,cut.c
4096,build
3322,head.c
2794,README.md
2746,utils.c
2389,format.c
2204,parse.h
1957,utils.h
1611,CMakeLists.txt
1472,LICENSE
1044,cmake_uninstall.cmake.in
```

Files owned by user=1000:

```
$ csv-ls -R -f parent,name,owner_id / 2>/dev/null | csv-grep -f owner_id -x -F 1000 | csv-cut -f parent,name
...
```

Files owned by user=marcin:

```
$ csv-ls -R -f parent,name,owner_name / 2>/dev/null | csv-grep -f owner_name -x -F marcin | csv-cut -f parent,name
...
```

Files anyone can read:

```
$ csv-ls -R -f parent,name,other_read / 2>/dev/null | csv-grep -f other_read -x -F 1 | csv-cut -f parent,name
...
```

Sum of sizes of all files in the current directory:

```
$ csv-ls -l . | csv-sum -f size
sum(size):int
78739
```

4 biggest files in the current directory:

```
$ csv-ls -f size,name . | csv-sort -r -f size | csv-head -n 4
size:int,name:string
14660,ls.c
7238,sort.c
6297,parse.c
6189,grep.c
$ csv-ls -f size,name . | csv-sort -f size | csv-tail -n 4
...
```

List of files, split name into base and extension:

```
$ csv-ls -f size,name | csv-split -f name -e . -n base,ext -r
size:int,name:string,base:string,ext:string
123,file1,file1,
456,file2.ext,file2,ext
789,file3.ext.in,file3.ext,in
```

Sum of sizes of all files with the "png" extension:

```
$ csv-ls -l . | csv-split -f name -e . -n base,ext -r |
csv-grep -f ext -x -F png | csv-sum -f size | csv-header --remove
94877
```

Number of files in the current directory:

```
$ csv-ls . | csv-count --rows
rows:int
19
```

List of files formatted in human-readable format (similar to ls -l) with disabled pager:

```
$ csv-ls -l | csv-show -s 1 --ui none --no-header
-rwxr-xr-x  1 marcin marcin 5089  2019-12-23 00:23:36.438335394 avg.c           
-rwxr-xr-x  1 marcin marcin 5108  2019-12-23 00:22:44.362494798 cat.c           
-rwxr-xr-x  1 marcin marcin 5353  2019-12-14 22:19:16.197571951 concat.c        
...
```

List of files whose 2nd character is 'o':

```
$ csv-ls -f name | csv-grep -f name -e '^.o'
name:string
concat.c
sort.c
```

or

```
$ csv-ls -f name | csv-substring -f name -n 2nd-char -p 2 -l 1 |
csv-grep -f 2nd-char -F o | csv-cut -f name
name:string
concat.c
sort.c
```

Full paths of all files in current directory and below:

```
$ csv-ls -R -f parent,name . | csv-concat full_path = %parent / %name | csv-cut -f full_path
....
```
or

```
$ csv-ls -R -f full_path .
```

Sum of file sizes and real allocated blocks in the current directory:

```
$ csv-ls -f size,blocks | csv-rpn-add -f space_used -e "%blocks 512 *" |
csv-sum -f size,blocks,space_used
sum(size):int,sum(blocks):int,sum(space_used):int
109679,288,147456
```

List of files whose size is between 2000 and 3000 bytes (from the slowest to the fastest method):

```
$ csv-ls -f size,name | csv-sqlite "select size, name from input where size > 2000 and size < 3000"
size:int,name:string
2204,parse.h
```

or

```
$ csv-ls -f size,name |
csv-rpn-add -f range2k-3k -e "%size 2000 >= %size 3000 < and" |
csv-grep -f range2k-3k -F 1 |
csv-cut -f size,name
...
```

or

```
$ csv-ls -f size,name | csv-sql "select size, name from input where size > 2000 and size < 3000"
...
```

or

```
$ csv-ls -f size,name | csv-rpn-filter -e "%size 2000 >= %size 3000 < and"
...
```


Files and their permissions printed in human-readable format (simpler version of column type_mode):

```
$ csv-ls -f mode,name | csv-rpn-add -f strmode -e "\
%mode 0400 & 'r' '-' if        %mode 0200 & 'w' '-' if concat %mode 0100 & 'x' '-' if concat \
%mode  040 & 'r' '-' if concat %mode  020 & 'w' '-' if concat %mode  010 & 'x' '-' if concat \
%mode   04 & 'r' '-' if concat %mode   02 & 'w' '-' if concat %mode   01 & 'x' '-' if concat" |
csv-cut -f mode,strmode,name -s

mode   strmode     name
0644   rw-r--r--   CMakeCache.txt
0755   rwxr-xr-x   CMakeFiles
0600   rw-------   core
...
```
or

```
$ csv-ls -f mode,name | csv-sql "select fmt_oct(mode) as 'mode',
if (mode & 0400, 'r', '-') || if (mode & 0200, 'w', '-') || if (mode & 0100, 'x', '-') ||
if (mode & 040,  'r', '-') || if (mode & 020,  'w', '-') || if (mode & 010,  'x', '-') ||
if (mode & 04,   'r', '-') || if (mode & 02,   'w', '-') || if (mode & 01,   'x', '-') as 'strmode', name" -s

mode   strmode     name
0644   rw-r--r--   CMakeCache.txt
0755   rwxr-xr-x   CMakeFiles
0600   rw-------   core
```

Remove all temporary files (ending with "~"), even if path contains spaces or line breaks:

```
$ csv-ls -R -f full_path . | csv-grep -f full_path -e '~$' | csv-exec -- rm -f %full_path
```

If file has .c extension, then replace it with .o, otherwise leave it as is:

```
$ csv-ls -R -f full_path . | csv-exec-add -f full_path -n new -- sed 's/.c$/.o/'
```
or 130x faster:

```
$ csv-ls -R -f full_path . | csv-replace -f full_path -E '(.*)\.c$' -r '%1.o' -n new
```
or 400x faster (3.1x faster than csv-replace):

```
$ csv-ls -R -f full_path . | csv-rpn-add -f new -e "%full_path -1 1 substr 'c' == %full_path 1 %full_path strlen 1 - substr 'o' concat %full_path if"
```

List all system users and the name of the default group they belong to:

```
$ csv-users -L user | csv-groups -M -L group | csv-sqlite -L "select user.name as user_name, 'group'.name as group_name from user, 'group' where user.gid = 'group'.gid"
```

List all network connections and processes they belong to:

```
$ csv-ls -L file -f name,symlink /proc/*/fd/* 2>/dev/null |
csv-grep -f file.symlink -E "socket:\[[0-9]*\]" |
csv-replace -f file.name -E '/proc/([0-9]*)/.*' -r '%1' -n file.pid |
csv-replace -f file.symlink -E 'socket:\[([0-9]*)\]' -r %1 -n file.inode |
csv-netstat -M |
csv-grep -v -f socket.family -x -F 'UNIX' |
csv-ps -M -f pid,cmd |
csv-sqlite -L 'select socket.protocol, socket.src_ip, socket.src_port, socket.dst_ip, socket.dst_port, socket.state, socket.uid, file.pid, proc.cmd from socket left outer join file on socket.inode = file.inode left outer join proc on file.pid = proc.pid' -s

protocol   src_ip                src_port   dst_ip                 dst_port   state         uid    pid     cmd
tcp        127.0.0.53            53         0.0.0.0                0          LISTEN        102            
tcp        127.0.0.1             631        0.0.0.0                0          LISTEN        0              
tcp        127.0.0.1             39455      0.0.0.0                0          LISTEN        0              
tcp        192.168.1.14          52980      $IP                    443        ESTABLISHED   1000   16175   firefox
tcp        192.168.1.14          39390      $IP                    443        ESTABLISHED   1000   16175   firefox
tcp        192.168.1.14          49640      $IP                    8008       ESTABLISHED   1000   6504    chrome
tcp        192.168.1.14          39220      $IP                    443        TIME_WAIT     0              
tcp        192.168.1.14          47420      $IP                    443        ESTABLISHED   1000   6504    chrome
tcp        192.168.1.14          51424      $IP                    443        ESTABLISHED   1000   6504    chrome
tcp        192.168.1.14          58330      $IP                    443        ESTABLISHED   1000   16175   firefox
tcp        192.168.1.14          48254      $IP                    8009       SYN_SENT      1000   6504    chrome
tcp        192.168.1.14          43784      $IP                    443        TIME_WAIT     0              
tcp        192.168.1.14          54018      $IP                    443        ESTABLISHED   1000   6504    chrome
tcp        192.168.1.14          48250      $IP                    8009       FIN_WAIT1     0              
tcp        ::1                   631        ::                     0          LISTEN        0              
tcp        ::ffff:192.168.0.14   37610      ::ffff:$IP             80         CLOSE_WAIT    1000   27184   WebKitWebProces
tcp        ::ffff:192.168.0.14   37610      ::ffff:$IP             80         CLOSE_WAIT    1000   8337    java
udp        127.0.0.53            53         0.0.0.0                0                        102            
udp        0.0.0.0               68         0.0.0.0                0                        0              
udp        0.0.0.0               631        0.0.0.0                0                        0              
udp        224.0.0.251           5353       0.0.0.0                0                        1000   6504    chrome
udp        224.0.0.251           5353       0.0.0.0                0                        1000   6470    chrome
udp        224.0.0.251           5353       0.0.0.0                0                        1000   6504    chrome
udp        0.0.0.0               5353       0.0.0.0                0                        107            
udp        0.0.0.0               46599      0.0.0.0                0                        107            
udp        ::                    59934      ::                     0                        107            
udp        ::                    5353       ::                     0                        107            
raw        ::                    58         ::                     0                        0              

```


# TODO (high level)
- more tests
- more data collection tools (id, env, uname, w/who, df, free, ifconfig/ip, ipcs, lsattr, lsusb, tcpdump?, route, lscpu, lshw, lsblk, lsns, last, etc)
- more processing tools (nl, tr, rev, drop, paste?, etc)
- more rpn operators/functions (split, rev, base64enc/dec, timestamp conversion, now, regex, sed/replace, tr)
- exporting tools (to-xml, to-json, to-sql)
- importing tools (from-xml, from-json)
- test regularly with Valgrind
- documentation
- i18n
- export as much as possible as a library(ies)
- Coverity
- fuzzing

# TODO (low level)
- sql, exec: add support for labeled streams
- header: implement add, add-types, change-type, guess-type, remove-types, rename
- ps: lots of work, detailed in ps.c
- show: ability to lock columns (make them always visible)
- show: implement "search" and "help" in the ncurses backend
- rpn/sql: make naming of functions consistent (if->case?, strlen->length, toint->cast)
- rpn: cache compiled regex
- netstat: support more protocols
- netstat: translate interface number to interface name
- netstat: figure out how to print inet\_diag\_info
- rpn/sql: binary number parsing
- write down data format spec
- strict column name verification
- show: exit if output fits on screen?

## Random ideas
- export data as bash/python/etc script?
- float support?
- importing from other tools (lspci -mm?, strace?, lsof -F, ss, dpkg, rpm)?
- loops and temporary variables in rpn?
- built-in pipes? (csv "ls | grep -f size -F 0 | cut -f name")
- shell?
- what about unicode?
- one multicommand binary? (csv ls, csv grep, ...)
- csv-show: column separators? header separator? number formatter? (see what csvlook from csvkit does)
- csv-show: browser (html) backend?
- other sink tools: diff/comm?, treeview (generic pstree)
- csv -> csvnt?
