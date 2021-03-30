################################################################################
# Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany          #
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

# Variables
option(GKFS_ENABLE_CODE_COVERAGE
  "Builds GekkoFS targets with code coverage instrumentation."
  OFF
  )

# Common initialization/checks
if(GKFS_ENABLE_CODE_COVERAGE AND NOT GKFS_CODE_COVERAGE_ADDED)

  set(GKFS_CODE_COVERAGE_ADDED ON)

  if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
     OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")

    message(STATUS "[gekkofs] Building with LLVM Code Coverage Tools")

  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                              "GNU")

    message(STATUS "[gekkofs] Building with GCC Code Coverage Tools")

    if(CMAKE_BUILD_TYPE)
      string(TOUPPER ${CMAKE_BUILD_TYPE} upper_build_type)
      if(NOT ${upper_build_type} STREQUAL "DEBUG")
        message(
          WARNING
            "Code coverage results with an optimized (non-Debug) build may be misleading"
        )
      endif()
    else()
      message(
        WARNING
          "Code coverage results with an optimized (non-Debug) build may be misleading"
      )
    endif()
  else()
    message(FATAL_ERROR "Code coverage requires Clang or GCC. Aborting.")
  endif()
endif()

# Adds code coverage instrumentation to libraries and executable targets.
# ~~~
# Required:
# TARGET_NAME - Name of the target to generate code coverage for.
# Optional:
# PUBLIC   - Sets the visibility for added compile options to targets to PUBLIC
#            instead of the default of PRIVATE.
# PRIVATE - Sets the visibility for added compile options to targets to
#           INTERFACE instead of the default of PRIVATE.
# ~~~
function(target_code_coverage TARGET_NAME)
  # Argument parsing
  set(options PUBLIC INTERFACE)
  cmake_parse_arguments(target_code_coverage "${options}" "" "" ${ARGN})

  # Set the visibility of target functions to PUBLIC, INTERFACE or default to
  # PRIVATE.
  if(target_code_coverage_PUBLIC)
    set(TARGET_VISIBILITY PUBLIC)
  elseif(target_code_coverage_INTERFACE)
    set(TARGET_VISIBILITY INTERFACE)
  else()
    set(TARGET_VISIBILITY PRIVATE)
  endif()

  if(GKFS_ENABLE_CODE_COVERAGE)

    # Add code coverage instrumentation to the target's linker command
    if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
       OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
      target_compile_options(${TARGET_NAME} ${TARGET_VISIBILITY}
                             -fprofile-instr-generate -fcoverage-mapping)
      target_link_options(${TARGET_NAME} ${TARGET_VISIBILITY}
                          -fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                                "GNU")
      target_compile_options(${TARGET_NAME} ${TARGET_VISIBILITY} -fprofile-arcs
        -ftest-coverage)
      target_link_libraries(${TARGET_NAME} ${TARGET_VISIBILITY} gcov)
    endif()
  endif()
endfunction()
