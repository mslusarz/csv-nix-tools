#
# Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

test("csv-plot -x col1 -y col2" plot/input.csv plot/oneline.gnuplot data/empty.txt 0
	plot_oneline)

if (GNUPLOT)
test("csv-plot -x col1 -y col2 -t 'dumb size 79,24' -g" plot/input.csv plot/oneline.txt data/empty.txt 0
	plot_oneline_dumb_terminal)
endif()

test("csv-plot -x col1 -y col2 -y col3" plot/input.csv plot/twolines.gnuplot data/empty.txt 0
	plot_twolines)

test("csv-plot -T num -x col1 -y col2" plot/input-with-table.csv plot/oneline.gnuplot data/empty.txt 0
	plot_oneline_table)

test("csv-plot -T num -x col1 -y col2 -y col3" plot/input-with-table.csv plot/twolines.gnuplot data/empty.txt 0
	plot_twolines_table)

test("csv-plot -x col1 -y col5" plot/input.csv data/empty.txt plot/column-not-found.txt 2
	plot_y_column_not_found)

test("csv-plot -x col5 -y col1" plot/input.csv data/empty.txt plot/column-not-found.txt 2
	plot_x_column_not_found)

test("csv-plot -T xxx -x col1 -y col2" plot/input-with-table.csv data/empty.txt plot/column-in-table-not-found.txt 2
	plot_y_column_on_table_not_found)

test("csv-plot -T xxx -x col1 -y col2" plot/input.csv data/empty.txt plot/table-column-not-found.txt 2
	plot_table_column_not_found)

test("csv-plot" plot/input.csv data/empty.txt plot/help.txt 2
	plot_no_options)

test("csv-plot -x col1" plot/input.csv data/empty.txt plot/help.txt 2
	plot_no_y)

test("csv-plot -y col2" plot/input.csv data/empty.txt plot/help.txt 2
	plot_no_x)

test("csv-plot --help" data/empty.csv plot/help.txt data/empty.txt 2
	plot_help)

test("csv-plot --version" data/empty.csv data/git-version.txt data/empty.txt 0
	plot_version)
