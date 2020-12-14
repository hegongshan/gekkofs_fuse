################################################################################
# Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
# Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany
#
# This software was partially supported by the
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).
#
# This software was partially supported by the
# ADA-FS project under the SPPEXA project funded by the DFG.
#
# SPDX-License-Identifier: MIT
################################################################################

add_library(arithmetic INTERFACE)
target_sources(arithmetic
  INTERFACE
    ${INCLUDE_DIR}/global/arithmetic/arithmetic.hpp
  )

target_include_directories(arithmetic
  INTERFACE
    ${INCLUDE_DIR}/global/arithmetic/
  )

target_code_coverage(arithmetic INTERFACE)
