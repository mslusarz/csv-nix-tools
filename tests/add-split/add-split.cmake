#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-add-split -c column3 -n before,after -e,"
	data/commas.csv add-split/commas.csv data/empty.txt 0
	add-split_commas)

test("csv-add-split -c column3 -n before,after -e, -r"
	data/commas.csv add-split/commas-reverse.csv data/empty.txt 0
	add-split_commas_reverse)

test("csv-add-split -c column3 -n before,after -e, -s"
	data/commas.csv add-split/commas.txt data/empty.txt 0
	add-split_commas_-s)

test("csv-add-split -T t1 -c name -e ' ' -n first_word,rest" data/2-tables.csv add-split/2-tables-add1.csv data/empty.txt 0
	add-split_2tables-add1)

test("csv-add-split --help" data/empty.csv add-split/help.txt data/empty.txt 2
	add-split_help)

test("csv-add-split --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-split_version)
