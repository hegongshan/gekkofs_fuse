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

# ensure helper programs in the testing harness get built
add_subdirectory(harness)

gkfs_enable_python_testing(
    BINARY_DIRECTORIES ${CMAKE_BINARY_DIR}/src/daemon/
                       ${CMAKE_BINARY_DIR}/src/client/
                       ${CMAKE_BINARY_DIR}/tests/integration/harness/
                       ${CMAKE_BINARY_DIR}/examples/gfind/
    LIBRARY_PREFIX_DIRECTORIES ${CMAKE_PREFIX_PATH}
)

# define CTest tests for functional test groups
gkfs_add_python_test(
    NAME test_directories
    PYTHON_VERSION 3.6
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/integration
    SOURCE directories/
)

if (GLIBC_HAS_STATX)
    gkfs_add_python_test(
        NAME test_status
        PYTHON_VERSION 3.6
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/integration
        SOURCE status/test_status.py
    )
endif()

gkfs_add_python_test(
    NAME test_operations
    PYTHON_VERSION 3.6
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/integration
    SOURCE operations/
)

if (GLIBC_HAS_STATX)
gkfs_add_python_test(
    NAME test_lseek
    PYTHON_VERSION 3.6
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/integration
    SOURCE position/test_lseek.py
)
endif()

gkfs_add_python_test(
    NAME test_shell
    PYTHON_VERSION 3.6
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/integration
    SOURCE shell/
)

if (GKFS_TESTS_FORWARDING)
gkfs_add_python_test(
    NAME forwarding
    PYTHON_VERSION 3.6
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/integration
    SOURCE forwarding/
)
endif ()

gkfs_add_python_test(
    NAME test_data
    PYTHON_VERSION 3.6
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/integration
    SOURCE data/
)

if(GKFS_INSTALL_TESTS)
    install(DIRECTORY harness
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
            REGEX ".*\\.py"
            PATTERN "__pycache__" EXCLUDE
            PATTERN ".pytest_cache" EXCLUDE
            PATTERN "gkfs.io" EXCLUDE
    )

    if(SYMLINK_SUPPORT)
        install(DIRECTORY directories
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
            FILES_MATCHING
                REGEX ".*\\.py"
                PATTERN "__pycache__" EXCLUDE
                PATTERN ".pytest_cache" EXCLUDE
        )
    else()
        install(DIRECTORY directories
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
        REGEX ".*\\.py"
        PATTERN "__pycache__" EXCLUDE
        PATTERN ".pytest_cache" EXCLUDE
        PATTERN "test_symlink.py" EXCLUDE

    )
   endif()

    install(DIRECTORY status
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
            REGEX ".*\\.py"
            PATTERN "__pycache__" EXCLUDE
            PATTERN ".pytest_cache" EXCLUDE
    )

    install(DIRECTORY operations
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
            REGEX ".*\\.py"
            PATTERN "__pycache__" EXCLUDE
            PATTERN ".pytest_cache" EXCLUDE
    )

    install(DIRECTORY position
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
            REGEX ".*\\.py"
            PATTERN "__pycache__" EXCLUDE
            PATTERN ".pytest_cache" EXCLUDE
    )

    install(DIRECTORY data
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
            REGEX ".*\\.py"
            PATTERN "__pycache__" EXCLUDE
            PATTERN ".pytest_cache" EXCLUDE
    )

    install(DIRECTORY shell
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
            REGEX ".*\\.py"
            PATTERN "__pycache__" EXCLUDE
            PATTERN ".pytest_cache" EXCLUDE
    )
    if (GKFS_TESTS_FORWARDING)
    install(DIRECTORY forwarding
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        FILES_MATCHING
            REGEX ".*\\.py"
            PATTERN "__pycache__" EXCLUDE
            PATTERN ".pytest_cache" EXCLUDE
    )
    endif ()
endif()
