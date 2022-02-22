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

add_library(metadata_db STATIC)
# If it exists, RocksDB has been build with liburing support.
# If check succeeds, we need to link with liburing with the daemon
set(CMAKE_REQUIRED_FLAGS "-luring")
check_cxx_source_compiles("
        #include <liburing.h>
        int main() {
          struct io_uring ring;
          io_uring_queue_init(1, &ring, 0);
          return 0;
        }
        " HAS_LIBURING)
set(liburing "")
if (HAS_LIBURING)
    set(LIBURING "-luring")
endif()

target_sources(metadata_db
    PUBLIC
    ${INCLUDE_DIR}/daemon/backend/metadata/db.hpp
    ${INCLUDE_DIR}/daemon/backend/exceptions.hpp
    PRIVATE
    ${INCLUDE_DIR}/daemon/backend/metadata/merge.hpp
    ${CMAKE_CURRENT_LIST_DIR}/merge.cpp
    ${CMAKE_CURRENT_LIST_DIR}/db.cpp
    )

add_library(metadata_module
  STATIC
)

target_sources(metadata_module
  PUBLIC
    ${INCLUDE_DIR}/daemon/backend/metadata/metadata_module.hpp
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/metadata_module.cpp
)

target_link_libraries(metadata_module
  PUBLIC
    spdlog
)

target_link_libraries(metadata_db
    # Required by RocksDB
    metadata_module
    -ldl
    metadata
    RocksDB
    spdlog
    path_util
    ${LIBURING}
    )
