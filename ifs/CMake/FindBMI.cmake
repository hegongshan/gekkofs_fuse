find_path(BMI_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        $ENV{HOME}/adafs/install
        )

find_path(BMI_INCLUDE_DIR bmi.h
        HINTS
        $ENV{HOME}/adafs/install
        $ENV{HOME}/adafs/install
        /usr
        /usr/local
        /usr/local/adafs
        ${BMI_DIR}
        PATH_SUFFIXES include
        )

find_library(BMI_LIBRARY bmi
        HINTS
        $ENV{HOME}/adafs/install/lib
        /usr
        /usr/local
        /usr/local/adafs
        ${BMI_DIR}
        )

set(BMI_INCLUDE_DIRS ${BMI_INCLUDE_DIR})
set(BMI_LIBRARIES ${BMI_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BMI DEFAULT_MSG BMI_LIBRARY BMI_INCLUDE_DIR)

mark_as_advanced(
        BMI_DIR
        BMI_LIBRARY
        BMI_INCLUDE_DIR
)