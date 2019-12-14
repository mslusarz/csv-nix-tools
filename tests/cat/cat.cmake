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

test("csv-cat ${DATA_DIR}/one-column-one-row.csv" data/empty.csv data/one-column-one-row.csv data/empty.txt 0
	cat_one_file)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv" data/empty.csv cat/2-files.csv data/empty.txt 0
	cat_two_files)

test("csv-cat - ${DATA_DIR}/one-column-one-row.csv" data/one-column-one-row.csv cat/2-files.csv data/empty.txt 0
	cat_stdin_and_file)

test("csv-cat ${DATA_DIR}/3-different-order-columns-3-rows.csv ${DATA_DIR}/../cat/3-columns-2-last-rows.csv" data/empty.csv cat/2-different-files.csv data/empty.txt 0
	cat_two_different_files)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/2-columns-3-rows.csv" data/empty.csv data/empty.csv cat/2-files-different-num-columns.txt 2
	cat_two_files_different_columns)

test("csv-cat --help" data/empty.csv cat/help.txt data/empty.txt 2
	cat_help)

test("csv-cat --version" data/empty.csv data/git-version.txt data/empty.txt 0
	cat_version)

test("csv-cat -s" data/2-columns-3-rows.csv cat/show.txt data/empty.txt 0
	cat_show)

test("csv-cat - -" data/2-columns-3-rows.csv data/empty.txt cat/stdin-twice.txt 2
	cat_stdin_twice)

test("csv-cat not-existing" data/empty.csv data/empty.txt cat/not-existing.txt 2
	cat_not_existing)
