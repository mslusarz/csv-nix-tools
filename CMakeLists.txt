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

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_executable(csv-columns src/columns.c src/parse.c)
add_executable(csv-cut src/cut.c src/parse.c)
add_executable(csv-grep src/grep.c src/parse.c src/utils.c)
add_executable(csv-head src/head.c src/parse.c src/utils.c)
add_executable(csv-ls src/ls.c src/utils.c)
add_executable(csv-rows src/rows.c src/parse.c)
add_executable(csv-show src/show.c src/parse.c src/utils.c)
add_executable(csv-sort src/sort.c src/parse.c src/utils.c)
add_executable(csv-split src/split.c src/parse.c src/utils.c)
add_executable(csv-substring src/substring.c src/parse.c src/utils.c)
add_executable(csv-sum src/sum.c src/parse.c src/utils.c)
add_executable(csv-tail src/tail.c src/parse.c src/utils.c)

install(FILES
		${CMAKE_BINARY_DIR}/csv-columns
		${CMAKE_BINARY_DIR}/csv-cut
		${CMAKE_BINARY_DIR}/csv-grep
		${CMAKE_BINARY_DIR}/csv-head
		${CMAKE_BINARY_DIR}/csv-ls
		${CMAKE_BINARY_DIR}/csv-rows
		${CMAKE_BINARY_DIR}/csv-show
		${CMAKE_BINARY_DIR}/csv-sort
		${CMAKE_BINARY_DIR}/csv-split
		${CMAKE_BINARY_DIR}/csv-substring
		${CMAKE_BINARY_DIR}/csv-sum
		${CMAKE_BINARY_DIR}/csv-tail
	DESTINATION ${CMAKE_INSTALL_BINDIR}/)
