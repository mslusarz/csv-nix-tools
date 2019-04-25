cmake_minimum_required(VERSION 3.3)
project(st C)

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
	pkg_check_modules(LIBXML REQUIRED libxml-2.0)
else()
	find_package(LIBXML REQUIRED 2.0)
endif()

set(CMAKE_C_STANDARD 99)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/st-ls.pc
	DESTINATION ${CMAKE_INSTALL_BINDIR}/)

add_executable(st-ls st-ls.c)
add_executable(st-format st-format.c)
add_executable(st-grep st-grep.c)

target_include_directories(st-format PUBLIC ${LIBXML_INCLUDE_DIRS})
target_link_libraries(st-format ${LIBXML_LIBRARIES})

target_include_directories(st-grep PUBLIC ${LIBXML_INCLUDE_DIRS})
target_link_libraries(st-grep ${LIBXML_LIBRARIES})
