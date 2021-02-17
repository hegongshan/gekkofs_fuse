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

find_library(Snappy_LIBRARY
        NAMES snappy
)

find_path(Snappy_INCLUDE_DIR
    NAMES snappy.h
)

set(Snappy_LIBRARIES ${Snappy_LIBRARY})
set(Snappy_INCLUDE_DIRS ${Snappy_INCLUDE_DIR})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Snappy DEFAULT_MSG Snappy_LIBRARY Snappy_INCLUDE_DIR)

mark_as_advanced(
        Snappy_LIBRARY
        Snappy_INCLUDE_DIR
)
