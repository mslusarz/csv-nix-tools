#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

if (BISON_FOUND AND FLEX_FOUND)

test("csv-sql 'select * from input'"
	data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	sql_select_all_from_input)

test("csv-sql 'select * from input' -s"
	data/3-columns-3-rows.csv data/3-columns-3-rows.txt data/empty.txt 0
	sql_select_all_from_input_-s)

test("csv-sql 'select * from input where id == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	sql_select_all_from_input_where_id==2)

test("csv-sql 'select * from input where id != 2'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	sql_select_all_from_input_where_id!=2)

test("csv-sql 'select * from input where id > 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	sql_select_all_from_input_where_id>2)

test("csv-sql 'select * from input where id >= 3'"
	data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	sql_select_all_from_input_where_id>=3)

test("csv-sql 'select * from input where id < 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sql_select_all_from_input_where_id<2)

test("csv-sql 'select * from input where id <= 1'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sql_select_all_from_input_where_id<=1)

test("csv-sql 'select * from input where id == 1 or id == 3'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	sql_select_all_from_input_where_id==1_or_id==3)

test("csv-sql 'select * from input where not id == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	sql_select_all_from_input_where_not_id==2)

# sql supports xor
test("csv-sql 'select * from input where id == 1 xor id == 3'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	sql_select_all_from_input_where_id==1_xor_id==3)

test("csv-sql 'select * from input where id + 1 == 3'"
	data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	sql_select_all_from_input_where_id+1==3)

test("csv-sql 'select * from input where id - 1 == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	sql_select_all_from_input_where_id-1==2)

test("csv-sql 'select * from input where id * 2 == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sql_select_all_from_input_where_id*2==2)

test("csv-sql 'select * from input where id / 2 == 1'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	sql_select_all_from_input_where_id_div_2==1)

test("csv-sql 'select * from input where length(name) == 11'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sql_select_all_from_input_where_length_name==11)

test("csv-sql \"select * from input where substr(name, 3, 1) == 'r'\""
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sql_select_all_from_input_where_substr_name-3-1==r)

test("csv-sql \"select * from input where name like '%th%'\""
	data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	sql_select_all_from_input_where_name_like_th)

test("csv-sql \"select * from input where name || '.suffix' == 'lorem ipsum.suffix'\""
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sql_select_all_from_input_where_name||.suffix==loremipsum.suffix)

# cast needed
test("csv-sql \"select * from input where tostring(id) == '2'\""
	data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	sql_select_all_from_input_where_tostring_id==string2)


test("csv-sql 'select num, num2, num3, num * 2 as num_mul_2 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.csv data/empty.txt 0
	sql_num_mul)

test("csv-sql 'select num, num2, num3, num + num2 as num_add_num2 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sum.csv data/empty.txt 0
	sql_num_sum)

test("csv-sql 'select num, num2, num3, num2 / num3 as num2_div_num3, num2 / (num + 1) as num2_div_num_add_1 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div.csv data/empty.txt 0
	sql_num_div)

# 1/0 is invalid in sql
test("csv-sql 'select num, num2, num3, num2 / num as num2_div_num from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div0.csv data/rpn-add-num-div0.txt 2
	sql_num_div0)

test("csv-sql 'select num, num2, num3, num2 - num as num2_sub_num from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sub.csv data/empty.txt 0
	sql_num_sub)

test("csv-sql 'select num, num2, num3, num2 % num3 as num2_mod_num3, num2 % (num + 1) as num2_mod_num_add_1 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mod.csv data/empty.txt 0
	sql_num_mod)


test("csv-sql 'select tostring(num2 & num3, 16) as num2_bit_and_num3 from input'"
	data/rpn-add-num-hex.csv data/sql-num-bit-and.csv data/empty.txt 0
	sql_num_bit_and)

test("csv-sql 'select tostring(num2 | num, 16) as num2_bit_or_num from input'"
	data/rpn-add-num-hex.csv data/sql-num-bit-or.csv data/empty.txt 0
	sql_num_bit_or)

test("csv-sql 'select tostring(num2 ^ num, 16) as num2_bit_xor_num from input'"
	data/rpn-add-num-hex.csv data/sql-num-bit-xor.csv data/empty.txt 0
	sql_num_bit_xor)

test("csv-sql 'select tostring(~num2, 16) as bit_neg_num2 from input'"
	data/rpn-add-num-hex.csv data/sql-num-bit-neg.csv data/empty.txt 0
	sql_num_bit_neg)

test("csv-sql 'select tostring(num2 << num, 16) as num2_lshift_num from input'"
	data/rpn-add-num-hex.csv data/sql-num-bit-lshift.csv data/empty.txt 0
	sql_num_bit_lshift)

test("csv-sql 'select tostring(num2 >> num, 16) as num2_rshift_num from input'"
	data/rpn-add-num-hex.csv data/sql-num-bit-rshift.csv data/empty.txt 0
	sql_num_bit_rshift)


# sql doesn't support gt, lt, ge, le, eq, ne
test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 > num3 as num2_gt_num3, num2 > num2_dup as num2_gt_num2_dup, num2 > num2_mul_2 as num2_gt_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-gt.csv data/empty.txt 0
	sql_num_>)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 >= num3 as num2_ge_num3, num2 >= num2_dup as num2_ge_num2_dup, num2 >= num2_mul_2 as num2_ge_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-ge.csv data/empty.txt 0
	sql_num_>=)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 < num3 as num2_lt_num3, num2 < num2_dup as num2_lt_num2_dup, num2 < num2_mul_2 as num2_lt_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-lt.csv data/empty.txt 0
	sql_num_<)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 <= num3 as num2_le_num3, num2 <= num2_dup as num2_le_num2_dup, num2 <= num2_mul_2 as num2_le_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-le.csv data/empty.txt 0
	sql_num_<=)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 == num2_dup as num2_eq_num2_dup, num2 == num2_mul_2 as num2_eq_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	sql_num_==)

test("csv-sql 'select num, num2, num3, num2_mul_2, num2_dup, num2 != num2_dup as num2_ne_num2_dup, num2 != num2_mul_2 as num2_ne_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	sql_num_!=)

test("csv-sql 'select num1, num2, num1 or num2 as num1_or_num2 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-or.csv data/empty.txt 0
	sql_num_logic_or)

test("csv-sql 'select num1, num2, num1 and num2 as num1_and_num2 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-and.csv data/empty.txt 0
	sql_num_logic_and)

# sql supports logic xor operator
test("csv-sql 'select num1, num2, num1 xor num2 as num1_xor_num2 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-xor.csv data/empty.txt 0
	sql_num_logic_xor)

test("csv-sql 'select num1, num2, not num1 as not_num1 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-not.csv data/empty.txt 0
	sql_num_logic_not)

# sql supports if operator, XXX rename if to case in rpn?
test("csv-sql 'select num1, num2, if(num1, \"one\", \"zero\") as desc_num1 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-if.csv data/empty.txt 0
	sql_num_logic_if)

#test("csv-sql 'select num1, num2, case when num1 then \"one\" else \"zero\" end as desc_num1 from input'"
#	data/rpn-add-num-logic.csv data/rpn-add-num-logic-if.csv data/empty.txt 0
#	sql_num_logic_if)

test("csv-sql 'select str1, str2, substr(str1, 2, 100) as substr2, substr(str1, -1, 1) as last_char from input'"
	data/rpn-add-str.csv data/rpn-add-str-substr.csv data/empty.txt 0
	sql_str_substr)

# XXX rename strlen to length in rpn?
test("csv-sql 'select str1, str2, length(str1) as len1, length(str2) as len2 from input'"
	data/rpn-add-str.csv data/rpn-add-str-strlen.csv data/empty.txt 0
	sql_str_length)

test("csv-sql 'select str1, str2, str1||str2 as concat1, str1||\" - \"||str2 as concat2 from input'"
	data/rpn-add-str.csv data/rpn-add-str-concat.csv data/empty.txt 0
	sql_str_concat)

test("csv-sql 'select str1, str2, str1 like \"%12\" as \"str1_like_%12\", str1 like \"%1%\" as \"str1_like_%1%\" from input'"
	data/rpn-add-str.csv data/rpn-add-str-like.csv data/empty.txt 0
	sql_str_like)

# toint doesn't exists in sql, XXX implement CAST?
#test("csv-sql 'select num, str, toint(str) as str_to_int,\
#			 tostring(num) as num_to_string,\
#			 tostring(num, 2) as num_to_string2,\
#			 tostring(num, 8) as num_to_string8,\
#			 tostring(num, 10) as num_to_string10,\
#			 tostring(num, 16) as num_to_string16\
#		    from input'"
#	data/rpn-add-num-base.csv data/rpn-add-convert.csv data/empty.txt 0
#	sql_str_to_int)

test("csv-sql --help" data/empty.csv sql/help.txt data/empty.txt 2
	sql_help)

test("csv-sql --version" data/empty.csv data/git-version.txt data/empty.txt 0
	sql_version)

endif(BISON_FOUND AND FLEX_FOUND)
