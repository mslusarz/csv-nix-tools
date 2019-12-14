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

test("csv-rpn-add -f 'num_mul_2' -e '%num 2 *'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.csv data/empty.txt 0
	rpn-add_num_mul)

test("csv-rpn-add -f 'num_mul_2' -e '%num 2 *' -s"
	data/rpn-add-num-dec.csv data/rpn-add-num-mul.txt data/empty.txt 0
	rpn-add_num_mul_-s)

test("csv-rpn-add -f 'num_add_num2' -e '%num %num2 +'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sum.csv data/empty.txt 0
	rpn-add_num_sum)

test("csv-rpn-add -f 'num2_div_num3' -e '%num2 %num3 /'\
		  -f 'num2_div_num_add_1' -e '%num2 %num 1 + /'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div.csv data/empty.txt 0
	rpn-add_num_div)

test("csv-rpn-add -f 'num2_div_num' -e '%num2 %num /'"
	data/rpn-add-num-dec.csv data/rpn-add-num-div0.csv data/rpn-add-num-div0.txt 2
	rpn-add_num_div0)

test("csv-rpn-add -f 'num2_sub_num' -e '%num2 %num -'"
	data/rpn-add-num-dec.csv data/rpn-add-num-sub.csv data/empty.txt 0
	rpn-add_num_sub)

test("csv-rpn-add -f 'num2_mod_num3' -e '%num2 %num3 %'\
		  -f 'num2_mod_num_add_1' -e '%num2 %num 1 + %'"
	data/rpn-add-num-dec.csv data/rpn-add-num-mod.csv data/empty.txt 0
	rpn-add_num_mod)


test("csv-rpn-add -f 'num2_bit_and_num3' -e '%num2 %num3 & tostring_base16'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-and.csv data/empty.txt 0
	rpn-add_num_bit_and)

test("csv-rpn-add -f 'num2_bit_or_num' -e '%num2 %num | tostring_base16'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-or.csv data/empty.txt 0
	rpn-add_num_bit_or)

test("csv-rpn-add -f 'num2_bit_xor_num' -e '%num2 %num ^ tostring_base16'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-xor.csv data/empty.txt 0
	rpn-add_num_bit_xor)

test("csv-rpn-add -f 'bit_neg_num2' -e '%num2 ~ tostring_base16'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-neg.csv data/empty.txt 0
	rpn-add_num_bit_neg)

test("csv-rpn-add -f 'num2_lshift_num' -e '%num2 %num << tostring_base16'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-lshift.csv data/empty.txt 0
	rpn-add_num_bit_lshift)

test("csv-rpn-add -f 'num2_rshift_num' -e '%num2 %num >> tostring_base16'"
	data/rpn-add-num-hex.csv data/rpn-add-num-bit-rshift.csv data/empty.txt 0
	rpn-add_num_bit_rshift)


test("csv-rpn-add -f num2_gt_num3 -e '%num2 %num3 >'  -f num2_gt_num2_dup -e '%num2 %num2_dup >'  -f num2_gt_num2_mul_2 -e '%num %num2_mul_2 >'"  data/rpn-add-num-compare.csv data/rpn-add-num-gt.csv data/empty.txt 0
	rpn-add_num_>)
test("csv-rpn-add -f num2_gt_num3 -e '%num2 %num3 gt' -f num2_gt_num2_dup -e '%num2 %num2_dup gt' -f num2_gt_num2_mul_2 -e '%num %num2_mul_2 gt'" data/rpn-add-num-compare.csv data/rpn-add-num-gt.csv data/empty.txt 0
	rpn-add_num_gt)

test("csv-rpn-add -f num2_ge_num3 -e '%num2 %num3 >=' -f num2_ge_num2_dup -e '%num2 %num2_dup >=' -f num2_ge_num2_mul_2 -e '%num %num2_mul_2 >='" data/rpn-add-num-compare.csv data/rpn-add-num-ge.csv data/empty.txt 0
	rpn-add_num_>=)
test("csv-rpn-add -f num2_ge_num3 -e '%num2 %num3 ge' -f num2_ge_num2_dup -e '%num2 %num2_dup ge' -f num2_ge_num2_mul_2 -e '%num %num2_mul_2 ge'" data/rpn-add-num-compare.csv data/rpn-add-num-ge.csv data/empty.txt 0
	rpn-add_num_ge)

test("csv-rpn-add -f num2_lt_num3 -e '%num2 %num3 <'  -f num2_lt_num2_dup -e '%num2 %num2_dup <'  -f num2_lt_num2_mul_2 -e '%num %num2_mul_2 <'"  data/rpn-add-num-compare.csv data/rpn-add-num-lt.csv data/empty.txt 0
	rpn-add_num_<)
test("csv-rpn-add -f num2_lt_num3 -e '%num2 %num3 lt' -f num2_lt_num2_dup -e '%num2 %num2_dup lt' -f num2_lt_num2_mul_2 -e '%num %num2_mul_2 lt'" data/rpn-add-num-compare.csv data/rpn-add-num-lt.csv data/empty.txt 0
	rpn-add_num_lt)

test("csv-rpn-add -f num2_le_num3 -e '%num2 %num3 <=' -f num2_le_num2_dup -e '%num2 %num2_dup <=' -f num2_le_num2_mul_2 -e '%num %num2_mul_2 <='" data/rpn-add-num-compare.csv data/rpn-add-num-le.csv data/empty.txt 0
	rpn-add_num_<=)
test("csv-rpn-add -f num2_le_num3 -e '%num2 %num3 le' -f num2_le_num2_dup -e '%num2 %num2_dup le' -f num2_le_num2_mul_2 -e '%num %num2_mul_2 le'" data/rpn-add-num-compare.csv data/rpn-add-num-le.csv data/empty.txt 0
	rpn-add_num_le)

test("csv-rpn-add -f num2_eq_num2_dup -e '%num2 %num2_dup ==' -f num2_eq_num2_mul_2 -e '%num2 %num2_mul_2 =='" data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	rpn-add_num_==)
test("csv-rpn-add -f num2_eq_num2_dup -e '%num2 %num2_dup eq' -f num2_eq_num2_mul_2 -e '%num2 %num2_mul_2 eq'" data/rpn-add-num-compare.csv data/rpn-add-num-eq.csv data/empty.txt 0
	rpn-add_num_eq)

test("csv-rpn-add -f num2_ne_num2_dup -e '%num2 %num2_dup !=' -f num2_ne_num2_mul_2 -e '%num2 %num2_mul_2 !='" data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	rpn-add_num_!=)
test("csv-rpn-add -f num2_ne_num2_dup -e '%num2 %num2_dup ne' -f num2_ne_num2_mul_2 -e '%num2 %num2_mul_2 ne'" data/rpn-add-num-compare.csv data/rpn-add-num-ne.csv data/empty.txt 0
	rpn-add_num_ne)


test("csv-rpn-add -f num1_or_num2 -e '%num1 %num2 or'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-or.csv data/empty.txt 0
	rpn-add_num_logic_or)

test("csv-rpn-add -f num1_and_num2 -e '%num1 %num2 and'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-and.csv data/empty.txt 0
	rpn-add_num_logic_and)

test("csv-rpn-add -f num1_xor_num2 -e '%num1 %num2 xor'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-xor.csv data/empty.txt 0
	rpn-add_num_logic_xor)

test("csv-rpn-add -f not_num1 -e '%num1 not'"
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-not.csv data/empty.txt 0
	rpn-add_num_logic_not)

test("csv-rpn-add -f desc_num1 -e \"%num1 'one' 'zero' if\""
	data/rpn-add-num-logic.csv data/rpn-add-num-logic-if.csv data/empty.txt 0
	rpn-add_num_logic_if)


test("csv-rpn-add -f substr2 -e '%str1 2 1000 substr'\
		  -f last_char -e '%str1 -1 1 substr'"
	data/rpn-add-str.csv data/rpn-add-str-substr.csv data/empty.txt 0
	rpn-add_str_substr)

test("csv-rpn-add -f len1 -e '%str1 strlen'\
		  -f len2 -e '%str2 strlen'"
	data/rpn-add-str.csv data/rpn-add-str-strlen.csv data/empty.txt 0
	rpn-add_str_strlen)

test("csv-rpn-add -f concat1 -e '%str1 %str2 concat'\
		  -f concat2 -e \"%str1 ' - ' %str2 concat concat\""
	data/rpn-add-str.csv data/rpn-add-str-concat.csv data/empty.txt 0
	rpn-add_str_concat)

test("csv-rpn-add -f str1_like_%12 -e \"%str1 '%12' like\"\
		  -f str1_like_%1% -e \"%str1 '%1%' like\""
	data/rpn-add-str.csv data/rpn-add-str-like.csv data/empty.txt 0
	rpn-add_str_like)


test("csv-rpn-add -f str_to_int -e '%str toint'\
		  -f num_to_string   -e '%num tostring'\
		  -f num_to_string2  -e '%num tostring_base2'\
		  -f num_to_string8  -e '%num tostring_base8'\
		  -f num_to_string10 -e '%num tostring_base10'\
		  -f num_to_string16 -e '%num tostring_base16'"
	data/rpn-add-num-base.csv data/rpn-add-convert.csv data/empty.txt 0
	rpn-add_str_to_int)

test("csv-rpn-add --help" data/empty.csv rpn-add/help.txt data/empty.txt 2
	rpn-add_help)

test("csv-rpn-add --version" data/empty.csv data/git-version.txt data/empty.txt 0
	rpn-add_version)
