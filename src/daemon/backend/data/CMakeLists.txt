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

add_library(storage STATIC)

target_sources(storage
    PUBLIC
    ${INCLUDE_DIR}/daemon/backend/data/chunk_storage.hpp
    PRIVATE
    ${INCLUDE_DIR}/common/path_util.hpp
    ${INCLUDE_DIR}/common/common_defs.hpp
    ${INCLUDE_DIR}/daemon/backend/data/data_module.hpp
    ${INCLUDE_DIR}/daemon/backend/data/file_handle.hpp
    ${CMAKE_CURRENT_LIST_DIR}/chunk_storage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/data_module.cpp
    )

target_link_libraries(storage
    PRIVATE
    spdlog
    # open issue for std::filesystem https://gitlab.kitware.com/cmake/cmake/-/issues/17834
    stdc++fs
    -ldl
    )

#target_include_directories(storage
#    PRIVATE
#    )
