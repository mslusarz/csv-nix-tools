#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-add-substring -c name -n substr2 -p 2"
	data/3-columns-3-rows.csv add-substring/p2.csv data/empty.txt 0
	add-substring_p2)

test("csv-add-substring -c name -n substr2 -p 2 -s"
	data/3-columns-3-rows.csv add-substring/p2.txt data/empty.txt 0
	add-substring_p2_-s)

test("csv-add-substring -c name -n substr2 -p 2 -S"
	data/3-columns-3-rows.csv add-substring/p2.txt data/empty.txt 0
	add-substring_p2_-S)

test("csv-add-substring -c name -n last_char -p -1 -l 1"
	data/3-columns-3-rows.csv add-substring/p-1.csv data/empty.txt 0
	add-substring_p_1)

test("csv-add-substring -T t1 -c name -p 2 -l 4 -n substr" data/2-tables.csv add-substring/2-tables-add1.csv data/empty.txt 0
	add-substring_2tables-add1)

test("csv-add-substring --help" data/empty.csv add-substring/help.txt data/empty.txt 2
	add-substring_help)

test("csv-add-substring --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-substring_version)
