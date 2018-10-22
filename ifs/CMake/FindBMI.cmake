find_path(BMI_INCLUDE_DIR
    NAMES bmi.h
)

find_library(BMI_LIBRARY
    NAMES bmi
)

set(BMI_INCLUDE_DIRS ${BMI_INCLUDE_DIR})
set(BMI_LIBRARIES ${BMI_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BMI DEFAULT_MSG BMI_LIBRARY BMI_INCLUDE_DIR)

mark_as_advanced(
        BMI_LIBRARY
        BMI_INCLUDE_DIR
)