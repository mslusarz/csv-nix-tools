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

cmake_minimum_required(VERSION 3.3)
project(csv-nix-tools C)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
		set(default_build_type "Debug")
	else()
		set(default_build_type "RelWithDebInfo")
	endif()

	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

include(GNUInstallDirs)

find_package(PkgConfig QUIET)
if(NOT PKG_CONFIG_FOUND)
	message(WARNING "Pkg-config not found. Detection of dependencies may fail.")
endif()

if(PKG_CONFIG_FOUND)
	pkg_check_modules(SQLITE3 sqlite3)
else()
	find_package(SQLITE3)
endif()

if(NOT SQLITE3_FOUND)
	message(WARNING "SQLite3 not found. csv-sqlite won't be built.")
endif()

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBPROCPS libprocps)
else()
	find_package(LIBPROCPS)
endif()

if(NOT LIBPROCPS_FOUND)
	message(WARNING "libprocps not found. csv-ps won't be built.")
endif()

if(PKG_CONFIG_FOUND)
	pkg_check_modules(NCURSES ncurses)
else()
	find_package(NCURSES)
endif()

if(NOT NCURSES_FOUND)
	message(WARNING "ncurses not found. csv-show won't support curses UI.")
endif()

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBMNL libmnl)
else()
	find_package(LIBMNL)
endif()

if(NOT LIBMNL_FOUND)
	message(WARNING "libmnl not found. csv-netstat won't be built.")
endif()

find_program(TMUX tmux)
if (NOT TMUX)
	message(WARNING "tmux not found. csv-show's tmux-based test won't be executed.")
endif()

find_program(PANDOC pandoc)
if (NOT PANDOC)
	message(WARNING "pandoc not found. Man pages won't be generated.")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

option(COVERAGE "Code coverage tracking" OFF)
if(COVERAGE)
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

add_library(csvshared STATIC src/parse.c src/utils.c)
add_library(csvrpn STATIC src/rpn_eval.c src/rpn_parse.c)

function(build_tool name)
	add_executable(${name} ${ARGN})
	target_link_libraries(${name} csvshared)
	install(FILES ${CMAKE_BINARY_DIR}/${name}
		DESTINATION ${CMAKE_INSTALL_BINDIR}/)
endfunction()

build_tool(csv-avg		src/avg.c)
build_tool(csv-cat		src/cat.c)
build_tool(csv-concat		src/concat.c)
build_tool(csv-count		src/count.c)
build_tool(csv-cut		src/cut.c)
build_tool(csv-exec		src/exec.c)
build_tool(csv-exec-add		src/exec-add.c)
build_tool(csv-grep		src/grep.c)
build_tool(csv-grep-rpn		src/grep-rpn.c)
build_tool(csv-groups		src/groups.c src/usr-grp.c src/merge_utils.c)
build_tool(csv-group-members	src/group-members.c src/usr-grp.c src/merge_utils.c)
build_tool(csv-head		src/head.c)
build_tool(csv-header		src/header.c)
build_tool(csv-ls		src/ls.c src/usr-grp-query.c src/merge_utils.c)
build_tool(csv-min		src/min.c)
build_tool(csv-max		src/max.c)
build_tool(csv-merge		src/merge.c)
build_tool(csv-replace		src/replace.c)
build_tool(csv-rpn-add		src/rpn-add.c)
build_tool(csv-sort		src/sort.c)
build_tool(csv-split		src/split.c)
build_tool(csv-substring	src/substring.c)
build_tool(csv-sum		src/sum.c)
build_tool(csv-tac		src/tac.c)
build_tool(csv-tail		src/tail.c)
build_tool(csv-uniq		src/uniq.c)
build_tool(csv-users		src/users.c src/usr-grp.c src/merge_utils.c)

target_link_libraries(csv-rpn-add	csvrpn)
target_link_libraries(csv-grep-rpn	csvrpn)

if(NCURSES_FOUND)
	if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
		link_directories(${NCURSES_LIBRARY_DIRS})
	endif()

	build_tool(csv-show src/show.c)
	target_compile_definitions(csv-show PRIVATE NCURSES_ENABLED)

	if (NCURSES_INCLUDE_DIRS)
		target_include_directories(csv-show ${NCURSES_INCLUDE_DIRS})
	endif()

	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
		target_link_directories(csv-show PRIVATE ${NCURSES_LIBRARY_DIRS})
	endif()

	target_link_libraries(csv-show ${NCURSES_LIBRARIES})
else()
	build_tool(csv-show src/show.c)
endif()

if(SQLITE3_FOUND)
	if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
		link_directories(${SQLITE3_LIBRARY_DIRS})
	endif()

	build_tool(csv-sqlite src/sqlite.c)

	if (SQLITE3_INCLUDE_DIRS)
		target_include_directories(csv-sqlite ${SQLITE3_INCLUDE_DIRS})
	endif()

	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
		target_link_directories(csv-sqlite PRIVATE ${SQLITE3_LIBRARY_DIRS})
	endif()

	target_link_libraries(csv-sqlite ${SQLITE3_LIBRARIES})
endif()

find_package(BISON)
find_package(FLEX)

if (BISON_FOUND AND FLEX_FOUND)
	FLEX_TARGET(SQLLexer src/sql.l ${CMAKE_CURRENT_BINARY_DIR}/sql_lexer.c)
	BISON_TARGET(SQLParser src/sql.y ${CMAKE_CURRENT_BINARY_DIR}/sql_parser.c)
	ADD_FLEX_BISON_DEPENDENCY(SQLLexer SQLParser)

	build_tool(csv-sql src/sql.c
			${BISON_SQLParser_OUTPUTS}
			${FLEX_SQLLexer_OUTPUTS})

	target_include_directories(csv-sql PRIVATE src)

	target_link_libraries(csv-sql ${FLEX_LIBRARIES})
	target_link_libraries(csv-sql csvrpn)
endif()

if(LIBPROCPS_FOUND)
	if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
		link_directories(${LIBPROCPS_LIBRARY_DIRS})
	endif()

	build_tool(csv-ps src/ps.c src/usr-grp-query.c src/merge_utils.c)
	if (LIBPROCPS_INCLUDE_DIRS)
		target_include_directories(csv-ps PRIVATE ${LIBPROCPS_INCLUDE_DIRS})
	endif()

	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
		target_link_directories(csv-ps PRIVATE ${LIBPROCPS_LIBRARY_DIRS})
	endif()

	target_link_libraries(csv-ps ${LIBPROCPS_LIBRARIES})
endif()

if(LIBMNL_FOUND)
	if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
		link_directories(${LIBMNL_LIBRARY_DIRS})
	endif()

	build_tool(csv-netstat src/netstat.c src/merge_utils.c)
	if (LIBMNL_INCLUDE_DIRS)
		target_include_directories(csv-netstat PRIVATE ${LIBMNL_INCLUDE_DIRS})
	endif()

	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
		target_link_directories(csv-netstat PRIVATE ${LIBMNL_LIBRARY_DIRS})
	endif()

	target_link_libraries(csv-netstat ${LIBMNL_LIBRARIES})
endif()

function(gen_doc name section)
	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/csv-${name}.${section}
			MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/doc/csv-${name}.${section}.md
			COMMAND ${PANDOC} -s -t man -o ${CMAKE_BINARY_DIR}/csv-${name}.${section} ${CMAKE_SOURCE_DIR}/doc/csv-${name}.${section}.md)

	add_custom_target(doc-csv-${name}
			DEPENDS ${CMAKE_BINARY_DIR}/csv-${name}.${section}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

	add_dependencies(doc doc-csv-${name})

	install(FILES ${CMAKE_BINARY_DIR}/csv-${name}.${section}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man${section})
endfunction()

function(gen1_doc name)
	gen_doc(${name} 1)
endfunction()

if(PANDOC)
	add_custom_target(doc ALL)

	gen_doc(nix-tools 7)

	gen1_doc(cat)
	gen1_doc(head)
	gen1_doc(show)
	gen1_doc(tac)
	gen1_doc(tail)
endif()

option(TEST_UNDER_MEMCHECK "Run tests under Valgrind memcheck" OFF)

enable_testing()
add_subdirectory(tests)
