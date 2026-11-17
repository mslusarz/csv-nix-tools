#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2026, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test_with_cwd("csv-ls -c name"
	data/empty.txt ls/files_and_dirs.csv data/empty.txt 0
	ls-files
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -c name,owner_execute,symlink,type_name -ls"
	data/empty.txt ls/files_and_dirs.txt data/empty.txt 0
	ls-files-s
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -c name -M"
	data/3-columns-3-rows-with-table.csv ls/files_and_dirs_merged.csv data/empty.txt 0
	ls-merge
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -c name -M -N lorem"
	data/3-columns-3-rows-with-table.csv ls/files_and_dirs_merged_table.csv data/empty.txt 0
	ls-merge-new-table
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -c name -M -N lorem"
	data/no-table.csv "" data/no-table-column-in-input.txt 2
	ls-merge-no-table
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls | csv-count -c"
	data/empty.txt ls/count-columns.csv data/empty.txt 0
	ls-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -l | csv-count -c"
	data/empty.txt ls/l-count-columns.csv data/empty.txt 0
	ls-l-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -ll | csv-count -c"
	data/empty.txt ls/ll-count-columns.csv data/empty.txt 0
	ls-ll-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test_with_cwd("csv-ls -lll | csv-count -c"
	data/empty.txt ls/lll-count-columns.csv data/empty.txt 0
	ls-lll-count-columns
	${CMAKE_SOURCE_DIR}/tests/ls/files)

test("csv-ls --help" data/empty.csv ls/help.txt data/empty.txt 2
	ls_help)

test("csv-ls --version" data/empty.csv data/git-version.txt data/empty.txt 0
	ls_version)

test([=[mkdir -p 'tree/.hidden' 'tree/quoted,dir' &&
	touch tree/.dot tree/Alpha tree/beta 'tree/.hidden/child' 'tree/quoted,dir/file"quoted' &&
	ln -s 'quoted,dir' tree/link &&
	csv-ls -aR -c parent,name,full_path tree/]=]
	data/empty.txt ls/recursive-all.csv data/empty.txt 0 ls_recursive_all)

test([=[mkdir directory && touch directory/child 'file,with,commas' &&
	csv-ls --directory --as-table -c name,full_path,type_name directory/ 'file,with,commas']=]
	data/empty.txt ls/directory-table.csv data/empty.txt 0 ls_directory_table)

test([=[umask 022 && mkdir tree && cd tree && mkdir directory && touch executable plain no-execute &&
	chmod 7755 executable && chmod 7600 no-execute && chmod 000 plain &&
	mkfifo -m 640 pipe && ln -s plain valid && ln -s missing broken &&
	csv-ls --colors -c name,type_name,type_mode,mode,symlink,symlink_color,name_color]=]
	data/empty.txt ls/modes-and-colors.csv data/empty.txt 0 ls_modes_and_colors)

test("touch present && csv-ls missing present"
	data/empty.txt ls/present.csv ls/missing.txt 2 ls_missing_operand)

test("csv-ls --show --show-full"
	data/empty.txt data/empty.txt ls/conflicting-show.txt 2 ls_conflicting_show)

test("touch present && csv-ls --colors present"
	data/empty.txt ls/colors.csv data/empty.txt 0 ls_color_columns)

test("touch present && csv-ls --colors --no-colors present"
	data/empty.txt ls/present.csv data/empty.txt 0 ls_no_colors)

find_program(LS_TEST_PYTHON python3)
if (LS_TEST_PYTHON)
	foreach(case special_files proc_symlinks unsorted directory_permissions)
		add_test(NAME ls_${case}
			COMMAND "${LS_TEST_PYTHON}" -E "${CMAKE_CURRENT_SOURCE_DIR}/ls/listing.py"
				"${case}" "$<TARGET_FILE:csv-ls>")
		set_tests_properties(ls_${case} PROPERTIES
			ENVIRONMENT "LC_ALL=C"
			SKIP_RETURN_CODE 77
			TIMEOUT 30)
	endforeach()
endif()

if (FAULT_INJECTION AND NOT TEST_UNDER_MEMCHECK)
	# An empty directory contains only the "." and ".." entries.
	math(EXPR ls_dirs_size "2 * ${CMAKE_SIZEOF_VOID_P}")
	test("mkdir tree && csv-ls tree"
		data/empty.txt ls/header.csv data/malloc-enomem.txt 2 ls_directory_entries_alloc_failure)
	append_envs(ls_directory_entries_alloc_failure
		"LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/fi/;CSVNIXTOOLS_XMALLOC_FAIL_ON_SIZE=${ls_dirs_size}")

	# The path buffer holds "tree/", followed by struct dirent's 256-byte name.
	test("mkdir tree && csv-ls tree"
		data/empty.txt ls/header.csv data/malloc-enomem.txt 2 ls_directory_path_alloc_failure)
	append_envs(ls_directory_path_alloc_failure
		"LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/fi/;CSVNIXTOOLS_XMALLOC_FAIL_ON_SIZE=261")

	set(ls_link_target "not-existing-file")
	string(LENGTH "${ls_link_target}" ls_link_length)
	math(EXPR ls_link_size "${ls_link_length} + 1")
	foreach(case listed explicit)
		set(ls_operand "")
		if (case STREQUAL "explicit")
			set(ls_operand " link")
		endif()
		test("mkdir tree && cd tree && ln -s ${ls_link_target} link && csv-ls -c name,symlink${ls_operand}"
			data/empty.txt ls/symlink-without-target.csv data/malloc-enomem.txt 2 ls_${case}_symlink_alloc_failure)
		append_envs(ls_${case}_symlink_alloc_failure
			"LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/fi/;CSVNIXTOOLS_XMALLOC_FAIL_ON_SIZE=${ls_link_size}")
	endforeach()
endif()
