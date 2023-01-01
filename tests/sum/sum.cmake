#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-sum -c id" data/id-column-3-rows.csv sum/id.csv data/empty.txt 0
	sum)

test("csv-sum -c id -n new" data/id-column-3-rows.csv sum/id-n.csv data/empty.txt 0
	sum-n)

test("csv-sum -c col1,col2,col3" data/3-numeric-columns-4-rows.csv sum/3-columns.csv data/empty.txt 0
	sum-3-columns)

test("csv-sum -c col3,col1" data/3-numeric-columns-4-rows.csv sum/2-columns.csv data/empty.txt 0
	sum-2-columns)

test("csv-sum -c col3,col1 -n new1" data/3-numeric-columns-4-rows.csv sum/2-columns-n1.csv data/empty.txt 0
	sum-2-columns-n1)

test("csv-sum -c col3,col1 -n new1,new2" data/3-numeric-columns-4-rows.csv sum/2-columns-n2.csv data/empty.txt 0
	sum-2-columns-n2)

test("csv-sum -c col3,col1 -s" data/3-numeric-columns-4-rows.csv sum/2-columns.txt data/empty.txt 0
	sum-2-columns-s)

test("csv-sum -c col1,col2,col3,col4" data/text1.csv sum/text1.csv data/empty.txt 0
	sum-text1)

test("csv-sum --separator=, -c col1,col2,col3,col4" data/text1.csv sum/text2.csv data/empty.txt 0
	sum-text2)

test("csv-sum -T t1 -c id,something" data/2-tables.csv sum/2-tables-sum1.csv data/empty.txt 0
	sum-2tables-sum1)

test("csv-sum -T t1 -c id,something -n new1" data/2-tables.csv sum/2-tables-sum1-n1.csv data/empty.txt 0
	sum-2tables-sum1-n1)

test("csv-sum -T t1 -c id,something -n new1,new2" data/2-tables.csv sum/2-tables-sum1-n2.csv data/empty.txt 0
	sum-2tables-sum1-n2)

test("csv-sum -T t2 -c id" data/2-tables.csv sum/2-tables-sum2.csv data/empty.txt 0
	sum-2tables-sum2)

test("csv-sum -T t2 -c id -n new1" data/2-tables.csv sum/2-tables-sum2-n1.csv data/empty.txt 0
	sum-2tables-sum2-n1)

test("csv-sum -c id,col1,col2,col3" data/floats.csv sum/floats.csv data/empty.txt 0
	sum-floats)

test("csv-sum --help" data/empty.csv sum/help.txt data/empty.txt 2
	sum_help)

test("csv-sum --version" data/empty.csv data/git-version.txt data/empty.txt 0
	sum_version)
