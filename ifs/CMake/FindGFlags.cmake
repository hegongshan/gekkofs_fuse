find_path(GFlags_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_path(GFlags_INCLUDE_DIR gflags.h
        HINTS
        $ENV{HOME}/adafs/install
        ${GFlags_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/gflags
        )

find_library(GFlags_LIBRARY gflags
        HINTS
        $ENV{HOME}/adafs/install
        ${GFlags_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/gflags
        )

set(GFlags_INCLUDE_DIRS ${GFlags_INCLUDE_DIR})
set(GFlags_LIBRARIES ${GFlags_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GFlags DEFAULT_MSG GFlags_LIBRARY GFlags_INCLUDE_DIR)

mark_as_advanced(
        GFlags_DIR
        GFlags_LIBRARY
        GFlags_INCLUDE_DIR
)
