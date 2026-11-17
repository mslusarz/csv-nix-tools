#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2026, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

cmake_minimum_required(VERSION 3.10)

set(root "${BIN_DIR}/tests/show_curses_${CASE}")
set(socket "${root}/tmux")
file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")

function(fail message)
	execute_process(COMMAND "${TMUX}" -S "${socket}" kill-server ERROR_QUIET)
	if (EXISTS "${run_dir}/stderr")
		file(READ "${run_dir}/stderr" stderr)
	endif()
	message(FATAL_ERROR "${run_dir}: ${message}\n${stderr}")
endfunction()

function(tmux)
	execute_process(COMMAND "${TMUX}" -f /dev/null -S "${socket}" ${ARGN}
		RESULT_VARIABLE result OUTPUT_VARIABLE output)
	if (NOT result EQUAL 0)
		fail("tmux ${ARGN} exited with ${result}")
	endif()
	set(pane "${output}" PARENT_SCOPE)
endfunction()

function(start name term width height input)
	set(run_dir "${root}/${name}")
	file(MAKE_DIRECTORY "${run_dir}")
	tmux(new-session -d -x "${width}" -y "${height}" -c "${run_dir}"
		env "TERM=${term}" sh -c [=[
			input=$1
			shift
			csv-show -S --debug redraw "$@" <"$input" >stdout 2>stderr
			printf '%s\n' "$?" >status.tmp
			mv status.tmp status
		]=] sh "${SRC_DIR}/${input}" ${ARGN})
	set(run_dir "${run_dir}" PARENT_SCOPE)
endfunction()

function(wait_for file)
	foreach(attempt RANGE 200)
		if (EXISTS "${run_dir}/${file}")
			if (file STREQUAL "status")
				return()
			endif()
			execute_process(COMMAND stat -c%s "${run_dir}/${file}"
				OUTPUT_VARIABLE size RESULT_VARIABLE result)
			if (result EQUAL 0 AND size GREATER_EQUAL ARGV1)
				return()
			endif()
		endif()
		if (file STREQUAL "redraw" AND EXISTS "${run_dir}/status")
			fail("csv-show exited before redrawing")
		endif()
		execute_process(COMMAND sleep 0.02)
	endforeach()
	fail("timed out waiting for ${file}")
endfunction()

function(check_file name expected)
	file(READ "${run_dir}/${name}" actual)
	if (NOT actual STREQUAL expected)
		fail("unexpected ${name}\nExpected:\n${expected}\nActual:\n${actual}")
	endif()
endfunction()

function(check_exit status stderr)
	wait_for(status)
	check_file(status "${status}\n")
	check_file(stderr "${stderr}")
endfunction()

function(check_pane expected)
	string(FIND "${pane}" "${expected}" index)
	if (index EQUAL -1)
		fail("pane missing '${expected}'\n${pane}")
	endif()
endfunction()

function(send_terminal_key capability)
	# Translate navigation keys using the same terminfo entry as csv-show.
	execute_process(COMMAND tput -T xterm-256color "${capability}"
		RESULT_VARIABLE result OUTPUT_VARIABLE sequence)
	if (NOT result EQUAL 0)
		fail("terminfo key ${capability} is unavailable")
	endif()
	string(REPLACE ";" "\\;" sequence "${sequence}")
	tmux(send-keys -l "${sequence}")
endfunction()

function(expect_error name term stderr)
	start("${name}" "${term}" 40 6 data/3-columns-3-rows.csv ${ARGN})
	check_exit(2 "${stderr}")
	check_file(stdout "")
endfunction()

if (CASE STREQUAL "interactive")
	start(selection xterm-256color 36 5 show/selection.csv --interactive
		--on-key "f1,save the selected row,cat >picked.csv,return"
		--on-key "f2,queue,stdout,return"
		--on-key "enter,select,stdout")
	wait_for(redraw 4)
	tmux(capture-pane -p)
	check_pane("F1=save the")
	check_pane("F2=queue")
	check_pane("ENTER=select")

	# Three data rows fit between the header and the key hints.
	send_terminal_key(khome)
	tmux(send-keys Up PPage j j j Up Up Up NPage j NPage NPage PPage PPage)
	send_terminal_key(kend)
	send_terminal_key(kend)
	tmux(send-keys Down F1)
	send_terminal_key(khome)
	tmux(send-keys j F2 j Enter)
	check_exit(0 "")
	check_file(picked.csv "name:string,id:int\nseventh,7\n")
	check_file(stdout "name:string,id:int\nsecond,2\nthird,3\n")

	start(default_selection xterm-256color 36 5 show/selection.csv --interactive --no-header
		--spacing 1)
	wait_for(redraw 4)
	tmux(capture-pane -p)
	check_pane("first   1")
	check_pane("ENTER=select")
	if (pane MATCHES "name +id")
		fail("column headers are visible\n${pane}")
	endif()
	send_terminal_key(kEND)
	tmux(send-keys Down Enter)
	check_exit(0 "")
	check_file(stdout "name:string,id:int\nsecond,2\n")
elseif (CASE STREQUAL "colors")
	start(table xterm-256color 24 8 show/colors.csv --with-types --use-color-columns
		--set-color "name:fg=white,bg=blue" --set-color "id:yellow")
	wait_for(redraw 4)
	tmux(capture-pane -p)
	check_pane("name:string   id:int")
	check_pane("alpha              7")
	check_pane("beta              42")
	if (pane MATCHES "_color")
		fail("color columns are visible\n${pane}")
	endif()

	string(ASCII 27 esc)
	tmux(capture-pane -p -e)
	check_pane("${esc}[37m${esc}[44mname")
	check_pane("${esc}[32m${esc}[40malpha")
	if (NOT pane MATCHES "${esc}\\[37m(${esc}\\[44m)?beta")
		fail("beta did not use the column's foreground color\n${pane}")
	endif()

	send_terminal_key(kEND)
	wait_for(redraw 8)
	tmux(capture-pane -p)
	check_pane("detail:string")
	check_pane("first")
	send_terminal_key(kEND)
	send_terminal_key(kHOM)
	send_terminal_key(kHOM)
	tmux(send-keys Down Space "]" Left Left "]" "]" "]" "[" "[" q)
	check_exit(0 "")
	check_file(stdout "")
elseif (CASE STREQUAL "errors")
	expect_error(unknown_key xterm-256color "unknown key 'TAB'\n"
		--interactive --on-key "tab,select,stdout")
	expect_error(invalid_function_key xterm-256color "value 'OO' is not an integer\n'OO' is not a number\n"
		--interactive --on-key "foo,select,stdout")
	expect_error(no_colors vt100 "Your terminal doesn't support colors\n"
		--set-color "name:red")
	expect_error(bad_color_option xterm-256color "Color not in name:spec format\n"
		--set-color "name")
	expect_error(unknown_column xterm-256color "column 'missing' not found\n"
		--set-color "missing:red")
	expect_error(invalid_rgb xterm-256color
		"Can't parse 'FF00G0' as color - it must be either a predefined name, in RRGGBB format or be a decimal number\n"
		--set-color "name:FF00G0")
	expect_error(invalid_color xterm-256color
		"value 'pink' is not an integer\nCan't parse 'pink' as color - it must be either a predefined name, in RRGGBB format or be a decimal number\n"
		--set-color "name:pink")
	expect_error(no_rgb xterm "Your terminal doesn't support changing colors\n"
		--set-color "name:FF0000")
	expect_error(excessive_spacing xterm-256color "max line length exceeds 2147483647\n"
		--spacing 2147483647)
endif()
