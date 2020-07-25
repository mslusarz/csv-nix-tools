#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

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
