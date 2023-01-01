#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-add-exec -n name2 -c name -- sed 's/file/name/g'" add-exec/files.csv add-exec/names.csv data/empty.txt 0
	add-exec_sed)

test("csv-add-exec -n name2 -c name -s -- sed 's/file/name/g'" add-exec/files.csv add-exec/names.txt data/empty.txt 0
	add-exec_sed_-s)

test("csv-add-exec -T t1 -n name2 -c name -- sed 's/o/O/g'" data/2-tables.csv add-exec/2-tables-add1.csv data/empty.txt 0
	add-exec_2tables-add1)

test("csv-add-exec --help" data/empty.csv add-exec/help.txt data/empty.txt 2
	add-exec_help)

test("csv-add-exec --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-exec_version)
