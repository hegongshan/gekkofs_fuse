find_path(MERCURY_UTIL_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_library(MERCURY_UTIL_LIBRARY
        NAMES mercury_util
        HINTS
        $ENV{HOME}/adafs/install/lib
        ${MERCURY_UTIL_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/mercury_util
        )

set(MERCURY_UTIL_LIBRARIES ${MERCURY_UTIL_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mercury_Util DEFAULT_MSG MERCURY_UTIL_LIBRARY)

mark_as_advanced(
        MERCURY_UTIL_DIR
        MERCURY_UTIL_LIBRARY
        MERCURY_UTIL_INCLUDE_DIR
)