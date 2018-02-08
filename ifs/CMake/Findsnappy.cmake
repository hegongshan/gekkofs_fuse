find_path(snappy_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        ${ADAFS_DEPS_INSTALL}
        )

find_path(snappy_INCLUDE_DIR snappy.h
        HINTS
        ${ADAFS_DEPS_INSTALL}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/snappy
        )

find_library(snappy_LIBRARY snappy
        HINTS
        ${ADAFS_DEPS_INSTALL}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/snappy
        )

set(snappy_INCLUDE_DIRS ${snappy_INCLUDE_DIR})
set(snappy_LIBRARIES ${snappy_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(snappy DEFAULT_MSG snappy_LIBRARY snappy_INCLUDE_DIR)

mark_as_advanced(
        snappy_DIR
        snappy_LIBRARY
        snappy_INCLUDE_DIR
)
