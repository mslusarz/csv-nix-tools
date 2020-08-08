# TODO (high level)
- more data collection tools (id, uname, w/who, df/mountinfo, fuser/lsof, free, top?, ifconfig/ip, ipcs, lsattr, lsusb, tcpdump?, route, lscpu, lshw, lsblk, lsns, last, etc)
- more processing tools (tr, other?)
- more rpn operators/functions (split, rev, timestamp conversion, now, tr)
- exporting tools (to-json)
- importing tools (from-xml, from-json)
- fix utf-8 support
- handle NUL bytes
- i18n
- export as much as possible as a library(ies)
- Coverity
- fuzzing

# TODO (low level)
- source-tools: add option to add and remove columns
- header: implement add, guess-types
- ps: lots of work, detailed in ps.c
- show: ability to lock columns (make them always visible)
- show: implement "search" and "help" in the ncurses backend
- sql: implement case, cast
- netstat: support more protocols
- netstat: translate interface number to interface name
- netstat: figure out how to print inet\_diag\_info
- sql: add support for tables
- sum,count,min,max,avg: --group-by=col1,col2
- uniq: --count
- show: fix column width calc when input is in utf
- sql: implement "group by", "having"
- cut: multiple input files
- ls: color output with -S
- sql/rpn: data(colname, relativerownum)
- csv-from-txt --format "year,-,month,-,day,hour,:,min,:,sec,\,,ms, ,level, ,file, ,message"

## Random ideas
- export data as bash/python/etc script?
- float support?
- importing from other tools (lspci -mm?, strace?, lsof -F, ss, dpkg, rpm)?
- loops and temporary variables in rpn?
- built-in pipes? (csv "ls | grep -c size -F 0 | cut -c name")
- shell?
- one multicommand binary? (csv ls, csv grep, ...)
- other sink tools: comm?, treeview (generic pstree)
- binary data?
- journalctl?
- netstat: add pid
- (auto?) input decompression/output compression
