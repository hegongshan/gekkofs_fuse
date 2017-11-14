#!/bin/bash

if [[ ( -z ${1+x} ) || ( -z ${2+x} ) ]]; then
    echo "Please give git destination path as first parameter and install path as second";
    exit
else
    echo "Git path is set to '$1'";
    echo "Install path is set to '$2'";
#    echo "Install output is logged at /tmp/adafs_dep_install.log"
fi

#LOG=/tmp/adafs_install.log
#echo "" &> $LOG
GIT=$1
INSTALL=$2

mkdir -p $GIT

prepare_build_dir() {
    if [ ! -d "$1/build" ]; then
        mkdir $1/build
    fi
    rm -rf $1/build/*
}

echo "Installing BMI"
# BMI
CURR=$GIT/bmi
prepare_build_dir $CURR
cd $CURR
./prepare || exit 1
cd $CURR/build
../configure --prefix=$INSTALL --enable-shared --enable-bmi-only  || exit 1
make -j8 || exit 1
make install || exit 1

echo "Installing Mercury"

# Mercury
CURR=$GIT/mercury
prepare_build_dir $CURR
cd $CURR/build
cmake -DMERCURY_USE_SELF_FORWARD:BOOL=ON -DMERCURY_USE_CHECKSUMS:BOOL=OFF -DBUILD_TESTING:BOOL=ON \
-DMERCURY_USE_BOOST_PP:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_INSTALL_PREFIX=$INSTALL \
-DCMAKE_BUILD_TYPE:STRING=Release -DNA_USE_BMI:BOOL=ON ../  || exit 1
make -j8  || exit 1
make install  || exit 1

echo "Installing Argobots"

# Argobots
CURR=$GIT/argobots
prepare_build_dir $CURR
cd $CURR
./autogen.sh || exit 1
cd $CURR/build
../configure --prefix=$INSTALL || exit 1
make -j8 || exit 1
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
make -j8 || exit 1
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
make -j8 || exit 1
make install || exit 1
make check || exit 1

echo "Installing Rocksdb"
# Margo
CURR=$GIT/rocksdb
cd $CURR
make clean || exit 1
sed -i.bak "s#INSTALL_PATH ?= /usr/local#INSTALL_PATH ?= $INSTALL#g" Makefile
make -j8 static_lib || exit 1
make install || exit 1

echo "Done"