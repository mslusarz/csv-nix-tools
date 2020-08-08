#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-to-html --with-types --set-color id:fg=white,bg=#ff0000" data/3-columns-3-rows.csv to-html/3-columns-3-rows-basic.html data/empty.txt 0
	to-html_3_cols_html_basic)

test("csv-to-html --datatables --with-types --set-color id:fg=white,bg=#ff0000" data/3-columns-3-rows.csv to-html/3-columns-3-rows-datatables.html data/empty.txt 0
	to-html_3_cols_html_datatables)

test("csv-to-html --tabulator --with-types --set-color id:fg=white,bg=#ff0000" data/3-columns-3-rows.csv to-html/3-columns-3-rows-tabulator.html data/empty.txt 0
	to-html_3_cols_html_tabulator)

test("csv-to-html --set-color id:fg=white,bg=#ff0000 --use-color-columns" to-html/3-columns-3-rows-colors.csv to-html/3-columns-3-rows-basic-color-column.html data/empty.txt 0
	to-html_3_cols_html_basic_color_column)

test("csv-to-html --help" data/empty.csv to-html/help.txt data/empty.txt 2
	to-html_help)

test("csv-to-html --version" data/empty.csv data/git-version.txt data/empty.txt 0
	to-html_version)
