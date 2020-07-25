#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-add-replace -c name -F ' i' -r 'STRING' -n new-col" data/3-columns-3-rows.csv add-replace/string.csv data/empty.txt 0
	add-replace_string)

test("csv-add-replace -c name -e '\\(.*\\)\\( i\\)\\(.*\\)' -r '%3 STRING %1' -n new-col" data/3-columns-3-rows.csv add-replace/regex.csv data/empty.txt 0
	add-replace_regex)

test("csv-add-replace -c name -E '(.*)( i)(.*)' -r '%3 STRING %1' -n new-col" data/3-columns-3-rows.csv add-replace/regex.csv data/empty.txt 0
	add-replace_eregex)

test("csv-add-replace -c name -E '(.*)( i)(.*)' -r '%3 STRING %1' -n new-col -s" data/3-columns-3-rows.csv add-replace/regex.txt data/empty.txt 0
	add-replace_eregex_-s)

test("csv-add-replace -T t1 -c name -F ' i' -r STRING -n new-col" data/2-tables.csv add-replace/2-tables-add1.csv data/empty.txt 0
	add-replace_2tables-add1)

test("csv-add-replace --help" data/empty.csv add-replace/help.txt data/empty.txt 2
	add-replace_help)

test("csv-add-replace --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-replace_version)
