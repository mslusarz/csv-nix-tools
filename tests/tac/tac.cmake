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

test("csv-tac ${DATA_DIR}/one-column-one-row.csv" data/empty.csv data/one-column-one-row.csv data/empty.txt 0
	tac_one_file)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv" data/empty.csv tac/2-files.csv data/empty.txt 0
	tac_two_files)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv -s" data/empty.csv tac/2-files.txt data/empty.txt 0
	tac_two_files_-s)

test("csv-tac ${DATA_DIR}/3-different-order-columns-3-rows.csv ${CMAKE_CURRENT_SOURCE_DIR}/tac/3-columns-2-last-rows.csv" data/empty.csv tac/2-different-files.csv data/empty.txt 0
	tac_two_different_files)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/2-columns-3-rows.csv" data/empty.csv data/empty.csv tac/2-files-different-num-columns.txt 2
	tac_two_files_different_columns)

test("csv-tac --help" data/empty.csv tac/help.txt data/empty.txt 2
	tac_help)

test("csv-tac --version" data/empty.csv data/git-version.txt data/empty.txt 0
	tac_version)
