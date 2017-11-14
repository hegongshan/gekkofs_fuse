find_path(ABT_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_path(ABT_INCLUDE_DIR abt.h
        HINTS
        $ENV{HOME}/adafs/install
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
        $ENV{HOME}/adafs/install/lib
        $ENV{HOME}/opt
        ${ABT_DIR}
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH SUFFIXES lib
        PATH SUFFIXES lib/argobots
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