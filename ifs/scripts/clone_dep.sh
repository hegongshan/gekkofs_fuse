#!/bin/bash

assertdir()
{
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

if [ -z ${1+x} ]; then
    echo "No git clone destination path given as first parameter.";
    exit
else
    echo "Clone path is set to '$1'";
    echo "Cloning output is logged at /tmp/adafs_clone.log"
fi

LOG=/tmp/adafs_clone.log
echo "" &> $LOG
GIT=$1

mkdir -p $GIT

# get BMI
assertdir "bmi" "git clone git://git.mcs.anl.gov/bmi" "2abbe991edc45b713e64c5fed78a20fdaddae59b"
# get Mercury
assertdir "mercury" "git clone --recurse-submodules https://github.com/mercury-hpc/mercury" "afd70055d21a6df2faefe38d5f6ce1ae11f365a5"
# get Argobots
assertdir "argobots" "git clone -b dev-get-dev-basic https://github.com/carns/argobots.git" "a5a6b2036c75ad05804ccb72d2fe31cea1bfef88"
# get Argobots-snoozer
assertdir "abt-snoozer" "git clone https://xgitlab.cels.anl.gov/sds/abt-snoozer.git" "3d9240eda290bfb89f08a5673cebd888194a4bd7"
# get Margo
assertdir "margo" "git clone https://xgitlab.cels.anl.gov/sds/margo.git" "68ef7f14178e9066cf38846d90d451e00aaca61d"
# get rocksdb
assertdir "rocksdb" "git clone https://github.com/facebook/rocksdb" "tags/v5.8"

echo "Nothing left to do. Exiting."
