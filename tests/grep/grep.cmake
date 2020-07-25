#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-grep" data/one-column-one-row.csv data/empty.csv grep/help.txt 2
	grep_no_args)

test("csv-grep -c x -F y" data/empty.csv data/empty.csv data/eof.txt 2
	grep_empty_input)


test("csv-grep -c name -e or" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-e_or)

test("csv-grep -c name -e or -s" data/3-columns-3-rows.csv grep/name-or.txt data/empty.txt 0
	grep_-c_name_-e_or_-s)

test("csv-grep -c name -x -e or" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-x_-e_or)

test("csv-grep -c name -e or.m" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-e_or.m)


test("csv-grep -c name -E or" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-E_or)

test("csv-grep -c name -x -E or" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-x_-E_or)

test("csv-grep -c name -E or.m" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-E_or.m)


test("csv-grep -c name -F or" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-F_or)

test("csv-grep -c name -x -F or" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-x_-F_or)

test("csv-grep -c name -F or.m" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-F_or.m)


test("csv-grep -c name -e or -v" data/3-columns-3-rows.csv grep/name-or-not.csv data/empty.txt 0
	grep_-c_name_-e_or_-v)

test("csv-grep -T t1 -c name -e or" grep/2-tables.csv grep/2-tables-t1-or.csv data/empty.txt 0
	grep_-T_t1_-c_name_-e_or)

test("csv-grep --help" data/empty.csv grep/help.txt data/empty.txt 2
	grep_help)

test("csv-grep --version" data/empty.csv data/git-version.txt data/empty.txt 0
	grep_version)
