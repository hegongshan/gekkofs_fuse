find_path(MERCURY_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_path(MERCURY_INCLUDE_DIR mercury.h
        HINTS
        $ENV{HOME}/adafs/install
        ${MERCURY_DIR}
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/mercury
        )

find_library(MERCURY_LIBRARY
        NAMES mercury mercury_debug
        HINTS
        $ENV{HOME}/adafs/install/lib
        ${MERCURY_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/mercury
        )

set(MERCURY_INCLUDE_DIRS ${MERCURY_INCLUDE_DIR})
set(MERCURY_LIBRARIES ${MERCURY_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mercury DEFAULT_MSG MERCURY_LIBRARY MERCURY_INCLUDE_DIR)

mark_as_advanced(
        MERCURY_DIR
        MERCURY_LIBRARY
        MERCURY_INCLUDE_DIR
)