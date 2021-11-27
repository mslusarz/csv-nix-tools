#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-printf '%d-%s:%d %s\n' id name something name" data/3-columns-3-rows.csv printf/basic.txt data/empty.txt 0
	printf_basic)

test("csv-printf '0x%x - 0%o - %d\n' col2 col3 col1" data/3-numeric-columns-4-rows.csv printf/numeric.txt data/empty.txt 0
	printf_numeric)

test("csv-printf --help" data/empty.csv printf/help.txt data/empty.txt 2
	printf_help)

test("csv-printf --version" data/empty.csv data/git-version.txt data/empty.txt 0
	printf_version)
