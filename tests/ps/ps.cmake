#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

if (LIBPROCPS_FOUND)

test("csv-ps | csv-count -c -R" data/empty.txt ps/columns.csv data/empty.txt 0
	ps_columns)

test("csv-ps -l | csv-count -c -R" data/empty.txt ps/columns-l.csv data/empty.txt 0
	ps_-l_columns)

test("csv-ps -ll | csv-count -c -R" data/empty.txt ps/columns-ll.csv data/empty.txt 0
	ps_-ll_columns)

test("csv-ps -lll | csv-count -c -R" data/empty.txt ps/columns-lll.csv data/empty.txt 0
	ps_-lll_columns)

test("csv-ps -llll | csv-count -c -R" data/empty.txt ps/columns-llll.csv data/empty.txt 0
	ps_-llll_columns)

test("csv-ps -M -c tid,cmd,age | csv-head -n 0" data/3-columns-3-rows-with-table.csv ps/columns-merged.csv data/empty.txt 0
	ps_merged)

test("csv-ps -M -c tid,cmd,age -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv ps/columns-merged-table.csv data/empty.txt 0
	ps_merged_table)

test("csv-ps --help" data/empty.csv ps/help.txt data/empty.txt 2
	ps_help)

test("csv-ps --version" data/empty.csv data/git-version.txt data/empty.txt 0
	ps_version)

endif(LIBPROCPS_FOUND)
