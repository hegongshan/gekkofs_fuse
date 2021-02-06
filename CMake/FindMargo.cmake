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

# Try to find Margo headers and library.
#
# Usage of this module as follows:
#
#     find_package(Margo)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  MARGO_ROOT_DIR          Set this variable to the root installation of
#                            Margo if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  MARGO_FOUND               System has Margo library/headers.
#  MARGO_LIBRARIES           The Margo library.
#  MARGO_INCLUDE_DIRS        The location of Margo headers.


find_path(MARGO_INCLUDE_DIR
    NAMES margo.h
)

find_library(MARGO_LIBRARY
    NAMES margo
)

set(MARGO_INCLUDE_DIRS ${MARGO_INCLUDE_DIR})
set(MARGO_LIBRARIES ${MARGO_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Margo DEFAULT_MSG MARGO_LIBRARY MARGO_INCLUDE_DIR)

mark_as_advanced(
        MARGO_LIBRARY
        MARGO_INCLUDE_DIR
)