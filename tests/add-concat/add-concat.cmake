#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2022, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-add-concat" data/empty.txt data/empty.csv add-concat/help.txt 2
	add-concat_no_args)

test("csv-add-concat new_column = %name ' - ' %id" data/3-columns-3-rows.csv add-concat/2-cols.csv data/empty.txt 0
	add-concat_2_cols)

test("csv-add-concat new_column = %name ' - ' %id -s" data/3-columns-3-rows.csv add-concat/2-cols-s.txt data/empty.txt 0
	add-concat_2_cols-s)

test("csv-add-concat new_column = %name ', ' %id" data/3-columns-3-rows.csv add-concat/2-cols-comma.csv data/empty.txt 0
	add-concat_2_cols_comma)

test("csv-add-concat -T t1 -- new = 'X ' %name ' Y'" data/2-tables.csv add-concat/2-tables-add1.csv data/empty.txt 0
	add-concat_2tables-add1)

test("csv-add-concat new_column = %name ' - ' %something" add-concat/no-types-quot-in.csv add-concat/no-types-quot-out.csv data/empty.txt 0
	add-concat_no_types_quot)

test("csv-add-concat new_column %name %something" data/empty.csv data/empty.txt add-concat/help.txt 2
	add-concat_no_equal)

test("csv-add-concat new_column = %naame ' - ' %id" data/3-columns-3-rows.csv data/empty.txt add-concat/column-not-found.txt 2
	add-concat_unknown_column)

test("csv-add-concat -T t1 -- new2 = 'X ' %name ' Y'" data/no-table.csv data/empty.txt add-concat/table-not-found.txt 2
	add-concat_table_not_found)

test("csv-add-concat --help" data/empty.csv add-concat/help.txt data/empty.txt 2
	add-concat_help)

test("csv-add-concat --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-concat_version)
