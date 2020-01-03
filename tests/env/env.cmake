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

test("csv-env | csv-count -c -R"
	data/empty.txt env/columns.csv data/empty.txt 0
	env_count)

test("csv-env -c value | csv-count -c -R"
	data/empty.txt env/columns-only-value.csv data/empty.txt 0
	env_-c)

test("csv-env -M | csv-head -n 0" data/3-columns-3-rows-with-table.csv env/columns-merged.csv data/empty.txt 0
	env_merged)

test("csv-env -M -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv env/columns-merged-table.csv data/empty.txt 0
	env_merged_table)

test("csv-env --help" data/empty.csv env/help.txt data/empty.txt 2
	env_help)

test("csv-env --version" data/empty.csv data/git-version.txt data/empty.txt 0
	env_version)
