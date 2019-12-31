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

test("csv-sort -c id" data/id-column-3-rows.csv data/id-column-3-rows.csv data/empty.txt 0
	sort_sorted)

test("csv-sort -c id -r" data/id-column-3-rows.csv sort/rsorted.csv data/empty.txt 0
	sort_rsorted)

test("csv-sort -c id,name" sort/2-cols.csv sort/2-cols-sorted.csv data/empty.txt 0
	sort_2_cols)

test("csv-sort -c id,name -r" sort/2-cols.csv sort/2-cols-rsorted.csv data/empty.txt 0
	sort_2_cols_rsorted)

test("csv-sort -c id,name -s" sort/2-cols.csv sort/2-cols-sorted.txt data/empty.txt 0
	sort_2_cols_-s)

test("csv-sort --help" data/empty.csv sort/help.txt data/empty.txt 2
	sort_help)

test("csv-sort --version" data/empty.csv data/git-version.txt data/empty.txt 0
	sort_version)
