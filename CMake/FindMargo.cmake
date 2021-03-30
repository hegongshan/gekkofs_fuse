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
# This file is part of GekkoFS.                                                #
#                                                                              #
# GekkoFS is free software: you can redistribute it and/or modify              #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation, either version 3 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# GekkoFS is distributed in the hope that it will be useful,                   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.            #
#                                                                              #
# SPDX-License-Identifier: GPL-3.0-or-later                                    #
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