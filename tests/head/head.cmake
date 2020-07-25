#
# SPDX-License-Identifier: BSD-3-Clause
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

test("csv-head" data/empty.csv data/empty.csv data/eof.txt 2
	head_empty_input)

test("csv-head" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_simple_pass_through)

test("csv-head --lines=0" data/one-column-one-row.csv head/zero-lines.csv data/empty.txt 0
	head_--lines=0)

test("csv-head -n 0" data/one-column-one-row.csv head/zero-lines.csv data/empty.txt 0
	head_-n_0)

test("csv-head --lines 1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_--lines=1)

test("csv-head -n 1" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_-n_1)

test("csv-head -n 1 -s" data/one-column-one-row.csv head/one-column-one-row.txt data/empty.txt 0
	head_-n_1_-s)

test("csv-head --lines 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_--lines=2_but_there_is_only_1_line_in_input)

test("csv-head -n 2" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	head_-n_2_but_there_is_only_1_line_in_input)

test("csv-head --lines=2" data/3-columns-3-rows.csv head/3-columns-2-first-rows.csv data/empty.txt 0
	head_--lines=2_and_there_are_3_lines_in_input)

test("csv-head -n 2" data/3-columns-3-rows.csv head/3-columns-2-first-rows.csv data/empty.txt 0
	head_-n_2_and_there_are_3_lines_in_input)

test("csv-head --lines=3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	head_--lines=3_and_there_are_3_lines_in_input)

test("csv-head -n 3" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	head_-n_3_and_there_are_3_lines_in_input)

test("csv-head --help" data/empty.csv head/help.txt data/empty.txt 2
	head_help)

test("csv-head --version" data/empty.csv data/git-version.txt data/empty.txt 0
	head_version)
