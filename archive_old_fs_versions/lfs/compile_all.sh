#!/bin/bash
HOME=/home/evie
GIT=$HOME/adafs/git
INSTALL=$HOME/adafs/install

#echo "Installing BMI"
# BMI
#CURR=$GIT/bmi

#rm -rf $CURR/build/*
#cd $CURR
#./prepare
#cd $CURR/build
#../configure --prefix=$INSTALL --enable-shared --enable-bmi-only
#make -j8
#make install

echo "Installing CCI"
# CCI
CURR=$GIT/cci
./autogen.pl
if [ ! -d "$CURR/build" ]; then
	mkdir $CURR/build
fi
cd $CURR/build
../configure --prefix=$INSTALL
make -j8
make install

echo "Installing Mercury"

# Mercury 
CURR=$GIT/mercury
if [ ! -d "$CURR/build" ]; then
	mkdir $CURR/build
fi
rm -rf $CURR/build/*
cd $CURR/build
cmake -DMERCURY_USE_SELF_FORWARD:BOOL=ON -DMERCURY_USE_CHECKSUMS:BOOL=OFF -DBUILD_TESTING:BOOL=ON -DMERCURY_USE_BOOST_PP:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_INSTALL_PREFIX=$INSTALL -DCMAKE_BUILD_TYPE:STRING=Release -DNA_USE_CCI:BOOL=ON ../
make -j8
make install

echo "Installing Argobots"

# Argobots
CURR=$GIT/argobots
if [ ! -d "$CURR/build" ]; then
	mkdir $CURR/build
fi
rm -rf $CURR/build/*
cd $CURR
./autogen.sh
cd $CURR/build
../configure --prefix=$INSTALL 
make -j8
make install
make check

echo "Installing Abt-snoozer"
# Abt snoozer
CURR=$GIT/abt-snoozer
if [ ! -d "$CURR/build" ]; then
	mkdir $CURR/build
fi
rm -rf $CURR/build/*
cd $CURR
./prepare.sh
cd $CURR/build
../configure --prefix=$INSTALL PKG_CONFIG_PATH=$INSTALL/lib/pkgconfig
make -j8
make install
make check

echo "Installing Margo"
# Margo
CURR=$GIT/margo
if [ ! -d "$CURR/build" ]; then
	mkdir $CURR/build
fi
rm -rf $CURR/build/*
cd $CURR
./prepare.sh
cd $CURR/build
../configure --prefix=$INSTALL PKG_CONFIG_PATH=$INSTALL/lib/pkgconfig CFLAGS="-g -Wall"
make -j8
make install
make check

echo "Done"
