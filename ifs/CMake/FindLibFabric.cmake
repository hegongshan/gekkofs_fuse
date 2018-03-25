find_path(LIBFABRIC_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        ${ADAFS_DEPS_INSTALL}
        )


find_path(LIBFABRIC_INCLUDE_DIR rdma/fabric.h
        HINTS
        ${LIBFABRIC_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt
    ${ADAFS_DEPS_INSTALL}
        PATH_SUFFIXES include
        PATH_SUFFIXES include/libfabric
        )

find_library(LIBFABRIC_LIBRARY NAMES fabric
        HINTS
        ${LIBFABRIC_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
    ${ADAFS_DEPS_INSTALL}
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/libfabric
        )


set(LIBFABRIC_LIBRARIES ${LIBFABRIC_LIBRARY} )
set(LIBFABRIC_INCLUDE_DIRS ${LIBFABRIC_INCLUDE_DIR} )


include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libfabric DEFAULT_MSG LIBFABRIC_LIBRARY LIBFABRIC_INCLUDE_DIR)

mark_as_advanced(
        LIBFABRIC_INCLUDE_DIR
        LIBFABRIC_LIBRARY
)