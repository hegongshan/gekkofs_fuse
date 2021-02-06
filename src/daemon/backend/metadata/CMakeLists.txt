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
# SPDX-License-Identifier: MIT                                                 #
################################################################################

add_library(metadata_db STATIC)

target_sources(metadata_db
    PUBLIC
    ${INCLUDE_DIR}/daemon/backend/metadata/db.hpp
    ${INCLUDE_DIR}/daemon/backend/exceptions.hpp
    PRIVATE
    ${INCLUDE_DIR}/global/path_util.hpp
    ${INCLUDE_DIR}/daemon/backend/metadata/merge.hpp
    ${CMAKE_CURRENT_LIST_DIR}/merge.cpp
    ${CMAKE_CURRENT_LIST_DIR}/db.cpp
    )

target_link_libraries(metadata_db
    # Required by RocksDB
    -ldl
    metadata
    RocksDB
    spdlog
    )
