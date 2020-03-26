---
title: csv-ps
section: 1
...

# NAME #

csv-ps - list processes in CSV format

# SYNOPSIS #

**csv-ps** [OPTION]...

# DESCRIPTION #

Print to standard output the list of system processes in the CSV format.

-c, \--columns=*NAME1*[,*NAME2*...]
:   choose the list of columns

-l
:   use a longer listing format (can be used up to 4 times)

-M, \--merge
:   merge output with a CSV stream in table form from standard input

-N, \--table-name *NAME*
:   produce output as table *NAME*

-p, \--pid=PID1[,PID2...]
:   select processes from this list

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--as-table
:   produce output as table *proc*

\--help
:   display this help and exit

\--version
:   output version information and exit

# COLUMNS #

| name                      | type     | description                     | level |
|---------------------------|----------|---------------------------------|-------|
| euid_name                 | string   |                                 | 0     |
| pid                       | int      |                                 | 0     |
| ppid                      | int      |                                 | 0     |
| %cpu                      | int      |                                 | 0     |
| vm_size_KiB               | int      |                                 | 0     |
| vm_rss_KiB                | int      |                                 | 0     |
| tty_id                    | int      |                                 | 0     |
| state                     | string   |                                 | 0     |
| start_time                | string   |                                 | 0     |
| cpu_time                  | string   |                                 | 0     |
| processor                 | int      |                                 | 0     |
| command                   | string   |                                 | 0     |
| euid                      | int      |                                 | 1     |
| tid                       | int      |                                 | 1     |
| cpu_time_ms               | int      |                                 | 1     |
| age                       | string   |                                 | 1     |
| egid_name                 | string   |                                 | 1     |
| ruid_name                 | string   |                                 | 1     |
| rgid_name                 | string   |                                 | 1     |
| suid_name                 | string   |                                 | 1     |
| sgid_name                 | string   |                                 | 1     |
| fuid_name                 | string   |                                 | 1     |
| fgid_name                 | string   |                                 | 1     |
| supgid_names              | string   |                                 | 1     |
| egid                      | int      |                                 | 2     |
| ruid                      | int      |                                 | 2     |
| rgid                      | int      |                                 | 2     |
| suid                      | int      |                                 | 2     |
| sgid                      | int      |                                 | 2     |
| fuid                      | int      |                                 | 2     |
| fgid                      | int      |                                 | 2     |
| supgid                    | string   |                                 | 2     |
| age_sec                   | int      |                                 | 2     |
| age_msec                  | int      |                                 | 2     |
| vm_size_B                 | int      |                                 | 2     |
| vm_lock_B                 | int      |                                 | 2     |
| vm_rss_B                  | int      |                                 | 2     |
| vm_rss_anon_B             | int      |                                 | 2     |
| vm_rss_file_B             | int      |                                 | 2     |
| vm_rss_shared_B           | int      |                                 | 2     |
| vm_data_B                 | int      |                                 | 2     |
| vm_stack_B                | int      |                                 | 2     |
| vm_swap_B                 | int      |                                 | 2     |
| vm_exe_B                  | int      |                                 | 2     |
| vm_lib_B                  | int      |                                 | 2     |
| priority                  | int      |                                 | 2     |
| nice                      | int      |                                 | 2     |
| cmd                       | string   |                                 | 3     |
| nlwp                      | int      |                                 | 3     |
| wchan                     | int      |                                 | 3     |
| pgrp                      | int      |                                 | 3     |
| session                   | int      |                                 | 3     |
| user_time_ms              | int      |                                 | 3     |
| system_time_ms            | int      |                                 | 3     |
| cumulative_user_time_ms   | int      |                                 | 3     |
| cumulative_system_time_ms | int      |                                 | 3     |
| start_time_sec            | int      |                                 | 3     |
| start_time_msec           | int      |                                 | 3     |
| rss_B                     | int      |                                 | 3     |
| rtprio                    | int      |                                 | 3     |
| sched                     | int      |                                 | 3     |
| min_flt                   | int      |                                 | 3     |
| maj_flt                   | int      |                                 | 3     |
| cmin_flt                  | int      |                                 | 3     |
| cmaj_flt                  | int      |                                 | 3     |
| tpgid                     | int      |                                 | 3     |
| size_B                    | int      |                                 | 3     |
| resident_B                | int      |                                 | 3     |
| share_B                   | int      |                                 | 3     |
| trs_B                     | int      |                                 | 3     |
| drs_B                     | int      |                                 | 3     |
| cmdline                   | string[] |                                 | 3     |
| alarm                     | int      |                                 | 4     |
| start_code                | int      |                                 | 4     |
| end_code                  | int      |                                 | 4     |
| start_stack               | int      |                                 | 4     |
| kstk_esp                  | int      |                                 | 4     |
| kstk_eip                  | int      |                                 | 4     |
| vsize_B                   | int      |                                 | 4     |
| rss_rlim                  | int      |                                 | 4     |
| flags                     | int      |                                 | 4     |
| exit_signal               | int      |                                 | 4     |
| pending_signals           | string   |                                 | 4     |
| blocked_signals           | string   |                                 | 4     |
| ignored_signals           | string   |                                 | 4     |
| caught_signals            | string   |                                 | 4     |
| pending_signals_per_task  | string   |                                 | 4     |
| oom_score                 | int      |                                 | 4     |
| oom_adj                   | int      |                                 | 4     |
| ns_ipc                    | int      |                                 | 4     |
| ns_mnt                    | int      |                                 | 4     |
| ns_net                    | int      |                                 | 4     |
| ns_pid                    | int      |                                 | 4     |
| ns_user                   | int      |                                 | 4     |
| ns_uts                    | int      |                                 | 4     |
| sd_mach                   | string   |                                 | 4     |
| sd_ouid                   | string   |                                 | 4     |
| sd_seat                   | string   |                                 | 4     |
| sd_sess                   | string   |                                 | 4     |
| sd_slice                  | string   |                                 | 4     |
| sd_unit                   | string   |                                 | 4     |
| sd_uunit                  | string   |                                 | 4     |
| lxcname                   | string   |                                 | 4     |
| environ                   | string   |                                 | 4     |
| cgroup                    | string[] |                                 | 4     |

# EXAMPLES #

`csv-ps -S`
:   browse all processes with custom pager

# SEE ALSO #

**[ps](http://man7.org/linux/man-pages/man1/ps.1.html)**(1),
**csv-show**(1), **csv-nix-tools**(7)
