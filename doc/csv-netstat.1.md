---
title: csv-netstat
section: 1
...

# NAME #

csv-netstat - list network connections in CSV format

# SYNOPSIS #

**csv-netstat** [OPTION]...

# DESCRIPTION #

Print to standard output the list of network sockets in the CSV format.

-c, \--columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-l
:   use a longer listing format (can be used up to 3 times)

-M, \--merge
:   merge output with a CSV stream in table form from standard input

-N, \--table-name *NAME*
:   produce output as table *NAME*

-r, \--resolve
:   resolve IPs and ports

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--as-table
:   produce output as table *socket*

-t, \--tcp
:   print information only about TCP sockets

-u, \--udp
:   print information only about UDP sockets

-w, \--raw
:   print information only about RAW sockets

-x, \--unix
:   print information only about UNIX sockets

-4, \--inet4
:   print information only about IPv4 sockets

-6, \--inet6
:   print information only about IPv6 sockets

\--help
:   display this help and exit

\--version
:   output version information and exit

# COLUMNS #

| name            | type   | description                             | level |
|-----------------|--------|-----------------------------------------|-------|
| family          | string | socket family (INET4, INET6, UNIX)      | 0     |
| protocol        | string | protocol (tcp, udp, raw)                | 0     |
| src_ip          | string | source IP                               | 0     |
| src_port        | int    | source port                             | 0     |
| dst_ip          | string | destination IP                          | 0     |
| dst_port        | int    | destination port                        | 0     |
| src_name        | string | source resolved name                    | 0  -r |
| src_port_name   | string | source port name                        | 0  -r |
| dst_name        | string | destination resolved name               | 0  -r |
| dst_port_name   | string | destination port name                   | 0  -r |
| state           | string | socket state                            | 0     |
| name            | string | name                                    | 0     |
| inode           | int    | local inode number                      | 0     |
| peer_inode      | int    | remote inode number                     | 0     |
| type            | string | type (DGRAM, PACKET, STREAM, SEQPACKET) | 0     |
| uid             | int    | user id of owner                        | 0     |
| interface       | int    | interface number                        | 0     |
| pending_conns   | int    | number of pending connections           | 1     |
| incoming_data   | int    | ?                                       | 1     |
| backlog_length  | int    | ?                                       | 1     |
| outgoing_data   | int    | ?                                       | 1     |
| rmem_alloc      | int    | ?                                       | 1     |
| rcvbuf          | int    | ?                                       | 1     |
| wmem_alloc      | int    | ?                                       | 1     |
| sndbuf          | int    | ?                                       | 1     |
| fwd_alloc       | int    | ? forward alloc                         | 1     |
| wmem_queued     | int    | ?                                       | 1     |
| optmem          | int    | ?                                       | 1     |
| backlog         | int    | ? backlog queue length                  | 1     |
| drops           | int    | ?                                       | 1     |
| shutdown        | int    | ?                                       | 1     |
| vfs_dev         | int    | ?                                       | 1     |
| vfs_ino         | int    | ?                                       | 1     |
| timer           | string | ?                                       | 1     |
| retransmits     | int    | ? number of retransmitted packets       | 1     |
| expires_ms      | int    | ? timer expiration in milliseconds      | 1     |
| rmem            | int    | ?                                       | 1     |
| wmem            | int    | ?                                       | 1     |
| fmem            | int    | ?                                       | 1     |
| tmem            | int    | ?                                       | 1     |
| ipv6only        | int    | is socket IPv6 only                     | 1     |
| user_protocol   | string | protocol for raw sockets                | 1     |
| cookie0         | int    | ?                                       | 2     |
| cookie1         | int    | ?                                       | 2     |
| icons           | int[]  | ?                                       | 2     |
| cong            | string | name of congestion control              | 2     |
| tos             | int    | ? type of service                       | 2     |
| tclass          | int    | ? traffic class                         | 2     |
| locals          | string | ?                                       | 2     |
| peers           | string | ?                                       | 2     |
| pad             | int    | ?                                       | 2     |
| mark            | int    | ? (SO_MARK, man 7 socket)               | 2     |
| class_id        | int    | ?                                       | 2     |
| md5sig          | string | ?                                       | 2     |
| vegas_enabled   | int    | ? TCP Vegas congestion control enabled  | 2     |
| vegas_rttcnt    | int    | ? number of RTTs measured within        | 2     |
|                 |        | last RTT                                |       |
| vegas_rtt       | int    | ? the minimum value of all Vegas RTT    | 2     |
|                 |        | measurements seen (usec)                |       |
| vegas_minrtt    | int    | ? minimum of RTTs measured within       | 2     |
|                 |        | last RTT (usec)                         |       |
| dctcp_enabled   | int    | ? DCTCP congestion control enabled      | 2     |
| dctcp_ce_state  | int    | ? 0/1                                   | 2     |
| dctcp_alpha     | int    | ?                                       | 2     |
| dctcp_ab_ecn    | int    | ? acknowledged bytes ecn                | 2     |
| dctcp_ab_tot    | int    | ? acknowledged bytes total              | 2     |
| bbr_bw_lo       | int    | ? BBR congestion control, bw lo 32bit   | 2     |
| bbr_bw_hi       | int    | ? bw hi 32bit                           | 2     |
| bbr_min_rtt     | int    | ? min-filtered RTT in usec              | 2     |
| bbr_pacing_gain | int    | ? pacing gain << 8                      | 2     |
| bbr_cwnd_gain   | int    | ? cwnd gain << 8                        | 2     |
| info            | string | ?                                       | 3     |

# EXAMPLES #

csv-netstat -ts
:   print information about all TCP sockets

csv-netstat -46s
:   print information about all IPv4 and IPv6 sockets

# SEE ALSO #

**[netstat](http://man7.org/linux/man-pages/man8/netstat.8.html)**(8),
**csv-show**(1), **csv-nix-tools**(7)
