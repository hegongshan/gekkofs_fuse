###############################################################################
# Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany          #
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

# Since we need to generate several libraries that share a lot of code, we
# create a `gkfs_intercept_common` utility target for that we can use as a
# direct dependency for each library.
#
# IMPORTANT: since the targets differ basically in their compile definitions, we
# need to keep any source files affected by such definitions (e.g.
# GFS_ENABLE_FORWARDING) in their own specific targets. Otherwise, they will not
# be build with the correct defines.
add_library(gkfs_intercept_common STATIC)
set_property(TARGET gkfs_intercept_common PROPERTY POSITION_INDEPENDENT_CODE ON)

target_sources(
  gkfs_intercept_common
  PRIVATE gkfs_functions.cpp
          hooks.cpp
          intercept.cpp
          logging.cpp
          open_file_map.cpp
          open_dir.cpp
          path.cpp
          preload_context.cpp
          ../common/rpc/rpc_util.cpp
          rpc/rpc_types.cpp
          rpc/forward_data.cpp
          rpc/forward_management.cpp
          rpc/forward_metadata.cpp
          syscalls/detail/syscall_info.c
  PUBLIC ${CMAKE_SOURCE_DIR}/include/client/gkfs_functions.hpp
         ${CMAKE_SOURCE_DIR}/include/config.hpp
         ${CMAKE_SOURCE_DIR}/include/client/env.hpp
         ${CMAKE_SOURCE_DIR}/include/client/hooks.hpp
         ${CMAKE_SOURCE_DIR}/include/client/intercept.hpp
         ${CMAKE_SOURCE_DIR}/include/client/logging.hpp
         ${CMAKE_SOURCE_DIR}/include/client/make_array.hpp
         ${CMAKE_SOURCE_DIR}/include/client/open_file_map.hpp
         ${CMAKE_SOURCE_DIR}/include/client/open_dir.hpp
         ${CMAKE_SOURCE_DIR}/include/client/path.hpp
         ${CMAKE_SOURCE_DIR}/include/client/preload.hpp
         ${CMAKE_SOURCE_DIR}/include/client/preload_context.hpp
         ${CMAKE_SOURCE_DIR}/include/client/preload_util.hpp
         ${CMAKE_SOURCE_DIR}/include/client/rpc/rpc_types.hpp
         ${CMAKE_SOURCE_DIR}/include/client/rpc/forward_management.hpp
         ${CMAKE_SOURCE_DIR}/include/client/rpc/forward_metadata.hpp
         ${CMAKE_SOURCE_DIR}/include/client/rpc/forward_data.hpp
         ${CMAKE_SOURCE_DIR}/include/client/syscalls/args.hpp
         ${CMAKE_SOURCE_DIR}/include/client/syscalls/decoder.hpp
         ${CMAKE_SOURCE_DIR}/include/client/syscalls/errno.hpp
         ${CMAKE_SOURCE_DIR}/include/client/syscalls/rets.hpp
         ${CMAKE_SOURCE_DIR}/include/client/syscalls/syscall.hpp
         ${CMAKE_SOURCE_DIR}/include/client/syscalls/detail/syscall_info.h
         ${CMAKE_SOURCE_DIR}/include/common/cmake_configure.hpp.in
         ${CMAKE_SOURCE_DIR}/include/common/common_defs.hpp
         ${CMAKE_SOURCE_DIR}/include/common/rpc/rpc_types.hpp
         ${CMAKE_SOURCE_DIR}/include/common/rpc/rpc_util.hpp
)

target_link_libraries(
  gkfs_intercept_common
  PRIVATE # internal
          metadata
          distributor
          env_util
          arithmetic
          path_util
          # external
  PUBLIC # internal
          Syscall_intercept::Syscall_intercept
          dl
          Mercury::Mercury
          hermes
          fmt::fmt
          Threads::Threads
          Date::TZ
)

# ##############################################################################
# This builds the `libgkfs_intercept.so` library: the primary GekkoFS client
# based on syscall interception.
# ##############################################################################
add_library(gkfs_intercept SHARED)

# We need to add here any files that may have different compile definitions
target_sources(gkfs_intercept PRIVATE preload.cpp preload_util.cpp)
target_link_libraries(gkfs_intercept PRIVATE gkfs_intercept_common)

install(
  TARGETS gkfs_intercept
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gkfs
)

# ##############################################################################
# This builds the `libgkfwd_intercept.so` library: the client for GekkoFS'
# forwarding mode based on syscall interception.
# ##############################################################################

if(GKFS_ENABLE_FORWARDING)
  add_library(gkfwd_intercept SHARED)

  # We need to add here any files that may have different compile definitions
  target_sources(gkfwd_intercept PRIVATE preload.cpp preload_util.cpp)
  target_compile_definitions(gkfwd_daemon PUBLIC GKFS_ENABLE_FORWARDING)

  if(GKFS_ENABLE_AGIOS)
    target_compile_definitions(gkfwd_daemon PUBLIC GKFS_ENABLE_AGIOS)
  endif()

  target_link_libraries(gkfwd_intercept PRIVATE gkfs_intercept_common)

  install(
    TARGETS gkfwd_intercept
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gkfs
  )
endif()
