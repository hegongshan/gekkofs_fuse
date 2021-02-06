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

find_path(ABT_INCLUDE_DIR
    NAMES abt.h
)

find_library(ABT_LIBRARY
    NAMES abt
)

set(ABT_INCLUDE_DIRS ${ABT_INCLUDE_DIR})
set(ABT_LIBRARIES ${ABT_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Abt DEFAULT_MSG ABT_LIBRARIES ABT_INCLUDE_DIRS)

mark_as_advanced(
        ABT_LIBRARY
        ABT_INCLUDE_DIR
)