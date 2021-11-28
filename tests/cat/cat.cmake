#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-cat ${DATA_DIR}/one-column-one-row.csv" data/empty.csv data/one-column-one-row.csv data/empty.txt 0
	cat_one_file)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv" data/empty.csv cat/2-files.csv data/empty.txt 0
	cat_two_files)

test("csv-cat - ${DATA_DIR}/one-column-one-row.csv" data/one-column-one-row.csv cat/2-files.csv data/empty.txt 0
	cat_stdin_and_file)

test("csv-cat ${DATA_DIR}/3-different-order-columns-3-rows.csv ${DATA_DIR}/../cat/3-columns-2-last-rows.csv" data/empty.csv cat/2-different-files.csv data/empty.txt 0
	cat_two_different_files)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/2-columns-3-rows.csv" data/empty.csv data/empty.csv cat/2-files-different-num-columns.txt 2
	cat_two_files_different_columns)

test("csv-cat ${DATA_DIR}/../cat/2-files-different-column-names-file1.csv ${DATA_DIR}/../cat/2-files-different-column-names-file2.csv" data/empty.csv data/empty.csv cat/2-files-different-column-names.txt 2
	cat_two_different_column_names)

test("csv-cat ${DATA_DIR}/../cat/2-files-different-column-types-file1.csv ${DATA_DIR}/../cat/2-files-different-column-types-file2.csv" data/empty.csv data/empty.csv cat/2-files-different-column-types.txt 2
	cat_two_different_column_types)

test("csv-cat --help" data/empty.csv cat/help.txt data/empty.txt 2
	cat_help)

test("csv-cat --version" data/empty.csv data/git-version.txt data/empty.txt 0
	cat_version)

test("csv-cat -s" data/2-columns-3-rows.csv cat/show.txt data/empty.txt 0
	cat_show)

test("csv-cat -S" data/2-columns-3-rows.csv cat/show.txt data/empty.txt 0
	cat_show_-S)

test("csv-cat - -" data/2-columns-3-rows.csv data/empty.txt cat/stdin-twice.txt 2
	cat_stdin_twice)

test("csv-cat not-existing" data/empty.csv data/empty.txt cat/not-existing.txt 2
	cat_not_existing)
