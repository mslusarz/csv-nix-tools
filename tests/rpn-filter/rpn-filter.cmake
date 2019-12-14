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

test("csv-rpn-filter -e \"%name '.suffix' concat 'lorem ipsum.suffix' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-1.csv data/empty.txt 0
	rpn-filter_concat)

test("csv-rpn-filter -e \"%id tostring '2' ==\"" data/3-columns-3-rows.csv data/rpn-filter-row-2.csv data/empty.txt 0
	rpn-filter_tostring)

test("csv-rpn-filter --help" data/empty.csv rpn-filter/help.txt data/empty.txt 2
	rpn-filter_help)

test("csv-rpn-filter --version" data/empty.csv data/git-version.txt data/empty.txt 0
	rpn-filter_version)
