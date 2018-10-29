find_path(CCI_INCLUDE_DIR
    NAMES cci.h
)

find_library(CCI_LIBRARY
    NAMES cci
)

set(CCI_INCLUDE_DIRS ${CCI_INCLUDE_DIR})
set(CCI_LIBRARIES ${CCI_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCI DEFAULT_MSG CCI_LIBRARY CCI_INCLUDE_DIR)

mark_as_advanced(
        CCI_LIBRARY
        CCI_INCLUDE_DIR
)