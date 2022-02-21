################################################################################
# Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany          #
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

# - Try to find jemalloc headers and libraries.
#
# Usage of this module as follows:
#
#     find_package(JeMalloc)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  JEMALLOC_ROOT_DIR Set this variable to the root installation of
#                    jemalloc if the module has problems finding
#                    the proper installation path.
#
# Variables defined by this module:
#
#  JEMALLOC_FOUND             System has jemalloc libs/headers
#  JEMALLOC_LIBRARIES         The jemalloc library/libraries
#  JEMALLOC_INCLUDE_DIR       The location of jemalloc headers

find_library(JEMALLOC_LIBRARY
    NAMES jemalloc
    HINTS ${JEMALLOC_ROOT_DIR}/lib
    )

find_path(JEMALLOC_INCLUDE_DIR
    NAMES jemalloc/jemalloc.h
    HINTS ${JEMALLOC_ROOT_DIR}/include
    )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JeMalloc DEFAULT_MSG
    JEMALLOC_LIBRARY
    JEMALLOC_INCLUDE_DIR
    )

mark_as_advanced(
    JEMALLOC_LIBRARY
    JEMALLOC_INCLUDE_DIR
)

set(JEMALLOC_LIBRARIES ${JEMALLOC_LIBRARY})
set(JEMALLOC_INCLUDE_DIRS ${JEMALLOC_INCLUDE_DIR})