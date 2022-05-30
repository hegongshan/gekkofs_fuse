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

# Since we need to generate several daemon executables that share a lot of code,
# we create a `gkfs_daemon_common` utility target for that we can use as a
# direct dependency for each executable.
#
# IMPORTANT: since the targets differ basically in their compile definitions, we
# need to keep any source files affected by such definitions (e.g.
# GFS_ENABLE_FORWARDING) in their own specific targets. Otherwise, they will not
# be build with the correct defines.
add_library(gkfs_daemon_common STATIC)

target_sources(
  gkfs_daemon_common
  PRIVATE ../common/rpc/rpc_util.cpp
          util.cpp
          ops/metadentry.cpp
          ops/data.cpp
          classes/fs_data.cpp
          classes/rpc_data.cpp
          handler/srv_metadata.cpp
          handler/srv_management.cpp
  PUBLIC ${CMAKE_SOURCE_DIR}/include/config.hpp
         ${CMAKE_SOURCE_DIR}/include/version.hpp.in
         ${CMAKE_SOURCE_DIR}/include/common/cmake_configure.hpp.in
         ${CMAKE_SOURCE_DIR}/include/common/common_defs.hpp
         ${CMAKE_SOURCE_DIR}/include/common/rpc/rpc_types.hpp
         ${CMAKE_SOURCE_DIR}/include/common/rpc/rpc_util.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/daemon.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/util.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/ops/data.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/ops/metadentry.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/classes/fs_data.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/classes/rpc_data.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/handler/rpc_defs.hpp
         ${CMAKE_SOURCE_DIR}/include/daemon/handler/rpc_util.hpp
)

target_link_libraries(
  gkfs_daemon_common
  PUBLIC # internal libs
          metadata
          metadata_backend
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

# ##############################################################################
# This builds the `gkfs_daemon` executable: the primary GekkoFS daemon.
# ##############################################################################
add_executable(gkfs_daemon)


pkg_check_modules(URING IMPORTED_TARGET liburing)

# We need to add here any files that may have different compile definitions
target_sources(
  gkfs_daemon PRIVATE ${CMAKE_CURRENT_LIST_DIR}/daemon.cpp
                      ${CMAKE_CURRENT_LIST_DIR}/handler/srv_data.cpp
)
target_link_libraries(gkfs_daemon PRIVATE gkfs_daemon_common)

if(GKFS_ENABLE_CODE_COVERAGE)
    target_code_coverage(gkfs_daemon AUTO)
endif()

install(TARGETS gkfs_daemon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ##############################################################################
# This builds the `gkfwd_daemon` executable: the daemon for GekkoFS data
# forwarding mode.
# ##############################################################################

if(GKFS_ENABLE_FORWARDING)

  add_executable(gkfwd_daemon)

  # We need to add here any files that may have different compile definitions
  target_sources(
    gkfwd_daemon PRIVATE ${CMAKE_CURRENT_LIST_DIR}/daemon.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/handler/srv_data.cpp
  )

  target_compile_definitions(
    gkfwd_daemon PUBLIC GKFS_ENABLE_FORWARDING
  )

  target_link_libraries(
    gkfwd_daemon PRIVATE gkfs_daemon_common
  )

  if(GKFS_ENABLE_AGIOS)
    target_sources(
      gkfwd_daemon
      PRIVATE scheduler/agios.cpp
      PUBLIC ${INCLUDE_DIR}/daemon/scheduler/agios.hpp
    )
    target_compile_definitions(
      gkfwd_daemon PUBLIC GKFS_ENABLE_AGIOS
    )

    target_link_libraries(
      gkfwd_daemon PRIVATE AGIOS::AGIOS
    )
  endif()

  install(TARGETS gkfwd_daemon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
