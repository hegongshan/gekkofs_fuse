find_path(LIBFABRIC_INCLUDE_DIR
    NAMES rdma/fabric.h
)

find_library(LIBFABRIC_LIBRARY
    NAMES fabric
)

set(LIBFABRIC_LIBRARIES ${LIBFABRIC_LIBRARY} )
set(LIBFABRIC_INCLUDE_DIRS ${LIBFABRIC_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libfabric DEFAULT_MSG LIBFABRIC_LIBRARY LIBFABRIC_INCLUDE_DIR)

mark_as_advanced(
    LIBFABRIC_INCLUDE_DIR
    LIBFABRIC_LIBRARY
)