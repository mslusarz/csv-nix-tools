#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-grep" data/one-column-one-row.csv data/empty.csv grep/help.txt 2
	grep_no_args)

test("csv-grep -c x -F y" data/empty.csv data/empty.csv data/eof.txt 2
	grep_empty_input)


test("csv-grep -c name -e or" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-e_or)

test("csv-grep -c name -e or -s" data/3-columns-3-rows.csv grep/name-or.txt data/empty.txt 0
	grep_-c_name_-e_or_-s)

test("csv-grep -c name -e or -S" data/3-columns-3-rows.csv grep/name-or.txt data/empty.txt 0
	grep_-c_name_-e_or_-S)

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

if (TEST_UTF8)
set(REQ_LOCALE "en_US.UTF-8")

test("csv-grep -c col1 -e St.phane" data/utf8.csv grep/utf8.csv data/empty.txt 0
	grep_utf8_bregex)

test("csv-grep -c col1 -E St.phane" data/utf8.csv grep/utf8.csv data/empty.txt 0
	grep_utf8_eregex)

test("csv-grep -c col1 -F Stéphane" data/utf8.csv grep/utf8.csv data/empty.txt 0
	grep_utf8_string)

test("csv-grep -c col1 -x -F Stéphane" data/utf8.csv grep/utf8.csv data/empty.txt 0
	grep_utf8_string_whole)

test("csv-grep -c col1 -i -F stéphane" data/utf8.csv grep/utf8.csv data/empty.txt 0
	grep_utf8_string_ignorecase)

test("csv-grep -c col1 -x -i -F stéphane" data/utf8.csv grep/utf8.csv data/empty.txt 0
	grep_utf8_string_while_ignorecase)

set(REQ_LOCALE "C")
endif()

test("csv-grep --help" data/empty.csv grep/help.txt data/empty.txt 2
	grep_help)

test("csv-grep --version" data/empty.csv data/git-version.txt data/empty.txt 0
	grep_version)
