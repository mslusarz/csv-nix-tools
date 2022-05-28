#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2022, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-grep-rpn" data/empty.csv data/empty.txt grep-rpn/help.txt 2
	grep-rpn_no_args)

test("csv-grep-rpn -e 'bleh'" data/3-columns-3-rows.csv data/empty.txt data/rpn-filter-invalid-expression.txt 2
	grep-rpn_invalid_expression)

test("csv-grep-rpn -e '%name'" data/3-columns-3-rows.csv data/empty.txt data/rpn-filter-non-numeric-expression.txt 2
	grep-rpn_non_numeric_expression)

test("csv-grep-rpn -e '%id 2 =='" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_eq)

test("csv-grep-rpn -e '%id 2 ==' -s" data/3-columns-3-rows.csv data/rpn-filter-row-2.txt data/empty.txt 0
	grep-rpn_eq_-s)

test("csv-grep-rpn -e '%id 2 ==' -S" data/3-columns-3-rows.csv data/rpn-filter-row-2.txt data/empty.txt 0
	grep-rpn_eq_-S)

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

test("csv-grep-rpn -e \"%name '.*th.*' 1 matches_ere\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-rpn_matches_ere)

test("csv-grep-rpn -e \"%name '.*th.*' 1 matches_bre\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-rpn_matches_bre)


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

test("csv-grep-rpn -e \"%id tostring '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_tostring)

test("csv-grep-rpn -e \"%id '2' toint ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_toint)

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

test("csv-grep-rpn -e '2 int2strb %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_int2strb)

test("csv-grep-rpn -e 'strlen %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_strlen)

test("csv-grep-rpn -e '10 strb2int %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_stackerr_strb2int)

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


test("csv-grep-rpn -e '%name %id + 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-add.txt 2
	grep-rpn_typeerr_add)

test("csv-grep-rpn -e '%name %id - 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-sub.txt 2
	grep-rpn_typeerr_sub)

test("csv-grep-rpn -e '%name %id * 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-mul.txt 2
	grep-rpn_typeerr_mul)

test("csv-grep-rpn -e '%name %id / 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-div.txt 2
	grep-rpn_typeerr_div)

test("csv-grep-rpn -e '%name %id % 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-mod.txt 2
	grep-rpn_typeerr_mod)

test("csv-grep-rpn -e '%name %id | 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-or.txt 2
	grep-rpn_typeerr_bit_or)

test("csv-grep-rpn -e '%name %id & 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-and.txt 2
	grep-rpn_typeerr_bit_and)

test("csv-grep-rpn -e '%name %id ^ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-xor.txt 2
	grep-rpn_typeerr_bit_xor)

test("csv-grep-rpn -e '%name %id << 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-lshift.txt 2
	grep-rpn_typeerr_bit_lshift)

test("csv-grep-rpn -e '%name %id >> 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-rshift.txt 2
	grep-rpn_typeerr_bit_rshift)

test("csv-grep-rpn -e '%name ~ 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-neg.txt 2
	grep-rpn_typeerr_bit_neg)

test("csv-grep-rpn -e '%name %id and 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-and.txt 2
	grep-rpn_typeerr_logic_and)

test("csv-grep-rpn -e '%name %id or 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-or.txt 2
	grep-rpn_typeerr_logic_or)

test("csv-grep-rpn -e '%name %id xor 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-xor.txt 2
	grep-rpn_typeerr_logic_xor)

test("csv-grep-rpn -e '%name not 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-not.txt 2
	grep-rpn_typeerr_logic_not)

test("csv-grep-rpn -e '%id 1 5 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-substr.txt 2
	grep-rpn_typeerr_substr_arg1)

test("csv-grep-rpn -e '%name %name 5 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-substr.txt 2
	grep-rpn_typeerr_substr_arg2)

test("csv-grep-rpn -e '%name 1 %name substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-substr.txt 2
	grep-rpn_typeerr_substr_arg3)

test("csv-grep-rpn -e '%id %name concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-concat.txt 2
	grep-rpn_typeerr_concat_arg1)

test("csv-grep-rpn -e '%name %id concat %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-concat.txt 2
	grep-rpn_typeerr_concat_arg2)

test("csv-grep-rpn -e '2 %name like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-like.txt 2
	grep-rpn_typeerr_like_arg1)

test("csv-grep-rpn -e '%name 2 like %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-like.txt 2
	grep-rpn_typeerr_like_arg2)

test("csv-grep-rpn -e '%name 2 int2strb %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-int2strb.txt 2
	grep-rpn_typeerr_int2strb)

test("csv-grep-rpn -e '%id strlen %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-strlen.txt 2
	grep-rpn_typeerr_strlen)

test("csv-grep-rpn -e '10 %id strb2int %id =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-strb2int.txt 2
	grep-rpn_typeerr_strb2int)

test("csv-grep-rpn -e '%id %name <'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-lt.txt 2
	grep-rpn_typeerr_lt)

test("csv-grep-rpn -e '%id %name <='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-le.txt 2
	grep-rpn_typeerr_le)

test("csv-grep-rpn -e '%id %name >'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-gt.txt 2
	grep-rpn_typeerr_gt)

test("csv-grep-rpn -e '%id %name >='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-ge.txt 2
	grep-rpn_typeerr_ge)

test("csv-grep-rpn -e '%id %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-eq.txt 2
	grep-rpn_typeerr_eq)

test("csv-grep-rpn -e '%id %name !='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-ne.txt 2
	grep-rpn_typeerr_ne)

test("csv-grep-rpn -e '%id %name %id if 2 =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-if.txt 2
	grep-rpn_typeerr_if)


test("csv-grep-rpn -e '2 %id and'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-and.txt 2
	grep-rpn_valueerr_logic_and_arg1)

test("csv-grep-rpn -e '%id 2 and'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-and.txt 2
	grep-rpn_valueerr_logic_and_arg2)

test("csv-grep-rpn -e '2 %id or'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-or.txt 2
	grep-rpn_valueerr_logic_or_arg1)

test("csv-grep-rpn -e '%id 2 or'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-or.txt 2
	grep-rpn_valueerr_logic_or_arg2)

test("csv-grep-rpn -e '2 %id xor'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-xor.txt 2
	grep-rpn_valueerr_logic_xor_arg1)

test("csv-grep-rpn -e '%id 2 xor'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-xor.txt 2
	grep-rpn_valueerr_logic_xor_arg2)

test("csv-grep-rpn -e '2 not'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-not.txt 2
	grep-rpn_valueerr_logic_not)

test("csv-grep-rpn -e '%name 2 -1 substr %name =='" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-substr.txt 2
	grep-rpn_valueerr_substr)

test("csv-grep-rpn -T t1 -e \"%name '%or%' like\"" grep-rpn/2-tables.csv grep-rpn/2-tables-t1-or.csv data/empty.txt 0
	grep-rpn_-T_t1_-e_name_or_like)

test("csv-grep-rpn -T t1 -e \"%name '%or%' like\"" data/no-table.csv data/empty.txt data/no-table-column.txt 2
	grep-rpn_no_table)

test("csv-grep-rpn -e '%col2 1000 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-eq-int)

test("csv-grep-rpn -e '%col2 1000.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-eq-float)

test("csv-grep-rpn -e '1000 %col2 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-eq-float)


test("csv-grep-rpn -e '%col2 1000 !='" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_float-ne-int)

test("csv-grep-rpn -e '%col2 1000.0 !='" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_float-ne-float)

test("csv-grep-rpn -e '1000 %col2 !='" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_int-ne-float)


test("csv-grep-rpn -e '%col2 1000 <'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_float-lt-int)

test("csv-grep-rpn -e '%col2 1000.0 <'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_float-lt-float)

test("csv-grep-rpn -e '999 %col2 <'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-lt-float)


test("csv-grep-rpn -e '%col2 999 <='" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_float-le-int)

test("csv-grep-rpn -e '%col2 999.999 <='" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_float-le-float)

test("csv-grep-rpn -e '999 %col2 <='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-le-float)


test("csv-grep-rpn -e '%col2 999 >'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-gt-int)

test("csv-grep-rpn -e '%col2 999.999 >'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-gt-float)

test("csv-grep-rpn -e '999 %col2 >'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_int-gt-float)


test("csv-grep-rpn -e '%col2 1000 >='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-ge-int)

test("csv-grep-rpn -e '%col2 1000.0 >='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-ge-float)

test("csv-grep-rpn -e '999 %col2 >='" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-rpn_int-ge-float)


test("csv-grep-rpn -e '%col2 1000.7 + 2000.7 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-add-float)

test("csv-grep-rpn -e '%col2 1000 + 2000.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-add-int)

test("csv-grep-rpn -e '1000 %col2 + 2000.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-add-float)


test("csv-grep-rpn -e '%col2 100.2 - 899.8 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-sub-float)

test("csv-grep-rpn -e '%col2 100 - 900.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-sub-int)

test("csv-grep-rpn -e '100 %col2 - -900.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-sub-float)


test("csv-grep-rpn -e '%col2 100.2 * 100200.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-mul-float)

test("csv-grep-rpn -e '%col2 100 * 100000.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-mul-int)

test("csv-grep-rpn -e '100 %col2 * 100000.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-mul-float)


test("csv-grep-rpn -e '%col2 400.0 / 2.5 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-div-float)

test("csv-grep-rpn -e '%col2 100 / 10.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-div-int)

test("csv-grep-rpn -e '100 %col2 / 0.1 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-div-float)


test("csv-grep-rpn -e '%col2 400.0 % 200.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-mod-float)

test("csv-grep-rpn -e '%col2 400 % 200.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_float-mod-int)

test("csv-grep-rpn -e '1400 %col2 % 400.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int-mod-float)


test("csv-grep-rpn -e '%id int2flt 4 / 0.5 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_int2flt)

test("csv-grep-rpn -e '%col2 flt2int 3 / 333 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_flt2int)

test("csv-grep-rpn -e '%col2 flt2str \"1000.000000\" =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_flt2str)

test("csv-grep-rpn -e '\"1000.0\" str2flt %col2 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_str2flt)

test("csv-grep-rpn -e \"%id int2str '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_int2str)

test("csv-grep-rpn -e \"%id '2' str2int ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_str2int)

test("csv-grep-rpn -e \"%id '0b10' 2 strb2int ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_strb2int_b2)

test("csv-grep-rpn -e \"%id '0b10' 0 strb2int ==\"" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-strb2int.txt 2
	grep-rpn_strb2int_b0)

test("csv-grep-rpn -e \"%id '2' 10 strb2int ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_strb2int_b10)


test("csv-grep-rpn -e 'int2flt 4 / 0.5 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_int2flt_stack_low)

test("csv-grep-rpn -e 'flt2int 3 / 333 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_flt2int_stack_low)

test("csv-grep-rpn -e 'flt2str \"1000.000000\" =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_flt2str_stack_low)

test("csv-grep-rpn -e 'str2flt %col2 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_str2flt_stack_low)

test("csv-grep-rpn -e \"int2str '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_int2str_stack_low)

test("csv-grep-rpn -e \"str2int %id ==\"" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_str2int_stack_low)


test("csv-grep-rpn -e '%col2 int2flt 4 / 0.5 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/typeerr-int2flt.txt 2
	grep-rpn_typeerr_int2flt)

test("csv-grep-rpn -e '%id flt2int 3 / 333 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/typeerr-flt2int.txt 2
	grep-rpn_typeerr_flt2int)

test("csv-grep-rpn -e '%id flt2str \"1000.000000\" =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/typeerr-flt2str.txt 2
	grep-rpn_typeerr_flt2str)

test("csv-grep-rpn -e '%id str2flt %col2 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/typeerr-str2flt.txt 2
	grep-rpn_typeerr_str2flt)

test("csv-grep-rpn -e \"%name int2str '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-int2str.txt 2
	grep-rpn_typeerr_int2str)

test("csv-grep-rpn -e \"%id str2int ==\"" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-str2int.txt 2
	grep-rpn_typeerr_str2int)


test("csv-grep-rpn -e '%id tofloat 4 / 0.5 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_tofloat_from_int)

test("csv-grep-rpn -e '\"1000.0\" tofloat %col2 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_tofloat_from_str)

test("csv-grep-rpn -e '%col2 tofloat 1000.0 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_tofloat_from_float)

test("csv-grep-rpn -e 'tofloat 1000.0 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_tofloat_stack_low)


test("csv-grep-rpn -e '%col2 toint 3 / 333 =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_toint_from_float)

test("csv-grep-rpn -e \"%id '2' toint ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_toint_from_string)

test("csv-grep-rpn -e \"%id '0b10' toint ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_toint_from_string_bin)

test("csv-grep-rpn -e \"%id toint 2 ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_toint_from_int)

test("csv-grep-rpn -e 'toint 3 / 333 =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_toint_stack_low)


test("csv-grep-rpn -e \"%id tostring '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_tostring_from_int)

test("csv-grep-rpn -e '%col2 tostring \"1000.000000\" =='" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-rpn_tostring_from_float)

test("csv-grep-rpn -e \"%name tostring 'not all that is gold' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-rpn_tostring_from_string)

test("csv-grep-rpn -e 'tostring \"1000.000000\" =='" data/floats.csv grep-rpn/float-no-rows.csv grep-rpn/stack-low.txt 2
	grep-rpn_tostring_stack_low)
