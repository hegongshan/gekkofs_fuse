find_path(ABT_IO_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_path(ABT_IO_INCLUDE_DIR abt-io.h
        HINTS
        $ENV{HOME}/adafs/install
        /usr
        /usr/local
        /usr/local/adafs
        ${ABT_IO_DIR}
        PATH_SUFFIXES include
        )

find_library(ABT_IO_LIBRARY abt-io
        HINTS
        $ENV{HOME}/adafs/install/lib
        /usr
        /usr/local
        /usr/local/adafs
        ${ABT_IO_DIR}
        )

set(ABT_IO_INCLUDE_DIRS ${ABT_IO_INCLUDE_DIR})
set(ABT_IO_LIBRARIES ${ABT_IO_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Abt-IO DEFAULT_MSG ABT_IO_LIBRARY ABT_IO_INCLUDE_DIR)

mark_as_advanced(
        ABT_IO_DIR
        ABT_IO_LIBRARY
        ABT_IO_INCLUDE_DIR
)