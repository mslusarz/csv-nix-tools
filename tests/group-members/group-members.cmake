#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-group-members | csv-count -c -R" data/empty.txt group-members/count-columns.csv data/empty.txt 0
	group-members)

test("csv-group-members -M | csv-head -n 0" data/3-columns-3-rows-with-table.csv group-members/columns-merged.csv data/empty.txt 0
	group-members_merged)

test("csv-group-members -M -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv group-members/columns-merged-table.csv data/empty.txt 0
	group-members_merged_table)

test("csv-group-members --help" data/empty.csv group-members/help.txt data/empty.txt 2
	group-members_help)

test("csv-group-members --version" data/empty.csv data/git-version.txt data/empty.txt 0
	group-members_version)
