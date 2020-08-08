#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

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

include(CheckCSourceCompiles)
include(FindThreads)
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
	pkg_check_modules(NCURSESW ncursesw)
else()
	find_package(NCURSESW)
endif()

if(NOT NCURSESW_FOUND)
	message(WARNING "ncursesw not found. csv-show won't support curses UI.")
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

#find_program(GNUPLOT gnuplot)
#if (NOT GNUPLOT)
#	message(WARNING "gnuplot not found. csv-plot's gnuplot-based test won't be executed.")
#endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wsign-conversion")

check_c_source_compiles("#ifdef __STDC_NO_THREADS__\n#error no threads\n#endif\nint main(){return 0;}" C11THREADS_FOUND)
if (NOT C11THREADS_FOUND)
	message(WARNING "C11 threads.h not available. csv-diff won't be built.")
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

option(COVERAGE "Code coverage tracking" OFF)
if(COVERAGE)
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

add_library(csvshared STATIC src/parse.c src/ht_posix.c src/ht_glibc.c src/utils.c src/utils_glibc.c)
add_library(csvrpn STATIC src/rpn_eval.c src/rpn_parse.c src/regex_cache.c src/ht_posix.c src/ht_glibc.c)

function(build_tool name)
	add_executable(${name} ${ARGN})
	target_link_libraries(${name} csvshared)
	install(FILES ${CMAKE_BINARY_DIR}/${name}
		DESTINATION ${CMAKE_INSTALL_BINDIR}/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
				GROUP_READ GROUP_EXECUTE
				WORLD_READ WORLD_EXECUTE)
endfunction()

build_tool(csv-add-concat	src/add-concat.c)
build_tool(csv-add-exec		src/add-exec.c)
build_tool(csv-add-replace	src/add-replace.c src/regex_cache.c)
build_tool(csv-add-rev		src/add-rev.c)
build_tool(csv-add-rpn		src/add-rpn.c)
build_tool(csv-add-split	src/add-split.c)
build_tool(csv-add-substring	src/add-substring.c)
build_tool(csv-avg		src/avg.c)
build_tool(csv-cat		src/cat.c)
build_tool(csv-count		src/count.c)
build_tool(csv-cut		src/cut.c)
build_tool(csv-env		src/env.c src/merge_utils.c)
build_tool(csv-exec		src/exec.c)
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
build_tool(csv-plot		src/plot.c)
build_tool(csv-sort		src/sort.c)
build_tool(csv-sum		src/sum.c)
build_tool(csv-tac		src/tac.c)
build_tool(csv-tail		src/tail.c)
build_tool(csv-to-html		src/to-html.c)
build_tool(csv-to-json		src/to-json.c)
build_tool(csv-to-xml		src/to-xml.c)
build_tool(csv-uniq		src/uniq.c)
build_tool(csv-users		src/users.c src/usr-grp.c src/merge_utils.c)

target_link_libraries(csv-add-rpn	csvrpn)
target_link_libraries(csv-grep-rpn	csvrpn)

if (C11THREADS_FOUND)
	build_tool(csv-diff		src/diff.c)
	target_link_libraries(csv-diff ${CMAKE_THREAD_LIBS_INIT})
endif()

if(NCURSESW_FOUND)
	if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
		link_directories(${NCURSESW_LIBRARY_DIRS})
	endif()

	build_tool(csv-show src/show.c src/show_ncurses.c src/show_static.c)
	target_compile_definitions(csv-show PRIVATE NCURSESW_ENABLED)

	if (NCURSESW_INCLUDE_DIRS)
		target_include_directories(csv-show PRIVATE ${NCURSESW_INCLUDE_DIRS})
	endif()

	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
		target_link_directories(csv-show PRIVATE ${NCURSESW_LIBRARY_DIRS})
	endif()

	if (NOT(NCURSESW_VERSION VERSION_LESS 6.2))
		target_compile_definitions(csv-show PRIVATE NCURSESW62_OR_LATER)
	endif()

	target_link_libraries(csv-show ${NCURSESW_LIBRARIES})
else()
	build_tool(csv-show src/show.c src/show_static.c)
endif()

# disable sign-compare/conversion warnings for csv-show because ncurses API is awful WRT signedness
set_target_properties(csv-show PROPERTIES COMPILE_FLAGS "-Wno-sign-conversion -Wno-sign-compare")

if(SQLITE3_FOUND)
	if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
		link_directories(${SQLITE3_LIBRARY_DIRS})
	endif()

	build_tool(csv-sqlite src/sqlite.c)

	if (SQLITE3_INCLUDE_DIRS)
		target_include_directories(csv-sqlite PRIVATE ${SQLITE3_INCLUDE_DIRS})
	endif()

	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
		target_link_directories(csv-sqlite PRIVATE ${SQLITE3_LIBRARY_DIRS})
	endif()

	target_link_libraries(csv-sqlite ${SQLITE3_LIBRARIES})
endif()

find_package(BISON)
find_package(FLEX)

if (BISON_FOUND AND FLEX_FOUND)
	set(BUILD_SQL 1)
else()
	set(BUILD_SQL 0)
	message(WARNING "bison or flex not found. csv-sql, csv-grep-sql and csv-add-sql won't be built.")
endif()

if (BUILD_SQL)
	set(PROG query)
	configure_file(${CMAKE_SOURCE_DIR}/src/sql.y.in ${CMAKE_CURRENT_BINARY_DIR}/sql.y)
	set(PROG condition)
	configure_file(${CMAKE_SOURCE_DIR}/src/sql.y.in ${CMAKE_CURRENT_BINARY_DIR}/grep-sql.y)
	set(PROG columns)
	configure_file(${CMAKE_SOURCE_DIR}/src/sql.y.in ${CMAKE_CURRENT_BINARY_DIR}/add-sql.y)

	FLEX_TARGET(SQLLexer src/sql.l ${CMAKE_CURRENT_BINARY_DIR}/sql_lexer.c)
	BISON_TARGET(SQLParser ${CMAKE_CURRENT_BINARY_DIR}/sql.y ${CMAKE_CURRENT_BINARY_DIR}/sql_parser.c)
	BISON_TARGET(GREPSQLParser ${CMAKE_CURRENT_BINARY_DIR}/grep-sql.y ${CMAKE_CURRENT_BINARY_DIR}/grep-sql_parser.c
			COMPILE_FLAGS -Wno-other) # -Wno-other to quiet down wanings about unused rules which *are* used in the full parser
	BISON_TARGET(ADDSQLParser ${CMAKE_CURRENT_BINARY_DIR}/add-sql.y ${CMAKE_CURRENT_BINARY_DIR}/add-sql_parser.c
			COMPILE_FLAGS -Wno-other) # -Wno-other to quiet down wanings about unused rules which *are* used in the full parser
	ADD_FLEX_BISON_DEPENDENCY(SQLLexer SQLParser GREPSQLParser ADDSQLParser)

	build_tool(csv-sql src/sql.c
			${BISON_SQLParser_OUTPUTS}
			${FLEX_SQLLexer_OUTPUTS})

	build_tool(csv-grep-sql src/grep-sql.c
			${BISON_GREPSQLParser_OUTPUTS}
			${FLEX_SQLLexer_OUTPUTS})

	build_tool(csv-add-sql src/add-sql.c
			${BISON_ADDSQLParser_OUTPUTS}
			${FLEX_SQLLexer_OUTPUTS})

	target_include_directories(csv-sql PRIVATE src)
	target_include_directories(csv-grep-sql PRIVATE src)
	target_include_directories(csv-add-sql PRIVATE src)

	target_link_libraries(csv-sql csvrpn)
	target_link_libraries(csv-grep-sql csvrpn)
	target_link_libraries(csv-add-sql csvrpn)
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
			COMMAND cat ${CMAKE_SOURCE_DIR}/doc/csv-${name}.${section}.md |
				grep -v "^SPDX-License-Identifier" |
				grep -v "^Copyright" |
				grep -v "^\\\<!--$" |
				grep -v "^--\\\>$" |
				sed 's/\\[\\\(\[a-z0-9-\]\*\\\)\\]\(http\[s\]*:\[^\)\]*\)/\\1/g' |
				${PANDOC} -s -t man -o ${CMAKE_BINARY_DIR}/csv-${name}.${section})

	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/csv-${name}.${section}.html
			MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/doc/csv-${name}.${section}.md
			COMMAND cat ${CMAKE_SOURCE_DIR}/doc/csv-${name}.${section}.md |
				grep -v "^SPDX-License-Identifier" |
				grep -v "^Copyright" |
				grep -v "^\\\<!--$" |
				grep -v "^--\\\>$" |
				sed 's/\\\*\\\*\\\(csv-[a-z-]*\\\)\\\*\\\*\(\\\([0-9]*\\\)\)/**\[\\1\]\(\\1.\\2.html\)**\(\\2\)/g' |
				${PANDOC} -s -t html -o ${CMAKE_BINARY_DIR}/csv-${name}.${section}.html)

	add_custom_target(man-csv-${name}.${section}
			DEPENDS ${CMAKE_BINARY_DIR}/csv-${name}.${section}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

	add_custom_target(html-csv-${name}.${section}
			DEPENDS ${CMAKE_BINARY_DIR}/csv-${name}.${section}.html
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

	add_dependencies(manpages man-csv-${name}.${section})

	add_dependencies(manpages-html html-csv-${name}.${section})

	install(FILES ${CMAKE_BINARY_DIR}/csv-${name}.${section}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man${section})
endfunction()

function(gen1_doc name)
	gen_doc(${name} 1)
endfunction()

if(PANDOC)
	add_custom_target(manpages ALL)
	add_custom_target(manpages-html)

	gen_doc(nix-tools 7)
	gen_doc(tables-tut 7)

	gen1_doc(add-concat)
	gen1_doc(add-exec)
	gen1_doc(add-replace)
	gen1_doc(add-rev)
	gen1_doc(add-rpn)
	gen1_doc(add-split)
if (BUILD_SQL)
	gen1_doc(add-sql)
endif()
	gen1_doc(add-substring)
	gen1_doc(avg)
	gen1_doc(cat)
	gen1_doc(count)
	gen1_doc(cut)
if (C11THREADS_FOUND)
	gen1_doc(diff)
endif()
	gen1_doc(env)
	gen1_doc(exec)
	gen1_doc(grep)
	gen1_doc(grep-rpn)
if (BUILD_SQL)
	gen1_doc(grep-sql)
endif()
	gen1_doc(group-members)
	gen1_doc(groups)
	gen1_doc(head)
	gen1_doc(header)
	gen1_doc(ls)
	gen1_doc(max)
	gen1_doc(merge)
	gen1_doc(min)
if(LIBMNL_FOUND)
	gen1_doc(netstat)
endif()
	gen1_doc(plot)
if(LIBPROCPS_FOUND)
	gen1_doc(ps)
endif()
	gen1_doc(show)
	gen1_doc(sort)
if (BUILD_SQL)
	gen1_doc(sql)
endif()
if(SQLITE3_FOUND)
	gen1_doc(sqlite)
endif()
	gen1_doc(sum)
	gen1_doc(tac)
	gen1_doc(tail)
	gen1_doc(to-html)
	gen1_doc(to-json)
	gen1_doc(to-xml)
	gen1_doc(uniq)
	gen1_doc(users)
endif()

option(TEST_UNDER_MEMCHECK "Run tests under Valgrind memcheck" OFF)
option(TEST_UTF8 "Test UTF-8 support (requires en_US.UTF-8 locale)" ON)

enable_testing()
add_subdirectory(tests)
