find_path(ABT_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        ${ADAFS_DEPS_INSTALL}
        )

find_path(ABT_INCLUDE_DIR abt.h
        HINTS
        ${ADAFS_DEPS_INSTALL}
        $ENV{HOME}/opt
        ${ABT_DIR}
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/argobots
        )

find_library(ABT_LIBRARY abt
        HINTS
        ${ADAFS_DEPS_INSTALL}
        $ENV{HOME}/opt
        ${ABT_DIR}
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/argobots
        )

set(ABT_INCLUDE_DIRS ${ABT_INCLUDE_DIR})
set(ABT_LIBRARIES ${ABT_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Abt DEFAULT_MSG ABT_LIBRARY ABT_INCLUDE_DIR)

mark_as_advanced(
        ABT_DIR
        ABT_LIBRARY
        ABT_INCLUDE_DIR
)