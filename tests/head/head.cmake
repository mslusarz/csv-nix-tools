#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-head" data/empty.csv data/empty.csv data/eof.txt 2
	head_empty_input)

test("csv-head" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_simple_pass_through)

test("csv-head --lines=0" data/one-column-one-row.csv head/zero-lines.csv data/empty.txt 0
	head_--lines=0)

test("csv-head -n 0" data/one-column-one-row.csv head/zero-lines.csv data/empty.txt 0
	head_-n_0)

test("csv-head --lines 1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_--lines=1)

test("csv-head -n 1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_-n_1)

test("csv-head -n 1 -s" data/one-column-one-row.csv head/one-column-one-row.txt data/empty.txt 0
	head_-n_1_-s)

test("csv-head --lines 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_--lines=2_but_there_is_only_1_line_in_input)

test("csv-head -n 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_-n_2_but_there_is_only_1_line_in_input)

test("csv-head --lines=2" data/3-columns-3-rows.csv head/3-columns-2-first-rows.csv data/empty.txt 0
	head_--lines=2_and_there_are_3_lines_in_input)

test("csv-head -n 2" data/3-columns-3-rows.csv head/3-columns-2-first-rows.csv data/empty.txt 0
	head_-n_2_and_there_are_3_lines_in_input)

test("csv-head --lines=3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	head_--lines=3_and_there_are_3_lines_in_input)

test("csv-head -n 3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	head_-n_3_and_there_are_3_lines_in_input)

test("csv-head --help" data/empty.csv head/help.txt data/empty.txt 2
	head_help)

test("csv-head --version" data/empty.csv data/git-version.txt data/empty.txt 0
	head_version)
