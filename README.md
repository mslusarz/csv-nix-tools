**csv-nix-tools**
=================

csv-nix-tools is a collection of tools for gathering and processing system information using csv as an intermediate format.

This project is in early prototyping stage.

# Requirements
- cmake >= 3.3
- glibc-devel

# How to build
```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

# Examples

raw data about one file
```
$ csv-ls build/Makefile
size:int,type:int,mode:int,owner_id:int,group_id:int,nlink:int,mtime_sec:int,mtime_nsec:int,ctime_sec:int,ctime_nsec:int,atime_sec:int,atime_nsec:int,dev:int,ino:int,rdev:int,blksize:int,blocks:int,symlink:string,parent:string,name:string
18746,0100000,0644,1000,1000,1,1557072822,286415067,1557072822,286415067,1557072849,958066747,0x801,16126121,0,4096,40,,,build/Makefile
```

raw and processed data about one file
```
$ csv-ls -l build/Makefile
size:int,type:int,mode:int,owner_id:int,group_id:int,nlink:int,mtime_sec:int,mtime_nsec:int,ctime_sec:int,ctime_nsec:int,atime_sec:int,atime_nsec:int,dev:int,ino:int,rdev:int,blksize:int,blocks:int,type:string,owner_name:string,group_name:string,owner_read:bool,owner_write:bool,owner_execute:bool,group_read:bool,group_write:bool,group_execute:bool,other_read:bool,other_write:bool,other_execute:bool,setuid:bool,setgid:bool,sticky:bool,mtime:string,ctime:string,atime:string,symlink:string,parent:string,name:string,full_path:string
18746,0100000,0644,1000,1000,1,1557072822,286415067,1557072822,286415067,1557072849,958066747,0x801,16126121,0,4096,40,reg,marcin,marcin,1,1,0,1,0,0,1,0,0,0,0,0,2019-05-05 18:13:42.286415067,2019-05-05 18:13:42.286415067,2019-05-05 18:14:09.958066747,,,build/Makefile,build/Makefile
```

all Makefiles in current directory and below
```
$ csv-ls -R . | csv-grep -e name=Makefile | csv-cut -f size,parent,name
size:int,parent:string,name:string
18746,./build,Makefile
```

files and their sizes sorted by size and name, in descending order
```
$ csv-ls | csv-sort -r -f size,name | csv-cut -f size,name
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
3006,rows.c
2794,README.md
2746,utils.c
2676,columns.c
2389,format.c
2204,parse.h
1957,utils.h
1611,CMakeLists.txt
1472,LICENSE
1044,cmake_uninstall.cmake.in
```

files owned by user=1000
```
$ csv-ls -R / 2>/dev/null | csv-grep -e owner_id=1000 | csv-cut -f parent,name
...
```
or by name
```
$ csv-ls -lR / 2>/dev/null | csv-grep -e owner_name=marcin | csv-cut -f parent,name
...
```

files anyone can read
```
$ csv-ls -lR / 2>/dev/null | csv-grep -e other_read=1 | csv-cut -f parent,name
...
```

sum of sizes of all files in the current directory
```
$ csv-ls . | csv-sum -f size
sum(size):int
78739
```

4 biggest files in the current directory
```
$ csv-ls . | csv-sort -r -f size | csv-head -n 4 | csv-cut -f size,name
size:int,name:string
14660,ls.c
7238,sort.c
6297,parse.c
6189,grep.c
$ csv-ls . | csv-sort -f size | csv-tail -n 4 | csv-cut -f size,name
...
```

files with names splitted into base and extension
```
$ csv-ls -f size,name | csv-split -f name -e . -n base,ext -r
size:int,name:string,base:string,ext:string
123,file1,file1,
456,file2.ext,file2,ext
789,file3.ext.in,file3.ext,in
```

sum of sizes of all files with "png" extension
```
$ csv-ls . | csv-split -f name -e . -n base,ext -r | \
csv-grep -e ext=png | csv-sum -f size --no-header
94877
```

number of files in the current directory, without header
```
$ csv-ls . | csv-rows --no-header
19
```

list of files formatted in human-readable format (similar to ls -l) with disabled pager
```
$ csv-ls -l | csv-cut -f mode,nlink,owner_name,group_name,size,mtime,name | \
csv-show -s 1 -p no --no-header
0644 1  someuser    somegroup    1234     2019-04-23 20:17:58.331813826 file1
0644 1  someuser    somegroup    30381380 2019-04-23 20:18:25.539676175 file2
0644 12 anotheruser anothergroup 897722   2019-04-24 23:21:46.644869396 file3
```

list of files whose 2nd character is 'o'
```
$ csv-ls | csv-grep -e 'name=^.o' | csv-cut -f name --no-header
columns.c
concat.c
rows.c
sort.c
```

or

```
$ csv-ls | csv-substring -f name -n 2nd-char -p 1 -l 1 | \
csv-grep -e 2nd-char=o | csv-cut -f name --no-header
columns.c
concat.c
rows.c
sort.c
```

full paths of all files in current directory and below
```
$ csv-ls -R -f parent,name . | \
csv-concat -f parent -c "/" -f name -n full_path | csv-cut -f full_path
....
```
or
```
$ csv-ls -R -f full_path .
```

sum of file sizes and real allocated blocks in the current directory
```
$ csv-ls | csv-cut -f size,blocks | csv-rpn-add -e "space_used=%blocks 512 *" | \
csv-sum -f size,blocks,space_used
sum(size):int,sum(blocks):int,sum(space_used):int
109679,288,147456
```

list of files whose size is between 2000 and 3000 bytes
```
$ csv-ls -f size,name | \
csv-rpn-add -e "range2k-3k=%size 2000 >= %size 3000 < and" | \
csv-grep -e range2k-3k=1 | csv-cut -f size,name
size:int,name:string
2893,columns.c
2204,parse.h
```
or
```
$ csv-ls -f size,name | csv-rpn-filter -e "%size 2000 >= %size 3000 < and"
size:int,name:string
2893,columns.c
2204,parse.h
```
or
```
$ csv-ls | csv-sqlite "select size, name from input where size > 2000 and size < 3000"
size:int,name:string
2893,columns.c
2204,parse.h
```


files and their permissions printed in human-readable format
```
$ csv-ls -f mode,name | csv-rpn-add -e "strmode=\
%mode 0400 & 'r' '-' if        %mode 0200 & 'w' '-' if concat %mode 0100 & 'x' '-' if concat \
%mode  040 & 'r' '-' if concat %mode  020 & 'w' '-' if concat %mode  010 & 'x' '-' if concat \
%mode   04 & 'r' '-' if concat %mode   02 & 'w' '-' if concat %mode   01 & 'x' '-' if concat" | \
csv-cut -f mode,strmode,name

mode:int,strmode:string,name:string
0644,rw-r--r--,CMakeCache.txt
0755,rwxr-xr-x,CMakeFiles
0600,rw-------,core
...
```

remove all temporary files (ending with "~"), even if path contains spaces or line breaks
```
$ csv-ls -R -f full_path . | csv-grep -e 'full_path=~$' | csv-exec -- rm -f %full_path
```

if file has .c extension, then replace it with .o, otherwise leave it as is
```
$ csv-ls -R -f full_path . | csv-exec-add -f full_path -n new -- sed 's/.c$/.o/'
```
or 400x faster:
```
$ csv-ls -R -f full_path . | csv-rpn-add -e "new=%full_path -1 1 substr 'c' == %full_path 0 %full_path strlen 1 - substr 'o' concat %full_path if"
```

# TODO (high level)
- more processing tools (tr, sed, uniq, rev, drop, paste?, etc)
- more data collection tools (ps, find, df, netstat, ifconfig/ip?, lsattr, lsusb, readlink, tcpdump?, accounts, etc)
- more rpn operators/functions (split, rev, base64enc/dec, timestamp conversion, now, regex, sed, tr)
- exporting tools (to-xml, to-json, to-sql)
- importing tools (from-xml, from-json)
- export as much as possible as a library(ies)
- tests
- CI
- documentation
- i18n

# TODO (low level)
- switch to deal with new lines in shell-compatible way (see what coreutils' ls does)

## Random ideas
- float support?
- importing from other tools (lspci -mm?, strace?, lsof -F, ss)?
- tool for encoding strings in safe for transport way (base64? just hex?)
- loops and temporary variables in rpn?
- built-in pipes?
- what about unicode?
- one multicommand binary?
