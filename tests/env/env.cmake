#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-env | csv-count -c -R"
	data/empty.txt env/columns.csv data/empty.txt 0
	env_count)

test("csv-env | csv-head -n 0"
	data/empty.txt env/header.csv data/empty.txt 0
	env_header)

test("csv-env -X | csv-head -n 0"
	data/empty.txt env/header-no-types.csv data/empty.txt 0
	env_header_no_types)

test("csv-env -c value | csv-count -c -R"
	data/empty.txt env/columns-only-value.csv data/empty.txt 0
	env_-c)

test("csv-env -M | csv-head -n 0" data/3-columns-3-rows-with-table.csv env/columns-merged.csv data/empty.txt 0
	env_merged)

test("csv-env -M -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv env/columns-merged-table.csv data/empty.txt 0
	env_merged_table)

test("csv-env --help" data/empty.csv env/help.txt data/empty.txt 2
	env_help)

test("csv-env --version" data/empty.csv data/git-version.txt data/empty.txt 0
	env_version)
