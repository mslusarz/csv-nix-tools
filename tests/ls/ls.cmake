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

test_with_cwd("csv-ls -c name"
	data/empty.txt ls/files_and_dirs.csv data/empty.txt 0
	ls-files
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -c name,mode,symlink,type_name -ls"
	data/empty.txt ls/files_and_dirs.txt data/empty.txt 0
	ls-files-s
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -c name -M"
	data/3-columns-3-rows-with-table.csv ls/files_and_dirs_merged.csv data/empty.txt 0
	ls-merge
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -c name -M -N lorem"
	data/3-columns-3-rows-with-table.csv ls/files_and_dirs_merged_table.csv data/empty.txt 0
	ls-merge-new-table
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls | csv-count -c"
	data/empty.txt ls/count-columns.csv data/empty.txt 0
	ls-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -l | csv-count -c"
	data/empty.txt ls/l-count-columns.csv data/empty.txt 0
	ls-l-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -ll | csv-count -c"
	data/empty.txt ls/ll-count-columns.csv data/empty.txt 0
	ls-ll-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -lll | csv-count -c"
	data/empty.txt ls/lll-count-columns.csv data/empty.txt 0
	ls-lll-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test("csv-ls --help" data/empty.csv ls/help.txt data/empty.txt 2
	ls_help)

test("csv-ls --version" data/empty.csv data/git-version.txt data/empty.txt 0
	ls_version)
