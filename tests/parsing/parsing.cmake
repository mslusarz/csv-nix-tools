#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-cat" data/empty.txt data/empty.csv data/eof.txt 2
	parsing_empty_input)

test("csv-cat" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	parsing_one_column_one_row)

test("csv-cat" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	parsing_3_columns_3_rows)

test("csv-cat" parsing/3-columns-3-rows-typeless.csv parsing/3-columns-3-rows-types.csv data/empty.txt 0
	parsing_input_without_types)

test("csv-cat" data/commas.csv data/commas.csv data/empty.txt 0
	parsing_commas)

test("csv-cat" data/newlines.csv data/newlines.csv data/empty.txt 0
	parsing_newlines)

test("csv-cat" data/quotes.csv data/quotes.csv data/empty.txt 0
	parsing_quotes)
