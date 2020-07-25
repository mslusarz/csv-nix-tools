#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

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
