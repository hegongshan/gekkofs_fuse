#!/bin/bash

usage() {

    echo "Usage:
    ./compile_dep [ clone_path ] [ install_path ] [ na_plugin ] [ cluster ]
    Valid na_plugin arguments: {bmi,cci,ofi,all}
    Valid cluster arguments: {mogon1}"
}

prepare_build_dir() {
    if [ ! -d "$1/build" ]; then
        mkdir $1/build
    fi
    rm -rf $1/build/*
}

if [[ ( -z ${1+x} ) || ( -z ${2+x} ) || ( -z ${3+x} ) ]]; then
    echo "Arguments missing."
    usage
    exit
fi
# if cluster is given, put it into a variable
CLUSTER=""
if [[ ! (-z ${4+x} ) ]]; then
    CLUSTER=$4
fi

#LOG=/tmp/adafs_install.log
#echo "" &> $LOG
GIT=$1
INSTALL=$2
NA_LAYER=$3
USE_BMI="-DNA_USE_BMI:BOOL=OFF"
USE_CCI="-DNA_USE_CCI:BOOL=OFF"
USE_OFI="-DNA_USE_OFI:BOOL=OFF"

CORES=$(grep -c ^processor /proc/cpuinfo)

if [ "$NA_LAYER" == "cci" ] || [ "$NA_LAYER" == "bmi" ] || [ "$NA_LAYER" == "ofi" ] || [ "$NA_LAYER" == "all" ]; then
    echo "$NA_LAYER plugin(s) selected"
else
    echo "No valid plugin selected"
    usage
    exit
fi

if [ "$CLUSTER" != "" ]; then
    if [ "$CLUSTER" == "mogon1" ]; then
        echo "$CLUSTER cluster configuration selected"
    else
        echo "$CLUSTER cluster configuration is invalid. Exiting ..."
        usage
        exit
    fi
else
    echo "No cluster configuration set."
fi



echo "Git path is set to '$1'";
echo "Install path is set to '$2'";

mkdir -p $GIT

# Set cluster dependencies first
if [ "$CLUSTER" == "mogon1" ]; then
    # load required modules
    echo "Setting cluster module settings ..."
    module load devel/CMake/3.8.0 || exit 1
    module load mpi/MVAPICH2/2.2-GCC-6.3.0-slurm || exit 1
    module load devel/Boost/1.63.0-foss-2017a || exit 1 # because of mercury
    echo "Done"
    # get libtool
    echo "Installing libtool"
    CURR=$GIT/libtool
    prepare_build_dir $CURR
    cd $CURR/build
    ../configure --prefix=$INSTALL || exit 1
    make -j$CORES || exit 1
    make install || exit 1
    # compile libev
    echo "Installing libev"
    CURR=$GIT/libev
    prepare_build_dir $CURR
    cd $CURR/build
    ../configure --prefix=$INSTALL || exit 1
    make -j$CORES || exit 1
    make install || exit 1
    # compile gflags
#    echo "Installing gflags"
#    CURR=$GIT/gflags
#    prepare_build_dir $CURR
#    cd $CURR/build
#    cmake -DCMAKE_INSTALL_PREFIX=$INSTALL -DCMAKE_BUILD_TYPE:STRING=Release .. || exit 1
#    make -j$CORES || exit 1
#    make install || exit 1
    # compile zstd
    echo "Installing zstd"
    CURR=$GIT/zstd/build/cmake
    prepare_build_dir $CURR
    cd $CURR/build
    cmake -DCMAKE_INSTALL_PREFIX=$INSTALL -DCMAKE_BUILD_TYPE:STRING=Release .. || exit 1
    make -j$CORES || exit 1
    make install || exit 1
    echo "Installing lz4"
    CURR=$GIT/lz4
    make -j$CORES || exit 1
    make DESTDIR=$INSTALL PREFIX= install || exit 1
    echo "Installing snappy"
    CURR=$GIT/snappy
    prepare_build_dir $CURR
    cd $CURR/build
    cmake -DCMAKE_INSTALL_PREFIX=$INSTALL -DCMAKE_BUILD_TYPE:STRING=Release .. || exit 1
    make -j$CORES || exit 1
    make install || exit 1


fi

if [ "$NA_LAYER" == "bmi" ] || [ "$NA_LAYER" == "all" ]; then
    USE_BMI="-DNA_USE_BMI:BOOL=ON"
    echo "Installing BMI"
    # BMI
    CURR=$GIT/bmi
    prepare_build_dir $CURR
    cd $CURR
    ./prepare || exit 1
    cd $CURR/build
    ../configure --prefix=$INSTALL --enable-shared --enable-bmi-only  || exit 1
    make -j$CORES || exit 1
    make install || exit 1
fi

if [ "$NA_LAYER" == "cci" ] || [ "$NA_LAYER" == "all" ]; then
    USE_CCI="-DNA_USE_CCI:BOOL=ON"
    echo "Installing CCI"
    # CCI
    CURR=$GIT/cci
    prepare_build_dir $CURR
    cd $CURR
    ./autogen.pl || exit 1
    cd $CURR/build
    ../configure --with-verbs --prefix=$INSTALL LIBS="-lpthread"  || exit 1
    make -j$CORES || exit 1
    make install || exit 1
    make check || exit 1
fi

if [ "$NA_LAYER" == "ofi" ] || [ "$NA_LAYER" == "all" ]; then
    USE_OFI="-DNA_USE_OFI:BOOL=ON"
    echo "Installing LibFabric"
    #libfabric
    CURR=$GIT/libfabric
    prepare_build_dir $CURR
    cd $CURR
    ./autogen.sh || exit 1
    cd $CURR/build
    ../configure --prefix=$INSTALL  || exit 1
    make -j$CORES || exit 1
    make install || exit 1
    make check || exit 1
fi

echo "Installing Mercury"

# Mercury
CURR=$GIT/mercury
prepare_build_dir $CURR
cd $CURR/build
# XXX Note: USE_EAGER_BULK is temporarily disabled due to bugs in Mercury with smaller amounts of data
cmake -DMERCURY_USE_SELF_FORWARD:BOOL=ON -DMERCURY_USE_CHECKSUMS:BOOL=OFF -DBUILD_TESTING:BOOL=ON \
-DMERCURY_USE_BOOST_PP:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_INSTALL_PREFIX=$INSTALL \
-DCMAKE_BUILD_TYPE:STRING=Release -DMERCURY_USE_EAGER_BULK:BOOL=OFF $USE_BMI $USE_CCI $USE_OFI ../  || exit 1
make -j$CORES  || exit 1
make install  || exit 1

echo "Installing Argobots"

# Argobots
CURR=$GIT/argobots
prepare_build_dir $CURR
cd $CURR
./autogen.sh || exit 1
cd $CURR/build
../configure --prefix=$INSTALL || exit 1
make -j$CORES || exit 1
make install || exit 1
make check || exit 1

echo "Installing Abt-snoozer"
# Abt snoozer
CURR=$GIT/abt-snoozer
prepare_build_dir $CURR
cd $CURR
./prepare.sh || exit 1
cd $CURR/build
../configure --prefix=$INSTALL PKG_CONFIG_PATH=$INSTALL/lib/pkgconfig || exit 1
make -j$CORES || exit 1
make install || exit 1
make check || exit 1

echo "Installing Margo"
# Margo
CURR=$GIT/margo
prepare_build_dir $CURR
cd $CURR
./prepare.sh || exit 1
cd $CURR/build
../configure --prefix=$INSTALL PKG_CONFIG_PATH=$INSTALL/lib/pkgconfig CFLAGS="-g -Wall" || exit 1
make -j$CORES || exit 1
make install || exit 1
make check || exit 1

echo "Installing Rocksdb"
# Margo
CURR=$GIT/rocksdb
cd $CURR
make clean || exit 1
sed -i.bak "s#INSTALL_PATH ?= /usr/local#INSTALL_PATH ?= $INSTALL#g" Makefile
make -j$CORES static_lib || exit 1
make install || exit 1

echo "Done"
