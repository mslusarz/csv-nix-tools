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

test("csv-show" data/3-columns-3-rows.csv show/3-columns-3-rows.txt data/empty.txt 0
	show_3_cols)

test("csv-show --ui=less" data/3-columns-3-rows.csv show/3-columns-3-rows.txt data/empty.txt 0
	show_3_cols_less)

test("csv-show" data/newlines.csv show/newlines.txt data/empty.txt 0
	show_newlines)

test("csv-show" data/quotes.csv show/quotes.txt data/empty.txt 0
	show_quotes)

test("csv-show" data/commas.csv show/commas.txt data/empty.txt 0
	show_commas)

test("csv-show" data/control-characters.csv show/control-characters.txt data/empty.txt 0
	show_control_characters)

if (TMUX)
# crazy test using tmux
add_test(NAME show_curses
	COMMAND ${CMAKE_COMMAND} --trace-expand
		-DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
		-DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}/..
		-DTMUX=${TMUX}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/show/curses.cmake)

set_tests_properties(show_curses PROPERTIES
		ENVIRONMENT "LC_ALL=C;PATH=${CMAKE_CURRENT_BINARY_DIR}/..:$ENV{PATH};")
endif()

test("csv-show --help" data/empty.csv show/help.txt data/empty.txt 2
	show_help)

test("csv-show --version" data/empty.csv data/git-version.txt data/empty.txt 0
	show_version)
