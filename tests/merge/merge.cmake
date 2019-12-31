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

test("csv-merge --table table1 --path-without-table ${DATA_DIR}/one-column-one-row.csv" data/empty.csv merge/table1-one-column-one-row.csv data/empty.txt 0
	merge_one_file)

test("csv-merge --table table1 --path-without-table ${DATA_DIR}/one-column-one-row.csv --table table2 --path-without-table ${DATA_DIR}/one-column-one-row.csv" data/empty.csv merge/2-files.csv data/empty.txt 0
	merge_two_files)

test("csv-merge --table table1 --path-without-table - --table table2 --path-without-table ${DATA_DIR}/one-column-one-row.csv" data/one-column-one-row.csv merge/2-files.csv data/empty.txt 0
	merge_stdin_and_file)

test("csv-merge --table table1 --path-without-table ${DATA_DIR}/one-column-one-row.csv --table table2 --path-without-table ${DATA_DIR}/2-columns-3-rows.csv" data/empty.csv merge/2-files-different-columns.csv data/empty.txt 0
	merge_two_files_different_columns)

test("csv-merge --path-with-table ${DATA_DIR}/../merge/2-files-different-columns.csv --table table3 --path-without-table ${DATA_DIR}/3-columns-3-rows.csv" data/empty.csv merge/table_normal.csv data/empty.txt 0
	merge_table_normal)

test("csv-merge --table table3 --path-without-table ${DATA_DIR}/3-columns-3-rows.csv --path-with-table ${DATA_DIR}/../merge/2-files-different-columns.csv" data/empty.csv merge/normal_table.csv data/empty.txt 0
	merge_normal_table)

test("csv-merge --path-with-table ${DATA_DIR}/../merge/2-files-different-columns.csv --table table3 --path-without-table ${DATA_DIR}/3-columns-3-rows.csv --path-with-table ${DATA_DIR}/../merge/table4-one-column-one-row.csv" data/empty.csv merge/table_normal_table.csv data/empty.txt 0
	merge_table_normal_table)

test("csv-merge --help" data/empty.csv merge/help.txt data/empty.txt 2
	merge_help)

test("csv-merge --version" data/empty.csv data/git-version.txt data/empty.txt 0
	merge_version)

test("csv-merge --table table1 --path-without-table - -s" data/2-columns-3-rows.csv merge/show.txt data/empty.txt 0
	merge_show)

test("csv-merge --table table1 --path-without-table - --table table2 --path-without-table -" data/2-columns-3-rows.csv data/empty.txt merge/stdin-twice.txt 2
	merge_stdin_twice)

test("csv-merge --table table1 --path-without-table not-existing" data/empty.csv data/empty.txt merge/not-existing.txt 2
	merge_not_existing)
