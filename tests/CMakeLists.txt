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

function(test cmd input stdout stderr expected_res name)
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
			-P ${CMAKE_CURRENT_SOURCE_DIR}/test.cmake)

	set_tests_properties(${name} PROPERTIES
			ENVIRONMENT "LC_ALL=C;PATH=${CMAKE_CURRENT_BINARY_DIR}/..:$ENV{PATH};")
endfunction()

############################### CSV PARSING ####################################

test("csv-cat" empty.txt empty.csv eof.txt 2
	cat-empty_input)

test("csv-cat" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cat-one-column-one-row)

test("csv-cat" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cat-3-columns-3-rows)

test("csv-cat" commas.csv commas.csv empty.txt 0
	cat-commas)

test("csv-cat" newlines.csv newlines.csv empty.txt 0
	cat-newlines)

test("csv-cat" quotes.csv quotes.csv empty.txt 0
	cat-quotes)

################################### CUT ########################################

test("csv-cut" empty.txt empty.csv eof.txt 2
	cut-empty_input)

test("csv-cut" one-column-one-row.csv cut-usage.txt empty.txt 2
	cut-simple_pass_through)

test("csv-cut -f" one-column-one-row.csv cut-usage.txt cut-no-fields-error.txt 2
	cut-simple_pass_through_with_-f)

test("csv-cut -f name" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cut-the_only_field_using_-f)

test("csv-cut --fields name" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cut-the_only_field_using_--fields_with_space)

test("csv-cut --fields=name" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cut-the_only_field_using_--fields_with_=)

test("csv-cut -f notExistingColumn" one-column-one-row.csv empty.txt column-not-found.txt 2
	cut-not_existing_field_using_-f)

test("csv-cut --fields notExistingColumn" one-column-one-row.csv empty.txt column-not-found.txt 2
	cut-not_existing_field_using_--fields_with_space)

test("csv-cut --fields=notExistingColumn" one-column-one-row.csv empty.txt column-not-found.txt 2
	cut-not_existing_field_using_--fields_with_=)


test("csv-cut -f name" 3-columns-3-rows.csv name-column-3-rows.csv empty.txt 0
	cut-the_name_field_using_-f)

test("csv-cut --fields name" 3-columns-3-rows.csv name-column-3-rows.csv empty.txt 0
	cut-the_name_field_using_--fields_with_space)

test("csv-cut --fields=name" 3-columns-3-rows.csv name-column-3-rows.csv empty.txt 0
	cut-the_name_field_using_--fields_with_=)


test("csv-cut -f id" 3-columns-3-rows.csv id-column-3-rows.csv empty.txt 0
	cut-the_id_field_using_-f)

test("csv-cut --fields id" 3-columns-3-rows.csv id-column-3-rows.csv empty.txt 0
	cut-the_id_field_using_--fields_with_space)

test("csv-cut --fields=id" 3-columns-3-rows.csv id-column-3-rows.csv empty.txt 0
	cut-the_id_field_using_--fields_with_=)


test("csv-cut -f something" 3-columns-3-rows.csv something-column-3-rows.csv empty.txt 0
	cut-the_something_field_using_-f)

test("csv-cut --fields something" 3-columns-3-rows.csv something-column-3-rows.csv empty.txt 0
	cut-the_something_field_using_--fields_with_space)

test("csv-cut --fields=something" 3-columns-3-rows.csv something-column-3-rows.csv empty.txt 0
	cut-the_something_field_using_--fields_with_=)


test("csv-cut -f name,something" 3-columns-3-rows.csv 2-columns-3-rows.csv empty.txt 0
	cut-2_fields_using_-f)

test("csv-cut --fields name,something" 3-columns-3-rows.csv 2-columns-3-rows.csv empty.txt 0
	cut-2_fields_using_--fields_with_spaces)

test("csv-cut --fields=name,something" 3-columns-3-rows.csv 2-columns-3-rows.csv empty.txt 0
	cut-2_fields_using_--fields_with_=)


test("csv-cut -f name,id,something" 3-different-order-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cut-3_fields_in_different_order_using_-f)

test("csv-cut --fields name,id,something" 3-different-order-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cut-3_fields_in_different_order_using_--fields_with_space)

test("csv-cut --fields=name,id,something" 3-different-order-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cut-3_fields_in_different_order_using_--fields_with_=)

################################### HEAD #######################################

test("csv-head" empty.csv empty.csv eof.txt 2
	head_empty_input)

test("csv-head" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	head_simple_pass_through)

test("csv-head --lines=0" one-column-one-row.csv zero-lines.csv empty.txt 0
	head_--lines=0)

test("csv-head -n 0" one-column-one-row.csv zero-lines.csv empty.txt 0
	head_-n_0)

test("csv-head --lines 1" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	head_--lines=1)

test("csv-head -n 1" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	head_-n_1)

test("csv-head --lines 2" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	head_--lines=2_but_there's_only_1_line_in_input)

test("csv-head -n 2" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	head_-n_2_but_there's_only_1_line_in_input)

test("csv-head --lines=2" 3-columns-3-rows.csv 3-columns-2-first-rows.csv empty.txt 0
	head_--lines=2_and_there_are_3_lines_in_input)

test("csv-head -n 2" 3-columns-3-rows.csv 3-columns-2-first-rows.csv empty.txt 0
	head_-n_2_and_there_are_3_lines_in_input)

test("csv-head --lines=3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	head_--lines=3_and_there_are_3_lines_in_input)

test("csv-head -n 3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	head_-n_3_and_there_are_3_lines_in_input)

################################## TAIL ########################################

test("csv-tail" empty.csv empty.csv eof.txt 2
	tail_empty_input)

test("csv-tail" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	tail_simple_pass_through)

test("csv-tail --lines=0" one-column-one-row.csv zero-lines.csv empty.txt 0
	tail_--lines=0)

test("csv-tail -n 0" one-column-one-row.csv zero-lines.csv empty.txt 0
	tail_-n_0)

test("csv-tail --lines=1" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	tail_--lines=1)

test("csv-tail -n 1" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	tail_-n_1)

test("csv-tail --lines 2" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	tail_--lines=2_but_there's_only_1_line_in_input)

test("csv-tail -n 2" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	tail_-n_2_but_there's_only_1_line_in_input)

test("csv-tail --lines=2" 3-columns-3-rows.csv 3-columns-2-last-rows.csv empty.txt 0
	tail_--lines=2_but_there_are_3_lines_in_input)

test("csv-tail -n 2" 3-columns-3-rows.csv 3-columns-2-last-rows.csv empty.txt 0
	tail_-n_2_but_there_are_3_lines_in_input)

test("csv-tail --lines=3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	tail_--lines=3_and_there_are_3_lines_in_input)

test("csv-tail -n 3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	tail_-n_3_and_there_are_3_lines_in_input)
