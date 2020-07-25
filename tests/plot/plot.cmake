#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-plot -x col1 -y col2" plot/input.csv plot/oneline.gnuplot data/empty.txt 0
	plot_oneline)

# unforrtunately different versions of gnuplot produce different results
#if (GNUPLOT)
#test("csv-plot -x col1 -y col2 -t 'dumb size 79,24' -g" plot/input.csv plot/oneline.txt data/empty.txt 0
#	plot_oneline_dumb_terminal)
#endif()

test("csv-plot -x col1 -y col2 -y col3" plot/input.csv plot/twolines.gnuplot data/empty.txt 0
	plot_twolines)

test("csv-plot -T num -x col1 -y col2" plot/input-with-table.csv plot/oneline.gnuplot data/empty.txt 0
	plot_oneline_table)

test("csv-plot -T num -x col1 -y col2 -y col3" plot/input-with-table.csv plot/twolines.gnuplot data/empty.txt 0
	plot_twolines_table)

test("csv-plot -x col1 -y col2 -z col3" plot/input.csv plot/3d.gnuplot data/empty.txt 0
	plot_3d)

test("csv-plot -x col5 -y col1" plot/input.csv data/empty.txt plot/column-not-found.txt 2
	plot_x_column_not_found)

test("csv-plot -x col1 -y col5" plot/input.csv data/empty.txt plot/column-not-found.txt 2
	plot_y_column_not_found)

test("csv-plot -x col1 -y col2 -z col5" plot/input.csv data/empty.txt plot/column-not-found.txt 2
	plot_z_column_not_found)

test("csv-plot -T xxx -x col1 -y col2" plot/input-with-table.csv data/empty.txt plot/column-in-table-not-found.txt 2
	plot_y_column_on_table_not_found)

test("csv-plot -T xxx -x col1 -y col2" plot/input.csv data/empty.txt plot/table-column-not-found.txt 2
	plot_table_column_not_found)

test("csv-plot" plot/input.csv data/empty.txt plot/help.txt 2
	plot_no_options)

test("csv-plot -x col1" plot/input.csv data/empty.txt plot/no_y.txt 2
	plot_no_y)

test("csv-plot -y col2" plot/input.csv data/empty.txt plot/no_x.txt 2
	plot_no_x)

test("csv-plot -x col1 -x col2" plot/input.csv data/empty.txt plot/dup_x.txt 2
	plot_dup_x)

test("csv-plot -x col1 -y col2 -y col2 -z col3" plot/input.csv data/empty.txt plot/dup_y.txt 2
	plot_dup_y)

test("csv-plot --help" data/empty.csv plot/help.txt data/empty.txt 2
	plot_help)

test("csv-plot --version" data/empty.csv data/git-version.txt data/empty.txt 0
	plot_version)
