#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

if (BUILD_SQL)

test("csv-add-sql -n 'num_mul_2' -e 'num * 2'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.csv data/empty.txt 0
	add-sql_num_mul)

test("csv-add-sql -e 'num * 2 as num_mul_2'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.csv data/empty.txt 0
	add-sql_num_mul_as)

test("csv-add-sql -n 'num_mul_2' -e 'num * 2' -s"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.txt data/empty.txt 0
	add-sql_num_mul_-s)

test("csv-add-sql -n 'num_add_num2' -e 'num + num2'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sum.csv data/empty.txt 0
	add-sql_num_sum)

test("csv-add-sql -n 'num2_div_num3' -e 'num2 / num3'\
		  -n 'num2_div_num_add_1' -e 'num2 / (num + 1)'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div.csv data/empty.txt 0
	add-sql_num_div)

test("csv-add-sql -e 'num2 / num3 as num2_div_num3, num2 / (num + 1) as num2_div_num_add_1'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div.csv data/empty.txt 0
	add-sql_num_div_as)

test("csv-add-sql -n 'num2_div_num' -e 'num2 / num'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div0.csv data/rpn-add-num-div0.txt 2
	add-sql_num_div0)

test("csv-add-sql -n 'num2_sub_num' -e 'num2 - num'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sub.csv data/empty.txt 0
	add-sql_num_sub)

test("csv-add-sql -n 'num2_mod_num3' -e 'num2 % num3'\
		  -n 'num2_mod_num_add_1' -e 'num2 % (num + 1)'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mod.csv data/empty.txt 0
	add-sql_num_mod)


test("csv-add-sql -n 'num2_bit_and_num3' -e 'int2strb(num2 & num3, 16)'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-and.csv data/empty.txt 0
	add-sql_num_bit_and)

test("csv-add-sql -n 'num2_bit_or_num' -e 'int2strb(num2 | num, 16)'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-or.csv data/empty.txt 0
	add-sql_num_bit_or)

test("csv-add-sql -n 'num2_bit_xor_num' -e 'int2strb(num2 ^ num, 16)'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-xor.csv data/empty.txt 0
	add-sql_num_bit_xor)

test("csv-add-sql -n 'bit_neg_num2' -e 'int2strb(~num2, 16)'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-neg.csv data/empty.txt 0
	add-sql_num_bit_neg)

test("csv-add-sql -n 'num2_lshift_num' -e 'int2strb(num2 << num, 16)'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-lshift.csv data/empty.txt 0
	add-sql_num_bit_lshift)

test("csv-add-sql -n 'num2_rshift_num' -e 'int2strb(num2 >> num, 16)'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-rshift.csv data/empty.txt 0
	add-sql_num_bit_rshift)


test("csv-add-sql -n num2_gt_num3 -e 'num2 > num3'  -n num2_gt_num2_dup -e 'num2 > num2_dup'  -n num2_gt_num2_mul_2 -e 'num > num2_mul_2'"  data/rpn-add-num-compare.csv data/rpn-add-num-gt.csv data/empty.txt 0
	add-sql_num_>)

test("csv-add-sql -n num2_ge_num3 -e 'num2 >= num3' -n num2_ge_num2_dup -e 'num2 >= num2_dup' -n num2_ge_num2_mul_2 -e 'num >= num2_mul_2'" data/rpn-add-num-compare.csv data/rpn-add-num-ge.csv data/empty.txt 0
	add-sql_num_>=)

test("csv-add-sql -n num2_lt_num3 -e 'num2 < num3'  -n num2_lt_num2_dup -e 'num2 < num2_dup'  -n num2_lt_num2_mul_2 -e 'num < num2_mul_2'"  data/rpn-add-num-compare.csv data/rpn-add-num-lt.csv data/empty.txt 0
	add-sql_num_<)

test("csv-add-sql -n num2_le_num3 -e 'num2 <= num3' -n num2_le_num2_dup -e 'num2 <= num2_dup' -n num2_le_num2_mul_2 -e 'num <= num2_mul_2'" data/rpn-add-num-compare.csv data/rpn-add-num-le.csv data/empty.txt 0
	add-sql_num_<=)

test("csv-add-sql -n num2_eq_num2_dup -e 'num2 == num2_dup' -n num2_eq_num2_mul_2 -e 'num2 == num2_mul_2'" data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	add-sql_num_==)

test("csv-add-sql -n num2_eq_num2_dup -e 'num2 = num2_dup' -n num2_eq_num2_mul_2 -e 'num2 = num2_mul_2'" data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	add-sql_num_=)

test("csv-add-sql -n num2_ne_num2_dup -e 'num2 != num2_dup' -n num2_ne_num2_mul_2 -e 'num2 != num2_mul_2'" data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	add-sql_num_!=)

test("csv-add-sql -n num2_ne_num2_dup -e 'num2 <> num2_dup' -n num2_ne_num2_mul_2 -e 'num2 <> num2_mul_2'" data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	add-sql_num_<>)


test("csv-add-sql -n num1_or_num2 -e 'num1 or num2'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-or.csv data/empty.txt 0
	add-sql_num_logic_or)

test("csv-add-sql -n num1_and_num2 -e 'num1 and num2'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-and.csv data/empty.txt 0
	add-sql_num_logic_and)

test("csv-add-sql -n num1_xor_num2 -e 'num1 xor num2'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-xor.csv data/empty.txt 0
	add-sql_num_logic_xor)

test("csv-add-sql -n not_num1 -e 'not num1'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-not.csv data/empty.txt 0
	add-sql_num_logic_not)

test("csv-add-sql -n desc_num1 -e \"if (num1, 'one', 'zero')\""
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-if.csv data/empty.txt 0
	add-sql_num_logic_if)


test("csv-add-sql -n substr2 -e 'substr(str1, 2, 1000)'\
		  -n last_char -e 'substr(str1, -1, 1)'"
	data/rpn-add-str.csv data/rpn-add-str-substr.csv data/empty.txt 0
	add-sql_str_substr)

test("csv-add-sql -n len1 -e 'length(str1)'\
		  -n len2 -e 'length(str2)'"
	data/rpn-add-str.csv data/rpn-add-str-strlen.csv data/empty.txt 0
	add-sql_str_length)

test("csv-add-sql -n concat1 -e 'str1 || str2'\
		  -n concat2 -e \"str1 || ' - ' || str2\""
	data/rpn-add-str.csv data/rpn-add-str-concat.csv data/empty.txt 0
	add-sql_str_concat)

test("csv-add-sql -n str1_like_%12 -e \"str1 like '%12'\"\
		  -n str1_like_%1% -e \"str1 like '%1%'\""
	data/rpn-add-str.csv data/rpn-add-str-like.csv data/empty.txt 0
	add-sql_str_like)


test("csv-add-sql -n str_to_int -e 'toint(str)'\
		  -n num_to_string   -e 'tostring(num)'\
		  -n num_to_string2  -e 'int2strb(num, 2)'\
		  -n num_to_string8  -e 'int2strb(num, 8)'\
		  -n num_to_string10 -e 'int2strb(num, 10)'\
		  -n num_to_string16 -e 'int2strb(num, 16)'"
	data/rpn-add-num-base.csv data/rpn-add-convert-noparse.csv data/empty.txt 0
	add-sql_str_to_int_num_to_str)

test("csv-add-sql -T t1 -n name_len -e 'length(name)'" data/2-tables.csv add-sql/2-tables-add1.csv data/empty.txt 0
	add-sql_2tables-add1)

test("csv-add-sql -T t2 -n name_plus_id_squared -e 'name || tostring(id * id)'" data/2-tables.csv add-sql/2-tables-add2.csv data/empty.txt 0
	add-sql_2tables-add2)

test("csv-add-sql -n x -e \"replace(str, 'pattern', 'replacement', 1)\"" add-rpn/replace-input.csv add-rpn/replace-output1.csv data/empty.txt 0
	add-sql_replace1)

test("csv-add-sql -n x -e \"replace(str, 'pattern', 'replacement')\"" add-rpn/replace-input.csv add-rpn/replace-output1.csv data/empty.txt 0
	add-sql_replace1a)

test("csv-add-sql -n x -e \"replace(str, 'pattern', 'replacement', 0)\"" add-rpn/replace-input.csv add-rpn/replace-output2.csv data/empty.txt 0
	add-sql_replace2)

test("csv-add-sql -n x -e \"replace_ere(str, '([^e]*)e(.*)', '%1 XXX %2', 1)\"" add-rpn/replace-input.csv add-rpn/replace_re-output1.csv data/empty.txt 0
	add-sql_replace_ere1)

test("csv-add-sql -n x -e \"replace_ere(str, '([^e]*)e(.*)', '%1 XXX %2')\"" add-rpn/replace-input.csv add-rpn/replace_re-output1.csv data/empty.txt 0
	add-sql_replace_ere1a)

test("csv-add-sql -n x -e \"replace_ere(str, '([^e]*)e(.*)', '%1 XXX %2', 0)\"" add-rpn/replace-input.csv add-rpn/replace_re-output2.csv data/empty.txt 0
	add-sql_replace_ere2)

test("csv-add-sql -n x -e \"replace_bre(str, '\\([^e]*\\)e\\(.*\\)', '%1 XXX %2', 1)\"" add-rpn/replace-input.csv add-rpn/replace_re-output1.csv data/empty.txt 0
	add-sql_replace_bre1)

test("csv-add-sql -n x -e \"replace_bre(str, '\\([^e]*\\)e\\(.*\\)', '%1 XXX %2')\"" add-rpn/replace-input.csv add-rpn/replace_re-output1.csv data/empty.txt 0
	add-sql_replace_bre1a)

test("csv-add-sql -n x -e \"replace_bre(str, '\\([^e]*\\)e\\(.*\\)', '%1 XXX %2', 0)\"" add-rpn/replace-input.csv add-rpn/replace_re-output2.csv data/empty.txt 0
	add-sql_replace_bre2)

test("csv-add-sql -n 'num' -e \"next('a')\""
	data/3-columns-3-rows.csv data/rpn-add-next.csv data/empty.txt 0
	add-sql_next1)

test("csv-add-sql -n 'num' -e \"next()\""
	data/3-columns-3-rows.csv data/rpn-add-next.csv data/empty.txt 0
	add-sql_next2)

test("csv-add-sql -n col2tripled -e 'col2 * 3'"
	data/floats.csv add-sql/floats.csv data/empty.txt 0
	add-sql_floats)

test("csv-add-sql --help" data/empty.csv add-sql/help.txt data/empty.txt 2
	add-sql_help)

test("csv-add-sql --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-sql_version)

endif(BUILD_SQL)
