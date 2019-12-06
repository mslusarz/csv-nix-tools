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

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

# conventions for test names:
# - start with a tool name, without "csv-" prefix
# - use _ as a word separator

# conventions for file names:
# - use tool- prefix for test-specific files
# - do not use tool- prefix for general purpose files
# - use - as a word separator
# - use .csv extension for csv files
# - use .txt extension for other files

############################### CSV PARSING ####################################

test("csv-cat" empty.txt empty.csv eof.txt 2
	cat_empty_input)

test("csv-cat" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cat_one_column_one_row)

test("csv-cat" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cat_3_columns_3_rows)

test("csv-cat" commas.csv commas.csv empty.txt 0
	cat_commas)

test("csv-cat" newlines.csv newlines.csv empty.txt 0
	cat_newlines)

test("csv-cat" quotes.csv quotes.csv empty.txt 0
	cat_quotes)

################################### AVG ########################################

test("csv-avg -f id" id-column-3-rows.csv avg-id.csv empty.txt 0
	avg)

test("csv-avg -f col1,col2,col3" 3-numeric-columns-4-rows.csv avg-3-columns.csv empty.txt 0
	avg-3-columns)

test("csv-avg -f col3,col1" 3-numeric-columns-4-rows.csv avg-2-columns.csv empty.txt 0
	avg-2-columns)

################################### CAT ########################################

test("csv-cat ${DATA_DIR}/one-column-one-row.csv" empty.csv one-column-one-row.csv empty.txt 0
	cat_one_file)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv" empty.csv cat-2-files.csv empty.txt 0
	cat_two_files)

test("csv-cat - ${DATA_DIR}/one-column-one-row.csv" one-column-one-row.csv cat-2-files.csv empty.txt 0
	cat_stdin_and_file)

test("csv-cat ${DATA_DIR}/3-different-order-columns-3-rows.csv ${DATA_DIR}/3-columns-2-last-rows.csv" empty.csv cat-2-different-files.csv empty.txt 0
	cat_two_different_files)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/2-columns-3-rows.csv" empty.csv empty.csv cat-2-files-different-num-columns.txt 2
	cat_two_files_different_columns)

test("csv-cat --help" empty.csv cat-help.txt empty.txt 2
	cat_help)

test("csv-cat --version" empty.csv git-version.txt empty.txt 0
	cat_version)

test("csv-cat --no-header" 2-columns-3-rows.csv cat-no-headers.txt empty.txt 0
	cat_no_header)

test("csv-cat -s" 2-columns-3-rows.csv cat-show.txt empty.txt 0
	cat_show)

test("csv-cat - -" 2-columns-3-rows.csv empty.txt cat-stdin-twice.txt 2
	cat_stdin_twice)

test("csv-cat not-existing" empty.csv empty.txt cat-not-existing.txt 2
	cat_not_existing)

################################# CONCAT #######################################

test("csv-concat" empty.txt empty.csv concat-no-args.txt 2
	concat_no_args)

test("csv-concat new_column = %name ' - ' %id" 3-columns-3-rows.csv concat-2-cols.csv empty.txt 0
	concat_2_cols)

test("csv-concat new_column = %name ', ' %id" 3-columns-3-rows.csv concat-2-cols-comma.csv empty.txt 0
	concat_2_cols_comma)

################################## COUNT #######################################

test("csv-count" empty.txt empty.csv count-no-args.txt 2
	count_no_args)

test("csv-count -c" empty.txt empty.csv eof.txt 2
	count_empty_input)

test("csv-count -c" 2-columns-3-rows.csv count-2-cols.txt empty.txt 0
	count_-c_2_cols)

test("csv-count -r" 2-columns-3-rows.csv count-3-rows.txt empty.txt 0
	count_-r_3_rows)

test("csv-count -c -r" 2-columns-3-rows.csv count-2-cols-3-rows.txt empty.txt 0
	count_-c_-r_2_cols_3_rows)

################################### CUT ########################################

test("csv-cut" empty.txt empty.csv eof.txt 2
	cut_empty_input)

test("csv-cut" one-column-one-row.csv empty.txt cut-usage.txt 2
	cut_simple_pass_through)

test("csv-cut -f" one-column-one-row.csv cut-usage.txt cut-no-fields-error.txt 2
	cut_simple_pass_through_with_-f)

test("csv-cut -f name" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cut_the_only_field_using_-f)

test("csv-cut --fields name" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cut_the_only_field_using_--fields_with_space)

test("csv-cut --fields=name" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	cut_the_only_field_using_--fields_with_=)

test("csv-cut -f notExistingColumn" one-column-one-row.csv empty.txt column-not-found.txt 2
	cut_not_existing_field_using_-f)

test("csv-cut --fields notExistingColumn" one-column-one-row.csv empty.txt column-not-found.txt 2
	cut_not_existing_field_using_--fields_with_space)

test("csv-cut --fields=notExistingColumn" one-column-one-row.csv empty.txt column-not-found.txt 2
	cut_not_existing_field_using_--fields_with_=)


test("csv-cut -f name" 3-columns-3-rows.csv name-column-3-rows.csv empty.txt 0
	cut_the_name_field_using_-f)

test("csv-cut --fields name" 3-columns-3-rows.csv name-column-3-rows.csv empty.txt 0
	cut_the_name_field_using_--fields_with_space)

test("csv-cut --fields=name" 3-columns-3-rows.csv name-column-3-rows.csv empty.txt 0
	cut_the_name_field_using_--fields_with_=)


test("csv-cut -f id" 3-columns-3-rows.csv id-column-3-rows.csv empty.txt 0
	cut_the_id_field_using_-f)

test("csv-cut --fields id" 3-columns-3-rows.csv id-column-3-rows.csv empty.txt 0
	cut_the_id_field_using_--fields_with_space)

test("csv-cut --fields=id" 3-columns-3-rows.csv id-column-3-rows.csv empty.txt 0
	cut_the_id_field_using_--fields_with_=)


test("csv-cut -f something" 3-columns-3-rows.csv something-column-3-rows.csv empty.txt 0
	cut_the_something_field_using_-f)

test("csv-cut --fields something" 3-columns-3-rows.csv something-column-3-rows.csv empty.txt 0
	cut_the_something_field_using_--fields_with_space)

test("csv-cut --fields=something" 3-columns-3-rows.csv something-column-3-rows.csv empty.txt 0
	cut_the_something_field_using_--fields_with_=)


test("csv-cut -f name,something" 3-columns-3-rows.csv 2-columns-3-rows.csv empty.txt 0
	cut_2_fields_using_-f)

test("csv-cut --fields name,something" 3-columns-3-rows.csv 2-columns-3-rows.csv empty.txt 0
	cut_2_fields_using_--fields_with_spaces)

test("csv-cut --fields=name,something" 3-columns-3-rows.csv 2-columns-3-rows.csv empty.txt 0
	cut_2_fields_using_--fields_with_=)


test("csv-cut -f name,id,something" 3-different-order-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cut_3_fields_in_different_order_using_-f)

test("csv-cut --fields name,id,something" 3-different-order-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cut_3_fields_in_different_order_using_--fields_with_space)

test("csv-cut --fields=name,id,something" 3-different-order-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	cut_3_fields_in_different_order_using_--fields_with_=)

################################### EXEC #######################################

test_with_cwd("csv-exec -- csv-ls -f size %name" files.csv sizes.txt empty.txt 0
	exec_csv-ls_-f_size
	${CMAKE_SOURCE_DIR}/tests/files/)

################################# EXEC-ADD #####################################

test("csv-exec-add -n name2 -f name -- sed 's/file/name/g'" files.csv exec-add-names.csv empty.txt 0
	exec-add_sed)

################################### GREP #######################################

test("csv-grep" one-column-one-row.csv empty.csv grep-usage.txt 2
	grep_no_args)

test("csv-grep -f x -F y" empty.csv empty.csv eof.txt 2
	grep_empty_input)


test("csv-grep -f name -e or" 3-columns-3-rows.csv grep-name-or.csv empty.txt 0
	grep_-f_name_-e_or)

test("csv-grep -f name -e or | csv-count -r --no-header" 3-columns-3-rows.csv 1.txt empty.txt 0
	grep_-f_name_-e_or_count)

test("csv-grep -f name -x -e or | csv-count -r --no-header" 3-columns-3-rows.csv 0.txt empty.txt 0
	grep_-f_name_-x_-e_or_count)

test("csv-grep -f name -e or.m | csv-count -r --no-header" 3-columns-3-rows.csv 1.txt empty.txt 0
	grep_-f_name_-e_or.m_count)


test("csv-grep -f name -E or" 3-columns-3-rows.csv grep-name-or.csv empty.txt 0
	grep_-f_name_-E_or)

test("csv-grep -f name -E or | csv-count -r --no-header" 3-columns-3-rows.csv 1.txt empty.txt 0
	grep_-f_name_-E_or_count)

test("csv-grep -f name -x -E or | csv-count -r --no-header" 3-columns-3-rows.csv 0.txt empty.txt 0
	grep_-f_name_-x_-E_or_count)

test("csv-grep -f name -E or.m | csv-count -r --no-header" 3-columns-3-rows.csv 1.txt empty.txt 0
	grep_-f_name_-E_or.m_count)


test("csv-grep -f name -F or" 3-columns-3-rows.csv grep-name-or.csv empty.txt 0
	grep_-f_name_-F_or)

test("csv-grep -f name -F or | csv-count -r --no-header" 3-columns-3-rows.csv 1.txt empty.txt 0
	grep_-f_name_-F_or_count)

test("csv-grep -f name -x -F or | csv-count -r --no-header" 3-columns-3-rows.csv 0.txt empty.txt 0
	grep_-f_name_-x_-F_or_count)

test("csv-grep -f name -F or.m | csv-count -r --no-header" 3-columns-3-rows.csv 0.txt empty.txt 0
	grep_-f_name_-F_or.m_count)

################################## GROUPS ######################################

test("csv-groups | csv-grep -f name -x -F root | csv-count -c --no-header" empty.txt 5.txt empty.txt 0
	groups)

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
	head_--lines=2_but_there_is_only_1_line_in_input)

test("csv-head -n 2" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	head_-n_2_but_there_is_only_1_line_in_input)

test("csv-head --lines=2" 3-columns-3-rows.csv 3-columns-2-first-rows.csv empty.txt 0
	head_--lines=2_and_there_are_3_lines_in_input)

test("csv-head -n 2" 3-columns-3-rows.csv 3-columns-2-first-rows.csv empty.txt 0
	head_-n_2_and_there_are_3_lines_in_input)

test("csv-head --lines=3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	head_--lines=3_and_there_are_3_lines_in_input)

test("csv-head -n 3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	head_-n_3_and_there_are_3_lines_in_input)

#################################### LS ########################################

test_with_cwd("csv-ls -f name"
	empty.txt files_and_dirs.csv empty.txt 0
	ls-files
	${CMAKE_SOURCE_DIR}/tests/files)

test_with_cwd("csv-ls | csv-count -c"
	empty.txt ls-count-columns.csv empty.txt 0
	ls-count-columns
	${CMAKE_SOURCE_DIR}/tests/files)

test_with_cwd("csv-ls -l | csv-count -c"
	empty.txt ls-l-count-columns.csv empty.txt 0
	ls-l-count-columns
	${CMAKE_SOURCE_DIR}/tests/files)

################################### MIN ########################################

test("csv-min -f id" id-column-3-rows.csv min-id.csv empty.txt 0
	min)

test("csv-min -f col1,col2,col3" 3-numeric-columns-4-rows.csv min-3-columns.csv empty.txt 0
	min-3-columns)

test("csv-min -f col3,col1" 3-numeric-columns-4-rows.csv min-2-columns.csv empty.txt 0
	min-2-columns)

################################### MAX ########################################

test("csv-max -f id" id-column-3-rows.csv max-id.csv empty.txt 0
	max)

test("csv-max -f col1,col2,col3" 3-numeric-columns-4-rows.csv max-3-columns.csv empty.txt 0
	max-3-columns)

test("csv-max -f col3,col1" 3-numeric-columns-4-rows.csv max-2-columns.csv empty.txt 0
	max-2-columns)

################################# NETSTAT ######################################

if (LIBMNL_FOUND)

test("csv-netstat | csv-count -c" empty.txt netstat-count-columns.csv empty.txt 0
	netstat_count_columns)

test("csv-netstat -e | csv-count -c" empty.txt netstat-e-count-columns.csv empty.txt 0
	netstat_e_count_columns)

test("csv-netstat -t | csv-count -c" empty.txt netstat-t-count-columns.csv empty.txt 0
	netstat_t_count_columns)

test("csv-netstat -u | csv-count -c" empty.txt netstat-u-count-columns.csv empty.txt 0
	netstat_u_count_columns)

test("csv-netstat -w | csv-count -c" empty.txt netstat-w-count-columns.csv empty.txt 0
	netstat_w_count_columns)

test("csv-netstat -x | csv-count -c" empty.txt netstat-x-count-columns.csv empty.txt 0
	netstat_x_count_columns)

test("csv-netstat -4 | csv-count -c" empty.txt netstat-4-count-columns.csv empty.txt 0
	netstat_4_count_columns)

test("csv-netstat -6 | csv-count -c" empty.txt netstat-6-count-columns.csv empty.txt 0
	netstat_6_count_columns)

endif(LIBMNL_FOUND)

################################### PS ########################################

if (LIBPROCPS_FOUND)

test("csv-ps | csv-count -c" empty.txt ps-columns.csv empty.txt 0
	ps_columns)

endif(LIBPROCPS_FOUND)

################################# REPLACE ######################################

test("csv-replace -f name -F ' i' -r 'STRING' -n new-col" 3-columns-3-rows.csv replace-string.csv empty.txt 0
	replace_string)

test("csv-replace -f name -e '\\(.*\\)\\( i\\)\\(.*\\)' -r '%3 STRING %1' -n new-col" 3-columns-3-rows.csv replace-regex.csv empty.txt 0
	replace_regex)

test("csv-replace -f name -E '(.*)( i)(.*)' -r '%3 STRING %1' -n new-col" 3-columns-3-rows.csv replace-regex.csv empty.txt 0
	replace_eregex)

################################# RPN-ADD ######################################

test("csv-rpn-add -f 'num_mul_2' -e '%num 2 *'"
	rpn-add-num-dec.csv rpn-add-num-mul.csv empty.txt 0
	rpn-add_num_mul)

test("csv-rpn-add -f 'num_add_num2' -e '%num %num2 +'"
	rpn-add-num-dec.csv rpn-add-num-sum.csv empty.txt 0
	rpn-add_num_sum)

test("csv-rpn-add -f 'num2_div_num3' -e '%num2 %num3 /'\
		  -f 'num2_div_num_add_1' -e '%num2 %num 1 + /'"
	rpn-add-num-dec.csv rpn-add-num-div.csv empty.txt 0
	rpn-add_num_div)

test("csv-rpn-add -f 'num2_div_num' -e '%num2 %num /'"
	rpn-add-num-dec.csv rpn-add-num-div0.csv rpn-add-num-div0.txt 2
	rpn-add_num_div0)

test("csv-rpn-add -f 'num2_sub_num' -e '%num2 %num -'"
	rpn-add-num-dec.csv rpn-add-num-sub.csv empty.txt 0
	rpn-add_num_sub)

test("csv-rpn-add -f 'num2_mod_num3' -e '%num2 %num3 %'\
		  -f 'num2_mod_num_add_1' -e '%num2 %num 1 + %'"
	rpn-add-num-dec.csv rpn-add-num-mod.csv empty.txt 0
	rpn-add_num_mod)


test("csv-rpn-add -f 'num2_bit_and_num3' -e '%num2 %num3 & tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-and.csv empty.txt 0
	rpn-add_num_bit_and)

test("csv-rpn-add -f 'num2_bit_or_num' -e '%num2 %num | tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-or.csv empty.txt 0
	rpn-add_num_bit_or)

test("csv-rpn-add -f 'num2_bit_xor_num' -e '%num2 %num ^ tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-xor.csv empty.txt 0
	rpn-add_num_bit_xor)

test("csv-rpn-add -f 'bit_neg_num2' -e '%num2 ~ tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-neg.csv empty.txt 0
	rpn-add_num_bit_neg)

test("csv-rpn-add -f 'num2_lshift_num' -e '%num2 %num << tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-lshift.csv empty.txt 0
	rpn-add_num_bit_lshift)

test("csv-rpn-add -f 'num2_rshift_num' -e '%num2 %num >> tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-rshift.csv empty.txt 0
	rpn-add_num_bit_rshift)


test("csv-rpn-add -f num2_gt_num3 -e '%num2 %num3 >'  -f num2_gt_num2_dup -e '%num2 %num2_dup >'  -f num2_gt_num2_mul_2 -e '%num %num2_mul_2 >'"  rpn-add-num-compare.csv rpn-add-num-gt.csv empty.txt 0
	rpn-add_num_>)
test("csv-rpn-add -f num2_gt_num3 -e '%num2 %num3 gt' -f num2_gt_num2_dup -e '%num2 %num2_dup gt' -f num2_gt_num2_mul_2 -e '%num %num2_mul_2 gt'" rpn-add-num-compare.csv rpn-add-num-gt.csv empty.txt 0
	rpn-add_num_gt)

test("csv-rpn-add -f num2_ge_num3 -e '%num2 %num3 >=' -f num2_ge_num2_dup -e '%num2 %num2_dup >=' -f num2_ge_num2_mul_2 -e '%num %num2_mul_2 >='" rpn-add-num-compare.csv rpn-add-num-ge.csv empty.txt 0
	rpn-add_num_>=)
test("csv-rpn-add -f num2_ge_num3 -e '%num2 %num3 ge' -f num2_ge_num2_dup -e '%num2 %num2_dup ge' -f num2_ge_num2_mul_2 -e '%num %num2_mul_2 ge'" rpn-add-num-compare.csv rpn-add-num-ge.csv empty.txt 0
	rpn-add_num_ge)

test("csv-rpn-add -f num2_lt_num3 -e '%num2 %num3 <'  -f num2_lt_num2_dup -e '%num2 %num2_dup <'  -f num2_lt_num2_mul_2 -e '%num %num2_mul_2 <'"  rpn-add-num-compare.csv rpn-add-num-lt.csv empty.txt 0
	rpn-add_num_<)
test("csv-rpn-add -f num2_lt_num3 -e '%num2 %num3 lt' -f num2_lt_num2_dup -e '%num2 %num2_dup lt' -f num2_lt_num2_mul_2 -e '%num %num2_mul_2 lt'" rpn-add-num-compare.csv rpn-add-num-lt.csv empty.txt 0
	rpn-add_num_lt)

test("csv-rpn-add -f num2_le_num3 -e '%num2 %num3 <=' -f num2_le_num2_dup -e '%num2 %num2_dup <=' -f num2_le_num2_mul_2 -e '%num %num2_mul_2 <='" rpn-add-num-compare.csv rpn-add-num-le.csv empty.txt 0
	rpn-add_num_<=)
test("csv-rpn-add -f num2_le_num3 -e '%num2 %num3 le' -f num2_le_num2_dup -e '%num2 %num2_dup le' -f num2_le_num2_mul_2 -e '%num %num2_mul_2 le'" rpn-add-num-compare.csv rpn-add-num-le.csv empty.txt 0
	rpn-add_num_le)

test("csv-rpn-add -f num2_eq_num2_dup -e '%num2 %num2_dup ==' -f num2_eq_num2_mul_2 -e '%num2 %num2_mul_2 =='" rpn-add-num-compare.csv rpn-add-num-eq.csv empty.txt 0
	rpn-add_num_==)
test("csv-rpn-add -f num2_eq_num2_dup -e '%num2 %num2_dup eq' -f num2_eq_num2_mul_2 -e '%num2 %num2_mul_2 eq'" rpn-add-num-compare.csv rpn-add-num-eq.csv empty.txt 0
	rpn-add_num_eq)

test("csv-rpn-add -f num2_ne_num2_dup -e '%num2 %num2_dup !=' -f num2_ne_num2_mul_2 -e '%num2 %num2_mul_2 !='" rpn-add-num-compare.csv rpn-add-num-ne.csv empty.txt 0
	rpn-add_num_!=)
test("csv-rpn-add -f num2_ne_num2_dup -e '%num2 %num2_dup ne' -f num2_ne_num2_mul_2 -e '%num2 %num2_mul_2 ne'" rpn-add-num-compare.csv rpn-add-num-ne.csv empty.txt 0
	rpn-add_num_ne)


test("csv-rpn-add -f num1_or_num2 -e '%num1 %num2 or'"
	rpn-add-num-logic.csv rpn-add-num-logic-or.csv empty.txt 0
	rpn-add_num_logic_or)

test("csv-rpn-add -f num1_and_num2 -e '%num1 %num2 and'"
	rpn-add-num-logic.csv rpn-add-num-logic-and.csv empty.txt 0
	rpn-add_num_logic_and)

test("csv-rpn-add -f num1_xor_num2 -e '%num1 %num2 xor'"
	rpn-add-num-logic.csv rpn-add-num-logic-xor.csv empty.txt 0
	rpn-add_num_logic_xor)

test("csv-rpn-add -f not_num1 -e '%num1 not'"
	rpn-add-num-logic.csv rpn-add-num-logic-not.csv empty.txt 0
	rpn-add_num_logic_not)

test("csv-rpn-add -f desc_num1 -e \"%num1 'one' 'zero' if\""
	rpn-add-num-logic.csv rpn-add-num-logic-if.csv empty.txt 0
	rpn-add_num_logic_if)


test("csv-rpn-add -f substr2 -e '%str1 2 1000 substr'\
		  -f last_char -e '%str1 -1 1 substr'"
	rpn-add-str.csv rpn-add-str-substr.csv empty.txt 0
	rpn-add_str_substr)

test("csv-rpn-add -f len1 -e '%str1 strlen'\
		  -f len2 -e '%str2 strlen'"
	rpn-add-str.csv rpn-add-str-strlen.csv empty.txt 0
	rpn-add_str_strlen)

test("csv-rpn-add -f concat1 -e '%str1 %str2 concat'\
		  -f concat2 -e \"%str1 ' - ' %str2 concat concat\""
	rpn-add-str.csv rpn-add-str-concat.csv empty.txt 0
	rpn-add_str_concat)

test("csv-rpn-add -f str1_like_%12 -e \"%str1 '%12' like\"\
		  -f str1_like_%1% -e \"%str1 '%1%' like\""
	rpn-add-str.csv rpn-add-str-like.csv empty.txt 0
	rpn-add_str_like)


test("csv-rpn-add -f str_to_int -e '%str toint'\
		  -f num_to_string   -e '%num tostring'\
		  -f num_to_string2  -e '%num tostring_base2'\
		  -f num_to_string8  -e '%num tostring_base8'\
		  -f num_to_string10 -e '%num tostring_base10'\
		  -f num_to_string16 -e '%num tostring_base16'"
	rpn-add-num-base.csv rpn-add-convert.csv empty.txt 0
	rpn-add_str_to_int)

################################ RPN-FILTER ####################################

test("csv-rpn-filter -e '%id 2 =='" 3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	rpn-filter_eq)

test("csv-rpn-filter -e '%id 2 !='" 3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	rpn-filter_ne)

test("csv-rpn-filter -e '%id 2 >'" 3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	rpn-filter_gt)

test("csv-rpn-filter -e '%id 3 >='" 3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	rpn-filter_ge)

test("csv-rpn-filter -e '%id 2 <'" 3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	rpn-filter_lt)

test("csv-rpn-filter -e '%id 1 <='" 3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	rpn-filter_le)

test("csv-rpn-filter -e '%id 1 == %id 3 == or'" 3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	rpn-filter_or)

test("csv-rpn-filter -e '%id 2 == not'" 3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	rpn-filter_not)

test("csv-rpn-filter -e '%id 1 == %id 3 == xor'" 3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	rpn-filter_xor)

test("csv-rpn-filter -e '%id 1 + 3 =='" 3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	rpn-filter_add)

test("csv-rpn-filter -e '%id 1 - 2 =='" 3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	rpn-filter_sub)

test("csv-rpn-filter -e '%id 2 * 2 =='" 3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	rpn-filter_mul)

test("csv-rpn-filter -e '%id 2 / 1 =='" 3-columns-3-rows.csv rpn-filter-rows-2-3.csv empty.txt 0
	rpn-filter_div)

test("csv-rpn-filter -e '%name strlen 11 =='" 3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	rpn-filter_strlen)

test("csv-rpn-filter -e \"%name 3 1 substr 'r' ==\"" 3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	rpn-filter_substr)

test("csv-rpn-filter -e \"%name '%th%' like\"" 3-columns-3-rows.csv rpn-filter-rows-2-3.csv empty.txt 0
	rpn-filter_like)

test("csv-rpn-filter -e \"%name '.suffix' concat 'lorem ipsum.suffix' ==\"" 3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	rpn-filter_concat)

test("csv-rpn-filter -e \"%id tostring '2' ==\"" 3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	rpn-filter_tostring)

################################### SHOW #######################################

test("csv-show" 3-columns-3-rows.csv show-3-columns-3-rows.txt empty.txt 0
	show_3_cols)

test("csv-show --ui=less" 3-columns-3-rows.csv show-3-columns-3-rows.txt empty.txt 0
	show_3_cols_less)

test("csv-show" newlines.csv show-newlines.txt empty.txt 0
	show_newlines)

test("csv-show" quotes.csv show-quotes.txt empty.txt 0
	show_quotes)

test("csv-show" commas.csv show-commas.txt empty.txt 0
	show_commas)

test("csv-show" control-characters.csv show-control-characters.txt empty.txt 0
	show_control_characters)

################################### SORT #######################################

test("csv-sort -f id" id-column-3-rows.csv id-column-3-rows.csv empty.txt 0
	sort_sorted)

test("csv-sort -f id -r" id-column-3-rows.csv sort-rsorted.csv empty.txt 0
	sort_rsorted)

test("csv-sort -f id,name" sort-2-cols.csv sort-2-cols-sorted.csv empty.txt 0
	sort_2_cols)

test("csv-sort -f id,name -r" sort-2-cols.csv sort-2-cols-rsorted.csv empty.txt 0
	sort_2_cols_rsorted)

################################## SPLIT #######################################

test("csv-split -f column3 -n before,after -e,"
	commas.csv split-commas.csv empty.txt 0
	split_commas)

test("csv-split -f column3 -n before,after -e, -r"
	commas.csv split-commas-reverse.csv empty.txt 0
	split_commas_reverse)

################################### SQL ########################################
if (BISON_FOUND AND FLEX_FOUND)

test("csv-sql 'select * from input'"
	3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	sql_select_all_from_input)

test("csv-sql 'select * from input where id == 2'"
	3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	sql_select_all_from_input_where_id==2)

test("csv-sql 'select * from input where id != 2'"
	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	sql_select_all_from_input_where_id!=2)

test("csv-sql 'select * from input where id > 2'"
	3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	sql_select_all_from_input_where_id>2)

test("csv-sql 'select * from input where id >= 3'"
	3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	sql_select_all_from_input_where_id>=3)

test("csv-sql 'select * from input where id < 2'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sql_select_all_from_input_where_id<2)

test("csv-sql 'select * from input where id <= 1'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sql_select_all_from_input_where_id<=1)

test("csv-sql 'select * from input where id == 1 or id == 3'"
	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	sql_select_all_from_input_where_id==1_or_id==3)

test("csv-sql 'select * from input where not id == 2'"
	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	sql_select_all_from_input_where_not_id==2)

# sql supports xor
test("csv-sql 'select * from input where id == 1 xor id == 3'"
	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	sql_select_all_from_input_where_id==1_xor_id==3)

test("csv-sql 'select * from input where id + 1 == 3'"
	3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	sql_select_all_from_input_where_id+1==3)

test("csv-sql 'select * from input where id - 1 == 2'"
	3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	sql_select_all_from_input_where_id-1==2)

test("csv-sql 'select * from input where id * 2 == 2'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sql_select_all_from_input_where_id*2==2)

test("csv-sql 'select * from input where id / 2 == 1'"
	3-columns-3-rows.csv rpn-filter-rows-2-3.csv empty.txt 0
	sql_select_all_from_input_where_id_div_2==1)

test("csv-sql 'select * from input where length(name) == 11'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sql_select_all_from_input_where_length_name==11)

test("csv-sql \"select * from input where substr(name, 3, 1) == 'r'\""
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sql_select_all_from_input_where_substr_name-3-1==r)

test("csv-sql \"select * from input where name like '%th%'\""
	3-columns-3-rows.csv rpn-filter-rows-2-3.csv empty.txt 0
	sql_select_all_from_input_where_name_like_th)

test("csv-sql \"select * from input where name || '.suffix' == 'lorem ipsum.suffix'\""
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sql_select_all_from_input_where_name||.suffix==loremipsum.suffix)

# cast needed
test("csv-sql \"select * from input where tostring(id) == '2'\""
	3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	sql_select_all_from_input_where_tostring_id==string2)


test("csv-sql 'select num, num2, num3, num * 2 as num_mul_2 from input'"
	rpn-add-num-dec.csv rpn-add-num-mul.csv empty.txt 0
	sql_num_mul)

test("csv-sql 'select num, num2, num3, num + num2 as num_add_num2 from input'"
	rpn-add-num-dec.csv rpn-add-num-sum.csv empty.txt 0
	sql_num_sum)

test("csv-sql 'select num, num2, num3, num2 / num3 as num2_div_num3, num2 / (num + 1) as num2_div_num_add_1 from input'"
	rpn-add-num-dec.csv rpn-add-num-div.csv empty.txt 0
	sql_num_div)

# 1/0 is invalid in sql
test("csv-sql 'select num, num2, num3, num2 / num as num2_div_num from input'"
	rpn-add-num-dec.csv rpn-add-num-div0.csv rpn-add-num-div0.txt 2
	sql_num_div0)

test("csv-sql 'select num, num2, num3, num2 - num as num2_sub_num from input'"
	rpn-add-num-dec.csv rpn-add-num-sub.csv empty.txt 0
	sql_num_sub)

test("csv-sql 'select num, num2, num3, num2 % num3 as num2_mod_num3, num2 % (num + 1) as num2_mod_num_add_1 from input'"
	rpn-add-num-dec.csv rpn-add-num-mod.csv empty.txt 0
	sql_num_mod)

# original encoding is lost
#test("csv-sql 'select num, num2, num3, fmt_hex(num2 & num3) as num2_bit_and_num3 from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-and.csv empty.txt 0
#	sql_num_bit_and)

#test("csv-sql 'select num, num2, num3, fmt_hex(num2 | num) as num2_bit_or_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-or.csv empty.txt 0
#	sql_num_bit_or)

#test("csv-sql 'select num, num2, num3, fmt_hex(num2 ^ num) as num2_bit_xor_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-xor.csv empty.txt 0
#	sql_num_bit_xor)

#test("csv-sql 'select num, num2, num3, fmt_hex(~num2) as bit_neg_num2 from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-neg.csv empty.txt 0
#	sql_num_bit_neg)

#test("csv-sql 'select num, num2, num3, fmt_hex(num2 << num) as num2_lshift_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-lshift.csv empty.txt 0
#	sql_num_bit_lshift)

#test("csv-sql 'select num, num2, num3, fmt_hex(num2 >> num) as num2_rshift_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-rshift.csv empty.txt 0
#	sql_num_bit_rshift)


# sql doesn't support gt, lt, ge, le, eq, ne
test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 > num3 as num2_gt_num3, num2 > num2_dup as num2_gt_num2_dup, num2 > num2_mul_2 as num2_gt_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-gt.csv empty.txt 0
	sql_num_>)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 >= num3 as num2_ge_num3, num2 >= num2_dup as num2_ge_num2_dup, num2 >= num2_mul_2 as num2_ge_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-ge.csv empty.txt 0
	sql_num_>=)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 < num3 as num2_lt_num3, num2 < num2_dup as num2_lt_num2_dup, num2 < num2_mul_2 as num2_lt_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-lt.csv empty.txt 0
	sql_num_<)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 <= num3 as num2_le_num3, num2 <= num2_dup as num2_le_num2_dup, num2 <= num2_mul_2 as num2_le_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-le.csv empty.txt 0
	sql_num_<=)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 == num2_dup as num2_eq_num2_dup, num2 == num2_mul_2 as num2_eq_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-eq.csv empty.txt 0
	sql_num_==)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 != num2_dup as num2_ne_num2_dup, num2 != num2_mul_2 as num2_ne_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-ne.csv empty.txt 0
	sql_num_!=)

test("csv-sql 'select num1, num2, num1 or num2 as num1_or_num2 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-or.csv empty.txt 0
	sql_num_logic_or)

test("csv-sql 'select num1, num2, num1 and num2 as num1_and_num2 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-and.csv empty.txt 0
	sql_num_logic_and)

# sql supports logic xor operator
test("csv-sql 'select num1, num2, num1 xor num2 as num1_xor_num2 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-xor.csv empty.txt 0
	sql_num_logic_xor)

test("csv-sql 'select num1, num2, not num1 as not_num1 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-not.csv empty.txt 0
	sql_num_logic_not)

# sql supports if operator, XXX rename if to case in rpn?
test("csv-sql 'select num1, num2, if(num1, \"one\", \"zero\") as desc_num1 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-if.csv empty.txt 0
	sql_num_logic_if)

#test("csv-sql 'select num1, num2, case when num1 then \"one\" else \"zero\" end as desc_num1 from input'"
#	rpn-add-num-logic.csv rpn-add-num-logic-if.csv empty.txt 0
#	sql_num_logic_if)

test("csv-sql 'select str1, str2, substr(str1, 2, 100) as substr2, substr(str1, -1, 1) as last_char from input'"
	rpn-add-str.csv rpn-add-str-substr.csv empty.txt 0
	sql_str_substr)

# XXX rename strlen to length in rpn?
test("csv-sql 'select str1, str2, length(str1) as len1, length(str2) as len2 from input'"
	rpn-add-str.csv rpn-add-str-strlen.csv empty.txt 0
	sql_str_length)

test("csv-sql 'select str1, str2, str1||str2 as concat1, str1||\" - \"||str2 as concat2 from input'"
	rpn-add-str.csv rpn-add-str-concat.csv empty.txt 0
	sql_str_concat)

test("csv-sql 'select str1, str2, str1 like \"%12\" as \"str1_like_%12\", str1 like \"%1%\" as \"str1_like_%1%\" from input'"
	rpn-add-str.csv rpn-add-str-like.csv empty.txt 0
	sql_str_like)

# toint doesn't exists in sql, XXX implement CAST?
#test("csv-sql 'select num, str, toint(str) as str_to_int,\
#			 tostring(num) as num_to_string,\
#			 tostring_base2(num) as num_to_string2,\
#			 tostring_base8(num) as num_to_string8,\
#			 tostring_base10(num) as num_to_string10,\
#			 tostring_base16(num) as num_to_string16\
#		    from input'"
#	rpn-add-num-base.csv rpn-add-convert.csv empty.txt 0
#	sql_str_to_int)

endif(BISON_FOUND AND FLEX_FOUND)

################################## SQLITE ######################################
if(SQLITE3_FOUND)

test("csv-sqlite 'select * from input'"
	3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	sqlite_select_all_from_input)

test("csv-sqlite 'select * from input where id == 2'"
	3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	sqlite_select_all_from_input_where_id==2)

test("csv-sqlite 'select * from input where id != 2'"
	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	sqlite_select_all_from_input_where_id!=2)

test("csv-sqlite 'select * from input where id > 2'"
	3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	sqlite_select_all_from_input_where_id>2)

test("csv-sqlite 'select * from input where id >= 3'"
	3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	sqlite_select_all_from_input_where_id>=3)

test("csv-sqlite 'select * from input where id < 2'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sqlite_select_all_from_input_where_id<2)

test("csv-sqlite 'select * from input where id <= 1'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sqlite_select_all_from_input_where_id<=1)

test("csv-sqlite 'select * from input where id == 1 or id == 3'"
	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	sqlite_select_all_from_input_where_id==1_or_id==3)

test("csv-sqlite 'select * from input where not id == 2'"
	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
	sqlite_select_all_from_input_where_not_id==2)

# sqlite doesn't support xor
#test("csv-sqlite 'select * from input where id == 1 xor id == 3'"
#	3-columns-3-rows.csv rpn-filter-rows-1-3.csv empty.txt 0
#	sqlite_select_all_from_input_where_id==1_xor_id==3)

test("csv-sqlite 'select * from input where id + 1 == 3'"
	3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	sqlite_select_all_from_input_where_id+1==3)

test("csv-sqlite 'select * from input where id - 1 == 2'"
	3-columns-3-rows.csv rpn-filter-row-3.csv empty.txt 0
	sqlite_select_all_from_input_where_id-1==2)

test("csv-sqlite 'select * from input where id * 2 == 2'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sqlite_select_all_from_input_where_id*2==2)

test("csv-sqlite 'select * from input where id / 2 == 1'"
	3-columns-3-rows.csv rpn-filter-rows-2-3.csv empty.txt 0
	sqlite_select_all_from_input_where_id_div_2==1)

test("csv-sqlite 'select * from input where length(name) == 11'"
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sqlite_select_all_from_input_where_length_name==11)

test("csv-sqlite \"select * from input where substr(name, 3, 1) == 'r'\""
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sqlite_select_all_from_input_where_substr_name_3_1==r)

test("csv-sqlite \"select * from input where name like '%th%'\""
	3-columns-3-rows.csv rpn-filter-rows-2-3.csv empty.txt 0
	sqlite_select_all_from_input_where_name_like_th)

test("csv-sqlite \"select * from input where name || '.suffix' == 'lorem ipsum.suffix'\""
	3-columns-3-rows.csv rpn-filter-row-1.csv empty.txt 0
	sqlite_select_all_from_input_where_name||.suffix==loremipsum.suffix)

# no cast needed
test("csv-sqlite \"select * from input where id == '2'\""
	3-columns-3-rows.csv rpn-filter-row-2.csv empty.txt 0
	sqlite_select_all_from_input_where_id==string2)


test("csv-sqlite 'select num, num2, num3, num * 2 as num_mul_2 from input'"
	rpn-add-num-dec.csv rpn-add-num-mul.csv empty.txt 0
	sqlite_num_mul)

test("csv-sqlite 'select num, num2, num3, num + num2 as num_add_num2 from input'"
	rpn-add-num-dec.csv rpn-add-num-sum.csv empty.txt 0
	sqlite_num_sum)

test("csv-sqlite 'select num, num2, num3, num2 / num3 as num2_div_num3, num2 / (num + 1) as num2_div_num_add_1 from input'"
	rpn-add-num-dec.csv rpn-add-num-div.csv empty.txt 0
	sqlite_num_div)

# 1/0 is NULL in sqlite
#test("csv-sqlite 'select num, num2, num3, num2 / num as num2_div_num from input'"
#	rpn-add-num-dec.csv rpn-add-num-div0.csv rpn-add-num-div0.txt 2
#	sqlite_num_div0)

test("csv-sqlite 'select num, num2, num3, num2 - num as num2_sub_num from input'"
	rpn-add-num-dec.csv rpn-add-num-sub.csv empty.txt 0
	sqlite_num_sub)

test("csv-sqlite 'select num, num2, num3, num2 % num3 as num2_mod_num3, num2 % (num + 1) as num2_mod_num_add_1 from input'"
	rpn-add-num-dec.csv rpn-add-num-mod.csv empty.txt 0
	sqlite_num_mod)

# original encoding is lost
#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 & num3) as num2_bit_and_num3 from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-and.csv empty.txt 0
#	sqlite_num_bit_and)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 | num) as num2_bit_or_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-or.csv empty.txt 0
#	sqlite_num_bit_or)

# + no support for bit xor
#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 ^ num) as num2_bit_xor_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-xor.csv empty.txt 0
#	sqlite_num_bit_xor)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", ~num2) as bit_neg_num2 from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-neg.csv empty.txt 0
#	sqlite_num_bit_neg)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 << num) as num2_lshift_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-lshift.csv empty.txt 0
#	sqlite_num_bit_lshift)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 >> num) as num2_rshift_num from input'"
#	rpn-add-num-hex.csv rpn-add-num-bit-rshift.csv empty.txt 0
#	sqlite_num_bit_rshift)


# sqlite doesn't support gt, lt, ge, le, eq, ne
test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 > num3 as num2_gt_num3, num2 > num2_dup as num2_gt_num2_dup, num2 > num2_mul_2 as num2_gt_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-gt.csv empty.txt 0
	sqlite_num_>)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 >= num3 as num2_ge_num3, num2 >= num2_dup as num2_ge_num2_dup, num2 >= num2_mul_2 as num2_ge_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-ge.csv empty.txt 0
	sqlite_num_>=)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 < num3 as num2_lt_num3, num2 < num2_dup as num2_lt_num2_dup, num2 < num2_mul_2 as num2_lt_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-lt.csv empty.txt 0
	sqlite_num_<)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 <= num3 as num2_le_num3, num2 <= num2_dup as num2_le_num2_dup, num2 <= num2_mul_2 as num2_le_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-le.csv empty.txt 0
	sqlite_num_<=)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 == num2_dup as num2_eq_num2_dup, num2 == num2_mul_2 as num2_eq_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-eq.csv empty.txt 0
	sqlite_num_==)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 != num2_dup as num2_ne_num2_dup, num2 != num2_mul_2 as num2_ne_num2_mul_2 from input'"
	rpn-add-num-compare.csv rpn-add-num-ne.csv empty.txt 0
	sqlite_num_!=)

test("csv-sqlite 'select num1, num2, num1 or num2 as num1_or_num2 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-or.csv empty.txt 0
	sqlite_num_logic_or)

test("csv-sqlite 'select num1, num2, num1 and num2 as num1_and_num2 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-and.csv empty.txt 0
	sqlite_num_logic_and)

# sqlite doesn't support logic xor operator
#test("csv-sqlite 'select num1, num2, num1 xor num2 as num1_xor_num2 from input'"
#	rpn-add-num-logic.csv rpn-add-num-logic-xor.csv empty.txt 0
#	sqlite_num_logic_xor)

test("csv-sqlite 'select num1, num2, not num1 as not_num1 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-not.csv empty.txt 0
	sqlite_num_logic_not)

# sqlite doesn't support if operator, but supports case, XXX rename if to case in rpn?
#test("csv-sqlite 'select num1, num2, num1 ? \"one\" : \"zero\" as desc_num1 from input'"
#	rpn-add-num-logic.csv rpn-add-num-logic-if.csv empty.txt 0
#	sqlite_num_logic_if)

test("csv-sqlite 'select num1, num2, case when num1 then \"one\" else \"zero\" end as desc_num1 from input'"
	rpn-add-num-logic.csv rpn-add-num-logic-if.csv empty.txt 0
	sqlite_num_logic_if)

test("csv-sqlite 'select str1, str2, substr(str1, 2, 100) as substr2, substr(str1, -1, 1) as last_char from input'"
	rpn-add-str.csv rpn-add-str-substr.csv empty.txt 0
	sqlite_str_substr)

# XXX rename strlen to length in rpn?
test("csv-sqlite 'select str1, str2, length(str1) as len1, length(str2) as len2 from input'"
	rpn-add-str.csv rpn-add-str-strlen.csv empty.txt 0
	sqlite_str_length)

test("csv-sqlite 'select str1, str2, str1||str2 as concat1, str1||\" - \"||str2 as concat2 from input'"
	rpn-add-str.csv rpn-add-str-concat.csv empty.txt 0
	sqlite_str_concat)

test("csv-sqlite 'select str1, str2, str1 like \"%12\" as \"str1_like_%12\", str1 like \"%1%\" as \"str1_like_%1%\" from input'"
	rpn-add-str.csv rpn-add-str-like.csv empty.txt 0
	sqlite_str_like)

# no such functions in sqlite
#test("csv-sqlite 'select num, str, toint(str) as str_to_int,\
#			 tostring(num) as num_to_string,\
#			 tostring_base2(num) as num_to_string2,\
#			 tostring_base8(num) as num_to_string8,\
#			 tostring_base10(num) as num_to_string10,\
#			 tostring_base16(num) as num_to_string16\
#		    from input'"
#	rpn-add-num-base.csv rpn-add-convert.csv empty.txt 0
#	sqlite_str_to_int)

# no binary formatting; original encoding is lost
#test("csv-sqlite 'select num, str, cast(str as int) as str_to_int,\
#			 printf(\"%d\", num) as num_to_string,\
#			 printf(\"0b%b\", num) as num_to_string2,\
#			 printf(\"0%o\", num) as num_to_string8,\
#			 printf(\"%d\", num) as num_to_string10,\
#			 printf(\"0x%x\", num) as num_to_string16\
#		    from input'"
#	rpn-add-num-base.csv rpn-add-convert.csv empty.txt 0
#	sqlite_str_to_int)

endif(SQLITE3_FOUND)

################################ SUBSTRING #####################################

test("csv-substring -f name -n substr2 -p 2"
	3-columns-3-rows.csv substring-p2.csv empty.txt 0
	substring_p2)

test("csv-substring -f name -n last_char -p -1 -l 1"
	3-columns-3-rows.csv substring-p-1.csv empty.txt 0
	substring_p_1)

################################### SUM ########################################

test("csv-sum -f id" id-column-3-rows.csv sum-id.csv empty.txt 0
	sum)

test("csv-sum -f col1,col2,col3" 3-numeric-columns-4-rows.csv sum-3-columns.csv empty.txt 0
	sum-3-columns)

test("csv-sum -f col3,col1" 3-numeric-columns-4-rows.csv sum-2-columns.csv empty.txt 0
	sum-2-columns)

################################### TAC ########################################

test("csv-tac ${DATA_DIR}/one-column-one-row.csv" empty.csv one-column-one-row.csv empty.txt 0
	tac_one_file)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv" empty.csv cat-2-files.csv empty.txt 0
	tac_two_files)

test("csv-tac ${DATA_DIR}/3-different-order-columns-3-rows.csv ${DATA_DIR}/3-columns-2-last-rows.csv" empty.csv tac-2-different-files.csv empty.txt 0
	tac_two_different_files)

test("csv-tac ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/2-columns-3-rows.csv" empty.csv empty.csv cat-2-files-different-num-columns.txt 2
	tac_two_files_different_columns)

################################### TAIL #######################################

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
	tail_--lines=2_but_there_is_only_1_line_in_input)

test("csv-tail -n 2" one-column-one-row.csv one-column-one-row.csv empty.txt 0
	tail_-n_2_but_there_is_only_1_line_in_input)

test("csv-tail --lines=2" 3-columns-3-rows.csv 3-columns-2-last-rows.csv empty.txt 0
	tail_--lines=2_but_there_are_3_lines_in_input)

test("csv-tail -n 2" 3-columns-3-rows.csv 3-columns-2-last-rows.csv empty.txt 0
	tail_-n_2_but_there_are_3_lines_in_input)

test("csv-tail --lines=3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	tail_--lines=3_and_there_are_3_lines_in_input)

test("csv-tail -n 3" 3-columns-3-rows.csv 3-columns-3-rows.csv empty.txt 0
	tail_-n_3_and_there_are_3_lines_in_input)

################################### UNIQ #######################################

test("csv-uniq -f col1" uniq-input1.csv uniq-output1.csv empty.txt 0 uniq-1)

test("csv-uniq -f col3,col2,col1" uniq-input2.csv uniq-output2.csv empty.txt 0 uniq-2)

test("csv-uniq -f col3" uniq-input2.csv uniq-output3.csv empty.txt 0 uniq-3)

################################### USERS ######################################

test("csv-users | csv-grep -f name -x -F root | csv-count -c --no-header" empty.txt 9.txt empty.txt 0
	users)
