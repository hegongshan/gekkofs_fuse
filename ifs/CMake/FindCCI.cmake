find_path(CCI_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        ${ADAFS_DEPS_INSTALL}
        )

find_path(CCI_INCLUDE_DIR cci.h
        HINTS
        ${ADAFS_DEPS_INSTALL}
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
        ${ADAFS_DEPS_INSTALL}
        ${CCI_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/cci
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