#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test_with_cwd("csv-exec -- csv-ls -c size %name" exec/files.csv exec/sizes.txt data/empty.txt 0
	exec_csv-ls_-f_size
	${CMAKE_SOURCE_DIR}/tests/ls/files/)

test("csv-exec --help" data/empty.csv exec/help.txt data/empty.txt 2
	exec_help)

test("csv-exec --version" data/empty.csv data/git-version.txt data/empty.txt 0
	exec_version)
