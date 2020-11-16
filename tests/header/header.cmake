#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-header" data/2-columns-3-rows.csv data/2-columns-3-rows.csv data/empty.txt 0
	header_pass)

test("csv-header -m" data/2-columns-3-rows.csv header/2-columns-3-rows-noheader.csv data/empty.txt 0
	header_remove)

test("csv-header -M" data/2-columns-3-rows.csv header/2-columns-3-rows-notypes.csv data/empty.txt 0
	header_remove_types)

test("csv-header -e name:string,something:int" header/2-columns-3-rows-notypes.csv data/2-columns-3-rows.csv data/empty.txt 0
	header_set_types)

test("csv-header -n name,newname -n something,somethingelse" data/2-columns-3-rows.csv header/rename.csv data/empty.txt 0
	header_rename)

test("csv-header -s" data/2-columns-3-rows.csv header/2-columns-3-rows.txt data/empty.txt 0
	header_-s)

test("csv-header -G" header/2-columns-3-rows-notypes.csv data/2-columns-3-rows.csv data/empty.txt 0
	header_guess_types1)

test("csv-header -G" header/guess-in.csv header/guess-out.csv data/empty.txt 0
	header_guess_types2)

test("csv-header --help" data/empty.csv header/help.txt data/empty.txt 2
	header_help)

test("csv-header --version" data/empty.csv data/git-version.txt data/empty.txt 0
	header_version)
