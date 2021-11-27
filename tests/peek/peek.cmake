#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-peek" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/3-columns-3-rows.csv 0
	peek_basic)

test("csv-peek -s" data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/3-columns-3-rows.txt 0
	peek_show)

test("csv-peek --help" data/empty.csv peek/help.txt data/empty.txt 2
	peek_help)

test("csv-peek --version" data/empty.csv data/git-version.txt data/empty.txt 0
	peek_version)
