#
# - Try to find Facebook zstd library
# This will define
# ZStd_FOUND
# ZStd_INCLUDE_DIR
# ZStd_LIBRARIES
#

find_path(ZStd_INCLUDE_DIR
    NAMES zstd.h
    )

find_library(ZStd_LIBRARY
    NAMES zstd
    )

set(ZStd_LIBRARIES ${ZStd_LIBRARY})
set(ZStd_INCLUDE_DIRS ${ZStd_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ZStd
    DEFAULT_MSG ZStd_LIBRARY ZStd_INCLUDE_DIR
    )

mark_as_advanced(
    ZStd_LIBRARY
    ZStd_INCLUDE_DIR
)