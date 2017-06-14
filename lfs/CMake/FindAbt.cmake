find_path(ABT_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        /home/evie/adafs/install
        )

find_path(ABT_INCLUDE_DIR abt.h
        HINTS
        /home/evie/adafs/install
        /usr
        /usr/local
        /usr/local/adafs
        ${ABT_DIR}
        PATH_SUFFIXES include
        )

find_library(ABT_LIBRARY abt
        HINTS
        /home/evie/adafs/install/lib
        /usr
        /usr/local
        /usr/local/adafs
        ${ABT_DIR}
        #        PATH SUFFIXES lib
        #        PATH_SUFFIXES lib/margo
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