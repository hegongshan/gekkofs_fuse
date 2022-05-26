################################################################################
# Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# This file is part of GekkoFS.                                                #
#                                                                              #
# GekkoFS is free software: you can redistribute it and/or modify              #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation, either version 3 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# GekkoFS is distributed in the hope that it will be useful,                   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.            #
#                                                                              #
# SPDX-License-Identifier: GPL-3.0-or-later                                    #
################################################################################

cmake_minimum_required(VERSION 3.6)

project(
    GekkoFS
    VERSION 0.9.1
)

enable_testing()

if (NOT CMAKE_COMPILER_IS_GNUCC)
    message(FATAL_ERROR "The choosen C compiler is not gcc and is not supported")
endif ()
if (NOT CMAKE_COMPILER_IS_GNUCXX)
    message(FATAL_ERROR "The choosen C++ compiler is not g++ and is not supported")
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Choose the type of build: Debug Release Memcheck" FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)
message(STATUS "[gekkofs] Build type: ${CMAKE_BUILD_TYPE}")

# Compiler flags for various cmake build types
set(WARNINGS_FLAGS "-Wall -Wextra --pedantic -Wno-unused-parameter -Wno-missing-field-initializers")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${WARNINGS_FLAGS} -g -O0 -DGKFS_DEBUG_BUILD -DHERMES_DEBUG_BUILD")
set(CMAKE_CXX_FLAGS_MEMCHECK "${WARNINGS_FLAGS} -g -O0 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_MAINTAINER "${WARNINGS_FLAGS} -g -O0 -pg -no-pie")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG -O3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${WARNINGS_FLAGS} -g -O0 -DGKFS_DEBUG_BUILD")
set(CMAKE_C_FLAGS_MEMCHECK "${WARNINGS_FLAGS} -g -O0 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_MAINTAINER "${WARNINGS_FLAGS} -g -O0 -pg -no-pie")
mark_as_advanced(CMAKE_CXX_FLAGS_MAINTAINER)

# CMake and general includes
include(CheckCXXSourceCompiles)
include(CMakeDependentOption)
include(GNUInstallDirs)

# Project version
set(GIT_VERSION_FOUND FALSE)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
    find_package(Git)
    if (Git_FOUND)
        execute_process(COMMAND /bin/bash -c "${GIT_EXECUTABLE} describe --tags --match='v*.*.*' --long --dirty"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_RET
            OUTPUT_VARIABLE GIT_VERSION
            ERROR_VARIABLE GIT_ERR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        if ("${GIT_RET}" STREQUAL "0")
            string(REGEX MATCH
                "^v([0-9]+)\.([0-9]+)\.([0-9]+)-([0-9]+)-(.*)$"
                DROP ${GIT_VERSION})
            set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
            set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
            set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
            set(GKFS_COMMIT_GAP ${CMAKE_MATCH_4})
            set(GKFS_COMMIT_HASH ${CMAKE_MATCH_5})
            set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-snapshot+${GKFS_COMMIT_GAP}-${GKFS_COMMIT_HASH}")
            set(GIT_VERSION_FOUND TRUE)
        else ()
            message(WARNING "Failed to run `git describe`: [${GIT_RET}] '${GIT_ERR}'")
        endif ()
    endif ()
endif ()
message(STATUS "[gekkofs] Project version: ${PROJECT_VERSION}")
set(GKFS_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(GKFS_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(GKFS_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(GKFS_VERSION_STRING ${PROJECT_VERSION})
configure_file(include/version.hpp.in include/version.hpp)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

# Import some convenience functions
include(gkfs-utils)

set(CMAKE_EXPORT_COMPILE_COMMANDS 0)


# ##############################################################################
# Check for and/or download dependencies:
#
# GekkoFS has two types of dependencies:
# 1. Dependencies that are installed as system-wide packages and provide
#    a FindXXX.cmake file to find them. These are handled using
#    `find_package()`.
# 2. Depedencies that are used directly from source code (e.g. header-only
#    libraries). The source code for these can be found locally or
#    remotely in a GIT repository. These are handled using our custom
#    `include_from_source()` function defined in `gkfs-utils.cmake`
# ##############################################################################

# Search for dependencies that should already be installed in the system
###############################################################################

### some dependencies don't provide CMake modules, but rely on pkg-config
### instead, make sure that pkg-config is available
find_package(PkgConfig REQUIRED)

### Mercury
message(STATUS "[${PROJECT_NAME}] Checking for Mercury")
find_package(Mercury 2.0.1 REQUIRED)

### Argobots
message(STATUS "[${PROJECT_NAME}] Checking for Argobots")
find_package(Argobots 1.1 REQUIRED)

### Margo
message(STATUS "[${PROJECT_NAME}] Checking for Margo")
find_package(Margo 0.9.6 REQUIRED)

### syscall-intercept
find_package(Syscall_intercept REQUIRED)

### Date (required by the libgkfs_intercept.so)
find_package(Date REQUIRED)

### AGIOS: required for scheduling I/O requests
if (GKFS_ENABLE_AGIOS)
    find_package(AGIOS REQUIRED)
endif ()

### Prometheus-cpp: required for the collection of GekkoFS stats
### (these expose the prometheus-cpp::pull, prometheus-cpp::push,
### prometheus-cpp::core, and curl imported targets
if(GKFS_ENABLE_PROMETHEUS)
    find_package(CURL 7.68.0 REQUIRED)
    find_package(prometheus-cpp REQUIRED) # >= 1.0.0
endif()

### Other stuff that can be found out using find_package:

# determine the thread library of the system
find_package(Threads REQUIRED)
# some compilers need extra flags for std::filesystem, such as -lstdc++fs, this
# produces a std::filesystem imported target that takes care of all these
# details transparently
find_package(Filesystem REQUIRED)


# Search for 'source-only' dependencies
###############################################################################

set(GIT_SUBMODULES_PATH ${CMAKE_SOURCE_DIR}/external)

### {fmt}: required for sensible output formatting
include_from_source(fmt
  MESSAGE "[${PROJECT_NAME}] Searching for {fmt}"
  SOURCE_DIR ${GIT_SUBMODULES_PATH}/fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG f94b7364b9409f05207c3af3fa4666730e11a854 # v6.1.2
)

# ensure that fmt is linked as PIC
set_property(TARGET fmt PROPERTY POSITION_INDEPENDENT_CODE ON)

### spdlog: required for logging
include_from_source(spdlog
  MESSAGE "[${PROJECT_NAME}] Searching for spdlog"
  SOURCE_DIR ${GIT_SUBMODULES_PATH}/spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG eb3220622e73a4889eee355ffa37972b3cac3df5 # v1.9.2
  )

### CLI11: used for parsing command-line options
include_from_source(cli11
  MESSAGE "[${PROJECT_NAME}] Searching for CLI11"
  SOURCE_DIR ${GIT_SUBMODULES_PATH}/CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG v2.2.0
  )
FetchContent_MakeAvailable(cli11)


################################################################################
## Build GekkoFS documentation
################################################################################
option(GKFS_BUILD_DOCUMENTATION "Build documentation" OFF)
if(GKFS_BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()

option(CREATE_CHECK_PARENTS "Check parent directory existance before creating child node" ON)
message(STATUS "[gekkofs] Create checks parents: ${CREATE_CHECK_PARENTS}")

option(GKFS_SYMLINK_SUPPORT "Compile with support for symlinks" OFF)
if (GKFS_SYMLINK_SUPPORT)
    add_definitions(-DHAS_SYMLINKS)
endif ()
option(GKFS_RENAME_SUPPORT "Compile with support for rename ops" OFF)
if (GKFS_RENAME_SUPPORT)
    # Rename depends on symlink support
    add_definitions(-DHAS_SYMLINKS)
    set(GKFS_SYMLINK_SUPPORT ON)
    add_definitions(-DHAS_RENAME)
endif ()
message(STATUS "[gekkofs] Symlink support: ${GKFS_SYMLINK_SUPPORT}")
message(STATUS "[gekkofs] Rename support: ${GKFS_RENAME_SUPPORT}")

set(MAX_INTERNAL_FDS 256 CACHE STRING "Number of file descriptors reserved for internal use")
add_definitions(-DMAX_INTERNAL_FDS=${MAX_INTERNAL_FDS})
message(STATUS "[gekkofs] File descriptors reserved for internal use: ${MAX_INTERNAL_FDS}")

execute_process(COMMAND getconf OPEN_MAX
    OUTPUT_VARIABLE GETCONF_MAX_FDS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if (NOT GETCONF_MAX_FDS)
    set(GETCONF_MAX_FDS=512)
endif ()
add_definitions(-DMAX_OPEN_FDS=${GETCONF_MAX_FDS})

option(ENABLE_CLIENT_LOG "Enable logging messages" ON)
if (ENABLE_CLIENT_LOG)
    add_definitions(-DGKFS_ENABLE_LOGGING)
endif ()
message(STATUS "[gekkofs] Client logging output: ${ENABLE_CLIENT_LOG}")

option(GKFS_ENABLE_FORWARDING "Enable forwarding mode" OFF)
option(GKFS_ENABLE_AGIOS "Enable AGIOS scheduling library" OFF)

option(GKFS_ENABLE_UNUSED_FUNCTIONS "Enable unused functions compilation" OFF)

option(GKFS_ENABLE_PARALLAX "Enable Parallax db backend" OFF)
option(GKFS_ENABLE_ROCKSDB "Enable ROCKSDB backend" ON)

set(CLIENT_LOG_MESSAGE_SIZE 1024 CACHE STRING "Maximum size of a log message in the client library")
add_definitions(-DLIBGKFS_LOG_MESSAGE_SIZE=${CLIENT_LOG_MESSAGE_SIZE})
message(STATUS "[gekkofs] Maximum log message size in the client library: ${CLIENT_LOG_MESSAGE_SIZE}")
mark_as_advanced(CLIENT_LOG_MESSAGE_SIZE)

option(GKFS_USE_GUIDED_DISTRIBUTION "Use guided data distributor " OFF)
message(STATUS "[gekkofs] Guided data distributor: ${GKFS_USE_GUIDED_DISTRIBUTION}")

if(GKFS_USE_GUIDED_DISTRIBUTION)
    set(GKFS_USE_GUIDED_DISTRIBUTION_PATH "/tmp/guided.txt" CACHE STRING "File Path for guided distributor")
    set_property(CACHE GKFS_USE_GUIDED_DISTRIBUTION_PATH PROPERTY STRINGS)
    message(STATUS "[gekkofs] Guided data distributor input file path: ${GKFS_USE_GUIDED_DISTRIBUTION_PATH}")
endif()

option(GKFS_ENABLE_PROMETHEUS "Enable Prometheus Push " OFF)
if(GKFS_ENABLE_PROMETHEUS)
    add_definitions(-DGKFS_ENABLE_PROMETHEUS)
endif ()
message(STATUS "[gekkofs] Prometheus Output: ${GKFS_ENABLE_PROMETHEUS}")

configure_file(include/common/cmake_configure.hpp.in include/common/cmake_configure.hpp)



# add_library(CLI11 INTERFACE)
# # we cannot use target_include_directories with CMake < 3.11
# set_target_properties(CLI11
#     PROPERTIES
#     INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external"
#     )

if (ENABLE_CLIENT_LOG)
    option(HERMES_LOGGING "" ON)
    option(HERMES_LOGGING_FMT_USE_BUNDLED "" OFF)
    option(HERMES_LOGGING_FMT_HEADER_ONLY "" OFF)
endif ()

option(HERMES_MARGO_COMPATIBLE_RPCS "" ON)
add_subdirectory(external/hermes)
target_compile_definitions(hermes INTERFACE)

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# define include directories that are relevant for all targets
include_directories(
    ${INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/include
)

# Common components
add_subdirectory(src/common)
# Daemon
add_subdirectory(src/daemon)
# Client library
add_subdirectory(src/client)

### Mark any CMake variables imported from {fmt} and spdlog as advanced, so
### that they don't appear in cmake-gui or ccmake. Similarly for FETCHCONTENT
### variables.
mark_variables_as_advanced(REGEX "^(FETCHCONTENT|fmt|FMT|spdlog|SPDLOG)_.*$")

option(GKFS_BUILD_TESTS "Build GekkoFS self tests" OFF)

cmake_dependent_option(GKFS_INSTALL_TESTS "Install GekkoFS self tests" OFF "GKFS_BUILD_TESTS" OFF)

if (GKFS_BUILD_TESTS)
    # Boost preprocessor header-only is supplied by the Mercury installation
    find_package(Boost_preprocessor REQUIRED)
    # check symbols exists doesn't work for statx. This is a workaround
    check_cxx_source_compiles("
        #include <fcntl.h>
        #include <sys/stat.h>

        int main() {
            struct statx buf;
            statx(AT_FDCWD, \"/foo\", AT_EMPTY_PATH, STATX_BASIC_STATS, &buf);
            return 0;
        }
        " GLIBC_HAS_STATX)
    # STATX_TYPE must be set for c++17 for statx() to be found for tests
    if (GLIBC_HAS_STATX)
        add_definitions(-DSTATX_TYPE=1)
    endif ()
    message(STATUS "[gekkofs] Preparing tests...")
    set(GKFS_TESTS_INTERFACE "lo" CACHE STRING "Network interface to use when running tests (default: lo)")
    message(STATUS "[gekkofs] Network interface for tests: ${GKFS_TESTS_INTERFACE}")

    message(STATUS "[gekkofs] Check for forwarding tests...")
    if (GKFS_ENABLE_FORWARDING)
        set(GKFS_TESTS_FORWARDING "ON" CACHE STRING "Enable I/O forwarding tests (default: OFF)")
    else ()
        set(GKFS_TESTS_FORWARDING "OFF" CACHE STRING "Enable I/O forwarding tests (default: OFF)")
    endif ()
    message(STATUS "[gekkofs] Forwarding tests: ${GKFS_TESTS_FORWARDING}")
    message(STATUS "[gekkofs] Check for guided distributor tests...")
    if (GKFS_USE_GUIDED_DISTRIBUTION)
        set(GKFS_TESTS_GUIDED_DISTRIBUTION "ON" CACHE STRING "Enable guided distributor tests (default: OFF)")
    else()
        set(GKFS_TESTS_GUIDED_DISTRIBUTION "OFF" CACHE STRING "Enable guided distributor tests (default: OFF)")
    endif()
    message(STATUS "[gekkofs] Guided distributor tests: ${GKFS_TESTS_GUIDED_DISTRIBUTION}")

    add_subdirectory(tests)
    add_subdirectory(examples/gfind)
else()
    unset(GKFS_TESTS_INTERFACE CACHE)
endif()

