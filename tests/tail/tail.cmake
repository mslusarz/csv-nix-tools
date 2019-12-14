#
# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
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

test("csv-tail" data/empty.csv data/empty.csv data/eof.txt 2
	tail_empty_input)

test("csv-tail" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_simple_pass_through)

test("csv-tail --lines=0" data/one-column-one-row.csv tail/zero-lines.csv data/empty.txt 0
	tail_--lines=0)

test("csv-tail -n 0" data/one-column-one-row.csv tail/zero-lines.csv data/empty.txt 0
	tail_-n_0)

test("csv-tail --lines=1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_--lines=1)

test("csv-tail -n 1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_-n_1)

test("csv-tail --lines 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_--lines=2_but_there_is_only_1_line_in_input)

test("csv-tail -n 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	tail_-n_2_but_there_is_only_1_line_in_input)

test("csv-tail --lines=2" data/3-columns-3-rows.csv tail/3-columns-2-last-rows.csv data/empty.txt 0
	tail_--lines=2_but_there_are_3_lines_in_input)

test("csv-tail -n 2" data/3-columns-3-rows.csv tail/3-columns-2-last-rows.csv data/empty.txt 0
	tail_-n_2_but_there_are_3_lines_in_input)

test("csv-tail --lines=3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	tail_--lines=3_and_there_are_3_lines_in_input)

test("csv-tail -n 3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	tail_-n_3_and_there_are_3_lines_in_input)

test("csv-tail -n 2 -s" data/3-columns-3-rows.csv tail/tail-n-2-s.txt data/empty.txt 0
	tail_-n_2_-s)

test("csv-tail --help" data/empty.csv tail/help.txt data/empty.txt 2
	tail_help)

test("csv-tail --version" data/empty.csv data/git-version.txt data/empty.txt 0
	tail_version)
