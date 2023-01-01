#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-min -c id" data/id-column-3-rows.csv min/id.csv data/empty.txt 0
	min)

test("csv-min -c id -n new" data/id-column-3-rows.csv min/id-n.csv data/empty.txt 0
	min-n)

test("csv-min -c col1,col2,col3" data/3-numeric-columns-4-rows.csv min/3-columns.csv data/empty.txt 0
	min-3-columns)

test("csv-min -c col3,col1" data/3-numeric-columns-4-rows.csv min/2-columns.csv data/empty.txt 0
	min-2-columns)

test("csv-min -c col3,col1 -n new1" data/3-numeric-columns-4-rows.csv min/2-columns-n1.csv data/empty.txt 0
	min-2-columns-n1)

test("csv-min -c col3,col1 -n new1,new2" data/3-numeric-columns-4-rows.csv min/2-columns-n2.csv data/empty.txt 0
	min-2-columns-n2)

test("csv-min -c col3,col1 -s" data/3-numeric-columns-4-rows.csv min/2-columns.txt data/empty.txt 0
	min-2-columns-s)

test("csv-min -c col1,col2,col3,col4" data/text1.csv min/text1.csv data/empty.txt 0
	min-text1)

test("csv-min -T t1 -c id,something" data/2-tables.csv min/2-tables-min1.csv data/empty.txt 0
	min-2tables-min1)

test("csv-min -T t1 -c id,something -n new1" data/2-tables.csv min/2-tables-min1-n1.csv data/empty.txt 0
	min-2tables-min1-n1)

test("csv-min -T t1 -c id,something -n new1,new2" data/2-tables.csv min/2-tables-min1-n2.csv data/empty.txt 0
	min-2tables-min1-n2)

test("csv-min -T t2 -c id" data/2-tables.csv min/2-tables-min2.csv data/empty.txt 0
	min-2tables-min2)

test("csv-min -T t2 -c id -n new1" data/2-tables.csv min/2-tables-min2-n1.csv data/empty.txt 0
	min-2tables-min2-n1)

test("csv-min -c id,col1,col2,col3" data/floats.csv min/floats.csv data/empty.txt 0
	min-floats)

test("csv-min --help" data/empty.csv min/help.txt data/empty.txt 2
	min_help)

test("csv-min --version" data/empty.csv data/git-version.txt data/empty.txt 0
	min_version)
