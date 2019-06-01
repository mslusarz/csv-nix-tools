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

add_library(csvshared STATIC src/parse.c src/utils.c)
add_library(csvrpn STATIC src/rpn_eval.c src/rpn_parse.c)

function(build_tool name)
	add_executable(${name} ${ARGN})
	target_link_libraries(${name} csvshared)
	install(FILES ${CMAKE_BINARY_DIR}/${name}
		DESTINATION ${CMAKE_INSTALL_BINDIR}/)
endfunction()

build_tool(csv-cat		src/cat.c)
build_tool(csv-concat		src/concat.c)
build_tool(csv-count		src/count.c)
build_tool(csv-cut		src/cut.c)
build_tool(csv-exec		src/exec.c)
build_tool(csv-exec-add		src/exec-add.c)
build_tool(csv-grep		src/grep.c)
build_tool(csv-groups		src/groups.c src/usr-grp.c)
build_tool(csv-head		src/head.c)
build_tool(csv-ls		src/ls.c)
build_tool(csv-rpn-add		src/rpn-add.c)
build_tool(csv-rpn-filter	src/rpn-filter.c)
build_tool(csv-show		src/show.c)
build_tool(csv-sort		src/sort.c)
build_tool(csv-split		src/split.c)
build_tool(csv-substring	src/substring.c)
build_tool(csv-sum		src/sum.c)
build_tool(csv-tac		src/tac.c)
build_tool(csv-tail		src/tail.c)
build_tool(csv-users		src/users.c src/usr-grp.c)

target_link_libraries(csv-rpn-add	csvrpn)
target_link_libraries(csv-rpn-filter	csvrpn)

if(SQLITE3_FOUND)
	build_tool(csv-sqlite src/sqlite.c src/usr-grp.c src/sqlite-usr-grp.c)
	if (SQLITE3_INCLUDE_DIRS)
		target_include_directories(csv-sqlite ${SQLITE3_INCLUDE_DIRS})
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
