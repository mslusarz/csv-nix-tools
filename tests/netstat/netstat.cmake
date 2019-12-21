#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

if (LIBMNL_FOUND)

test("csv-netstat | csv-count -c -R" data/empty.txt netstat/count-columns.csv data/empty.txt 0
	netstat_count_columns)

test("csv-netstat -e | csv-count -c -R" data/empty.txt netstat/e-count-columns.csv data/empty.txt 0
	netstat_e_count_columns)

test("csv-netstat -t | csv-count -c -R" data/empty.txt netstat/t-count-columns.csv data/empty.txt 0
	netstat_t_count_columns)

test("csv-netstat -u | csv-count -c -R" data/empty.txt netstat/u-count-columns.csv data/empty.txt 0
	netstat_u_count_columns)

test("csv-netstat -w | csv-count -c -R" data/empty.txt netstat/w-count-columns.csv data/empty.txt 0
	netstat_w_count_columns)

test("csv-netstat -x | csv-count -c -R" data/empty.txt netstat/x-count-columns.csv data/empty.txt 0
	netstat_x_count_columns)

test("csv-netstat -4 | csv-count -c -R" data/empty.txt netstat/4-count-columns.csv data/empty.txt 0
	netstat_4_count_columns)

test("csv-netstat -6 | csv-count -c -R" data/empty.txt netstat/6-count-columns.csv data/empty.txt 0
	netstat_6_count_columns)

test("csv-netstat -f family,protocol | csv-uniq -f family,protocol | csv-head -n 0" data/empty.txt netstat/f-count-columns.csv data/empty.txt 0
	netstat_f_count_columns)

test("csv-netstat -M -f family,inode,state | csv-head -n 0" data/3-columns-3-rows-with-label.csv netstat/columns-merged.csv data/empty.txt 0
	netstat_merged)

test("csv-netstat -M -f family,inode,state -L meh | csv-head -n 0" data/3-columns-3-rows-with-label.csv netstat/columns-merged-label.csv data/empty.txt 0
	netstat_merged_label)

test("csv-netstat --help" data/empty.csv netstat/help.txt data/empty.txt 2
	netstat_help)

test("csv-netstat --version" data/empty.csv data/git-version.txt data/empty.txt 0
	netstat_version)

endif(LIBMNL_FOUND)
