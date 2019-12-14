#
# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
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

test("csv-cut" data/empty.txt data/empty.csv data/eof.txt 2
	cut_empty_input)

test("csv-cut" data/one-column-one-row.csv data/empty.txt cut/help.txt 2
	cut_simple_pass_through)

test("csv-cut -f" data/one-column-one-row.csv cut/help.txt cut/no-fields-error.txt 2
	cut_simple_pass_through_with_-f)

test("csv-cut -f name" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	cut_the_only_field_using_-f)

test("csv-cut --fields name" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	cut_the_only_field_using_--fields_with_space)

test("csv-cut --fields=name" data/one-column-one-row.csv data/one-column-one-row.csv data/empty.txt 0
	cut_the_only_field_using_--fields_with_=)

test("csv-cut -f notExistingColumn" data/one-column-one-row.csv data/empty.txt cut/column-not-found.txt 2
	cut_not_existing_field_using_-f)

test("csv-cut --fields notExistingColumn" data/one-column-one-row.csv data/empty.txt cut/column-not-found.txt 2
	cut_not_existing_field_using_--fields_with_space)

test("csv-cut --fields=notExistingColumn" data/one-column-one-row.csv data/empty.txt cut/column-not-found.txt 2
	cut_not_existing_field_using_--fields_with_=)


test("csv-cut -f name" data/3-columns-3-rows.csv cut/name-column-3-rows.csv data/empty.txt 0
	cut_the_name_field_using_-f)

test("csv-cut --fields name" data/3-columns-3-rows.csv cut/name-column-3-rows.csv data/empty.txt 0
	cut_the_name_field_using_--fields_with_space)

test("csv-cut --fields=name" data/3-columns-3-rows.csv cut/name-column-3-rows.csv data/empty.txt 0
	cut_the_name_field_using_--fields_with_=)


test("csv-cut -f id" data/3-columns-3-rows.csv data/id-column-3-rows.csv data/empty.txt 0
	cut_the_id_field_using_-f)

test("csv-cut --fields id" data/3-columns-3-rows.csv data/id-column-3-rows.csv data/empty.txt 0
	cut_the_id_field_using_--fields_with_space)

test("csv-cut --fields=id" data/3-columns-3-rows.csv data/id-column-3-rows.csv data/empty.txt 0
	cut_the_id_field_using_--fields_with_=)


test("csv-cut -f something" data/3-columns-3-rows.csv cut/something-column-3-rows.csv data/empty.txt 0
	cut_the_something_field_using_-f)

test("csv-cut --fields something" data/3-columns-3-rows.csv cut/something-column-3-rows.csv data/empty.txt 0
	cut_the_something_field_using_--fields_with_space)

test("csv-cut --fields=something" data/3-columns-3-rows.csv cut/something-column-3-rows.csv data/empty.txt 0
	cut_the_something_field_using_--fields_with_=)


test("csv-cut -f name,something" data/3-columns-3-rows.csv data/2-columns-3-rows.csv data/empty.txt 0
	cut_2_fields_using_-f)

test("csv-cut -f name,something -s" data/3-columns-3-rows.csv cut/2-columns-3-rows.txt data/empty.txt 0
	cut_2_fields_using_-f_-s)

test("csv-cut --fields name,something" data/3-columns-3-rows.csv data/2-columns-3-rows.csv data/empty.txt 0
	cut_2_fields_using_--fields_with_spaces)

test("csv-cut --fields=name,something" data/3-columns-3-rows.csv data/2-columns-3-rows.csv data/empty.txt 0
	cut_2_fields_using_--fields_with_=)


test("csv-cut -f name,id,something" data/3-different-order-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	cut_3_fields_in_different_order_using_-f)

test("csv-cut --fields name,id,something" data/3-different-order-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	cut_3_fields_in_different_order_using_--fields_with_space)

test("csv-cut --fields=name,id,something" data/3-different-order-columns-3-rows.csv data/3-columns-3-rows.csv data/empty.txt 0
	cut_3_fields_in_different_order_using_--fields_with_=)

test("csv-cut --help" data/empty.csv cut/help.txt data/empty.txt 2
	cut_help)

test("csv-cut --version" data/empty.csv data/git-version.txt data/empty.txt 0
	cut_version)
