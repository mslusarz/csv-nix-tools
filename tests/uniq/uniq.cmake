#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2022, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-uniq -c col1" uniq/input1.csv uniq/output1.csv data/empty.txt 0 uniq-1)

test("csv-uniq -c col3,col2,col1" uniq/input2.csv uniq/output2.csv data/empty.txt 0 uniq-2)

test("csv-uniq -c col3,col2,col1 -s" uniq/input2.csv uniq/output2.txt data/empty.txt 0 uniq-2-s)

test("csv-uniq -c col3" uniq/input2.csv uniq/output3.csv data/empty.txt 0 uniq-3)

test("csv-uniq -c _table,t1.str,t1.int,t2.string,t2.integer" uniq/input3.csv uniq/output4a.csv data/empty.txt 0 uniq-no-table)

test("csv-uniq -c str -T t1" uniq/input3.csv uniq/output4b.csv data/empty.txt 0 uniq-table1)

test("csv-uniq -c string -T t2" uniq/input3.csv uniq/output4c.csv data/empty.txt 0 uniq-table2)

test("csv-uniq" uniq/input1.csv data/empty.txt uniq/no-columns.txt 2 uniq-no-columns)

test("csv-uniq -c not-exists" uniq/input1.csv data/empty.txt uniq/unknown-column.txt 2 uniq-unknown-column)

test("csv-uniq -c str -T t1" uniq/input1.csv data/empty.txt uniq/table-not-found.txt 2 uniq-table-not-found)

test("csv-uniq --help" data/empty.csv uniq/help.txt data/empty.txt 2
	uniq_help)

test("csv-uniq --version" data/empty.csv data/git-version.txt data/empty.txt 0
	uniq_version)
