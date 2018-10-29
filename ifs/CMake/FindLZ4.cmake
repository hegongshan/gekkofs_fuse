# Finds liblz4.
#
# This module defines:
# LZ4_FOUND
# LZ4_INCLUDE_DIR
# LZ4_LIBRARY
#

find_path(LZ4_INCLUDE_DIR
    NAMES lz4.h
)

find_library(LZ4_LIBRARY
    NAMES lz4
)

set(LZ4_LIBRARIES ${LZ4_LIBRARY} )
set(LZ4_INCLUDE_DIRS ${LZ4_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(lz4 DEFAULT_MSG LZ4_LIBRARY LZ4_INCLUDE_DIR)

mark_as_advanced(
    LZ4_LIBRARY
    LZ4_INCLUDE_DIR
)
