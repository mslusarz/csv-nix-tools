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

set(TMP_DIR ${BIN_DIR}/tests/${NAME})

execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${TMP_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR})
if (CWD STREQUAL "")
	set(CWD ${TMP_DIR})
endif()

if (TEST_UNDER_MEMCHECK)
	set(CMD "valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=123 --log-file='${TMP_DIR}/${NAME}.memcheck' ${CMD}")
endif()

set(ENV{XDG_CACHE_HOME} ${TMP_DIR})

execute_process(COMMAND sh -c "${CMD}"
		WORKING_DIRECTORY "${CWD}"
		INPUT_FILE ${SRC_DIR}/${INPUT}
		OUTPUT_FILE ${TMP_DIR}/${NAME}.stdout
		ERROR_FILE ${TMP_DIR}/${NAME}.stderr
		RESULT_VARIABLE res)
if (NOT res EQUAL ${EXPECTED_RES})
	file(READ ${TMP_DIR}/${NAME}.stdout DATA)
	message(STATUS "Stdout:\n${DATA}\nEnd of stdout")

	file(READ ${TMP_DIR}/${NAME}.stderr DATA)
	message(STATUS "Stderr:\n${DATA}\nEnd of stderr")

	if (TEST_UNDER_MEMCHECK AND res EQUAL 123)
		file(READ ${TMP_DIR}/${NAME}.memcheck DATA)
		message(STATUS "Memcheck:\n${DATA}\nEnd of memcheck")
	endif()

	message(FATAL_ERROR "Unexpected exit code: ${res} != ${EXPECTED_RES}")
endif()

if (STDOUT)
	execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files
				${TMP_DIR}/${NAME}.stdout
				${SRC_DIR}/${STDOUT}
			RESULT_VARIABLE res)
	if (NOT res EQUAL 0)
		file(READ ${TMP_DIR}/${NAME}.stdout DATA)
		message(STATUS "Stdout:\n${DATA}\nEnd of stdout")

		file(READ ${SRC_DIR}/${STDOUT} DATA)
		message(STATUS "Expected stdout:\n${DATA}\nEnd of expected stdout")

		execute_process(COMMAND diff -u ${SRC_DIR}/${STDOUT} ${TMP_DIR}/${NAME}.stdout)

		message(FATAL_ERROR "stdout differ")
	endif()
endif()

if (STDERR)
	execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files
				${TMP_DIR}/${NAME}.stderr
				${SRC_DIR}/${STDERR}
			RESULT_VARIABLE res)
	if (NOT res EQUAL 0)
		file(READ ${TMP_DIR}/${NAME}.stderr DATA)
		message(STATUS "Stderr:\n${DATA}\nEnd of stderr")

		file(READ ${SRC_DIR}/${STDERR} DATA)
		message(STATUS "Expected stderr:\n${DATA}\nEnd of expected stderr")

		execute_process(COMMAND diff -u ${SRC_DIR}/${STDERR} ${TMP_DIR}/${NAME}.stderr)

		message(FATAL_ERROR "stderr differ")
	endif()
endif()
