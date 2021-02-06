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


# - Find Lz4
# Find the lz4 compression library and includes
#
# LZ4_FOUND - True if lz4 found.
# LZ4_LIBRARIES - List of libraries when using lz4.
# LZ4_INCLUDE_DIR - where to find lz4.h, etc.

find_path(LZ4_INCLUDE_DIR
    NAMES lz4.h
)

find_library(LZ4_LIBRARY
    NAMES lz4
)

set(LZ4_LIBRARIES ${LZ4_LIBRARY} )
set(LZ4_INCLUDE_DIRS ${LZ4_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZ4 DEFAULT_MSG LZ4_LIBRARY LZ4_INCLUDE_DIR)

mark_as_advanced(
    LZ4_LIBRARY
    LZ4_INCLUDE_DIR
)
