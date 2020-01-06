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

test("csv-add-substring -c name -n substr2 -p 2"
	data/3-columns-3-rows.csv add-substring/p2.csv data/empty.txt 0
	add-substring_p2)

test("csv-add-substring -c name -n substr2 -p 2 -s"
	data/3-columns-3-rows.csv add-substring/p2.txt data/empty.txt 0
	add-substring_p2_-s)

test("csv-add-substring -c name -n last_char -p -1 -l 1"
	data/3-columns-3-rows.csv add-substring/p-1.csv data/empty.txt 0
	add-substring_p_1)

test("csv-add-substring -T t1 -c name -p 2 -l 4 -n substr" add-substring/2-tables.csv add-substring/2-tables-add1.csv data/empty.txt 0
	add-substring_2tables-add1)

test("csv-add-substring --help" data/empty.csv add-substring/help.txt data/empty.txt 2
	add-substring_help)

test("csv-add-substring --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-substring_version)
