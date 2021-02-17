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

add_library(storage STATIC)

target_sources(storage
    PUBLIC
    ${INCLUDE_DIR}/daemon/backend/data/chunk_storage.hpp
    PRIVATE
    ${INCLUDE_DIR}/global/path_util.hpp
    ${INCLUDE_DIR}/global/global_defs.hpp
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
