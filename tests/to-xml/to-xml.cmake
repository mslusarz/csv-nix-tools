#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-to-xml" data/3-columns-3-rows.csv to-xml/basic.xml data/empty.txt 0
	to-xml_basic)

test("csv-to-xml --no-header" data/3-columns-3-rows.csv to-xml/no-header.xml data/empty.txt 0
	to-xml_no-header)

test("csv-to-xml --generic-names" data/3-columns-3-rows.csv to-xml/generic-names.xml data/empty.txt 0
	to-xml_generic-names)

if (TEST_UTF8)
set(REQ_LOCALE "en_US.UTF-8")

test("csv-to-xml" data/3-columns-3-rows.csv to-xml/basic-utf8.xml data/empty.txt 0
	to-xml_basic_utf8)

set(REQ_LOCALE "C")
endif()

test("csv-to-xml --help" data/empty.csv to-xml/help.txt data/empty.txt 2
	to-xml_help)

test("csv-to-xml --version" data/empty.csv data/git-version.txt data/empty.txt 0
	to-xml_version)
