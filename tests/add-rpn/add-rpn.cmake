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

test("csv-add-rpn -n 'num_mul_2' -e '%num 2 *'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.csv data/empty.txt 0
	add-rpn_num_mul)

test("csv-add-rpn -n 'num_mul_2' -e '%num 2 *' -s"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.txt data/empty.txt 0
	add-rpn_num_mul_-s)

test("csv-add-rpn -n 'num_add_num2' -e '%num %num2 +'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sum.csv data/empty.txt 0
	add-rpn_num_sum)

test("csv-add-rpn -n 'num2_div_num3' -e '%num2 %num3 /'\
		  -n 'num2_div_num_add_1' -e '%num2 %num 1 + /'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div.csv data/empty.txt 0
	add-rpn_num_div)

test("csv-add-rpn -n 'num2_div_num' -e '%num2 %num /'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div0.csv data/rpn-add-num-div0.txt 2
	add-rpn_num_div0)

test("csv-add-rpn -n 'num2_sub_num' -e '%num2 %num -'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sub.csv data/empty.txt 0
	add-rpn_num_sub)

test("csv-add-rpn -n 'num2_mod_num3' -e '%num2 %num3 %'\
		  -n 'num2_mod_num_add_1' -e '%num2 %num 1 + %'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mod.csv data/empty.txt 0
	add-rpn_num_mod)


test("csv-add-rpn -n 'num2_bit_and_num3' -e '%num2 %num3 & 16 tostring'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-and.csv data/empty.txt 0
	add-rpn_num_bit_and)

test("csv-add-rpn -n 'num2_bit_or_num' -e '%num2 %num | 16 tostring'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-or.csv data/empty.txt 0
	add-rpn_num_bit_or)

test("csv-add-rpn -n 'num2_bit_xor_num' -e '%num2 %num ^ 16 tostring'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-xor.csv data/empty.txt 0
	add-rpn_num_bit_xor)

test("csv-add-rpn -n 'bit_neg_num2' -e '%num2 ~ 16 tostring'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-neg.csv data/empty.txt 0
	add-rpn_num_bit_neg)

test("csv-add-rpn -n 'num2_lshift_num' -e '%num2 %num << 16 tostring'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-lshift.csv data/empty.txt 0
	add-rpn_num_bit_lshift)

test("csv-add-rpn -n 'num2_rshift_num' -e '%num2 %num >> 16 tostring'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-rshift.csv data/empty.txt 0
	add-rpn_num_bit_rshift)


test("csv-add-rpn -n num2_gt_num3 -e '%num2 %num3 >'  -n num2_gt_num2_dup -e '%num2 %num2_dup >'  -n num2_gt_num2_mul_2 -e '%num %num2_mul_2 >'"  data/rpn-add-num-compare.csv data/rpn-add-num-gt.csv data/empty.txt 0
	add-rpn_num_>)
test("csv-add-rpn -n num2_gt_num3 -e '%num2 %num3 gt' -n num2_gt_num2_dup -e '%num2 %num2_dup gt' -n num2_gt_num2_mul_2 -e '%num %num2_mul_2 gt'" data/rpn-add-num-compare.csv data/rpn-add-num-gt.csv data/empty.txt 0
	add-rpn_num_gt)

test("csv-add-rpn -n num2_ge_num3 -e '%num2 %num3 >=' -n num2_ge_num2_dup -e '%num2 %num2_dup >=' -n num2_ge_num2_mul_2 -e '%num %num2_mul_2 >='" data/rpn-add-num-compare.csv data/rpn-add-num-ge.csv data/empty.txt 0
	add-rpn_num_>=)
test("csv-add-rpn -n num2_ge_num3 -e '%num2 %num3 ge' -n num2_ge_num2_dup -e '%num2 %num2_dup ge' -n num2_ge_num2_mul_2 -e '%num %num2_mul_2 ge'" data/rpn-add-num-compare.csv data/rpn-add-num-ge.csv data/empty.txt 0
	add-rpn_num_ge)

test("csv-add-rpn -n num2_lt_num3 -e '%num2 %num3 <'  -n num2_lt_num2_dup -e '%num2 %num2_dup <'  -n num2_lt_num2_mul_2 -e '%num %num2_mul_2 <'"  data/rpn-add-num-compare.csv data/rpn-add-num-lt.csv data/empty.txt 0
	add-rpn_num_<)
test("csv-add-rpn -n num2_lt_num3 -e '%num2 %num3 lt' -n num2_lt_num2_dup -e '%num2 %num2_dup lt' -n num2_lt_num2_mul_2 -e '%num %num2_mul_2 lt'" data/rpn-add-num-compare.csv data/rpn-add-num-lt.csv data/empty.txt 0
	add-rpn_num_lt)

test("csv-add-rpn -n num2_le_num3 -e '%num2 %num3 <=' -n num2_le_num2_dup -e '%num2 %num2_dup <=' -n num2_le_num2_mul_2 -e '%num %num2_mul_2 <='" data/rpn-add-num-compare.csv data/rpn-add-num-le.csv data/empty.txt 0
	add-rpn_num_<=)
test("csv-add-rpn -n num2_le_num3 -e '%num2 %num3 le' -n num2_le_num2_dup -e '%num2 %num2_dup le' -n num2_le_num2_mul_2 -e '%num %num2_mul_2 le'" data/rpn-add-num-compare.csv data/rpn-add-num-le.csv data/empty.txt 0
	add-rpn_num_le)

test("csv-add-rpn -n num2_eq_num2_dup -e '%num2 %num2_dup ==' -n num2_eq_num2_mul_2 -e '%num2 %num2_mul_2 =='" data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	add-rpn_num_==)
test("csv-add-rpn -n num2_eq_num2_dup -e '%num2 %num2_dup eq' -n num2_eq_num2_mul_2 -e '%num2 %num2_mul_2 eq'" data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	add-rpn_num_eq)

test("csv-add-rpn -n num2_ne_num2_dup -e '%num2 %num2_dup !=' -n num2_ne_num2_mul_2 -e '%num2 %num2_mul_2 !='" data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	add-rpn_num_!=)
test("csv-add-rpn -n num2_ne_num2_dup -e '%num2 %num2_dup ne' -n num2_ne_num2_mul_2 -e '%num2 %num2_mul_2 ne'" data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	add-rpn_num_ne)


test("csv-add-rpn -n num1_or_num2 -e '%num1 %num2 or'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-or.csv data/empty.txt 0
	add-rpn_num_logic_or)

test("csv-add-rpn -n num1_and_num2 -e '%num1 %num2 and'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-and.csv data/empty.txt 0
	add-rpn_num_logic_and)

test("csv-add-rpn -n num1_xor_num2 -e '%num1 %num2 xor'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-xor.csv data/empty.txt 0
	add-rpn_num_logic_xor)

test("csv-add-rpn -n not_num1 -e '%num1 not'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-not.csv data/empty.txt 0
	add-rpn_num_logic_not)

test("csv-add-rpn -n desc_num1 -e \"%num1 'one' 'zero' if\""
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-if.csv data/empty.txt 0
	add-rpn_num_logic_if)


test("csv-add-rpn -n substr2 -e '%str1 2 1000 substr'\
		  -n last_char -e '%str1 -1 1 substr'"
	data/rpn-add-str.csv data/rpn-add-str-substr.csv data/empty.txt 0
	add-rpn_str_substr)

test("csv-add-rpn -n len1 -e '%str1 strlen'\
		  -n len2 -e '%str2 strlen'"
	data/rpn-add-str.csv data/rpn-add-str-strlen.csv data/empty.txt 0
	add-rpn_str_strlen)

test("csv-add-rpn -n concat1 -e '%str1 %str2 concat'\
		  -n concat2 -e \"%str1 ' - ' %str2 concat concat\""
	data/rpn-add-str.csv data/rpn-add-str-concat.csv data/empty.txt 0
	add-rpn_str_concat)

test("csv-add-rpn -n str1_like_%12 -e \"%str1 '%12' like\"\
		  -n str1_like_%1% -e \"%str1 '%1%' like\""
	data/rpn-add-str.csv data/rpn-add-str-like.csv data/empty.txt 0
	add-rpn_str_like)


test("csv-add-rpn -n str_to_int -e '%str 10 toint'\
		  -n num_to_string   -e '%num 10 tostring'\
		  -n num_to_string2  -e '%num 2 tostring'\
		  -n num_to_string8  -e '%num 8 tostring'\
		  -n num_to_string10 -e '%num 10 tostring'\
		  -n num_to_string16 -e '%num 16 tostring'"
	data/rpn-add-num-base.csv data/rpn-add-convert-noparse.csv data/empty.txt 0
	add-rpn_str_to_int)

test("csv-add-rpn -T t1 -n concat -e '%name %id 10 tostring concat'" data/2-tables.csv add-rpn/2-tables-add1.csv data/empty.txt 0
	add-rpn_2tables-add1)

test("csv-add-rpn -T t2 -n concat -e '%name %id 10 tostring concat'" data/2-tables.csv add-rpn/2-tables-add2.csv data/empty.txt 0
	add-rpn_2tables-add2)

test("csv-add-rpn -n x -e \"%str 'pattern' 'replacement' 1 replace\"" add-rpn/replace-input.csv add-rpn/replace-output1.csv data/empty.txt 0
	add-rpn_replace1)

test("csv-add-rpn -n x -e \"%str 'pattern' 'replacement' 0 replace\"" add-rpn/replace-input.csv add-rpn/replace-output2.csv data/empty.txt 0
	add-rpn_replace2)

test("csv-add-rpn -n x -e \"%str '([^e]*)e(.*)' '%1 XXX %2' 1 replace_ere\"" add-rpn/replace-input.csv add-rpn/replace_re-output1.csv data/empty.txt 0
	add-rpn_replace_ere1)

test("csv-add-rpn -n x -e \"%str '([^e]*)e(.*)' '%1 XXX %2' 0 replace_ere\"" add-rpn/replace-input.csv add-rpn/replace_re-output2.csv data/empty.txt 0
	add-rpn_replace_ere2)

test("csv-add-rpn -n x -e \"%str '\\([^e]*\\)e\\(.*\\)' '%1 XXX %2' 1 replace_bre\"" add-rpn/replace-input.csv add-rpn/replace_re-output1.csv data/empty.txt 0
	add-rpn_replace_bre1)

test("csv-add-rpn -n x -e \"%str '\\([^e]*\\)e\\(.*\\)' '%1 XXX %2' 0 replace_bre\"" add-rpn/replace-input.csv add-rpn/replace_re-output2.csv data/empty.txt 0
	add-rpn_replace_bre2)

test("csv-add-rpn -n 'num' -e \"'a' next\""
	data/3-columns-3-rows.csv data/rpn-add-next.csv data/empty.txt 0
	add-rpn_next)

test("csv-add-rpn --help" data/empty.csv add-rpn/help.txt data/empty.txt 2
	add-rpn_help)

test("csv-add-rpn --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-rpn_version)
