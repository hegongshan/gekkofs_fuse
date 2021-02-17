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

#
# - Try to find Facebook zstd library
# This will define
# ZStd_FOUND
# ZStd_INCLUDE_DIR
# ZStd_LIBRARIES
#

find_path(ZStd_INCLUDE_DIR
    NAMES zstd.h
    )

find_library(ZStd_LIBRARY
    NAMES zstd
    )

set(ZStd_LIBRARIES ${ZStd_LIBRARY})
set(ZStd_INCLUDE_DIRS ${ZStd_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ZStd
    DEFAULT_MSG ZStd_LIBRARY ZStd_INCLUDE_DIR
    )

mark_as_advanced(
    ZStd_LIBRARY
    ZStd_INCLUDE_DIR
)