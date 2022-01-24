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
# This file is part of GekkoFS' POSIX interface.                               #
#                                                                              #
# GekkoFS' POSIX interface is free software: you can redistribute it and/or    #
# modify it under the terms of the GNU Lesser General Public License as        #
# published by the Free Software Foundation, either version 3 of the License,  #
# or (at your option) any later version.                                       #
#                                                                              #
# GekkoFS' POSIX interface is distributed in the hope that it will be useful,  #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU Lesser General Public License for more details.                          #
#                                                                              #
# You should have received a copy of the GNU Lesser General Public License     #
# along with GekkoFS' POSIX interface.  If not, see                            #
# <https://www.gnu.org/licenses/>.                                             #
#                                                                              #
# SPDX-License-Identifier: LGPL-3.0-or-later                                   #
################################################################################
set(PRELOAD_SOURCES
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/gkfs_functions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hooks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/intercept.cpp
    ${CMAKE_CURRENT_LIST_DIR}/logging.cpp
    ${CMAKE_CURRENT_LIST_DIR}/open_file_map.cpp
    ${CMAKE_CURRENT_LIST_DIR}/open_dir.cpp
    ${CMAKE_CURRENT_LIST_DIR}/path.cpp
    ${CMAKE_CURRENT_LIST_DIR}/preload.cpp
    ${CMAKE_CURRENT_LIST_DIR}/preload_context.cpp
    ${CMAKE_CURRENT_LIST_DIR}/preload_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rpc/rpc_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rpc/rpc_types.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rpc/forward_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rpc/forward_management.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rpc/forward_metadata.cpp
    ${CMAKE_CURRENT_LIST_DIR}/syscalls/detail/syscall_info.c
    PUBLIC
    ${INCLUDE_DIR}/client/gkfs_functions.hpp
    ${INCLUDE_DIR}/config.hpp
    ${INCLUDE_DIR}/client/env.hpp
    ${INCLUDE_DIR}/client/hooks.hpp
    ${INCLUDE_DIR}/client/intercept.hpp
    ${INCLUDE_DIR}/client/logging.hpp
    ${INCLUDE_DIR}/client/make_array.hpp
    ${INCLUDE_DIR}/client/open_file_map.hpp
    ${INCLUDE_DIR}/client/open_dir.hpp
    ${INCLUDE_DIR}/client/path.hpp
    ${INCLUDE_DIR}/client/preload.hpp
    ${INCLUDE_DIR}/client/preload_context.hpp
    ${INCLUDE_DIR}/client/preload_util.hpp
    ${INCLUDE_DIR}/client/rpc/rpc_types.hpp
    ${INCLUDE_DIR}/client/rpc/forward_management.hpp
    ${INCLUDE_DIR}/client/rpc/forward_metadata.hpp
    ${INCLUDE_DIR}/client/rpc/forward_data.hpp
    ${INCLUDE_DIR}/client/syscalls/args.hpp
    ${INCLUDE_DIR}/client/syscalls/decoder.hpp
    ${INCLUDE_DIR}/client/syscalls/errno.hpp
    ${INCLUDE_DIR}/client/syscalls/rets.hpp
    ${INCLUDE_DIR}/client/syscalls/syscall.hpp
    ${INCLUDE_DIR}/client/syscalls/detail/syscall_info.h
    ${INCLUDE_DIR}/common/cmake_configure.hpp.in
    ${INCLUDE_DIR}/common/common_defs.hpp
    ${INCLUDE_DIR}/common/rpc/rpc_types.hpp
    ${INCLUDE_DIR}/common/rpc/rpc_util.hpp
    )
set(PRELOAD_LINK_LIBRARIES
    PRIVATE
    # internal
    metadata
    distributor
    env_util
    arithmetic
    path_util
    # external
    Syscall_intercept::Syscall_intercept
    dl
    mercury
    hermes
    fmt::fmt
    Threads::Threads
    Date::TZ
    )
set(PRELOAD_INCLUDE_DIRS
    PRIVATE
    ${ABT_INCLUDE_DIRS}
    ${MARGO_INCLUDE_DIRS}
    )

add_library(gkfs_intercept SHARED)
target_sources(gkfs_intercept ${PRELOAD_SOURCES})
target_link_libraries(gkfs_intercept PRIVATE ${PRELOAD_LINK_LIBRARIES})
target_include_directories(gkfs_intercept PRIVATE ${PRELOAD_INCLUDE_DIRS})

if(GKFS_ENABLE_CODE_COVERAGE)
  target_code_coverage(gkfs_intercept AUTO)
endif()

install(TARGETS gkfs_intercept
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gkfs
    )
################### Forwarding client ###################
if (GKFS_ENABLE_FORWARDING)
    add_library(gkfwd_intercept SHARED)
    target_sources(gkfwd_intercept ${PRELOAD_SOURCES})
    if (GKFS_ENABLE_AGIOS)
        target_compile_definitions(gkfwd_daemon
            PUBLIC
            GKFS_ENABLE_FORWARDING
            GKFS_ENABLE_AGIOS
            )
    else ()
        target_compile_definitions(gkfwd_daemon
            PUBLIC
            GKFS_ENABLE_FORWARDING
            )
    endif ()

    message(STATUS "[gekkofs] Forwarding mode: ${GKFS_ENABLE_FORWARDING}")
    message(STATUS "[gekkofs] AGIOS scheduling: ${GKFS_ENABLE_AGIOS}")

    target_link_libraries(gkfwd_intercept PRIVATE ${PRELOAD_LINK_LIBRARIES})
    target_include_directories(gkfwd_intercept PRIVATE ${PRELOAD_INCLUDE_DIRS})

    if(GKFS_ENABLE_CODE_COVERAGE)
      target_code_coverage(gkfwd_intercept AUTO)
    endif()

    install(TARGETS gkfwd_intercept
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gkfs
        )
endif ()
