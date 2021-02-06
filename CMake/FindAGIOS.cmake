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

find_path(AGIOS_INCLUDE_DIR
    NAMES agios.h
)

find_library(AGIOS_LIBRARY
    NAMES agios
)

set(AGIOS_INCLUDE_DIRS ${AGIOS_INCLUDE_DIR})
set(AGIOS_LIBRARIES ${AGIOS_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AGIOS DEFAULT_MSG AGIOS_LIBRARIES AGIOS_INCLUDE_DIRS)

mark_as_advanced(
    AGIOS_LIBRARY
    AGIOS_INCLUDE_DIR
)