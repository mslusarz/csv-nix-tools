List of files in `doc` directory:

```
$ csv-ls doc
name
csv-cat.1.md
csv-head.1.md
csv-nix-tools.7.md
csv-show.1.md
csv-tac.1.md
csv-tail.1.md
```

Extended information about files in `doc` directory:

```
$ csv-ls -l doc
type_mode,nlink:int,owner_name,group_name,size:int,mtime,name,symlink
-rwxr-xr-x ,1,marcin,marcin,773,2019-12-23 01:24:58.478029642,csv-cat.1.md,
-rwxr-xr-x ,1,marcin,marcin,721,2019-12-23 02:15:21.074174728,csv-head.1.md,
-rwxr-xr-x ,1,marcin,marcin,1673,2019-12-23 02:16:07.494064188,csv-nix-tools.7.md,
-rwxr-xr-x ,1,marcin,marcin,1476,2019-12-23 02:43:08.227018848,csv-show.1.md,
-rwxr-xr-x ,1,marcin,marcin,825,2019-12-23 01:25:09.873956973,csv-tac.1.md,
-rwxr-xr-x ,1,marcin,marcin,710,2019-12-23 02:15:38.110135517,csv-tail.1.md,
```

or in table form:

```
$ csv-ls -ls doc
type_mode     nlink   owner_name   group_name   size   mtime                           name                 symlink
-rwxr-xr-x        1   marcin       marcin        773   2019-12-23 01:24:58.478029642   csv-cat.1.md         
-rwxr-xr-x        1   marcin       marcin        721   2019-12-23 02:15:21.074174728   csv-head.1.md        
-rwxr-xr-x        1   marcin       marcin       1673   2019-12-23 02:16:07.494064188   csv-nix-tools.7.md   
-rwxr-xr-x        1   marcin       marcin       1476   2019-12-23 02:43:08.227018848   csv-show.1.md        
-rwxr-xr-x        1   marcin       marcin        825   2019-12-23 01:25:09.873956973   csv-tac.1.md         
-rwxr-xr-x        1   marcin       marcin        710   2019-12-23 02:15:38.110135517   csv-tail.1.md        
```


Extended^3 information about files in `doc` directory:

```
$ csv-ls -lll doc
type_mode,nlink:int,owner_name,group_name,size:int,mtime,name,symlink,parent,type:int,type_name,mode:int,owner_read:int,owner_write:int,owner_execute:int,group_read:int,group_write:int,group_execute:int,other_read:int,other_write:int,other_execute:int,setuid:int,setgid:int,sticky:int,owner_id:int,group_id:int,ctime,atime,full_path,mtime_sec:int,mtime_nsec:int,ctime_sec:int,ctime_nsec:int,atime_sec:int,atime_nsec:int,dev:int,ino:int,rdev:int,blksize:int,blocks:int
-rwxr-xr-x ,1,marcin,marcin,773,2019-12-23 01:24:58.478029642,csv-cat.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 01:24:58.478029642,2019-12-29 20:13:34.710830508,doc/csv-cat.1.md,1577060698,478029642,1577060698,478029642,1577646814,710830508,0x801,16122673,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,721,2019-12-23 02:15:21.074174728,csv-head.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:15:21.074174728,2019-12-29 20:13:34.710830508,doc/csv-head.1.md,1577063721,74174728,1577063721,74174728,1577646814,710830508,0x801,16122671,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,1673,2019-12-23 02:16:07.494064188,csv-nix-tools.7.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:16:07.494064188,2019-12-29 20:13:34.710830508,doc/csv-nix-tools.7.md,1577063767,494064188,1577063767,494064188,1577646814,710830508,0x801,16125292,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,1476,2019-12-23 02:43:08.227018848,csv-show.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:43:08.227018848,2019-12-29 20:13:34.710830508,doc/csv-show.1.md,1577065388,227018848,1577065388,227018848,1577646814,710830508,0x801,16125313,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,825,2019-12-23 01:25:09.873956973,csv-tac.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 01:25:09.873956973,2019-12-29 20:13:34.710830508,doc/csv-tac.1.md,1577060709,873956973,1577060709,873956973,1577646814,710830508,0x801,16124245,0,4096,8
-rwxr-xr-x ,1,marcin,marcin,710,2019-12-23 02:15:38.110135517,csv-tail.1.md,,doc,0100000,reg,0644,1,1,0,1,0,0,1,0,0,0,0,0,1000,1000,2019-12-23 02:15:38.110135517,2019-12-29 20:13:34.710830508,doc/csv-tail.1.md,1577063738,110135517,1577063738,110135517,1577646814,710830508,0x801,16125317,0,4096,8
```

All Makefiles in the current directory and below:

```
$ csv-ls -c size,parent,name -R . | csv-grep -c name -x -F Makefile
size:int,parent,name
55358,./build,Makefile
6345,./build/tests,Makefile
```

Files and their sizes sorted by size and name, in descending order:

```
$ csv-ls -c size,name | csv-sort -r -c size,name
size:int,name
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
$ csv-ls -R -c parent,name,owner_id / 2>/dev/null | csv-grep -c owner_id -x -F 1000 | csv-cut -c parent,name
```

Files owned by user=marcin:

```
$ csv-ls -R -c parent,name,owner_name / 2>/dev/null | csv-grep -c owner_name -x -F marcin | csv-cut -c parent,name
```

Files anyone can read:

```
$ csv-ls -R -c parent,name,other_read / 2>/dev/null | csv-grep -c other_read -x -F 1 | csv-cut -c parent,name
```

Sum of sizes of all files in the current directory:

```
$ csv-ls -l . | csv-sum -c size
sum(size):int
78739
```

4 biggest files in the current directory:

```
$ csv-ls -c size,name . | csv-sort -r -c size | csv-head -n 4
size:int,name
14660,ls.c
7238,sort.c
6297,parse.c
6189,grep.c
$ csv-ls -c size,name . | csv-sort -c size | csv-tail -n 4
...
```

List of files, split name into base and extension:

```
$ csv-ls -c size,name | csv-add-split -c name -e . -n base,ext -r
size:int,name,base,ext
123,file1,file1,
456,file2.ext,file2,ext
789,file3.ext.in,file3.ext,in
```

Sum of sizes of all files with the "png" extension:

```
$ csv-ls -l . |
  csv-add-split -c name -e . -n base,ext -r |
  csv-grep -c ext -x -F png |
  csv-sum -c size |
  csv-header --remove
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
-rwxr-xr-x  1 marcin marcin 5089 2019-12-23 00:23:36.438335394 avg.c           
-rwxr-xr-x  1 marcin marcin 5108 2019-12-23 00:22:44.362494798 cat.c           
-rwxr-xr-x  1 marcin marcin 5353 2019-12-14 22:19:16.197571951 concat.c        
```

List of files whose 2nd character is 'o':

```
$ csv-ls | csv-grep -c name -e '^.o'
name
concat.c
sort.c
```

or

```
$ csv-ls |
  csv-add-substring -c name -n 2nd-char -p 2 -l 1 |
  csv-grep -c 2nd-char -F o |
  csv-cut -c name
```

or

```
$ csv-ls | csv-grep-rpn -e "%name 2 1 substr 'o' =="
```

or

```
$ csv-ls | csv-grep-sql -e "substr(name, 2, 1) == 'o'"
```


Full paths of all files in the current directory and below:

```
$ csv-ls -R -c parent,name . | csv-add-concat full_path = %parent / %name | csv-cut -c full_path
```
or

```
$ csv-ls -R -c full_path .
```

Sum of file sizes and real allocated blocks in the current directory:

```
$ csv-ls -c size,blocks | csv-add-rpn -n space_used -e "%blocks 512 *" |
  csv-sum -c size,blocks,space_used
sum(size):int,sum(blocks):int,sum(space_used):int
109679,288,147456
```

or

```
$ csv-ls -c size,blocks | csv-add-sql -e "blocks * 512 AS space_used" |
  csv-sum -c size,blocks,space_used
```

List of files whose size is between 2000 and 3000 bytes (from the slowest to the fastest method):

```
$ csv-ls -c size,name | csv-sqlite "select size, name from input where size > 2000 and size < 3000"
size:int,name
2204,parse.h
```

or

```
$ csv-ls -c size,name |
  csv-add-rpn -n range2k-3k -e "%size 2000 > %size 3000 < and" |
  csv-grep -c range2k-3k -F 1 |
  csv-cut -c size,name
```

or

```
$ csv-ls -c size,name | csv-sql "select size, name from input where size > 2000 and size < 3000"
```

or

```
$ csv-ls -c size,name | csv-grep-sql -e "size > 2000 and size < 3000"
```

or

```
$ csv-ls -c size,name | csv-grep-rpn -e "%size 2000 > %size 3000 < and"
```


Files and their permissions printed in human-readable format (simpler version of column type_mode):

```
$ csv-ls -c mode,name | csv-add-rpn -n strmode -e "
%mode 0400 & 'r' '-' if        %mode 0200 & 'w' '-' if concat %mode 0100 & 'x' '-' if concat
%mode  040 & 'r' '-' if concat %mode  020 & 'w' '-' if concat %mode  010 & 'x' '-' if concat
%mode   04 & 'r' '-' if concat %mode   02 & 'w' '-' if concat %mode   01 & 'x' '-' if concat" |
csv-cut -c mode,strmode,name -s

mode   strmode     name
0644   rw-r--r--   CMakeCache.txt
0755   rwxr-xr-x   CMakeFiles
0600   rw-------   core
```
or

```
$ csv-ls -c mode,name | csv-sql "select tostring(mode, 8) as 'mode',
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
$ csv-ls -R -c full_path . | csv-grep -c full_path -e '~$' | csv-exec -- rm -f %full_path
```

If file has .c extension, then replace it with .o, otherwise leave it as is:

```
$ csv-ls -R -c full_path . | csv-add-exec -c full_path -n new -- sed 's/.c$/.o/'
```
or 130x faster:

```
$ csv-ls -R -c full_path . | csv-add-replace -c full_path -E '(.*)\.c$' -r '%1.o' -n new
```
or 400x faster (3.1x faster than csv-add-replace):

```
$ csv-ls -R -c full_path . |
  csv-add-rpn -n new -e "%full_path -1 1 substr 'c' == %full_path 1 %full_path strlen 1 - substr 'o' concat %full_path if"
```

or as fast as the above:

```
$ csv-ls -R -c full_path . |
  csv-add-sql -n new -e "if (substr(full_path, -1, 1) == 'c', substr(full_path, 1, strlen(full_path) - 1) || 'o', full_path)"
```


List all system users and the name of the default group they belong to:

```
$ csv-users -T | csv-groups -M -N grp |
  csv-sqlite -T "select user.name as user_name, grp.name as group_name from user, grp where user.gid = grp.gid"
```

List all network connections and processes they belong to:

```
$ csv-ls -T -c name,symlink /proc/*/fd/* 2>/dev/null |
  csv-grep -T file -c symlink -E "socket:\[[0-9]*\]" |
  csv-add-replace -c file.name -E '/proc/([0-9]*)/.*' -r '%1' -n file.pid |
  csv-add-replace -c file.symlink -E 'socket:\[([0-9]*)\]' -r %1 -n file.inode |
  csv-netstat -M |
  csv-grep -T socket -v -c family -x -F 'UNIX' |
  csv-ps -M -c pid,cmd |
  csv-sqlite -T '
    select socket.protocol, socket.src_ip, socket.src_port, socket.dst_ip, socket.dst_port,
           socket.state, socket.uid, file.pid, proc.cmd
      from socket
      left outer join file on socket.inode = file.inode
      left outer join proc on file.pid = proc.pid' -s

protocol   src_ip                src_port   dst_ip       dst_port   state          uid     pid   cmd
tcp        127.0.0.53                  53   0.0.0.0             0   LISTEN         102           
tcp        127.0.0.1                  631   0.0.0.0             0   LISTEN           0           
tcp        127.0.0.1                39455   0.0.0.0             0   LISTEN           0           
tcp        192.168.1.14             52980   $IP               443   ESTABLISHED   1000   16175   firefox
tcp        192.168.1.14             39390   $IP               443   ESTABLISHED   1000   16175   firefox
tcp        192.168.1.14             49640   $IP              8008   ESTABLISHED   1000    6504   chrome
tcp        192.168.1.14             39220   $IP               443   TIME_WAIT        0           
tcp        192.168.1.14             47420   $IP               443   ESTABLISHED   1000    6504   chrome
tcp        192.168.1.14             51424   $IP               443   ESTABLISHED   1000    6504   chrome
tcp        192.168.1.14             58330   $IP               443   ESTABLISHED   1000   16175   firefox
tcp        192.168.1.14             48254   $IP              8009   SYN_SENT      1000    6504   chrome
tcp        192.168.1.14             43784   $IP               443   TIME_WAIT        0           
tcp        192.168.1.14             54018   $IP               443   ESTABLISHED   1000    6504   chrome
tcp        192.168.1.14             48250   $IP              8009   FIN_WAIT1        0           
tcp        ::1                        631   ::                  0   LISTEN           0           
tcp        ::ffff:192.168.0.14      37610   ::ffff:$IP         80   CLOSE_WAIT    1000   27184   WebKitWebProces
tcp        ::ffff:192.168.0.14      37610   ::ffff:$IP         80   CLOSE_WAIT    1000    8337   java
udp        127.0.0.53                  53   0.0.0.0             0                  102           
udp        0.0.0.0                     68   0.0.0.0             0                    0           
udp        0.0.0.0                    631   0.0.0.0             0                    0           
udp        224.0.0.251               5353   0.0.0.0             0                 1000    6504   chrome
udp        224.0.0.251               5353   0.0.0.0             0                 1000    6470   chrome
udp        224.0.0.251               5353   0.0.0.0             0                 1000    6504   chrome
udp        0.0.0.0                   5353   0.0.0.0             0                  107           
udp        0.0.0.0                  46599   0.0.0.0             0                  107           
udp        ::                       59934   ::                  0                  107           
udp        ::                        5353   ::                  0                  107           
raw        ::                          58   ::                  0                    0           

```

Kill process $PID and all its descendants:

```
$ csv-ps | csv-tree --key pid --parent ppid --filter $PID | csv-exec kill %pid
```

or:

```
$ csv-pstree -f $PID | csv-exec kill %pid
```
