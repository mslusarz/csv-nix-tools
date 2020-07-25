#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-count" data/empty.txt data/empty.csv count/help.txt 2
	count_no_args)

test("csv-count -c" data/empty.txt data/empty.csv data/eof.txt 2
	count_empty_input)

test("csv-count -c" data/2-columns-3-rows.csv count/2-cols.txt data/empty.txt 0
	count_-c_2_cols)

test("csv-count -c -R" data/2-columns-3-rows.csv count/2-cols.txt data/empty.txt 0
	count_-c_2_cols_-R)

test("csv-count -r" data/2-columns-3-rows.csv count/3-rows.txt data/empty.txt 0
	count_-r_3_rows)

test("csv-count -c -r" data/2-columns-3-rows.csv count/2-cols-3-rows.txt data/empty.txt 0
	count_-c_-r_2_cols_3_rows)

test("csv-count -c -r -s" data/2-columns-3-rows.csv count/2-cols-3-rows-s.txt data/empty.txt 0
	count_-c_-r_2_cols_3_rows-s)

test("csv-count --help" data/empty.csv count/help.txt data/empty.txt 2
	count_help)

test("csv-count --version" data/empty.csv data/git-version.txt data/empty.txt 0
	count_version)
