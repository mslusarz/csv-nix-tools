#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-users | csv-count -c -R"
	data/empty.txt users/columns.csv data/empty.txt 0
	users_count)

test("csv-users -l | csv-count -c -R"
	data/empty.txt users/columns-l.csv data/empty.txt 0
	users_-l_count)

test("csv-users -ll | csv-count -c -R"
	data/empty.txt users/columns-ll.csv data/empty.txt 0
	users_-ll_count)

test("csv-users -M | csv-head -n 0" data/3-columns-3-rows-with-table.csv users/columns-merged.csv data/empty.txt 0
	users_merged)

test("csv-users -M -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv users/columns-merged-table.csv data/empty.txt 0
	users_merged_table)

test("csv-users --help" data/empty.csv users/help.txt data/empty.txt 2
	users_help)

test("csv-users --version" data/empty.csv data/git-version.txt data/empty.txt 0
	users_version)
