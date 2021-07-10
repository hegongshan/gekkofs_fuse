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

add_subdirectory(backend)

set(DAEMON_SRC
    ../common/rpc/rpc_util.cpp
    daemon.cpp
    util.cpp
    ops/metadentry.cpp
    ops/data.cpp
    classes/fs_data.cpp
    classes/rpc_data.cpp
    handler/srv_data.cpp
    handler/srv_metadata.cpp
    handler/srv_management.cpp
    )
set(DAEMON_HEADERS
    ../../include/config.hpp
    ../../include/version.hpp
    ../../include/common/cmake_configure.hpp
    ../../include/common/common_defs.hpp
    ../../include/common/rpc/rpc_types.hpp
    ../../include/common/rpc/rpc_util.hpp
    ../../include/daemon/daemon.hpp
    ../../include/daemon/util.hpp
    ../../include/daemon/ops/data.hpp
    ../../include/daemon/ops/metadentry.hpp
    ../../include/daemon/classes/fs_data.hpp
    ../../include/daemon/classes/rpc_data.hpp
    ../../include/daemon/handler/rpc_defs.hpp
    ../../include/daemon/handler/rpc_util.hpp
    )
set(DAEMON_LINK_LIBRARIES
    # internal libs
    metadata
    metadata_db
    storage
    distributor
    log_util
    env_util
    spdlog
    fmt::fmt
    path_util
    # margo libs
    ${ABT_LIBRARIES}
    mercury
    ${MARGO_LIBRARIES}
    # others
    Boost::boost
    Boost::program_options
    Threads::Threads
    )
set(DAEMON_INCLUDE_DIRS
    ${ABT_INCLUDE_DIRS}
    ${MARGO_INCLUDE_DIRS}
    )

add_executable(gkfs_daemon ${DAEMON_SRC} ${DAEMON_HEADERS})
target_link_libraries(gkfs_daemon PRIVATE ${DAEMON_LINK_LIBRARIES})

target_include_directories(gkfs_daemon PRIVATE ${DAEMON_INCLUDE_DIRS})

if(GKFS_ENABLE_CODE_COVERAGE)
  target_code_coverage(gkfs_daemon AUTO)
endif()

install(TARGETS gkfs_daemon
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
################### Forwarding daemon ###################
if (GKFS_ENABLE_FORWARDING)
    if (GKFS_ENABLE_AGIOS)
        set(FWD_DAEMON_SRC
            ${DAEMON_SRC}
            scheduler/agios.cpp
            )
        set(FWD_DAEMON_HEADERS
            ${DAEMON_HEADERS}
            ../../include/daemon/scheduler/agios.hpp
            )
        add_executable(gkfwd_daemon ${FWD_DAEMON_SRC} ${FWD_DAEMON_HEADERS})

        target_compile_definitions(gkfwd_daemon
            PUBLIC
            GKFS_ENABLE_FORWARDING
            GKFS_ENABLE_AGIOS
            )
    else ()
        set(FWD_DAEMON_SRC
            ${DAEMON_SRC}
            )
        set(FWD_DAEMON_HEADERS
            ${DAEMON_HEADERS}
            )
        add_executable(gkfwd_daemon ${FWD_DAEMON_SRC} ${FWD_DAEMON_HEADERS})

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
        ${AGIOS_LIBRARIES}
        )

    target_include_directories(gkfwd_daemon
        PRIVATE
        ${DAEMON_INCLUDE_DIRS}
        ${AGIOS_INCLUDE_DIRS}
        )

    if(GKFS_ENABLE_CODE_COVERAGE)
      target_code_coverage(gkfwd_daemon AUTO)
    endif()

    install(TARGETS gkfwd_daemon
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
endif ()
