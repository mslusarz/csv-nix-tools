#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

if (LIBMNL_FOUND)

test("csv-netstat | csv-count -c -R" data/empty.txt netstat/count-columns.csv data/empty.txt 0
	netstat_count_columns)

test("csv-netstat | csv-head -n 0" data/empty.txt netstat/header.csv data/empty.txt 0
	netstat_header)

test("csv-netstat -X | csv-head -n 0" data/empty.txt netstat/header-no-types.csv data/empty.txt 0
	netstat_header_no_types)

test("csv-netstat -l | csv-count -c -R" data/empty.txt netstat/l-count-columns.csv data/empty.txt 0
	netstat_l_count_columns)

test("csv-netstat -ll | csv-count -c -R" data/empty.txt netstat/ll-count-columns.csv data/empty.txt 0
	netstat_ll_count_columns)

test("csv-netstat -lll | csv-count -c -R" data/empty.txt netstat/lll-count-columns.csv data/empty.txt 0
	netstat_lll_count_columns)

test("csv-netstat -t | csv-count -c -R" data/empty.txt netstat/t-count-columns.csv data/empty.txt 0
	netstat_t_count_columns)

test("csv-netstat -tl | csv-count -c -R" data/empty.txt netstat/tl-count-columns.csv data/empty.txt 0
	netstat_tl_count_columns)

test("csv-netstat -tll | csv-count -c -R" data/empty.txt netstat/tll-count-columns.csv data/empty.txt 0
	netstat_tll_count_columns)

test("csv-netstat -tlll | csv-count -c -R" data/empty.txt netstat/tlll-count-columns.csv data/empty.txt 0
	netstat_tlll_count_columns)

test("csv-netstat -u | csv-count -c -R" data/empty.txt netstat/u-count-columns.csv data/empty.txt 0
	netstat_u_count_columns)

test("csv-netstat -ul | csv-count -c -R" data/empty.txt netstat/ul-count-columns.csv data/empty.txt 0
	netstat_ul_count_columns)

test("csv-netstat -ull | csv-count -c -R" data/empty.txt netstat/ull-count-columns.csv data/empty.txt 0
	netstat_ull_count_columns)

test("csv-netstat -ulll | csv-count -c -R" data/empty.txt netstat/ulll-count-columns.csv data/empty.txt 0
	netstat_ulll_count_columns)

test("csv-netstat -w | csv-count -c -R" data/empty.txt netstat/w-count-columns.csv data/empty.txt 0
	netstat_w_count_columns)

test("csv-netstat -wl | csv-count -c -R" data/empty.txt netstat/wl-count-columns.csv data/empty.txt 0
	netstat_wl_count_columns)

test("csv-netstat -wll | csv-count -c -R" data/empty.txt netstat/wll-count-columns.csv data/empty.txt 0
	netstat_wll_count_columns)

test("csv-netstat -wlll | csv-count -c -R" data/empty.txt netstat/wlll-count-columns.csv data/empty.txt 0
	netstat_wlll_count_columns)

test("csv-netstat -x | csv-count -c -R" data/empty.txt netstat/x-count-columns.csv data/empty.txt 0
	netstat_x_count_columns)

test("csv-netstat -xl | csv-count -c -R" data/empty.txt netstat/xl-count-columns.csv data/empty.txt 0
	netstat_xl_count_columns)

test("csv-netstat -xll | csv-count -c -R" data/empty.txt netstat/xll-count-columns.csv data/empty.txt 0
	netstat_xll_count_columns)

test("csv-netstat -xlll | csv-count -c -R" data/empty.txt netstat/xlll-count-columns.csv data/empty.txt 0
	netstat_xlll_count_columns)

test("csv-netstat -4 | csv-count -c -R" data/empty.txt netstat/4-count-columns.csv data/empty.txt 0
	netstat_4_count_columns)

test("csv-netstat -4l | csv-count -c -R" data/empty.txt netstat/4l-count-columns.csv data/empty.txt 0
	netstat_4l_count_columns)

test("csv-netstat -4ll | csv-count -c -R" data/empty.txt netstat/4ll-count-columns.csv data/empty.txt 0
	netstat_4ll_count_columns)

test("csv-netstat -4lll | csv-count -c -R" data/empty.txt netstat/4lll-count-columns.csv data/empty.txt 0
	netstat_4lll_count_columns)

test("csv-netstat -6 | csv-count -c -R" data/empty.txt netstat/6-count-columns.csv data/empty.txt 0
	netstat_6_count_columns)

test("csv-netstat -6l | csv-count -c -R" data/empty.txt netstat/6l-count-columns.csv data/empty.txt 0
	netstat_6l_count_columns)

test("csv-netstat -6ll | csv-count -c -R" data/empty.txt netstat/6ll-count-columns.csv data/empty.txt 0
	netstat_6ll_count_columns)

test("csv-netstat -6lll | csv-count -c -R" data/empty.txt netstat/6lll-count-columns.csv data/empty.txt 0
	netstat_6lll_count_columns)

test("csv-netstat -c family,protocol | csv-uniq -c family,protocol | csv-head -n 0" data/empty.txt netstat/f-count-columns.csv data/empty.txt 0
	netstat_f_count_columns)

test("csv-netstat -M -c family,inode,state | csv-head -n 0" data/3-columns-3-rows-with-table.csv netstat/columns-merged.csv data/empty.txt 0
	netstat_merged)

test("csv-netstat -M -c family,inode,state -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv netstat/columns-merged-table.csv data/empty.txt 0
	netstat_merged_table)

test("csv-netstat --help" data/empty.csv netstat/help.txt data/empty.txt 2
	netstat_help)

test("csv-netstat --version" data/empty.csv data/git-version.txt data/empty.txt 0
	netstat_version)

endif(LIBMNL_FOUND)
