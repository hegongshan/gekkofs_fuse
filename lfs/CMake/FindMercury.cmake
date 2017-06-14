find_path(MERCURY_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        /home/evie/adafs/install
        )

find_path(MERCURY_INCLUDE_DIR mercury.h
        HINTS
        /home/evie/adafs/install
        /usr
        /usr/local
        /usr/local/adafs
        ${MERCURY_DIR}
        PATH_SUFFIXES include
        )

find_library(MERCURY_LIBRARY
        NAMES mercury
        HINTS
        /home/evie/adafs/install/lib
        /usr
        /usr/local
        /usr/local/adafs
        ${MERCURY_DIR}
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