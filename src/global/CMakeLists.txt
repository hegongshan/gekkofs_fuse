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

add_subdirectory(arithmetic)

add_library(distributor STATIC)
set_property(TARGET distributor PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(distributor
    PUBLIC
    ${INCLUDE_DIR}/global/rpc/distributor.hpp
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/rpc/distributor.cpp
    )

if(GKFS_ENABLE_CODE_COVERAGE)
  target_code_coverage(distributor AUTO)
endif()

add_library(log_util STATIC)
set_property(TARGET log_util PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(log_util
    PUBLIC
    ${INCLUDE_DIR}/global/log_util.hpp
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/log_util.cpp
    )
target_link_libraries(log_util
  PRIVATE
    spdlog
    )

if(GKFS_ENABLE_CODE_COVERAGE)
  target_code_coverage(log_util AUTO)
endif()

add_library(env_util STATIC)
set_property(TARGET env_util PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(env_util
    PUBLIC
    ${INCLUDE_DIR}/global/env_util.hpp
    PRIVATE
    ${INCLUDE_DIR}/config.hpp
    ${CMAKE_CURRENT_LIST_DIR}/env_util.cpp
    )

if(GKFS_ENABLE_CODE_COVERAGE)
  target_code_coverage(env_util AUTO)
endif()

add_library(metadata STATIC)
set_property(TARGET metadata PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(metadata
    PUBLIC
    ${INCLUDE_DIR}/global/metadata.hpp
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/metadata.cpp
    )
target_link_libraries(metadata
  PRIVATE
    fmt::fmt
    )

if(GKFS_ENABLE_CODE_COVERAGE)
  target_code_coverage(metadata AUTO)
endif()
