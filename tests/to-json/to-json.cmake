#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-to-json" data/3-columns-3-rows.csv to-json/basic.json data/empty.txt 0
	to-json_basic)

test("csv-to-json" data/control-characters.csv to-json/control-characters.json data/empty.txt 0
	to-json_control-characters)

test("csv-to-json" data/newlines.csv to-json/newlines.json data/empty.txt 0
	to-json_newlines)

test("csv-to-json --help" data/empty.csv to-json/help.txt data/empty.txt 2
	to-json_help)

test("csv-to-json --version" data/empty.csv data/git-version.txt data/empty.txt 0
	to-json_version)
