find_path(BMI_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        ${ADAFS_DEPS_INSTALL}
        )

find_path(BMI_INCLUDE_DIR bmi.h
        HINTS
        ${ADAFS_DEPS_INSTALL}
        ${BMI_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/bmi
        )

find_library(BMI_LIBRARY bmi
        HINTS
        ${ADAFS_DEPS_INSTALL}
        ${BMI_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/bmi
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