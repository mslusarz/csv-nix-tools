#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-avg -c id" data/id-column-3-rows.csv avg/id.csv data/empty.txt 0
	avg)

test("csv-avg -c id -n new" data/id-column-3-rows.csv avg/id-n.csv data/empty.txt 0
	avg-n)

test("csv-avg -c col1,col2,col3" data/3-numeric-columns-4-rows.csv avg/3-columns.csv data/empty.txt 0
	avg-3-columns)

test("csv-avg -c col3,col1" data/3-numeric-columns-4-rows.csv avg/2-columns.csv data/empty.txt 0
	avg-2-columns)

test("csv-avg -c col3,col1 -n new1" data/3-numeric-columns-4-rows.csv avg/2-columns-n1.csv data/empty.txt 0
	avg-2-columns-n1)

test("csv-avg -c col3,col1 -n new1,new2" data/3-numeric-columns-4-rows.csv avg/2-columns-n2.csv data/empty.txt 0
	avg-2-columns-n2)

test("csv-avg -c col3,col1 -s" data/3-numeric-columns-4-rows.csv avg/2-columns-s.txt data/empty.txt 0
	avg-2-columns-s)

test("csv-avg -T t1 -c id,something" data/2-tables.csv avg/2-tables-avg1.csv data/empty.txt 0
	avg-2tables-avg1)

test("csv-avg -T t1 -c id,something -n new1" data/2-tables.csv avg/2-tables-avg1-n1.csv data/empty.txt 0
	avg-2tables-avg1-n1)

test("csv-avg -T t1 -c id,something -n new1,new2" data/2-tables.csv avg/2-tables-avg1-n2.csv data/empty.txt 0
	avg-2tables-avg1-n2)

test("csv-avg -T t2 -c id" data/2-tables.csv avg/2-tables-avg2.csv data/empty.txt 0
	avg-2tables-avg2)

test("csv-avg -T t2 -c id -n new1" data/2-tables.csv avg/2-tables-avg2-n1.csv data/empty.txt 0
	avg-2tables-avg2-n1)

test("csv-avg -c val" avg/0rows.csv avg/0rows-avg.csv data/empty.txt 0
	avg-0-rows)

test("csv-avg -c id,col1,col2,col3" data/floats.csv avg/floats.csv data/empty.txt 0
	avg-floats)

test("csv-avg --help" data/empty.csv avg/help.txt data/empty.txt 2
	avg_help)

test("csv-avg --version" data/empty.csv data/git-version.txt data/empty.txt 0
	avg_version)
