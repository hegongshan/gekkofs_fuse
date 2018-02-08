# Try to find libev
# Once done, this will define
#
# LIBEV_FOUND        - system has libev
# LIBEV_INCLUDE_DIRS - libev include directories
# LIBEV_LIBRARIES    - libraries needed to use libev

if (LIBEV_INCLUDE_DIRS AND LIBEV_LIBRARIES)
    set(LIBEV_FIND_QUIETLY TRUE)
else ()
    find_path(
            LIBEV_INCLUDE_DIR
            NAMES ev.h
            HINTS
            ${LIBEV_ROOT_DIR}
            ${ADAFS_DEPS_INSTALL}
            $ENV{HOME}/opt
            /usr
            /usr/local
            /usr/local/adafs
            /opt/
            PATH_SUFFIXES include
            PATH_SUFFIXES include/libev
    )

    find_library(
            LIBEV_LIBRARY
            NAME ev
            HINTS
            ${LIBEV_ROOT_DIR}
            ${ADAFS_DEPS_INSTALL}
            $ENV{HOME}/opt
            /usr
            /usr/local
            /usr/local/adafs
            /opt/
            PATH_SUFFIXES lib
            PATH_SUFFIXES lib/libev
    )

    set(LIBEV_INCLUDE_DIRS ${LIBEV_INCLUDE_DIR})
    set(LIBEV_LIBRARIES ${LIBEV_LIBRARY})

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
            libev DEFAULT_MSG LIBEV_LIBRARY LIBEV_INCLUDE_DIR)

    mark_as_advanced(LIBEV_LIBRARY LIBEV_INCLUDE_DIR)
endif ()