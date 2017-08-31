find_path(CCI_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        $ENV{HOME}/adafs/install
        )

find_path(CCI_INCLUDE_DIR cci.h
        HINTS
        $ENV{HOME}/adafs/install
        $ENV{HOME}/adafs/install
        /usr
        /usr/local
        /usr/local/adafs
        ${CCI_DIR}
        PATH_SUFFIXES include
        )

find_library(CCI_LIBRARY cci
        HINTS
        $ENV{HOME}/adafs/install/lib
        /usr
        /usr/local
        /usr/local/adafs
        ${CCI_DIR}
        )

set(CCI_INCLUDE_DIRS ${CCI_INCLUDE_DIR})
set(CCI_LIBRARIES ${CCI_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCI DEFAULT_MSG CCI_LIBRARY CCI_INCLUDE_DIR)

mark_as_advanced(
        CCI_DIR
        CCI_LIBRARY
        CCI_INCLUDE_DIR
)