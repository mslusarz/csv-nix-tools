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

test("csv-add-split -c column3 -n before,after -e,"
	data/commas.csv add-split/commas.csv data/empty.txt 0
	add-split_commas)

test("csv-add-split -c column3 -n before,after -e, -r"
	data/commas.csv add-split/commas-reverse.csv data/empty.txt 0
	add-split_commas_reverse)

test("csv-add-split -c column3 -n before,after -e, -s"
	data/commas.csv add-split/commas.txt data/empty.txt 0
	add-split_commas_-s)

test("csv-add-split -T t1 -c name -e ' ' -n first_word,rest" add-split/2-tables.csv add-split/2-tables-add1.csv data/empty.txt 0
	add-split_2tables-add1)

test("csv-add-split --help" data/empty.csv add-split/help.txt data/empty.txt 2
	add-split_help)

test("csv-add-split --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-split_version)
