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

test("csv-grep" data/one-column-one-row.csv data/empty.csv grep/help.txt 2
	grep_no_args)

test("csv-grep -c x -F y" data/empty.csv data/empty.csv data/eof.txt 2
	grep_empty_input)


test("csv-grep -c name -e or" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-e_or)

test("csv-grep -c name -e or -s" data/3-columns-3-rows.csv grep/name-or.txt data/empty.txt 0
	grep_-c_name_-e_or_-s)

test("csv-grep -c name -x -e or" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-x_-e_or)

test("csv-grep -c name -e or.m" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-e_or.m)


test("csv-grep -c name -E or" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-E_or)

test("csv-grep -c name -x -E or" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-x_-E_or)

test("csv-grep -c name -E or.m" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-E_or.m)


test("csv-grep -c name -F or" data/3-columns-3-rows.csv grep/name-or.csv data/empty.txt 0
	grep_-c_name_-F_or)

test("csv-grep -c name -x -F or" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-x_-F_or)

test("csv-grep -c name -F or.m" data/3-columns-3-rows.csv grep/not-found.csv data/empty.txt 0
	grep_-c_name_-F_or.m)


test("csv-grep -c name -e or -v" data/3-columns-3-rows.csv grep/name-or-not.csv data/empty.txt 0
	grep_-c_name_-e_or_-v)

test("csv-grep --help" data/empty.csv grep/help.txt data/empty.txt 2
	grep_help)

test("csv-grep --version" data/empty.csv data/git-version.txt data/empty.txt 0
	grep_version)
