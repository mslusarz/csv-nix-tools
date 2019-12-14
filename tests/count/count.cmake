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

test("csv-count" data/empty.txt data/empty.csv count/help.txt 2
	count_no_args)

test("csv-count -c" data/empty.txt data/empty.csv data/eof.txt 2
	count_empty_input)

test("csv-count -c" data/2-columns-3-rows.csv count/2-cols.txt data/empty.txt 0
	count_-c_2_cols)

test("csv-count -r" data/2-columns-3-rows.csv count/3-rows.txt data/empty.txt 0
	count_-r_3_rows)

test("csv-count -c -r" data/2-columns-3-rows.csv count/2-cols-3-rows.txt data/empty.txt 0
	count_-c_-r_2_cols_3_rows)

test("csv-count -c -r -s" data/2-columns-3-rows.csv count/2-cols-3-rows-s.txt data/empty.txt 0
	count_-c_-r_2_cols_3_rows-s)

test("csv-count --help" data/empty.csv count/help.txt data/empty.txt 2
	count_help)

test("csv-count --version" data/empty.csv data/git-version.txt data/empty.txt 0
	count_version)
