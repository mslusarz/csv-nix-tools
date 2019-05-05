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

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_executable(csv-ls ls.c)
add_executable(csv-format format.c)
add_executable(csv-grep grep.c parse.c)

install(FILES ${CMAKE_BINARY_DIR}/csv-ls
	DESTINATION ${CMAKE_INSTALL_BINDIR}/)
install(FILES ${CMAKE_BINARY_DIR}/csv-grep
	DESTINATION ${CMAKE_INSTALL_BINDIR}/)
#install(FILES ${CMAKE_BINARY_DIR}/csv-format
#	DESTINATION ${CMAKE_INSTALL_BINDIR}/)

