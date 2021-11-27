#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-tac ${DATA_DIR}/one-column-one-row.csv" data/empty.csv data/one-column-one-row.csv data/empty.txt 0
	tac_one_file)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv" data/empty.csv tac/2-files.csv data/empty.txt 0
	tac_two_files)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv -s" data/empty.csv tac/2-files.txt data/empty.txt 0
	tac_two_files_-s)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv -S" data/empty.csv tac/2-files.txt data/empty.txt 0
	tac_two_files_-S)

test("csv-tac ${DATA_DIR}/3-different-order-columns-3-rows.csv ${CMAKE_CURRENT_SOURCE_DIR}/tac/3-columns-2-last-rows.csv" data/empty.csv tac/2-different-files.csv data/empty.txt 0
	tac_two_different_files)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/2-columns-3-rows.csv" data/empty.csv data/empty.csv tac/2-files-different-num-columns.txt 2
	tac_two_files_different_columns)

test("csv-tac --help" data/empty.csv tac/help.txt data/empty.txt 2
	tac_help)

test("csv-tac --version" data/empty.csv data/git-version.txt data/empty.txt 0
	tac_version)
