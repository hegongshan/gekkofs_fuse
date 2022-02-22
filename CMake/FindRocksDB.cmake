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

# Try to find RocksDB headers and library.
#
# Usage of this module as follows:
#
#     find_package(RocksDB)
#
# Variables defined by this module:
#
#  ROCKSDB_FOUND               System has RocksDB library/headers.
#  ROCKSDB_LIBRARIES           The RocksDB library.
#  ROCKSDB_INCLUDE_DIRS        The location of RocksDB headers.

find_library(ROCKSDB_LIBRARY
    NAMES rocksdb
)

find_path(ROCKSDB_INCLUDE_DIR
    NAMES rocksdb/db.h
)

set(ROCKSDB_LIBRARIES ${ROCKSDB_LIBRARY})
set(ROCKSDB_INCLUDE_DIRS ${ROCKSDB_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RocksDB DEFAULT_MSG
    ROCKSDB_LIBRARY
    ROCKSDB_INCLUDE_DIR
)

mark_as_advanced(
    ROCKSDB_LIBRARY
    ROCKSDB_INCLUDE_DIR
)