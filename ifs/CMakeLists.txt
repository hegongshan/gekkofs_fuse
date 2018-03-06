cmake_minimum_required(VERSION 3.6)
project(ifs)

if(NOT CMAKE_COMPILER_IS_GNUCC)
    message(FATAL_ERROR "The choosen C compiler is not gcc and is not supported")
endif()
if(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(FATAL_ERROR "The choosen C++ compiler is not g++ and is not supported")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
            CACHE STRING "Choose the type of build: Debug Release Memcheck" FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)
message("* Current build type is : ${CMAKE_BUILD_TYPE}")

# Compiler flags for various cmake build types
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --pedantic -g -O0")
set(CMAKE_CXX_FLAGS_MEMCHECK "-Wall --pedantic -g -O0 -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_MAINTAINER "-Wall --pedantic -g -O0 -pg -no-pie")
mark_as_advanced(CMAKE_CXX_FLAGS_MAINTAINER)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

set(CMAKE_EXPORT_COMPILE_COMMANDS 0)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# required packages
set(ADAFS_DEPS_INSTALL CACHE PATH "Set the path to the required ADA-FS dependencies for libraries and headers")
if (NOT ADAFS_DEPS_INSTALL)
    set(ADAFS_DEPS_INSTALL /usr/local)
    message("* ADA-FS dependency install path set to DEFAULT PATH: \"${ADAFS_DEPS_INSTALL}\"\n Use cmake argument \"-DADAFS_DEPS_INSTALL:PATH=<path>\" to expicitly set it.")
else ()
    message("* ADA-FS dependency install path set to \"${ADAFS_DEPS_INSTALL}\"")
endif (NOT ADAFS_DEPS_INSTALL)

# Rocksdb dependencies
find_package(LZ4 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(GFlags REQUIRED)
find_package(snappy REQUIRED)
find_package(ZStd REQUIRED)
find_package(RocksDB REQUIRED)
# margo dependencies
find_package(Libev REQUIRED)
find_package(Mercury REQUIRED)
find_package(MercuryUtil REQUIRED)
find_package(Abt REQUIRED)
find_package(Abt-Snoozer REQUIRED)
find_package(Abt-IO REQUIRED)
find_package(Margo REQUIRED)

option(USE_OFI_VERBS "Use libfabric plugin with verbs." OFF)
if (USE_OFI_VERBS)
    find_package(LibFabric REQUIRED)
    set(NA_LIB
            ${NA_LIB}
            ${LIBFABRIC_LIBRARIES}
            )
    add_definitions(-DRPC_PROTOCOL="ofi+verbs")
endif()

option(USE_OFI_PSM2 "Use libfabric plugin with verbs." OFF)
if (USE_OFI_PSM2)
    find_package(LibFabric REQUIRED)
    set(NA_LIB
            ${NA_LIB}
            ${LIBFABRIC_LIBRARIES}
            )
    add_definitions(-DRPC_PROTOCOL="ofi+psm2")
endif ()

option(USE_CCI "Use cci plugin." OFF)
if(USE_CCI)
    find_package(CCI REQUIRED)
    set(NA_LIB
            ${NA_LIB}
            ${CCI_LIBRARIES}
            )
    add_definitions(-DRPC_PROTOCOL="cci+verbs")
endif()

option(USE_BMI "Use bmi plugin." OFF)
if(USE_BMI)
    find_package(BMI REQUIRED)
    set(NA_LIB
            ${NA_LIB}
            ${BMI_LIBRARIES}
            )
    add_definitions(-DRPC_PROTOCOL="bmi+tcp")
endif()

if (NOT USE_OFI_VERBS AND NOT USE_OFI_PSM2 AND NOT USE_CCI AND NOT USE_BMI)
    message("* No Mercury NA plugin selected. BMI automatically selected ... Use -DUSE_{BMI,CCI,OFI_VERBS,OFI_PSM2}:BOOL=ON for other plugins")
    find_package(BMI REQUIRED)
    set(NA_LIB
            ${NA_LIB}
            ${BMI_LIBRARIES}
            )
    add_definitions(-DRPC_PROTOCOL="bmi+tcp")
endif()
# boost dependencies, system is required for filesystem
find_package(Boost 1.53 REQUIRED COMPONENTS system filesystem)

# define include directories that are relevant for all targets
include_directories(
    include
    ${ABT_INCLUDE_DIRS}
    ${ABT_SNOOZER_INCLUDE_DIRS}
    ${LIBEV_INCLUDE_DIRS}
    ${MERCURY_INCLUDE_DIRS}
    ${MARGO_INCLUDE_DIRS}
)

# Daemon
add_subdirectory(src/daemon)

# Client library
add_subdirectory(src/preload)
