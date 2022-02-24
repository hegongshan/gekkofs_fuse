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

set(DB_DEFAULT_SOURCES 
    PUBLIC
    ${INCLUDE_DIR}/daemon/backend/metadata/db.hpp
    ${INCLUDE_DIR}/daemon/backend/exceptions.hpp
    ${INCLUDE_DIR}/daemon/backend/metadata/metadata_backend.hpp
    PRIVATE
    ${INCLUDE_DIR}/daemon/backend/metadata/merge.hpp
    ${CMAKE_CURRENT_LIST_DIR}/merge.cpp
    ${CMAKE_CURRENT_LIST_DIR}/db.cpp
)


if (GKFS_ENABLE_ROCKSDB)
      # Rocksdb dependencies
      find_package(LZ4 REQUIRED)
      find_package(JeMalloc) # required if rocksdb has been build with jemalloc
      find_package(RocksDB REQUIRED)
      # Imported target
      add_library(RocksDB INTERFACE IMPORTED GLOBAL)
      target_link_libraries(RocksDB
          INTERFACE
          ${ROCKSDB_LIBRARIES}
          # rocksdb libs
          ${LZ4_LIBRARIES}
          )


      if (${JeMalloc_FOUND})
          target_link_libraries(RocksDB
              INTERFACE
              ${JEMALLOC_LIBRARIES}
              )
      endif ()
      # we cannot use target_include_directories with CMake < 3.11
      set_target_properties(RocksDB
          PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES ${ROCKSDB_INCLUDE_DIRS}
          )

      set(DB_ROCKSDB_SOURCES
      PUBLIC
      ${INCLUDE_DIR}/daemon/backend/metadata/rocksdb_backend.hpp
      PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/rocksdb_backend.cpp
      )
      target_compile_definitions(metadata_db
      PUBLIC
      GKFS_ENABLE_ROCKSDB
      )
      
endif()

if (GKFS_ENABLE_PARALLAX)
      find_package(Parallax REQUIRED)
      set(DB_PARALLAX_SOURCES
      PUBLIC
      ${INCLUDE_DIR}/daemon/backend/metadata/parallax_backend.hpp
      PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/parallax_backend.cpp
      )
      target_compile_definitions(metadata_db
      PUBLIC
      GKFS_ENABLE_PARALLAX
      )
      add_library(Parallax INTERFACE IMPORTED GLOBAL)
endif()

target_sources(metadata_db 
${DB_DEFAULT_SOURCES} ${DB_PARALLAX_SOURCES} ${DB_ROCKSDB_SOURCES})

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
    ${PARALLAX_LIBRARY}
    ${LOG_LIBRARY}
    yaml   
     # Required by RocksDB
    metadata_module
    -ldl
    metadata
    RocksDB
    spdlog
    path_util
    ${LIBURING}
    )
