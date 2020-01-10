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

test("csv-grep-rpn -e '%id 2 =='" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_eq)

test("csv-grep-rpn -e '%id 2 ==' -s" data/3-columns-3-rows.csv data/rpn-filter-row-2.txt data/empty.txt 0
	grep-rpn_eq_-s)

test("csv-grep-rpn -e '%id 2 !='" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-rpn_ne)

test("csv-grep-rpn -e '%id 2 >'" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	grep-rpn_gt)

test("csv-grep-rpn -e '%id 3 >='" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	grep-rpn_ge)

test("csv-grep-rpn -e '%id 2 <'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-rpn_lt)

test("csv-grep-rpn -e '%id 1 <='" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-rpn_le)

test("csv-grep-rpn -e '%id 1 == %id 3 == or'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-rpn_or)

test("csv-grep-rpn -e '%id 2 == not'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-rpn_not)

test("csv-grep-rpn -e '%id 1 == %id 3 == xor'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-rpn_xor)

test("csv-grep-rpn -e '%id 1 + 3 =='" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_add)

test("csv-grep-rpn -e '%id 1 - 2 =='" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	grep-rpn_sub)

test("csv-grep-rpn -e '%id 2 * 2 =='" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-rpn_mul)

test("csv-grep-rpn -e '%id 2 / 1 =='" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-rpn_div)

test("csv-grep-rpn -e '%name strlen 11 =='" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-rpn_strlen)

test("csv-grep-rpn -e \"%name 3 1 substr 'r' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-rpn_substr)

test("csv-grep-rpn -e \"%name '%th%' like\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-rpn_like)


test("csv-grep-rpn -e \"%name '%ing.%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found1.csv data/empty.txt 0
	grep-rpn_like_escape_dot)

test("csv-grep-rpn -e \"%name '%ing?%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found2.csv data/empty.txt 0
	grep-rpn_like_escape_quest)

test("csv-grep-rpn -e \"%name '%ing*%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found3.csv data/empty.txt 0
	grep-rpn_like_escape_star)

test("csv-grep-rpn -e \"%name '%ing+%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found4.csv data/empty.txt 0
	grep-rpn_like_escape_plus)

test("csv-grep-rpn -e \"%name '%ing[%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found5.csv data/empty.txt 0
	grep-rpn_like_escape_sq_open)

test("csv-grep-rpn -e \"%name '%ing]%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found6.csv data/empty.txt 0
	grep-rpn_like_escape_sq_close)

test("csv-grep-rpn -e \"%name '%ing(%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found7.csv data/empty.txt 0
	grep-rpn_like_escape_rnd_open)

test("csv-grep-rpn -e \"%name '%ing)%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found8.csv data/empty.txt 0
	grep-rpn_like_escape_rnd_close)

test("csv-grep-rpn -e \"%name '%ing$%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found9.csv data/empty.txt 0
	grep-rpn_like_escape_dollar)

test("csv-grep-rpn -e \"%name '%ing^%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found10.csv data/empty.txt 0
	grep-rpn_like_escape_caret)

test("csv-grep-rpn -e \"%name '%ing\\%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found11.csv data/empty.txt 0
	grep-rpn_like_escape_backslash)

test("csv-grep-rpn -e \"%name '%ing|%' like\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found12.csv data/empty.txt 0
	grep-rpn_like_escape_vbar)


test("csv-grep-rpn -e \"%name '.suffix' concat 'lorem ipsum.suffix' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-rpn_concat)

test("csv-grep-rpn -e \"%id 10 tostring '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_tostring)

test("csv-grep-rpn --help" data/empty.csv grep-rpn/help.txt data/empty.txt 2
	grep-rpn_help)

test("csv-grep-rpn --version" data/empty.csv data/git-version.txt data/empty.txt 0
	grep-rpn_version)


test("csv-grep-rpn -e '%id + 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_add)

test("csv-grep-rpn -e '%id - 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_sub)

test("csv-grep-rpn -e '%id * 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_mul)

test("csv-grep-rpn -e '%id / 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_div)

test("csv-grep-rpn -e '%id % 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_mod)

test("csv-grep-rpn -e '%id | 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_bit_or)

test("csv-grep-rpn -e '%id & 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_bit_and)

test("csv-grep-rpn -e '%id ^ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_bit_xor)

test("csv-grep-rpn -e '%id << 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_bit_lshift)

test("csv-grep-rpn -e '%id >> 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_bit_rshift)

test("csv-grep-rpn -e '~ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_bit_neg)

test("csv-grep-rpn -e '%id and 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_logic_and)

test("csv-grep-rpn -e '%id or 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_logic_or)

test("csv-grep-rpn -e '%id xor 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_logic_xor)

test("csv-grep-rpn -e 'not 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_logic_not)

test("csv-grep-rpn -e '%name 1 substr %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_substr)

test("csv-grep-rpn -e '%name concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_concat)

test("csv-grep-rpn -e '%name like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_like)

test("csv-grep-rpn -e '2 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_tostring2)

test("csv-grep-rpn -e '8 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_tostring8)

test("csv-grep-rpn -e '10 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_tostring10)

test("csv-grep-rpn -e '16 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_tostring16)

test("csv-grep-rpn -e 'strlen %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_strlen)

test("csv-grep-rpn -e '10 toint %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_toint)

test("csv-grep-rpn -e '%id <'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_lt)

test("csv-grep-rpn -e '%id <='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_le)

test("csv-grep-rpn -e '%id >'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_gt)

test("csv-grep-rpn -e '%id >='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_ge)

test("csv-grep-rpn -e '%id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_eq)

test("csv-grep-rpn -e '%id !='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_ne)

test("csv-grep-rpn -e '%id %name if 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_if)


test("csv-grep-rpn -e '%name %id + 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_add)

test("csv-grep-rpn -e '%name %id - 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_sub)

test("csv-grep-rpn -e '%name %id * 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_mul)

test("csv-grep-rpn -e '%name %id / 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_div)

test("csv-grep-rpn -e '%name %id % 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_mod)

test("csv-grep-rpn -e '%name %id | 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_bit_or)

test("csv-grep-rpn -e '%name %id & 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_bit_and)

test("csv-grep-rpn -e '%name %id ^ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_bit_xor)

test("csv-grep-rpn -e '%name %id << 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_bit_lshift)

test("csv-grep-rpn -e '%name %id >> 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_bit_rshift)

test("csv-grep-rpn -e '%name ~ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/1-arg-not-numeric.txt 2
	grep-rpn_typeerr_bit_neg)

test("csv-grep-rpn -e '%name %id and 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_logic_and)

test("csv-grep-rpn -e '%name %id or 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_logic_or)

test("csv-grep-rpn -e '%name %id xor 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-rpn_typeerr_logic_xor)

test("csv-grep-rpn -e '%name not 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/1-arg-not-numeric.txt 2
	grep-rpn_typeerr_logic_not)

test("csv-grep-rpn -e '%id 1 5 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substr-invalid-types.txt 2
	grep-rpn_typeerr_substr_arg1)

test("csv-grep-rpn -e '%name %name 5 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substr-invalid-types.txt 2
	grep-rpn_typeerr_substr_arg2)

test("csv-grep-rpn -e '%name 1 %name substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substr-invalid-types.txt 2
	grep-rpn_typeerr_substr_arg3)

test("csv-grep-rpn -e '%id %name concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/concat-invalid-types.txt 2
	grep-rpn_typeerr_concat_arg1)

test("csv-grep-rpn -e '%name %id concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/concat-invalid-types.txt 2
	grep-rpn_typeerr_concat_arg2)

test("csv-grep-rpn -e '2 %name like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/like-invalid-types.txt 2
	grep-rpn_typeerr_like_arg1)

test("csv-grep-rpn -e '%name 2 like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/like-invalid-types.txt 2
	grep-rpn_typeerr_like_arg2)

test("csv-grep-rpn -e '%name 2 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/tostring-invalid-types.txt 2
	grep-rpn_typeerr_tostring2)

test("csv-grep-rpn -e '%name 8 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/tostring-invalid-types.txt 2
	grep-rpn_typeerr_tostring8)

test("csv-grep-rpn -e '%name 10 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/tostring-invalid-types.txt 2
	grep-rpn_typeerr_tostring10)

test("csv-grep-rpn -e '%name 16 tostring %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/tostring-invalid-types.txt 2
	grep-rpn_typeerr_tostring16)

test("csv-grep-rpn -e '%id strlen %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/strlen-invalid-types.txt 2
	grep-rpn_typeerr_strlen)

test("csv-grep-rpn -e '10 %id toint %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/toint-invalid-types.txt 2
	grep-rpn_typeerr_toint)

test("csv-grep-rpn -e '%id %name <'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-rpn_typeerr_lt)

test("csv-grep-rpn -e '%id %name <='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-rpn_typeerr_le)

test("csv-grep-rpn -e '%id %name >'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-rpn_typeerr_gt)

test("csv-grep-rpn -e '%id %name >='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-rpn_typeerr_ge)

test("csv-grep-rpn -e '%id %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-rpn_typeerr_eq)

test("csv-grep-rpn -e '%id %name !='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-rpn_typeerr_ne)

test("csv-grep-rpn -e '%id %name %id if 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/if-invalid-types.txt 2
	grep-rpn_typeerr_if)


test("csv-grep-rpn -e '2 %id and'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-rpn_valueerr_logic_and_arg1)

test("csv-grep-rpn -e '%id 2 and'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-rpn_valueerr_logic_and_arg2)

test("csv-grep-rpn -e '2 %id or'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-rpn_valueerr_logic_or_arg1)

test("csv-grep-rpn -e '%id 2 or'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-rpn_valueerr_logic_or_arg2)

test("csv-grep-rpn -e '2 %id xor'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-rpn_valueerr_logic_xor_arg1)

test("csv-grep-rpn -e '%id 2 xor'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-rpn_valueerr_logic_xor_arg2)

test("csv-grep-rpn -e '2 not'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-rpn_valueerr_logic_not)

test("csv-grep-rpn -e '%name 2 -1 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substring-invalid-length.txt 2
	grep-rpn_valueerr_substr)

test("csv-grep-rpn -T t1 -e \"%name '%or%' like\"" grep-rpn/2-tables.csv grep-rpn/2-tables-t1-or.csv data/empty.txt 0
	grep-rpn_-T_t1_-e_name_or_like)
