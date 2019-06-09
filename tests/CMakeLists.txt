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

################################### CAT ########################################

test("csv-cat ${DATA_DIR}/one-column-one-row.csv" empty.csv one-column-one-row.csv empty.txt 0
	cat_one_file)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/one-column-one-row.csv" empty.csv cat-2-files.csv empty.txt 0
	cat_two_files)

test("csv-cat ${DATA_DIR}/3-different-order-columns-3-rows.csv ${DATA_DIR}/3-columns-2-last-rows.csv" empty.csv cat-2-different-files.csv empty.txt 0
	cat_two_different_files)

test("csv-cat ${DATA_DIR}/one-column-one-row.csv ${DATA_DIR}/2-columns-3-rows.csv" empty.csv empty.csv cat-2-files-different-num-columns.txt 2
	cat_two_files_different_columns)

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

################################# EXEC-ADD #####################################

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

test("csv-groups -f name | csv-grep -f name -F root | csv-count -c --no-header" empty.txt 1.txt empty.txt 0
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

#################################### LS ########################################

################################# RPN-ADD ######################################

test("csv-rpn-add -f 'num*2' -e '%num 2 *'"
	rpn-add-num-dec.csv rpn-add-num-mul.csv empty.txt 0
	rpn-add_num_mul)

test("csv-rpn-add -f 'num+num2' -e '%num %num2 +'"
	rpn-add-num-dec.csv rpn-add-num-sum.csv empty.txt 0
	rpn-add_num_sum)

test("csv-rpn-add -f 'num2/num3' -e '%num2 %num3 /'\
		  -f 'num2/num+1' -e '%num2 %num 1 + /'"
	rpn-add-num-dec.csv rpn-add-num-div.csv empty.txt 0
	rpn-add_num_div)

test("csv-rpn-add -f 'num2/num' -e '%num2 %num /'"
	rpn-add-num-dec.csv rpn-add-num-div0.csv rpn-add-num-div0.txt 2
	rpn-add_num_div0)

test("csv-rpn-add -f 'num2-num' -e '%num2 %num -'"
	rpn-add-num-dec.csv rpn-add-num-sub.csv empty.txt 0
	rpn-add_num_sub)

test("csv-rpn-add -f 'num2%num3' -e '%num2 %num3 %'\
		  -f 'num2%num+1' -e '%num2 %num 1 + %'"
	rpn-add-num-dec.csv rpn-add-num-mod.csv empty.txt 0
	rpn-add_num_mod)


test("csv-rpn-add -f 'num2&num3' -e '%num2 %num3 & tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-and.csv empty.txt 0
	rpn-add_num_bit_and)

test("csv-rpn-add -f 'num2|num' -e '%num2 %num | tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-or.csv empty.txt 0
	rpn-add_num_bit_or)

test("csv-rpn-add -f 'num2^num' -e '%num2 %num ^ tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-xor.csv empty.txt 0
	rpn-add_num_bit_xor)

test("csv-rpn-add -f '~num2' -e '%num2 ~ tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-neg.csv empty.txt 0
	rpn-add_num_bit_neg)

test("csv-rpn-add -f 'num2<<num' -e '%num2 %num << tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-lshift.csv empty.txt 0
	rpn-add_num_bit_lshift)

test("csv-rpn-add -f 'num2>>num' -e '%num2 %num >> tostring_base16'"
	rpn-add-num-hex.csv rpn-add-num-bit-rshift.csv empty.txt 0
	rpn-add_num_bit_rshift)


test("csv-rpn-add -f 'num2>num3' -e '%num2 %num3 >'  -f 'num2>num2_dup' -e '%num2 %num2_dup >'  -f 'num2>num2*2' -e '%num %num2*2 >'"  rpn-add-num-compare.csv rpn-add-num-gt.csv empty.txt 0
	rpn-add_num_>)
test("csv-rpn-add -f 'num2>num3' -e '%num2 %num3 gt' -f 'num2>num2_dup' -e '%num2 %num2_dup gt' -f 'num2>num2*2' -e '%num %num2*2 gt'" rpn-add-num-compare.csv rpn-add-num-gt.csv empty.txt 0
	rpn-add_num_gt)

test("csv-rpn-add -f 'num2>=num3' -e '%num2 %num3 >=' -f 'num2>=num2_dup' -e '%num2 %num2_dup >=' -f 'num2>=num2*2' -e '%num %num2*2 >='" rpn-add-num-compare.csv rpn-add-num-ge.csv empty.txt 0
	rpn-add_num_>=)
test("csv-rpn-add -f 'num2>=num3' -e '%num2 %num3 ge' -f 'num2>=num2_dup' -e '%num2 %num2_dup ge' -f 'num2>=num2*2' -e '%num %num2*2 ge'" rpn-add-num-compare.csv rpn-add-num-ge.csv empty.txt 0
	rpn-add_num_ge)

test("csv-rpn-add -f 'num2<num3' -e '%num2 %num3 <'  -f 'num2<num2_dup' -e '%num2 %num2_dup <'  -f 'num2<num2*2' -e '%num %num2*2 <'"  rpn-add-num-compare.csv rpn-add-num-lt.csv empty.txt 0
	rpn-add_num_<)
test("csv-rpn-add -f 'num2<num3' -e '%num2 %num3 lt' -f 'num2<num2_dup' -e '%num2 %num2_dup lt' -f 'num2<num2*2' -e '%num %num2*2 lt'" rpn-add-num-compare.csv rpn-add-num-lt.csv empty.txt 0
	rpn-add_num_lt)

test("csv-rpn-add -f 'num2<=num3' -e '%num2 %num3 <=' -f 'num2<=num2_dup' -e '%num2 %num2_dup <=' -f 'num2<=num2*2' -e '%num %num2*2 <='" rpn-add-num-compare.csv rpn-add-num-le.csv empty.txt 0
	rpn-add_num_<=)
test("csv-rpn-add -f 'num2<=num3' -e '%num2 %num3 le' -f 'num2<=num2_dup' -e '%num2 %num2_dup le' -f 'num2<=num2*2' -e '%num %num2*2 le'" rpn-add-num-compare.csv rpn-add-num-le.csv empty.txt 0
	rpn-add_num_le)

test("csv-rpn-add -f 'num2==num2_dup' -e '%num2 %num2_dup ==' -f 'num2==num2*2' -e '%num2 %num2*2 =='" rpn-add-num-compare.csv rpn-add-num-eq.csv empty.txt 0
	rpn-add_num_==)
test("csv-rpn-add -f 'num2==num2_dup' -e '%num2 %num2_dup eq' -f 'num2==num2*2' -e '%num2 %num2*2 eq'" rpn-add-num-compare.csv rpn-add-num-eq.csv empty.txt 0
	rpn-add_num_eq)

test("csv-rpn-add -f 'num2!=num2_dup' -e '%num2 %num2_dup !=' -f 'num2!=num2*2' -e '%num2 %num2*2 !='" rpn-add-num-compare.csv rpn-add-num-ne.csv empty.txt 0
	rpn-add_num_!=)
test("csv-rpn-add -f 'num2!=num2_dup' -e '%num2 %num2_dup ne' -f 'num2!=num2*2' -e '%num2 %num2*2 ne'" rpn-add-num-compare.csv rpn-add-num-ne.csv empty.txt 0
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

################################## SQLITE ######################################

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

################################### USERS ######################################

test("csv-users -f name | csv-grep -f name -F root | csv-count -c --no-header" empty.txt 1.txt empty.txt 0
	users)
