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

add_subdirectory(backend)

set(DAEMON_SOURCES
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../common/rpc/rpc_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/daemon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ops/metadentry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ops/data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/classes/fs_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/classes/rpc_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/handler/srv_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/handler/srv_metadata.cpp
    ${CMAKE_CURRENT_LIST_DIR}/handler/srv_management.cpp
    PUBLIC
    ${INCLUDE_DIR}/config.hpp
    ${INCLUDE_DIR}/version.hpp.in
    ${INCLUDE_DIR}/common/cmake_configure.hpp.in
    ${INCLUDE_DIR}/common/common_defs.hpp
    ${INCLUDE_DIR}/common/rpc/rpc_types.hpp
    ${INCLUDE_DIR}/common/rpc/rpc_util.hpp
    ${INCLUDE_DIR}/daemon/daemon.hpp
    ${INCLUDE_DIR}/daemon/util.hpp
    ${INCLUDE_DIR}/daemon/ops/data.hpp
    ${INCLUDE_DIR}/daemon/ops/metadentry.hpp
    ${INCLUDE_DIR}/daemon/classes/fs_data.hpp
    ${INCLUDE_DIR}/daemon/classes/rpc_data.hpp
    ${INCLUDE_DIR}/daemon/handler/rpc_defs.hpp
    ${INCLUDE_DIR}/daemon/handler/rpc_util.hpp
    )
set(DAEMON_LINK_LIBRARIES
    PRIVATE
    # internal libs
    metadata
    metadata_db
    storage
    distributor
    statistics
    log_util
    env_util
    path_util
    # external libs
    CLI11::CLI11
    fmt::fmt
    Mercury::Mercury
    Argobots::Argobots
    Margo::Margo
    # others
    Threads::Threads
    )
set(DAEMON_INCLUDE_DIRS
    PRIVATE
    ${ABT_INCLUDE_DIRS}
    ${MARGO_INCLUDE_DIRS}
  ${CLI11_INCLUDE_DIRS}
    )

#add_executable(gkfs_daemon)
#target_sources(gkfs_daemon ${DAEMON_SOURCES})
#target_link_libraries(gkfs_daemon ${DAEMON_LINK_LIBRARIES})
#target_include_directories(gkfs_daemon ${DAEMON_INCLUDE_DIRS})

add_executable(gkfs_daemon)
target_sources(gkfs_daemon
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/../common/rpc/rpc_util.cpp
  ${CMAKE_CURRENT_LIST_DIR}/daemon.cpp
  ${CMAKE_CURRENT_LIST_DIR}/util.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ops/metadentry.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ops/data.cpp
  ${CMAKE_CURRENT_LIST_DIR}/classes/fs_data.cpp
  ${CMAKE_CURRENT_LIST_DIR}/classes/rpc_data.cpp
  ${CMAKE_CURRENT_LIST_DIR}/handler/srv_data.cpp
  ${CMAKE_CURRENT_LIST_DIR}/handler/srv_metadata.cpp
  ${CMAKE_CURRENT_LIST_DIR}/handler/srv_management.cpp
  PUBLIC
  ${INCLUDE_DIR}/config.hpp
  ${INCLUDE_DIR}/version.hpp.in
  ${INCLUDE_DIR}/common/cmake_configure.hpp.in
  ${INCLUDE_DIR}/common/common_defs.hpp
  ${INCLUDE_DIR}/common/rpc/rpc_types.hpp
  ${INCLUDE_DIR}/common/rpc/rpc_util.hpp
  ${INCLUDE_DIR}/daemon/daemon.hpp
  ${INCLUDE_DIR}/daemon/util.hpp
  ${INCLUDE_DIR}/daemon/ops/data.hpp
  ${INCLUDE_DIR}/daemon/ops/metadentry.hpp
  ${INCLUDE_DIR}/daemon/classes/fs_data.hpp
  ${INCLUDE_DIR}/daemon/classes/rpc_data.hpp
  ${INCLUDE_DIR}/daemon/handler/rpc_defs.hpp
  ${INCLUDE_DIR}/daemon/handler/rpc_util.hpp
  )
target_link_libraries(gkfs_daemon
  PRIVATE
  # internal libs
  metadata
  metadata_db
  storage
  distributor
  statistics
  log_util
  env_util
  path_util
  # external libs
  CLI11::CLI11
  fmt::fmt
  Mercury::Mercury
  Argobots::Argobots
  Margo::Margo
  # others
  Threads::Threads
  )

install(TARGETS gkfs_daemon
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
################### Forwarding daemon ###################
if (GKFS_ENABLE_FORWARDING)
    if (GKFS_ENABLE_AGIOS)
        set(FWD_DAEMON_SOURCES
            PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/scheduler/agios.cpp
            PUBLIC
            ${INCLUDE_DIR}/daemon/scheduler/agios.hpp
            )
        add_executable(gkfwd_daemon)
        target_sources(gkfwd_daemon ${DAEMON_SOURCES} ${FWD_DAEMON_SOURCES})
        target_compile_definitions(gkfwd_daemon
            PUBLIC
            GKFS_ENABLE_FORWARDING
            GKFS_ENABLE_AGIOS
            )
    else ()
        add_executable(gkfwd_daemon)
        target_sources(gkfwd_daemon ${DAEMON_SOURCES})
        target_compile_definitions(gkfwd_daemon
            PUBLIC
            GKFS_ENABLE_FORWARDING
            )
    endif ()

    message(STATUS "[gekkofs] Forwarding mode: ${GKFS_ENABLE_FORWARDING}")
    message(STATUS "[gekkofs] AGIOS scheduling: ${GKFS_ENABLE_AGIOS}")

    target_link_libraries(gkfwd_daemon
        PRIVATE
        ${DAEMON_LINK_LIBRARIES}
        AGIOS::AGIOS
        )
    target_include_directories(gkfwd_daemon
        PRIVATE
        ${DAEMON_INCLUDE_DIRS}
        ${AGIOS_INCLUDE_DIRS}
        )

    install(TARGETS gkfwd_daemon
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
endif ()
