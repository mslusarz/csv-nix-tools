#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

if (C11THREADS_FOUND)

test("csv-diff ${DATA_DIR}/../diff/data1.csv ${DATA_DIR}/../diff/data2.csv" data/empty.csv diff/diff.csv data/empty.txt 0
	diff_all_columns)

test("csv-diff -c x,y1 ${DATA_DIR}/../diff/data1.csv ${DATA_DIR}/../diff/data2.csv" data/empty.csv diff/diff_-c.csv data/empty.txt 0
	diff_columns)

test("csv-diff -d1 ${DATA_DIR}/../diff/data1.csv ${DATA_DIR}/../diff/data2.csv" data/empty.csv diff/diff_-d1.csv data/empty.txt 0
	diff_all_columns_diff)

test("csv-diff -d1 -c x,y1 ${DATA_DIR}/../diff/data1.csv ${DATA_DIR}/../diff/data2.csv" data/empty.csv diff/diff_-c_-d1.csv data/empty.txt 0
	diff_columns_diff)

test("csv-diff -C1 ${DATA_DIR}/../diff/data1.csv ${DATA_DIR}/../diff/data2.csv" data/empty.csv diff/diff_-C1.csv data/empty.txt 0
	diff_all_columns_colors)

test("csv-diff -C1 -c x,y1 ${DATA_DIR}/../diff/data1.csv ${DATA_DIR}/../diff/data2.csv" data/empty.csv diff/diff_-c_-C1.csv data/empty.txt 0
	diff_columns_colors)

test("csv-diff --help" data/empty.csv diff/help.txt data/empty.txt 2
	diff_help)

test("csv-diff --version" data/empty.csv data/git-version.txt data/empty.txt 0
	diff_version)

endif()
