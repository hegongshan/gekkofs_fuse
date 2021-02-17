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

include(GkfsPythonTesting)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process
            --verbose
            --output-on-failure
)

# integration tests
add_subdirectory(integration)

# unit tests
add_subdirectory(unit)
