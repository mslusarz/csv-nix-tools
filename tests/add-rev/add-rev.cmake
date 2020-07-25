#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-add-rev -c name -n name-reversed"
	data/3-columns-3-rows.csv add-rev/string-reversed.csv data/empty.txt 0
	add-rev_string)

test("csv-add-rev -c name -n name-reversed -s"
	data/3-columns-3-rows.csv add-rev/string-reversed.txt data/empty.txt 0
	add-rev_string_-s)

test("csv-add-rev -c id -n id-reversed"
	data/3-columns-3-rows.csv data/empty.txt add-rev/rev-int.txt 2
	add-rev_int)

test("csv-add-rev -T t1 -c name -n rev-name" data/2-tables.csv add-rev/2-tables-add1.csv data/empty.txt 0
	add-rev_2tables-add1)

test("csv-add-rev --help" data/empty.csv add-rev/help.txt data/empty.txt 2
	add-rev_help)

test("csv-add-rev --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-rev_version)
