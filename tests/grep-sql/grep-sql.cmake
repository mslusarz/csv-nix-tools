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

test("csv-grep-sql -e 'id == 2'" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	grep-sql_eq)

test("csv-grep-sql -e 'id == 2' -s" data/3-columns-3-rows.csv data/rpn-filter-row-2.txt data/empty.txt 0
	grep-sql_eq_-s)

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


test("csv-grep-sql -e 'name + id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_add)

test("csv-grep-sql -e 'name - id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_sub)

test("csv-grep-sql -e 'name * id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_mul)

test("csv-grep-sql -e 'name / id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_div)

test("csv-grep-sql -e 'name % id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_mod)

test("csv-grep-sql -e 'name | id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_bit_or)

test("csv-grep-sql -e 'name & id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_bit_and)

test("csv-grep-sql -e 'name ^ id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_bit_xor)

test("csv-grep-sql -e 'name << id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_bit_lshift)

test("csv-grep-sql -e 'name >> id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_bit_rshift)

test("csv-grep-sql -e '~name == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/1-arg-not-numeric.txt 2
	grep-sql_typeerr_bit_neg)

test("csv-grep-sql -e 'name and id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_logic_and)

test("csv-grep-sql -e 'name or id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_logic_or)

test("csv-grep-sql -e 'name xor id == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/2-args-not-numeric.txt 2
	grep-sql_typeerr_logic_xor)

test("csv-grep-sql -e '(not name) == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/1-arg-not-numeric.txt 2
	grep-sql_typeerr_logic_not)

test("csv-grep-sql -e 'substr(id, 1, 5) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substr-invalid-types.txt 2
	grep-sql_typeerr_substr_arg1)

test("csv-grep-sql -e 'substr(name, name, 5) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substr-invalid-types.txt 2
	grep-sql_typeerr_substr_arg2)

test("csv-grep-sql -e 'substr(name, 1, name) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substr-invalid-types.txt 2
	grep-sql_typeerr_substr_arg3)

test("csv-grep-sql -e 'id || name == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/concat-invalid-types.txt 2
	grep-sql_typeerr_concat_arg1)

test("csv-grep-sql -e 'name || id == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/concat-invalid-types.txt 2
	grep-sql_typeerr_concat_arg2)

test("csv-grep-sql -e '2 like name == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/like-invalid-types.txt 2
	grep-sql_typeerr_like_arg1)

test("csv-grep-sql -e 'name like 2 == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/like-invalid-types.txt 2
	grep-sql_typeerr_like_arg2)

test("csv-grep-sql -e 'tostring(name, 2) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/tostring-invalid-types.txt 2
	grep-sql_typeerr_tostring)

test("csv-grep-sql -e 'length(id) == id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/strlen-invalid-types.txt 2
	grep-sql_typeerr_length)

test("csv-grep-sql -e 'id < name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-sql_typeerr_lt)

test("csv-grep-sql -e 'id <= name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-sql_typeerr_le)

test("csv-grep-sql -e 'id > name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-sql_typeerr_gt)

test("csv-grep-sql -e 'id >= name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-sql_typeerr_ge)

test("csv-grep-sql -e 'id == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-sql_typeerr_eq)

test("csv-grep-sql -e 'id != name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/comparison-invalid-types.txt 2
	grep-sql_typeerr_ne)

test("csv-grep-sql -e 'if(id, name, id) == 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/if-invalid-types.txt 2
	grep-sql_typeerr_if)


test("csv-grep-sql -e '2 and id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-sql_valueerr_logic_and_arg1)

test("csv-grep-sql -e 'id and 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-sql_valueerr_logic_and_arg2)

test("csv-grep-sql -e '2 or id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-sql_valueerr_logic_or_arg1)

test("csv-grep-sql -e 'id or 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-sql_valueerr_logic_or_arg2)

test("csv-grep-sql -e '2 xor id'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-sql_valueerr_logic_xor_arg1)

test("csv-grep-sql -e 'id xor 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-sql_valueerr_logic_xor_arg2)

test("csv-grep-sql -e 'not 2'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/logic-invalid-values.txt 2
	grep-sql_valueerr_logic_not)

test("csv-grep-sql -e 'substr(name, 2, -1) == name'" data/3-columns-3-rows.csv data/rpn-filter-no-rows.csv grep-rpn/substring-invalid-length.txt 2
	grep-sql_valueerr_substr)

test("csv-grep-sql -T t1 -e \"name like '%or%'\"" grep-rpn/2-tables.csv grep-rpn/2-tables-t1-or.csv data/empty.txt 0
	grep-sql_-T_t1_-e_name_or_like)

endif(BISON_FOUND AND FLEX_FOUND)
