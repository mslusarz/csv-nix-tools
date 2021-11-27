#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

# conventions for test names:
# - start with a tool name, without "csv-" prefix
# - use _ as a word separator

# conventions for file names:
# - use tool- prefix for test-specific files
# - do not use tool- prefix for general purpose files
# - use - as a word separator
# - use .csv extension for csv files
# - use .txt extension for other files

set(REQ_LOCALE "C")

function(test_with_cwd cmd input stdout stderr expected_res name cwd)
	if (${TEST_UNDER_MEMCHECK} AND NOT ${expected_res} EQUAL 0)
		return()
	endif()

	add_test(NAME ${name}
			COMMAND ${CMAKE_COMMAND}
			--trace-expand
			-DNAME=${name}
			-DINPUT=${input}
			-DCMD=${cmd}
			-DSTDOUT=${stdout}
			-DSTDERR=${stderr}
			-DEXPECTED_RES=${expected_res}
			-DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
			-DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}/..
			-DCWD=${cwd}
			-DTEST_UNDER_MEMCHECK=${TEST_UNDER_MEMCHECK}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/test.cmake)

	set_tests_properties(${name} PROPERTIES
			ENVIRONMENT "LC_ALL=${REQ_LOCALE};PATH=${CMAKE_CURRENT_BINARY_DIR}/..:$ENV{PATH};")
endfunction()

function(test cmd input stdout stderr expected_res name)
	test_with_cwd(${cmd} ${input} ${stdout} ${stderr} ${expected_res} ${name} "")
endfunction()

function(inc name)
	include(${name}/${name}.cmake)
endfunction()

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

inc(parsing)

inc(add-concat)
inc(add-exec)
inc(add-replace)
inc(add-rev)
inc(add-rpn)
inc(add-split)
inc(add-sql)
inc(add-substring)
inc(avg)
inc(cat)
inc(count)
inc(cut)
inc(diff)
inc(env)
inc(exec)
inc(grep)
inc(grep-rpn)
inc(grep-sql)
inc(groups)
inc(group-members)
inc(head)
inc(header)
inc(ls)
inc(max)
inc(merge)
inc(min)
inc(netstat)
inc(peek)
inc(plot)
inc(ps)
inc(show)
inc(sort)
inc(sql)
inc(sqlite)
inc(sum)
inc(tac)
inc(tail)
inc(to-html)
inc(to-json)
inc(to-xml)
inc(uniq)
inc(users)
