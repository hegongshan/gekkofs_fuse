find_path(LIBFABRIC_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        $ENV{HOME}/adafs/install
        )


find_path(LIBFABRIC_INCLUDE_DIR rdma/fabric.h
        HINTS
        $ENV{HOME}/adafs/install
        $ENV{HOME}/adafs/install
        /usr
        /usr/local
        /usr/local/adafs
        ${LIBFABRIC_DIR}
        PATH_SUFFIXES include)

find_library(LIBFABRIC_LIBRARY NAMES fabric
        HINTS
        $ENV{HOME}/adafs/install/lib
        /usr
        /usr/local
        /usr/local/adafs
        ${LIBFABRIC_DIR}
        )




set(LIBFABRIC_LIBRARIES ${LIBFABRIC_LIBRARY} )
set(LIBFABRIC_INCLUDE_DIRS ${LIBFABRIC_INCLUDE_DIR} )


include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libfabric DEFAULT_MSG LIBFABRIC_LIBRARY LIBFABRIC_INCLUDE_DIR)

mark_as_advanced(
        LIBFABRIC_INCLUDE_DIR
        LIBFABRIC_LIBRARY
)