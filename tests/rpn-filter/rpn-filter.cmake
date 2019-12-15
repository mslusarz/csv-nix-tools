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

test("csv-rpn-filter -e '%id 2 =='" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	rpn-filter_eq)

test("csv-rpn-filter -e '%id 2 ==' -s" data/3-columns-3-rows.csv data/rpn-filter-row-2.txt data/empty.txt 0
	rpn-filter_eq_-s)

test("csv-rpn-filter -e '%id 2 !='" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	rpn-filter_ne)

test("csv-rpn-filter -e '%id 2 >'" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	rpn-filter_gt)

test("csv-rpn-filter -e '%id 3 >='" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	rpn-filter_ge)

test("csv-rpn-filter -e '%id 2 <'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	rpn-filter_lt)

test("csv-rpn-filter -e '%id 1 <='" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	rpn-filter_le)

test("csv-rpn-filter -e '%id 1 == %id 3 == or'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	rpn-filter_or)

test("csv-rpn-filter -e '%id 2 == not'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	rpn-filter_not)

test("csv-rpn-filter -e '%id 1 == %id 3 == xor'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	rpn-filter_xor)

test("csv-rpn-filter -e '%id 1 + 3 =='" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	rpn-filter_add)

test("csv-rpn-filter -e '%id 1 - 2 =='" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	rpn-filter_sub)

test("csv-rpn-filter -e '%id 2 * 2 =='" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	rpn-filter_mul)

test("csv-rpn-filter -e '%id 2 / 1 =='" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	rpn-filter_div)

test("csv-rpn-filter -e '%name strlen 11 =='" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	rpn-filter_strlen)

test("csv-rpn-filter -e \"%name 3 1 substr 'r' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	rpn-filter_substr)

test("csv-rpn-filter -e \"%name '%th%' like\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	rpn-filter_like)


test("csv-rpn-filter -e \"%name '%ing.%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found1.csv data/empty.txt 0
	rpn-filter_like_escape_dot)

test("csv-rpn-filter -e \"%name '%ing?%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found2.csv data/empty.txt 0
	rpn-filter_like_escape_quest)

test("csv-rpn-filter -e \"%name '%ing*%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found3.csv data/empty.txt 0
	rpn-filter_like_escape_star)

test("csv-rpn-filter -e \"%name '%ing+%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found4.csv data/empty.txt 0
	rpn-filter_like_escape_plus)

test("csv-rpn-filter -e \"%name '%ing[%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found5.csv data/empty.txt 0
	rpn-filter_like_escape_sq_open)

test("csv-rpn-filter -e \"%name '%ing]%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found6.csv data/empty.txt 0
	rpn-filter_like_escape_sq_close)

test("csv-rpn-filter -e \"%name '%ing(%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found7.csv data/empty.txt 0
	rpn-filter_like_escape_rnd_open)

test("csv-rpn-filter -e \"%name '%ing)%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found8.csv data/empty.txt 0
	rpn-filter_like_escape_rnd_close)

test("csv-rpn-filter -e \"%name '%ing$%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found9.csv data/empty.txt 0
	rpn-filter_like_escape_dollar)

test("csv-rpn-filter -e \"%name '%ing^%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found10.csv data/empty.txt 0
	rpn-filter_like_escape_caret)

test("csv-rpn-filter -e \"%name '%ing\\%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found11.csv data/empty.txt 0
	rpn-filter_like_escape_backslash)

test("csv-rpn-filter -e \"%name '%ing|%' like\"" rpn-filter/special-chars.csv rpn-filter/special-chars-found12.csv data/empty.txt 0
	rpn-filter_like_escape_vbar)


test("csv-rpn-filter -e \"%name '.suffix' concat 'lorem ipsum.suffix' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	rpn-filter_concat)

test("csv-rpn-filter -e \"%id tostring '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	rpn-filter_tostring)

test("csv-rpn-filter --help" data/empty.csv rpn-filter/help.txt data/empty.txt 2
	rpn-filter_help)

test("csv-rpn-filter --version" data/empty.csv data/git-version.txt data/empty.txt 0
	rpn-filter_version)


test("csv-rpn-filter -e '%id + 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_add)

test("csv-rpn-filter -e '%id - 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_sub)

test("csv-rpn-filter -e '%id * 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_mul)

test("csv-rpn-filter -e '%id / 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_div)

test("csv-rpn-filter -e '%id % 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_mod)

test("csv-rpn-filter -e '%id | 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_bit_or)

test("csv-rpn-filter -e '%id & 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_bit_and)

test("csv-rpn-filter -e '%id ^ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_bit_xor)

test("csv-rpn-filter -e '%id << 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_bit_lshift)

test("csv-rpn-filter -e '%id >> 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_bit_rshift)

test("csv-rpn-filter -e '~ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_bit_neg)

test("csv-rpn-filter -e '%id and 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_logic_and)

test("csv-rpn-filter -e '%id or 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_logic_or)

test("csv-rpn-filter -e '%id xor 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_logic_xor)

test("csv-rpn-filter -e 'not 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_logic_not)

test("csv-rpn-filter -e '%name 1 substr %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_substr)

test("csv-rpn-filter -e '%name concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_concat)

test("csv-rpn-filter -e '%name like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_like)

test("csv-rpn-filter -e 'tostring_base2 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_tostring_base2)

test("csv-rpn-filter -e 'tostring_base8 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_tostring_base8)

test("csv-rpn-filter -e 'tostring_base10 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_tostring_base10)

test("csv-rpn-filter -e 'tostring_base16 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_tostring_base16)

test("csv-rpn-filter -e 'strlen %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_strlen)

test("csv-rpn-filter -e 'toint %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_toint)

test("csv-rpn-filter -e '%id <'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_lt)

test("csv-rpn-filter -e '%id <='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_le)

test("csv-rpn-filter -e '%id >'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_gt)

test("csv-rpn-filter -e '%id >='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_ge)

test("csv-rpn-filter -e '%id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_eq)

test("csv-rpn-filter -e '%id !='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_ne)

test("csv-rpn-filter -e '%id %name if 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/stack-low.txt 2
	rpn-filter_stackerr_if)


test("csv-rpn-filter -e '%name %id + 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_add)

test("csv-rpn-filter -e '%name %id - 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_sub)

test("csv-rpn-filter -e '%name %id * 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_mul)

test("csv-rpn-filter -e '%name %id / 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_div)

test("csv-rpn-filter -e '%name %id % 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_mod)

test("csv-rpn-filter -e '%name %id | 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_bit_or)

test("csv-rpn-filter -e '%name %id & 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_bit_and)

test("csv-rpn-filter -e '%name %id ^ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_bit_xor)

test("csv-rpn-filter -e '%name %id << 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_bit_lshift)

test("csv-rpn-filter -e '%name %id >> 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_bit_rshift)

test("csv-rpn-filter -e '%name ~ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/1-arg-not-numeric.txt 2
	rpn-filter_typeerr_bit_neg)

test("csv-rpn-filter -e '%name %id and 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_logic_and)

test("csv-rpn-filter -e '%name %id or 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_logic_or)

test("csv-rpn-filter -e '%name %id xor 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/2-args-not-numeric.txt 2
	rpn-filter_typeerr_logic_xor)

test("csv-rpn-filter -e '%name not 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/1-arg-not-numeric.txt 2
	rpn-filter_typeerr_logic_not)

test("csv-rpn-filter -e '%id 1 5 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/substr-invalid-types.txt 2
	rpn-filter_typeerr_substr_arg1)

test("csv-rpn-filter -e '%name %name 5 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/substr-invalid-types.txt 2
	rpn-filter_typeerr_substr_arg2)

test("csv-rpn-filter -e '%name 1 %name substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/substr-invalid-types.txt 2
	rpn-filter_typeerr_substr_arg3)

test("csv-rpn-filter -e '%id %name concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/concat-invalid-types.txt 2
	rpn-filter_typeerr_concat_arg1)

test("csv-rpn-filter -e '%name %id concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/concat-invalid-types.txt 2
	rpn-filter_typeerr_concat_arg2)

test("csv-rpn-filter -e '2 %name like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/like-invalid-types.txt 2
	rpn-filter_typeerr_like_arg1)

test("csv-rpn-filter -e '%name 2 like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/like-invalid-types.txt 2
	rpn-filter_typeerr_like_arg2)

test("csv-rpn-filter -e '%name tostring_base2 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/tostring-invalid-types.txt 2
	rpn-filter_typeerr_tostring_base2)

test("csv-rpn-filter -e '%name tostring_base8 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/tostring-invalid-types.txt 2
	rpn-filter_typeerr_tostring_base8)

test("csv-rpn-filter -e '%name tostring_base10 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/tostring-invalid-types.txt 2
	rpn-filter_typeerr_tostring_base10)

test("csv-rpn-filter -e '%name tostring_base16 %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/tostring-invalid-types.txt 2
	rpn-filter_typeerr_tostring_base16)

test("csv-rpn-filter -e '%id strlen %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/strlen-invalid-types.txt 2
	rpn-filter_typeerr_strlen)

test("csv-rpn-filter -e '%id toint %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/toint-invalid-types.txt 2
	rpn-filter_typeerr_toint)

test("csv-rpn-filter -e '%id %name <'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/comparison-invalid-types.txt 2
	rpn-filter_typeerr_lt)

test("csv-rpn-filter -e '%id %name <='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/comparison-invalid-types.txt 2
	rpn-filter_typeerr_le)

test("csv-rpn-filter -e '%id %name >'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/comparison-invalid-types.txt 2
	rpn-filter_typeerr_gt)

test("csv-rpn-filter -e '%id %name >='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/comparison-invalid-types.txt 2
	rpn-filter_typeerr_ge)

test("csv-rpn-filter -e '%id %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/comparison-invalid-types.txt 2
	rpn-filter_typeerr_eq)

test("csv-rpn-filter -e '%id %name !='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/comparison-invalid-types.txt 2
	rpn-filter_typeerr_ne)

test("csv-rpn-filter -e '%id %name %id if 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/if-invalid-types.txt 2
	rpn-filter_typeerr_if)


test("csv-rpn-filter -e '2 %id and'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/logic-invalid-values.txt 2
	rpn-filter_valueerr_logic_and_arg1)

test("csv-rpn-filter -e '%id 2 and'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/logic-invalid-values.txt 2
	rpn-filter_valueerr_logic_and_arg2)

test("csv-rpn-filter -e '2 %id or'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/logic-invalid-values.txt 2
	rpn-filter_valueerr_logic_or_arg1)

test("csv-rpn-filter -e '%id 2 or'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/logic-invalid-values.txt 2
	rpn-filter_valueerr_logic_or_arg2)

test("csv-rpn-filter -e '2 %id xor'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/logic-invalid-values.txt 2
	rpn-filter_valueerr_logic_xor_arg1)

test("csv-rpn-filter -e '%id 2 xor'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/logic-invalid-values.txt 2
	rpn-filter_valueerr_logic_xor_arg2)

test("csv-rpn-filter -e '2 not'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/logic-invalid-values.txt 2
	rpn-filter_valueerr_logic_not)

test("csv-rpn-filter -e '%name 2 -1 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv rpn-filter/substring-invalid-length.txt 2
	rpn-filter_valueerr_substr)
