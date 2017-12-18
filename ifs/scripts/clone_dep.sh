#!/bin/bash

clonedeps() {
    FOLDER=$1
    GITCLONE=$2
    COMMIT=$3

    echo "#########################################################"
    echo "Cloning into $GIT/$FOLDER ..."


    if [ -d "$GIT/$FOLDER" ]; then
        echo "$FOLDER directory exists. Pulling instead."
        cd $GIT/$FOLDER && git pull origin master &>> $LOG
    else
        cd $GIT && $GITCLONE &>> $LOG
    fi
    # fix the version
    cd $GIT/$FOLDER && git checkout $COMMIT &>> $LOG
    echo "Done"
}

wgetdeps() {
    FOLDER=$1
    URL=$2
    echo "#########################################################"
    echo "Wgetting into $GIT/$FOLDER ..."
    if [ -d "$GIT/$FOLDER" ]; then
        echo "$FOLDER directory exists. Removing its content first."
        rm -rf $GIT/$FOLDER/* &>> $LOG
    else
        mkdir $GIT/$FOLDER
    fi
    cd $GIT
    FILENAME=$(basename $URL)
    if [ -f "$GIT/$FILENAME" ]; then
        rm $GIT/$FILENAME
    fi
    wget $URL &>> $LOG || exit 1
    tar -xf $FILENAME --directory $GIT/$FOLDER  &>> $LOG
    rm $FILENAME
    echo "Done"
}

usage() {

    echo "Usage:
    ./clone_dep [ clone_path ] [ NA_Plugin ]
    Valid NA_Plugin arguments: {bmi,cci,ofi,all}
    Valid cluster arguments: {mogon1}"
}

if [[ ( -z ${1+x} ) || ( -z ${2+x} ) ]]; then
    echo "Arguments missing."
    usage
    exit
fi
# if cluster is given, put it into a variable
CLUSTER=""
if [[ ! (-z ${3+x} ) ]]; then
    CLUSTER=$3
fi

LOG=/tmp/adafs_clone.log
echo "" &> $LOG
GIT=$1
NA_LAYER=$2
# sanity checks
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

echo "Clone path is set to '$1'"
echo "Cloning output is logged at /tmp/adafs_clone.log"

mkdir -p $GIT

# get cluster dependencies
if [ "$CLUSTER" == "mogon1" ]; then
    # get libtool for cci
    wgetdeps "libtool" "http://ftpmirror.gnu.org/libtool/libtool-2.4.6.tar.gz"
    # get libev for mercury
    wgetdeps "libev" "http://dist.schmorp.de/libev/libev-4.24.tar.gz"
    # get gflags for rocksdb
    wgetdeps "gflags" "https://github.com/gflags/gflags/archive/v2.2.1.tar.gz"
    # get zstd for fast compression in rocksdb
    wgetdeps "zstd" "https://github.com/facebook/zstd/archive/v1.3.2.tar.gz"
fi

# get BMI
if [ "$NA_LAYER" == "bmi" ] || [ "$NA_LAYER" == "all" ]; then
    clonedeps "bmi" "git clone git://git.mcs.anl.gov/bmi" "2abbe991edc45b713e64c5fed78a20fdaddae59b"
fi
# get CCI
if [ "$NA_LAYER" == "cci" ] || [ "$NA_LAYER" == "all" ]; then
    clonedeps "cci" "git clone https://github.com/CCI/cci" "58fd58ea2aa60c116c2b77c5653ae36d854d78f2"
fi
# get libfabric
if [ "$NA_LAYER" == "ofi" ] || [ "$NA_LAYER" == "all" ]; then
    clonedeps "libfabric" "git clone https://github.com/ofiwg/libfabric" "tags/v1.5.2"
fi
# get Mercury
clonedeps "mercury" "git clone --recurse-submodules https://github.com/mercury-hpc/mercury" "afd70055d21a6df2faefe38d5f6ce1ae11f365a5"
# get Argobots
clonedeps "argobots" "git clone -b dev-get-dev-basic https://github.com/carns/argobots.git" "a5a6b2036c75ad05804ccb72d2fe31cea1bfef88"
# get Argobots-snoozer
clonedeps "abt-snoozer" "git clone https://xgitlab.cels.anl.gov/sds/abt-snoozer.git" "3d9240eda290bfb89f08a5673cebd888194a4bd7"
# get Margo
clonedeps "margo" "git clone https://xgitlab.cels.anl.gov/sds/margo.git" "68ef7f14178e9066cf38846d90d451e00aaca61d"
# get rocksdb
clonedeps "rocksdb" "git clone https://github.com/facebook/rocksdb" "tags/v5.8"

echo "Nothing left to do. Exiting."
