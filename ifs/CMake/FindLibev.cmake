# Try to find libev
# Once done, this will define
#
# LIBEV_FOUND        - system has libev
# LIBEV_INCLUDE_DIRS - libev include directories
# LIBEV_LIBRARIES    - libraries needed to use libev

find_path(LIBEV_INCLUDE_DIR
    NAMES ev.h
)

find_library(LIBEV_LIBRARY
    NAME ev
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    libev DEFAULT_MSG LIBEV_LIBRARY LIBEV_INCLUDE_DIR)

mark_as_advanced(LIBEV_LIBRARY LIBEV_INCLUDE_DIR)

set(LIBEV_INCLUDE_DIRS ${LIBEV_INCLUDE_DIR})
set(LIBEV_LIBRARIES ${LIBEV_LIBRARY})