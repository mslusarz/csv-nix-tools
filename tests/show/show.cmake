#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-show" data/3-columns-3-rows.csv show/3-columns-3-rows.txt data/empty.txt 0
	show_3_cols)

test("csv-show --ui=less" data/3-columns-3-rows.csv show/3-columns-3-rows.txt data/empty.txt 0
	show_3_cols_less)

test("csv-show" data/newlines.csv show/newlines.txt data/empty.txt 0
	show_newlines)

test("csv-show" data/quotes.csv show/quotes.txt data/empty.txt 0
	show_quotes)

test("csv-show" data/commas.csv show/commas.txt data/empty.txt 0
	show_commas)

test("csv-show" data/control-characters.csv show/control-characters.txt data/empty.txt 0
	show_control_characters)

if (TMUX)
# crazy test using tmux
add_test(NAME show_curses
	COMMAND ${CMAKE_COMMAND} --trace-expand
		-DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
		-DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}/..
		-DTMUX=${TMUX}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/show/curses.cmake)

set_tests_properties(show_curses PROPERTIES
		ENVIRONMENT "LC_ALL=C;PATH=${CMAKE_CURRENT_BINARY_DIR}/..:$ENV{PATH};")
endif()

test("csv-show -u html=basic --with-types --set-color id:fg=white,bg=ff0000" data/3-columns-3-rows.csv show/3-columns-3-rows-basic.html data/empty.txt 0
	show_3_cols_html_basic)

test("csv-show -u html=datatables --with-types --set-color id:fg=white,bg=ff0000" data/3-columns-3-rows.csv show/3-columns-3-rows-datatables.html data/empty.txt 0
	show_3_cols_html_datatables)

test("csv-show -u html=tabulator --with-types --set-color id:fg=white,bg=ff0000" data/3-columns-3-rows.csv show/3-columns-3-rows-tabulator.html data/empty.txt 0
	show_3_cols_html_tabulator)

test("csv-show -u html=basic --set-color id:fg=white,bg=ff0000 --use-color-columns" show/3-columns-3-rows-colors.csv show/3-columns-3-rows-basic-color-column.html data/empty.txt 0
	show_3_cols_html_basic_color_column)

test("csv-show --help" data/empty.csv show/help.txt data/empty.txt 2
	show_help)

test("csv-show --version" data/empty.csv data/git-version.txt data/empty.txt 0
	show_version)
