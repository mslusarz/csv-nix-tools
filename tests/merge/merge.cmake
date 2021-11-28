#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-merge" data/empty.csv data/empty.csv merge/no-input.txt 2
	merge_no_input)

test("csv-merge --table table1 --path-without-table ${DATA_DIR}/one-column-one-row.csv" data/empty.csv merge/table1-one-column-one-row.csv data/empty.txt 0
	merge_one_file)

test("csv-merge --table table1 --path-without-table ${DATA_DIR}/one-column-one-row.csv --table table2 --path-without-table ${DATA_DIR}/one-column-one-row.csv" data/empty.csv merge/2-files.csv data/empty.txt 0
	merge_two_files)

test("csv-merge --table table1 --path-without-table - --table table2 --path-without-table ${DATA_DIR}/one-column-one-row.csv" data/one-column-one-row.csv merge/2-files.csv data/empty.txt 0
	merge_stdin_and_file)

test("csv-merge --table table1 --path-without-table ${DATA_DIR}/one-column-one-row.csv --table table2 --path-without-table ${DATA_DIR}/2-columns-3-rows.csv" data/empty.csv merge/2-files-different-columns.csv data/empty.txt 0
	merge_two_files_different_columns)

test("csv-merge --path-with-table ${DATA_DIR}/../merge/2-files-different-columns.csv --table table3 --path-without-table ${DATA_DIR}/3-columns-3-rows.csv" data/empty.csv merge/table_normal.csv data/empty.txt 0
	merge_table_normal)

test("csv-merge --table table3 --path-without-table ${DATA_DIR}/3-columns-3-rows.csv --path-with-table ${DATA_DIR}/../merge/2-files-different-columns.csv" data/empty.csv merge/normal_table.csv data/empty.txt 0
	merge_normal_table)

test("csv-merge -P ${DATA_DIR}/../merge/2-files-different-columns.csv --table table3 --path-without-table ${DATA_DIR}/3-columns-3-rows.csv --path-with-table ${DATA_DIR}/../merge/table4-one-column-one-row.csv" data/empty.csv merge/table_normal_table.csv data/empty.txt 0
	merge_table_normal_table)

test("csv-merge -P ${DATA_DIR}/../merge/2-files-different-columns.csv -P ${DATA_DIR}/3-columns-3-rows.csv" data/empty.csv data/empty.csv data/no-table-column.txt 2
	merge_no_table_column)

test("csv-merge -N t -p ${DATA_DIR}/../merge/double_column_no_table.csv -P ${DATA_DIR}/../merge/normal_table.csv" data/empty.csv data/empty.csv merge/column-already-exists.txt 2
	merge_double_column_no_table)

test("csv-merge      -P ${DATA_DIR}/../merge/double_column_table.csv    -P ${DATA_DIR}/../merge/normal_table.csv" data/empty.csv data/empty.csv merge/column-already-exists.txt 2
	merge_double_column_table)

test("csv-merge --help" data/empty.csv merge/help.txt data/empty.txt 2
	merge_help)

test("csv-merge --version" data/empty.csv data/git-version.txt data/empty.txt 0
	merge_version)

test("csv-merge --table table1 --path-without-table - -s" data/2-columns-3-rows.csv merge/show.txt data/empty.txt 0
	merge_show)

test("csv-merge --table table1 --path-without-table - -S" data/2-columns-3-rows.csv merge/show.txt data/empty.txt 0
	merge_show_-S)

test("csv-merge --table table1 --path-without-table - --table table2 --path-without-table -" data/2-columns-3-rows.csv data/empty.txt merge/stdin-twice.txt 2
	merge_stdin_twice)

test("csv-merge --table table1 --path-without-table not-existing" data/empty.csv data/empty.txt merge/not-existing.txt 2
	merge_not_existing)

test("csv-merge --path-without-table doesntmatter" data/empty.csv data/empty.txt merge/no-table.txt 2
	merge_no_table)
