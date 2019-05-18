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

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_executable(csv-cat src/cat.c src/parse.c src/utils.c)
add_executable(csv-columns src/columns.c src/parse.c)
add_executable(csv-concat src/concat.c src/parse.c src/utils.c)
add_executable(csv-cut src/cut.c src/parse.c src/utils.c)
add_executable(csv-grep src/grep.c src/parse.c src/utils.c)
add_executable(csv-head src/head.c src/parse.c src/utils.c)
add_executable(csv-ls src/ls.c src/utils.c)
add_executable(csv-rows src/rows.c src/parse.c)
add_executable(csv-rpn-add src/rpn-add.c src/parse.c src/rpn_eval.c src/rpn_parse.c src/utils.c)
add_executable(csv-rpn-filter src/rpn-filter.c src/parse.c src/rpn_eval.c src/rpn_parse.c src/utils.c)
add_executable(csv-show src/show.c src/parse.c src/utils.c)
add_executable(csv-sort src/sort.c src/parse.c src/utils.c)
add_executable(csv-split src/split.c src/parse.c src/utils.c)
add_executable(csv-substring src/substring.c src/parse.c src/utils.c)
add_executable(csv-sum src/sum.c src/parse.c src/utils.c)
add_executable(csv-tac src/tac.c src/parse.c src/utils.c)
add_executable(csv-tail src/tail.c src/parse.c src/utils.c)

if(SQLITE3_FOUND)
	add_executable(csv-sqlite src/sqlite.c src/parse.c src/utils.c)
	if (SQLITE3_INCLUDE_DIRS)
		target_include_directories(csv-sqlite ${SQLITE3_INCLUDE_DIRS})
	endif()
	target_link_libraries(csv-sqlite ${SQLITE3_LIBRARIES})
endif()

install(FILES
		${CMAKE_BINARY_DIR}/csv-cat
		${CMAKE_BINARY_DIR}/csv-columns
		${CMAKE_BINARY_DIR}/csv-concat
		${CMAKE_BINARY_DIR}/csv-cut
		${CMAKE_BINARY_DIR}/csv-grep
		${CMAKE_BINARY_DIR}/csv-head
		${CMAKE_BINARY_DIR}/csv-ls
		${CMAKE_BINARY_DIR}/csv-rows
		${CMAKE_BINARY_DIR}/csv-rpn-add
		${CMAKE_BINARY_DIR}/csv-rpn-filter
		${CMAKE_BINARY_DIR}/csv-show
		${CMAKE_BINARY_DIR}/csv-sort
		${CMAKE_BINARY_DIR}/csv-split
		${CMAKE_BINARY_DIR}/csv-substring
		${CMAKE_BINARY_DIR}/csv-sum
		${CMAKE_BINARY_DIR}/csv-tac
		${CMAKE_BINARY_DIR}/csv-tail
	DESTINATION ${CMAKE_INSTALL_BINDIR}/)

if(SQLITE3_FOUND)
	install(FILES ${CMAKE_BINARY_DIR}/csv-sqlite
		DESTINATION ${CMAKE_INSTALL_BINDIR}/)
endif()
