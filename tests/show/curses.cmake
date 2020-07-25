#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

set(TMP_DIR ${BIN_DIR}/tests/show_ncurses)

execute_process(COMMAND sh -c "${TMUX} -S ${TMP_DIR}/tmux kill-session")

execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${TMP_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR})

set(LOG ${TMP_DIR}/log)
execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${LOG})

function(get_file_size file)
	execute_process(COMMAND stat -c%s ${file}
			OUTPUT_VARIABLE size
			RESULT_VARIABLE res)
	if (NOT res EQUAL 0)
		message(FATAL_ERROR "unable to stat file '${file}'")
	endif()
	set(file_size ${size} PARENT_SCOPE)
endfunction()

function(fatal msg)
	execute_process(COMMAND sh -c "${TMUX} -S ${TMP_DIR}/tmux kill-session")

	file(READ ${TMP_DIR}/stderr DATA)
	message(STATUS "Stderr:\n${DATA}\nEnd of stderr")

	message(FATAL_ERROR "${msg}")
endfunction()

function(run_only args)
	execute_process(COMMAND sh -c "${TMUX} -S ${TMP_DIR}/tmux ${args}"
			WORKING_DIRECTORY ${SRC_DIR}
			RESULT_VARIABLE res)
	if (NOT res EQUAL 0)
		fatal("'${args}': failed with exit code: ${res}")
	endif()
endfunction()

function(run args)
	run_only(${args})

	set(expected_file_size ${cmd_num})
	math(EXPR expected_file_size "${expected_file_size} * 4")

	get_file_size(${LOG})
	message(STATUS "size: ${file_size}, expected: ${expected_file_size}")

	while (NOT file_size EQUAL expected_file_size)
		execute_process(COMMAND sh -c "sleep 0.01")

		execute_process(COMMAND sh -c "${TMUX} -S ${TMP_DIR}/tmux has-session"
				RESULT_VARIABLE res)
		if (NOT res EQUAL 0)
			fatal("has-session unexpected exit code: ${res} != 0")
		endif()
		get_file_size(${LOG})
		message(STATUS "size: ${file_size}, expected: ${expected_file_size}")
	endwhile()

	math(EXPR cmd_num2 "${cmd_num} + 1")
	set(cmd_num ${cmd_num2} PARENT_SCOPE)
endfunction()

set(cmd_num 1)

run("new-session -d sh -c 'csv-ls -lR | csv-show --ui=curses --debug ${LOG} 2>${TMP_DIR}/stderr'")
run("send-keys 'Up'")
run("send-keys 'Down'")
run("send-keys 'Up'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'NPage'")
run("send-keys 'Down'")
run("send-keys 'Right'")
run("send-keys 'Right'")
run("send-keys 'Right'")
run("send-keys 'Right'")
run("send-keys 'Right'")
run("send-keys 'Right'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'PPage'")
run("send-keys 'Left'")
run("send-keys 'Left'")
run("send-keys 'Left'")
run("send-keys 'Left'")
run("send-keys 'Left'")
run("send-keys 'Left'")
run("send-keys 'Left'")
run("send-keys 'End'")
run("send-keys 'End'")
run("send-keys ']'")
run("send-keys 'Right'")
run("send-keys 'Home'")
run("send-keys 'Home'")
run("send-keys 'Left'")
run("send-keys ']'")
run("send-keys ']'")
run("send-keys '['")
run("send-keys '['")
run("send-keys '['")

run_only("send-keys 'q'")

math(EXPR expected_file_size "${cmd_num} * 4")
get_file_size(${LOG})

while (NOT file_size EQUAL expected_file_size)
	execute_process(COMMAND sh -c "${TMUX} -S ${TMP_DIR}/tmux has-session"
			RESULT_VARIABLE res)

	get_file_size(${LOG})
	message(STATUS "size: ${file_size}, expected: ${expected_file_size}")

	if (NOT res EQUAL 0)
		# if session doesn't exit then file size MUST match
		if (NOT file_size EQUAL expected_file_size)
			fatal("session exited but file size doesn't match")
		endif()
	endif()
endwhile()

execute_process(COMMAND sh -c "${TMUX} -S ${TMP_DIR}/tmux kill-session")
