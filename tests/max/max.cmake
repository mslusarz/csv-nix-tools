#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-max -c id" data/id-column-3-rows.csv max/id.csv data/empty.txt 0
	max)

test("csv-max -c id -n new" data/id-column-3-rows.csv max/id-n.csv data/empty.txt 0
	max-n)

test("csv-max -c col1,col2,col3" data/3-numeric-columns-4-rows.csv max/3-columns.csv data/empty.txt 0
	max-3-columns)

test("csv-max -c col3,col1" data/3-numeric-columns-4-rows.csv max/2-columns.csv data/empty.txt 0
	max-2-columns)

test("csv-max -c col3,col1 -n new1" data/3-numeric-columns-4-rows.csv max/2-columns-n1.csv data/empty.txt 0
	max-2-columns-n1)

test("csv-max -c col3,col1 -n new1,new2" data/3-numeric-columns-4-rows.csv max/2-columns-n2.csv data/empty.txt 0
	max-2-columns-n2)

test("csv-max -c col3,col1 -s" data/3-numeric-columns-4-rows.csv max/2-columns.txt data/empty.txt 0
	max-2-columns-s)

test("csv-max -c col1,col2,col3,col4" data/text1.csv max/text1.csv data/empty.txt 0
	max-text1)

test("csv-max -T t1 -c id,something" data/2-tables.csv max/2-tables-max1.csv data/empty.txt 0
	max-2tables-max1)

test("csv-max -T t2 -c id" data/2-tables.csv max/2-tables-max2.csv data/empty.txt 0
	max-2tables-max2)

test("csv-max -T t1 -c id,something -n new1" data/2-tables.csv max/2-tables-max1-n1.csv data/empty.txt 0
	max-2tables-max1-n1)

test("csv-max -T t1 -c id,something -n new1,new2" data/2-tables.csv max/2-tables-max1-n2.csv data/empty.txt 0
	max-2tables-max1-n2)

test("csv-max -T t2 -c id -n new1" data/2-tables.csv max/2-tables-max2-n1.csv data/empty.txt 0
	max-2tables-max2-n1)

test("csv-max -c id,col1,col2,col3" data/floats.csv max/floats.csv data/empty.txt 0
	max-floats)

test("csv-max --help" data/empty.csv max/help.txt data/empty.txt 2
	max_help)

test("csv-max --version" data/empty.csv data/git-version.txt data/empty.txt 0
	max_version)
