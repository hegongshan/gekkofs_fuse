find_path(ABT_SNOOZER_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        $ENV{HOME}/adafs/install
        )

find_path(ABT_SNOOZER_INCLUDE_DIR abt-snoozer.h
        HINTS
        $ENV{HOME}/adafs/install
        ${ABT_SNOOZER_DIR}
        /usr
        /usr/local
        /usr/local/adafs
        /opt
        PATH_SUFFIXES include
        PATH_SUFFIXES include/abt-snoozer
        )

find_library(ABT_SNOOZER_LIBRARY abt-snoozer
        HINTS
        $ENV{HOME}/adafs/install/lib
        ${ABT_SNOOZER_DIR}
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/argobots
        )

set(ABT_SNOOZER_INCLUDE_DIRS ${ABT_SNOOZER_INCLUDE_DIR})
set(ABT_SNOOZER_LIBRARIES ${ABT_SNOOZER_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Abt-Snoozer DEFAULT_MSG ABT_SNOOZER_LIBRARY ABT_SNOOZER_INCLUDE_DIR)

mark_as_advanced(
        ABT_SNOOZER_DIR
        ABT_SNOOZER_LIBRARY
        ABT_SNOOZER_INCLUDE_DIR
)