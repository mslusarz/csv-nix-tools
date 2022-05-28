#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2022, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

if (BUILD_SQL)

test("csv-grep-sql -e 'id == 2'" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_eq)

test("csv-grep-sql -e 'id == 2' -s" data/3-columns-3-rows.csv data/rpn-filter-row-2.txt data/empty.txt 0
	grep-sql_eq_-s)

test("csv-grep-sql -e 'id == 2' -S" data/3-columns-3-rows.csv data/rpn-filter-row-2.txt data/empty.txt 0
	grep-sql_eq_-S)

test("csv-grep-sql -e 'id != 2'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-sql_ne)

test("csv-grep-sql -e 'id > 2'" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	grep-sql_gt)

test("csv-grep-sql -e 'id >= 3'" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	grep-sql_ge)

test("csv-grep-sql -e 'id < 2'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_lt)

test("csv-grep-sql -e 'id <= 1'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_le)

test("csv-grep-sql -e 'id == 1 or id == 3'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-sql_or)

test("csv-grep-sql -e 'not (id == 2)'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-sql_not)

test("csv-grep-sql -e 'id == 1 xor id == 3'" data/3-columns-3-rows.csv data/rpn-filter-rows-1-3.csv data/empty.txt 0
	grep-sql_xor)

test("csv-grep-sql -e 'id + 1 == 3'" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_add)

test("csv-grep-sql -e 'id - 1 == 2'" data/3-columns-3-rows.csv data/rpn-filter-row-3.csv data/empty.txt 0
	grep-sql_sub)

test("csv-grep-sql -e 'id * 2 == 2'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_mul)

test("csv-grep-sql -e 'id / 2 == 1'" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-sql_div)

test("csv-grep-sql -e 'length(name) == 11'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_length)

test("csv-grep-sql -e 'length(name) == 0xb'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_length_hex)

test("csv-grep-sql -e 'length(name) == 013'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_length_oct)

test("csv-grep-sql -e 'length(name) == 0b1011'" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_length_bin)

test("csv-grep-sql -e \"substr(name, 3, 1) == 'r'\"" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_substr)

test("csv-grep-sql -e \"name like '%th%'\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-sql_like)

test("csv-grep-sql -e \"matches_ere(name, '.*th.*', 1)\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-sql_matches_ere1)

test("csv-grep-sql -e \"matches_ere(name, '.*th.*')\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-sql_matches_ere1a)

test("csv-grep-sql -e \"matches_bre(name,'.*th.*', 1)\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-sql_matches_bre1)

test("csv-grep-sql -e \"matches_bre(name,'.*th.*')\"" data/3-columns-3-rows.csv data/rpn-filter-rows-2-3.csv data/empty.txt 0
	grep-sql_matches_bre1a)


test("csv-grep-sql -e \"name like '%ing.%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found1.csv data/empty.txt 0
	grep-sql_like_escape_dot)

test("csv-grep-sql -e \"name like '%ing?%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found2.csv data/empty.txt 0
	grep-sql_like_escape_quest)

test("csv-grep-sql -e \"name like '%ing*%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found3.csv data/empty.txt 0
	grep-sql_like_escape_star)

test("csv-grep-sql -e \"name like '%ing+%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found4.csv data/empty.txt 0
	grep-sql_like_escape_plus)

test("csv-grep-sql -e \"name like '%ing[%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found5.csv data/empty.txt 0
	grep-sql_like_escape_sq_open)

test("csv-grep-sql -e \"name like '%ing]%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found6.csv data/empty.txt 0
	grep-sql_like_escape_sq_close)

test("csv-grep-sql -e \"name like '%ing(%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found7.csv data/empty.txt 0
	grep-sql_like_escape_rnd_open)

test("csv-grep-sql -e \"name like '%ing)%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found8.csv data/empty.txt 0
	grep-sql_like_escape_rnd_close)

test("csv-grep-sql -e \"name like '%ing$%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found9.csv data/empty.txt 0
	grep-sql_like_escape_dollar)

test("csv-grep-sql -e \"name like '%ing^%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found10.csv data/empty.txt 0
	grep-sql_like_escape_caret)

test("csv-grep-sql -e \"name like '%ing\\%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found11.csv data/empty.txt 0
	grep-sql_like_escape_backslash)

test("csv-grep-sql -e \"name like '%ing|%'\"" grep-rpn/special-chars.csv grep-rpn/special-chars-found12.csv data/empty.txt 0
	grep-sql_like_escape_vbar)


test("csv-grep-sql -e \"name || '.suffix' == 'lorem ipsum.suffix'\"" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	grep-sql_concat)

test("csv-grep-sql -e \"tostring(id) == '2'\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_tostring)

test("csv-grep-sql --help" data/empty.csv grep-sql/help.txt data/empty.txt 2
	grep-sql_help)

test("csv-grep-sql --version" data/empty.csv data/git-version.txt data/empty.txt 0
	grep-sql_version)


test("csv-grep-sql -e 'name + id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-add.txt 2
	grep-sql_typeerr_add)

test("csv-grep-sql -e 'name - id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-sub.txt 2
	grep-sql_typeerr_sub)

test("csv-grep-sql -e 'name * id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-mul.txt 2
	grep-sql_typeerr_mul)

test("csv-grep-sql -e 'name / id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-div.txt 2
	grep-sql_typeerr_div)

test("csv-grep-sql -e 'name % id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-mod.txt 2
	grep-sql_typeerr_mod)

test("csv-grep-sql -e 'name | id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-or.txt 2
	grep-sql_typeerr_bit_or)

test("csv-grep-sql -e 'name & id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-and.txt 2
	grep-sql_typeerr_bit_and)

test("csv-grep-sql -e 'name ^ id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-xor.txt 2
	grep-sql_typeerr_bit_xor)

test("csv-grep-sql -e 'name << id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-lshift.txt 2
	grep-sql_typeerr_bit_lshift)

test("csv-grep-sql -e 'name >> id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-rshift.txt 2
	grep-sql_typeerr_bit_rshift)

test("csv-grep-sql -e '~name == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-bit-neg.txt 2
	grep-sql_typeerr_bit_neg)

test("csv-grep-sql -e 'name and id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-and.txt 2
	grep-sql_typeerr_logic_and)

test("csv-grep-sql -e 'name or id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-or.txt 2
	grep-sql_typeerr_logic_or)

test("csv-grep-sql -e 'name xor id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-xor.txt 2
	grep-sql_typeerr_logic_xor)

test("csv-grep-sql -e '(not name) == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-logic-not.txt 2
	grep-sql_typeerr_logic_not)

test("csv-grep-sql -e 'substr(id, 1, 5) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-substr.txt 2
	grep-sql_typeerr_substr_arg1)

test("csv-grep-sql -e 'substr(name, name, 5) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-substr.txt 2
	grep-sql_typeerr_substr_arg2)

test("csv-grep-sql -e 'substr(name, 1, name) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-substr.txt 2
	grep-sql_typeerr_substr_arg3)

test("csv-grep-sql -e 'id || name == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-concat.txt 2
	grep-sql_typeerr_concat_arg1)

test("csv-grep-sql -e 'name || id == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-concat.txt 2
	grep-sql_typeerr_concat_arg2)

test("csv-grep-sql -e '2 like name == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-like.txt 2
	grep-sql_typeerr_like_arg1)

test("csv-grep-sql -e 'name like 2 == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-like.txt 2
	grep-sql_typeerr_like_arg2)

test("csv-grep-sql -e 'int2str(name) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-int2str.txt 2
	grep-sql_typeerr_int2str)

test("csv-grep-sql -e 'length(id) == id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-strlen.txt 2
	grep-sql_typeerr_length)

test("csv-grep-sql -e 'id < name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-lt.txt 2
	grep-sql_typeerr_lt)

test("csv-grep-sql -e 'id <= name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-le.txt 2
	grep-sql_typeerr_le)

test("csv-grep-sql -e 'id > name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-gt.txt 2
	grep-sql_typeerr_gt)

test("csv-grep-sql -e 'id >= name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-ge.txt 2
	grep-sql_typeerr_ge)

test("csv-grep-sql -e 'id == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-eq.txt 2
	grep-sql_typeerr_eq)

test("csv-grep-sql -e 'id != name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-ne.txt 2
	grep-sql_typeerr_ne)

test("csv-grep-sql -e 'if(id, name, id) == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/typeerr-if.txt 2
	grep-sql_typeerr_if)


test("csv-grep-sql -e '2 and id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-and.txt 2
	grep-sql_valueerr_logic_and_arg1)

test("csv-grep-sql -e 'id and 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-and.txt 2
	grep-sql_valueerr_logic_and_arg2)

test("csv-grep-sql -e '2 or id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-or.txt 2
	grep-sql_valueerr_logic_or_arg1)

test("csv-grep-sql -e 'id or 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-or.txt 2
	grep-sql_valueerr_logic_or_arg2)

test("csv-grep-sql -e '2 xor id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-xor.txt 2
	grep-sql_valueerr_logic_xor_arg1)

test("csv-grep-sql -e 'id xor 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-xor.txt 2
	grep-sql_valueerr_logic_xor_arg2)

test("csv-grep-sql -e 'not 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-logic-not.txt 2
	grep-sql_valueerr_logic_not)

test("csv-grep-sql -e 'substr(name, 2, -1) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/valueerr-substr.txt 2
	grep-sql_valueerr_substr)

test("csv-grep-sql -T t1 -e \"name like '%or%'\"" grep-rpn/2-tables.csv grep-rpn/2-tables-t1-or.csv data/empty.txt 0
	grep-sql_-T_t1_-e_name_or_like)







test("csv-grep-sql -e 'col2 == 1000'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-eq-int)

test("csv-grep-sql -e 'col2 == 1000.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-eq-float)

test("csv-grep-sql -e '1000 == col2'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-eq-float)


test("csv-grep-sql -e 'col2 != 1000'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_float-ne-int)

test("csv-grep-sql -e 'col2 != 1000.0'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_float-ne-float)

test("csv-grep-sql -e '1000 != col2'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_int-ne-float)


test("csv-grep-sql -e 'col2 < 1000'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_float-lt-int)

test("csv-grep-sql -e 'col2 < 1000.0'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_float-lt-float)

test("csv-grep-sql -e '999 < col2'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-lt-float)


test("csv-grep-sql -e 'col2 <= 999'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_float-le-int)

test("csv-grep-sql -e 'col2 <= 999.999'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_float-le-float)

test("csv-grep-sql -e '999 <= col2'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-le-float)


test("csv-grep-sql -e 'col2 > 999'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-gt-int)

test("csv-grep-sql -e 'col2 > 999.999'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-gt-float)

test("csv-grep-sql -e '999 > col2'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_int-gt-float)


test("csv-grep-sql -e 'col2 >= 1000'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-ge-int)

test("csv-grep-sql -e 'col2 >= 1000.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-ge-float)

test("csv-grep-sql -e '999 >= col2'" data/floats.csv grep-rpn/float-not-1000.csv data/empty.txt 0
	grep-sql_int-ge-float)


test("csv-grep-sql -e 'col2 + 1000.7 == 2000.7'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-add-float)

test("csv-grep-sql -e 'col2 + 1000 == 2000.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-add-int)

test("csv-grep-sql -e '1000 + col2 == 2000.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-add-float)


test("csv-grep-sql -e 'col2 - 100.2 == 899.8'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-sub-float)

test("csv-grep-sql -e 'col2 - 100 == 900.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-sub-int)

test("csv-grep-sql -e '100 - col2 == -900.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-sub-float)


test("csv-grep-sql -e 'col2 * 100.2 == 100200.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-mul-float)

test("csv-grep-sql -e 'col2 * 100 == 100000.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-mul-int)

test("csv-grep-sql -e '100 * col2 == 100000.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-mul-float)


test("csv-grep-sql -e 'col2 / 400.0 == 2.5'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-div-float)

test("csv-grep-sql -e 'col2 / 100 == 10.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-div-int)

test("csv-grep-sql -e '100 / col2 == 0.1'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-div-float)


test("csv-grep-sql -e 'col2 % 400.0 == 200.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-mod-float)

test("csv-grep-sql -e 'col2 % 400 == 200.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_float-mod-int)

test("csv-grep-sql -e '1400 % col2 == 400.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int-mod-float)


test("csv-grep-sql -e 'id / int2flt(4) == 0.5'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_int2flt)

test("csv-grep-sql -e 'flt2int(col2) / 3 == 333'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_flt2int)

test("csv-grep-sql -e 'flt2str(col2) == \"1000.000000\"'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_flt2str)

test("csv-grep-sql -e 'str2flt(\"1000.0\") == col2'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_str2flt)

test("csv-grep-sql -e \"int2str(id) == '2'\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_int2str)

test("csv-grep-sql -e \"id == str2int('2')\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_str2int)


test("csv-grep-sql -e 'id / tofloat(4) == 0.5'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_tofloat_from_int)

test("csv-grep-sql -e 'tofloat(\"1000.0\") == col2'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_tofloat_from_str)

test("csv-grep-sql -e 'tofloat(col2) == 1000.0'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_tofloat_from_float)


test("csv-grep-sql -e 'toint(col2) / 3 == 333'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_toint_from_float)

test("csv-grep-sql -e \"id == toint('2')\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_toint_from_string)

test("csv-grep-sql -e \"toint(id) == 2\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_toint_from_int)


test("csv-grep-sql -e \"tostring(id) == '2'\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_tostring_from_int)

test("csv-grep-sql -e 'tostring(col2) == \"1000.000000\"'" data/floats.csv grep-rpn/float-1000.csv data/empty.txt 0
	grep-sql_tostring_from_float)

test("csv-grep-sql -e \"tostring(name) == 'not all that is gold'\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_tostring_from_string)

endif(BUILD_SQL)
