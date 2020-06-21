# TODO (high level)
- more data collection tools (id, uname, w/who, df, free, ifconfig/ip, ipcs, lsattr, lsusb, tcpdump?, route, lscpu, lshw, lsblk, lsns, last, etc)
- more processing tools (tr, other?)
- more rpn operators/functions (split, rev, timestamp conversion, now, tr)
- exporting tools (to-xml, to-json, to-sql)
- importing tools (from-xml, from-json)
- test regularly with Valgrind
- i18n
- export as much as possible as a library(ies)
- Coverity
- fuzzing

# TODO (low level)
- source-tools: add option to add and remove columns
- header: implement add, add-types, change-type, guess-type, remove-types, rename
- ps: lots of work, detailed in ps.c
- show: ability to lock columns (make them always visible)
- show: implement "search" and "help" in the ncurses backend
- sql: implement case, cast
- netstat: support more protocols
- netstat: translate interface number to interface name
- netstat: figure out how to print inet\_diag\_info
- sql: add support for tables
- write down data format spec
- strict column name verification
- sum,count,min,max,avg: --group-by=col1,col2
- uniq: --count
- show: fix column width calc when input is in utf
- sql: implement "group by", "having"

## Random ideas
- export data as bash/python/etc script?
- float support?
- importing from other tools (lspci -mm?, strace?, lsof -F, ss, dpkg, rpm)?
- loops and temporary variables in rpn?
- built-in pipes? (csv "ls | grep -c size -F 0 | cut -c name")
- shell?
- one multicommand binary? (csv ls, csv grep, ...)
- show: column separators? header separator? number formatter? (see what csvlook from csvkit does)
- show: browser (html) backend?
- other sink tools: diff/comm?, treeview (generic pstree)
- csv -> csvnt?
- graph generator?
