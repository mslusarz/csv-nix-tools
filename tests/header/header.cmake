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

test("csv-header" data/2-columns-3-rows.csv data/2-columns-3-rows.csv data/empty.txt 0
	header_pass)

test("csv-header -m" data/2-columns-3-rows.csv header/2-columns-3-rows-noheader.csv data/empty.txt 0
	header_remove)

test("csv-header -M" data/2-columns-3-rows.csv header/2-columns-3-rows-notypes.csv data/empty.txt 0
	header_remove_types)

test("csv-header -e name:string,something:int" header/2-columns-3-rows-notypes.csv data/2-columns-3-rows.csv data/empty.txt 0
	header_set_types)

test("csv-header -n name,newname -n something,somethingelse" data/2-columns-3-rows.csv header/rename.csv data/empty.txt 0
	header_rename)

test("csv-header -s" data/2-columns-3-rows.csv header/2-columns-3-rows.txt data/empty.txt 0
	header_-s)

test("csv-header --help" data/empty.csv header/help.txt data/empty.txt 2
	header_help)

test("csv-header --version" data/empty.csv data/git-version.txt data/empty.txt 0
	header_version)
