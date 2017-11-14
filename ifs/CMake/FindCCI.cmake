find_path(CCI_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_path(CCI_INCLUDE_DIR cci.h
        HINTS
        $ENV{HOME}/adafs/install
        ${CCI_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/cci
        )

find_library(CCI_LIBRARY cci
        HINTS
        $ENV{HOME}/adafs/install/lib
        ${CCI_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH SUFFIXES lib
        PATH SUFFIXES lib/cci
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