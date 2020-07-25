#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-sort -c id" data/id-column-3-rows.csv data/id-column-3-rows.csv data/empty.txt 0
	sort_sorted)

test("csv-sort -c id -r" data/id-column-3-rows.csv sort/rsorted.csv data/empty.txt 0
	sort_rsorted)

test("csv-sort -c id,name" sort/2-cols.csv sort/2-cols-sorted.csv data/empty.txt 0
	sort_2_cols)

test("csv-sort -c id,name -r" sort/2-cols.csv sort/2-cols-rsorted.csv data/empty.txt 0
	sort_2_cols_rsorted)

test("csv-sort -c id,name -s" sort/2-cols.csv sort/2-cols-sorted.txt data/empty.txt 0
	sort_2_cols_-s)

test("csv-sort -T t1 -c id" sort/2-tables.csv sort/2-tables-sorted1.csv data/empty.txt 0
	sort_2_tables_sort_1st)

test("csv-sort -T t2 -c col2" sort/2-tables.csv sort/2-tables-sorted2.csv data/empty.txt 0
	sort_2_tables_sort_2nd)

test("csv-sort --help" data/empty.csv sort/help.txt data/empty.txt 2
	sort_help)

test("csv-sort --version" data/empty.csv data/git-version.txt data/empty.txt 0
	sort_version)
