#
# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# conventions for test names:
# - start with a tool name, without "csv-" prefix
# - use _ as a word separator

# conventions for file names:
# - use tool- prefix for test-specific files
# - do not use tool- prefix for general purpose files
# - use - as a word separator
# - use .csv extension for csv files
# - use .txt extension for other files

function(test_with_cwd cmd input stdout stderr expected_res name cwd)
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
			ENVIRONMENT "LC_ALL=C;PATH=${CMAKE_CURRENT_BINARY_DIR}/..:$ENV{PATH};")
endfunction()

function(test cmd input stdout stderr expected_res name)
	test_with_cwd(${cmd} ${input} ${stdout} ${stderr} ${expected_res} ${name} "")
endfunction()

function(inc name)
	include(${name}/${name}.cmake)
endfunction()

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

inc(parsing)

inc(add-exec)
inc(add-rpn)
inc(avg)
inc(cat)
inc(concat)
inc(count)
inc(cut)
inc(exec)
inc(grep)
inc(grep-rpn)
inc(groups)
inc(group-members)
inc(head)
inc(header)
inc(ls)
inc(max)
inc(merge)
inc(min)
inc(netstat)
inc(ps)
inc(replace)
inc(show)
inc(sort)
inc(split)
inc(sql)
inc(sqlite)
inc(substring)
inc(sum)
inc(tac)
inc(tail)
inc(uniq)
inc(users)
