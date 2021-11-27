#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-tail" data/empty.csv data/empty.csv data/eof.txt 2
	tail_empty_input)

test("csv-tail" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_simple_pass_through)

test("csv-tail --lines=0" data/one-column-one-row.csv tail/zero-lines.csv data/empty.txt 0
	tail_--lines=0)

test("csv-tail -n 0" data/one-column-one-row.csv tail/zero-lines.csv data/empty.txt 0
	tail_-n_0)

test("csv-tail -n meh" data/one-column-one-row.csv data/empty.txt tail/invalid-num.txt 2
	tail_-n_invalid)

test("csv-tail --lines=1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_--lines=1)

test("csv-tail -n 1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_-n_1)

test("csv-tail --lines 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_--lines=2_but_there_is_only_1_line_in_input)

test("csv-tail -n 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_-n_2_but_there_is_only_1_line_in_input)

test("csv-tail --lines=2" data/3-columns-3-rows.csv tail/3-columns-2-last-rows.csv data/empty.txt 0
	tail_--lines=2_but_there_are_3_lines_in_input)

test("csv-tail -n 2" data/3-columns-3-rows.csv tail/3-columns-2-last-rows.csv data/empty.txt 0
	tail_-n_2_but_there_are_3_lines_in_input)

test("csv-tail --lines=3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	tail_--lines=3_and_there_are_3_lines_in_input)

test("csv-tail -n 3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	tail_-n_3_and_there_are_3_lines_in_input)

test("csv-tail -n 2 -s" data/3-columns-3-rows.csv tail/tail-n-2-s.txt data/empty.txt 0
	tail_-n_2_-s)

test("csv-tail -n 2 -S" data/3-columns-3-rows.csv tail/tail-n-2-s.txt data/empty.txt 0
	tail_-n_2_-S)

test("csv-tail --help" data/empty.csv tail/help.txt data/empty.txt 2
	tail_help)

test("csv-tail --version" data/empty.csv data/git-version.txt data/empty.txt 0
	tail_version)
