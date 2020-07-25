#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-groups | csv-count -c -R" data/empty.txt groups/count-columns.csv data/empty.txt 0
	groups_count)

test("csv-groups -l | csv-count -c -R" data/empty.txt groups/count-columns-l.csv data/empty.txt 0
	groups_-l_count)

test("csv-groups -M | csv-head -n 0" data/3-columns-3-rows-with-table.csv groups/columns-merged.csv data/empty.txt 0
	groups_merged)

test("csv-groups -M -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv groups/columns-merged-table.csv data/empty.txt 0
	groups_merged_table)

test("csv-groups --help" data/empty.csv groups/help.txt data/empty.txt 2
	groups_help)

test("csv-groups --version" data/empty.csv data/git-version.txt data/empty.txt 0
	groups_version)
