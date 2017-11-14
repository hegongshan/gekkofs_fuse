find_path(ABT_IO_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_path(ABT_IO_INCLUDE_DIR abt-io.h
        HINTS
        ${ABT_IO_DIR}
        $ENV{HOME}/adafs/install
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/abt-io
        )

find_library(ABT_IO_LIBRARY abt-io
        HINTS
        ${ABT_IO_DIR}
        $ENV{HOME}/adafs/install/lib
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/abt-io
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