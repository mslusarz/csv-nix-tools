#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

test("csv-max -c id" data/id-column-3-rows.csv max/id.csv data/empty.txt 0
	max)

test("csv-max -c col1,col2,col3" data/3-numeric-columns-4-rows.csv max/3-columns.csv data/empty.txt 0
	max-3-columns)

test("csv-max -c col3,col1" data/3-numeric-columns-4-rows.csv max/2-columns.csv data/empty.txt 0
	max-2-columns)

test("csv-max -c col3,col1 -s" data/3-numeric-columns-4-rows.csv max/2-columns.txt data/empty.txt 0
	max-2-columns-s)

test("csv-max -c col1,col2,col3,col4" data/text1.csv max/text1.csv data/empty.txt 0
	max-text1)

test("csv-max -T t1 -c id,something" max/2-tables.csv max/2-tables-max1.csv data/empty.txt 0
	max-2tables-max1)

test("csv-max -T t2 -c id" max/2-tables.csv max/2-tables-max2.csv data/empty.txt 0
	max-2tables-max2)

test("csv-max --help" data/empty.csv max/help.txt data/empty.txt 2
	max_help)

test("csv-max --version" data/empty.csv data/git-version.txt data/empty.txt 0
	max_version)
