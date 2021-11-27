#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-tree -k full_path -p parent -i name -m size" tree/ls-sys-module8250.csv tree/ls-sys-module8250-tree.csv data/empty.txt 0
	tree_basic)

test("csv-tree -k full_path -p parent -i name -m size -s" tree/ls-sys-module8250.csv tree/ls-sys-module8250-tree.txt data/empty.txt 0
	tree_basic_-s)

test("csv-tree -k full_path -p parent -i name -m size -S" tree/ls-sys-module8250.csv tree/ls-sys-module8250-tree.txt data/empty.txt 0
	tree_basic_-S)

test("csv-tree --help" data/empty.csv tree/help.txt data/empty.txt 2
	tree_help)

test("csv-tree --version" data/empty.csv data/git-version.txt data/empty.txt 0
	tree_version)
