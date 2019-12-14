#
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

if(SQLITE3_FOUND)

test("csv-sqlite 'select * from input'"
	data/3-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	sqlite_select_all_from_input)

test("csv-sqlite 'select * from input' -s"
	data/3-columns-3-rows.csv data/3-columns-3-rows.txt data/empty.txt 0
	sqlite_select_all_from_input_-s)

test("csv-sqlite 'select * from input where id == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id==2)

test("csv-sqlite 'select * from input where id != 2'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id!=2)

test("csv-sqlite 'select * from input where id > 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id>2)

test("csv-sqlite 'select * from input where id >= 3'"
	data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id>=3)

test("csv-sqlite 'select * from input where id < 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id<2)

test("csv-sqlite 'select * from input where id <= 1'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id<=1)

test("csv-sqlite 'select * from input where id == 1 or id == 3'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id==1_or_id==3)

test("csv-sqlite 'select * from input where not id == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_not_id==2)

# sqlite doesn't support xor
#test("csv-sqlite 'select * from input where id == 1 xor id == 3'"
#	data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
#	sqlite_select_all_from_input_where_id==1_xor_id==3)

test("csv-sqlite 'select * from input where id + 1 == 3'"
	data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id+1==3)

test("csv-sqlite 'select * from input where id - 1 == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id-1==2)

test("csv-sqlite 'select * from input where id * 2 == 2'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id*2==2)

test("csv-sqlite 'select * from input where id / 2 == 1'"
	data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id_div_2==1)

test("csv-sqlite 'select * from input where length(name) == 11'"
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sqlite_select_all_from_input_where_length_name==11)

test("csv-sqlite \"select * from input where substr(name, 3, 1) == 'r'\""
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sqlite_select_all_from_input_where_substr_name_3_1==r)

test("csv-sqlite \"select * from input where name like '%th%'\""
	data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	sqlite_select_all_from_input_where_name_like_th)

test("csv-sqlite \"select * from input where name || '.suffix' == 'lorem ipsum.suffix'\""
	data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	sqlite_select_all_from_input_where_name||.suffix==loremipsum.suffix)

# no cast needed
test("csv-sqlite \"select * from input where id == '2'\""
	data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	sqlite_select_all_from_input_where_id==string2)


test("csv-sqlite 'select num, num2, num3, num * 2 as num_mul_2 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.csv data/empty.txt 0
	sqlite_num_mul)

test("csv-sqlite 'select num, num2, num3, num + num2 as num_add_num2 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sum.csv data/empty.txt 0
	sqlite_num_sum)

test("csv-sqlite 'select num, num2, num3, num2 / num3 as num2_div_num3, num2 / (num + 1) as num2_div_num_add_1 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div.csv data/empty.txt 0
	sqlite_num_div)

# 1/0 is NULL in sqlite
#test("csv-sqlite 'select num, num2, num3, num2 / num as num2_div_num from input'"
#	data/rpn-add-num-dec.csv data/rpn-add-num-div0.csv data/rpn-add-num-div0.txt 2
#	sqlite_num_div0)

test("csv-sqlite 'select num, num2, num3, num2 - num as num2_sub_num from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sub.csv data/empty.txt 0
	sqlite_num_sub)

test("csv-sqlite 'select num, num2, num3, num2 % num3 as num2_mod_num3, num2 % (num + 1) as num2_mod_num_add_1 from input'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mod.csv data/empty.txt 0
	sqlite_num_mod)

# original encoding is lost
#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 & num3) as num2_bit_and_num3 from input'"
#	data/rpn-add-num-hex.csv data/rpn-add-num-bit-and.csv data/empty.txt 0
#	sqlite_num_bit_and)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 | num) as num2_bit_or_num from input'"
#	data/rpn-add-num-hex.csv data/rpn-add-num-bit-or.csv data/empty.txt 0
#	sqlite_num_bit_or)

# + no support for bit xor
#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 ^ num) as num2_bit_xor_num from input'"
#	data/rpn-add-num-hex.csv data/rpn-add-num-bit-xor.csv data/empty.txt 0
#	sqlite_num_bit_xor)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", ~num2) as bit_neg_num2 from input'"
#	data/rpn-add-num-hex.csv data/rpn-add-num-bit-neg.csv data/empty.txt 0
#	sqlite_num_bit_neg)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 << num) as num2_lshift_num from input'"
#	data/rpn-add-num-hex.csv data/rpn-add-num-bit-lshift.csv data/empty.txt 0
#	sqlite_num_bit_lshift)

#test("csv-sqlite 'select printf(\"0x%x\", num) as num, printf(\"0x%x\", num2) as num2, printf(\"0x%x\", num3) as num3, printf(\"0x%x\", num2 >> num) as num2_rshift_num from input'"
#	data/rpn-add-num-hex.csv data/rpn-add-num-bit-rshift.csv data/empty.txt 0
#	sqlite_num_bit_rshift)


# sqlite doesn't support gt, lt, ge, le, eq, ne
test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 > num3 as num2_gt_num3, num2 > num2_dup as num2_gt_num2_dup, num2 > num2_mul_2 as num2_gt_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-gt.csv data/empty.txt 0
	sqlite_num_>)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 >= num3 as num2_ge_num3, num2 >= num2_dup as num2_ge_num2_dup, num2 >= num2_mul_2 as num2_ge_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-ge.csv data/empty.txt 0
	sqlite_num_>=)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 < num3 as num2_lt_num3, num2 < num2_dup as num2_lt_num2_dup, num2 < num2_mul_2 as num2_lt_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-lt.csv data/empty.txt 0
	sqlite_num_<)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 <= num3 as num2_le_num3, num2 <= num2_dup as num2_le_num2_dup, num2 <= num2_mul_2 as num2_le_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-le.csv data/empty.txt 0
	sqlite_num_<=)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 == num2_dup as num2_eq_num2_dup, num2 == num2_mul_2 as num2_eq_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	sqlite_num_==)

test("csv-sqlite 'select num, num2, num3, num2_mul_2, num2_dup, num2 != num2_dup as num2_ne_num2_dup, num2 != num2_mul_2 as num2_ne_num2_mul_2 from input'"
	data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	sqlite_num_!=)

test("csv-sqlite 'select num1, num2, num1 or num2 as num1_or_num2 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-or.csv data/empty.txt 0
	sqlite_num_logic_or)

test("csv-sqlite 'select num1, num2, num1 and num2 as num1_and_num2 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-and.csv data/empty.txt 0
	sqlite_num_logic_and)

# sqlite doesn't support logic xor operator
#test("csv-sqlite 'select num1, num2, num1 xor num2 as num1_xor_num2 from input'"
#	data/rpn-add-num-logic.csv data/rpn-add-num-logic-xor.csv data/empty.txt 0
#	sqlite_num_logic_xor)

test("csv-sqlite 'select num1, num2, not num1 as not_num1 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-not.csv data/empty.txt 0
	sqlite_num_logic_not)

# sqlite doesn't support if operator, but supports case, XXX rename if to case in rpn?
#test("csv-sqlite 'select num1, num2, num1 ? \"one\" : \"zero\" as desc_num1 from input'"
#	data/rpn-add-num-logic.csv data/rpn-add-num-logic-if.csv data/empty.txt 0
#	sqlite_num_logic_if)

test("csv-sqlite 'select num1, num2, case when num1 then \"one\" else \"zero\" end as desc_num1 from input'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-if.csv data/empty.txt 0
	sqlite_num_logic_if)

test("csv-sqlite 'select str1, str2, substr(str1, 2, 100) as substr2, substr(str1, -1, 1) as last_char from input'"
	data/rpn-add-str.csv data/rpn-add-str-substr.csv data/empty.txt 0
	sqlite_str_substr)

# XXX rename strlen to length in rpn?
test("csv-sqlite 'select str1, str2, length(str1) as len1, length(str2) as len2 from input'"
	data/rpn-add-str.csv data/rpn-add-str-strlen.csv data/empty.txt 0
	sqlite_str_length)

test("csv-sqlite 'select str1, str2, str1||str2 as concat1, str1||\" - \"||str2 as concat2 from input'"
	data/rpn-add-str.csv data/rpn-add-str-concat.csv data/empty.txt 0
	sqlite_str_concat)

test("csv-sqlite 'select str1, str2, str1 like \"%12\" as \"str1_like_%12\", str1 like \"%1%\" as \"str1_like_%1%\" from input'"
	data/rpn-add-str.csv data/rpn-add-str-like.csv data/empty.txt 0
	sqlite_str_like)

# no such functions in sqlite
#test("csv-sqlite 'select num, str, toint(str) as str_to_int,\
#			 tostring(num) as num_to_string,\
#			 tostring_base2(num) as num_to_string2,\
#			 tostring_base8(num) as num_to_string8,\
#			 tostring_base10(num) as num_to_string10,\
#			 tostring_base16(num) as num_to_string16\
#		    from input'"
#	data/rpn-add-num-base.csv data/rpn-add-convert.csv data/empty.txt 0
#	sqlite_str_to_int)

# no binary formatting; original encoding is lost
#test("csv-sqlite 'select num, str, cast(str as int) as str_to_int,\
#			 printf(\"%d\", num) as num_to_string,\
#			 printf(\"0b%b\", num) as num_to_string2,\
#			 printf(\"0%o\", num) as num_to_string8,\
#			 printf(\"%d\", num) as num_to_string10,\
#			 printf(\"0x%x\", num) as num_to_string16\
#		    from input'"
#	data/rpn-add-num-base.csv data/rpn-add-convert.csv data/empty.txt 0
#	sqlite_str_to_int)

test("csv-sqlite -l users 'select * from users' | csv-count -c -R"
	data/one-column-one-row.csv sqlite/users-columns.csv data/empty.txt 0
	sqlite_users_count_columns)

test("csv-sqlite -l groups 'select * from groups' | csv-count -c -R"
	data/one-column-one-row.csv sqlite/groups-columns.csv data/empty.txt 0
	sqlite_groups_count_columns)

test("csv-sqlite -l group_members 'select * from group_members' | csv-count -c -R"
	data/one-column-one-row.csv sqlite/group_members-columns.csv data/empty.txt 0
	sqlite_group_members_count_columns)

test("csv-sqlite --help" data/empty.csv sqlite/help.txt data/empty.txt 2
	sqlite_help)

test("csv-sqlite --version" data/empty.csv data/git-version.txt data/empty.txt 0
	sqlite_version)

endif(SQLITE3_FOUND)
