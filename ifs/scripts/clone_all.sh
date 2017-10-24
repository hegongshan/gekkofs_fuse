#!/bin/bash
HOME=/home/evie
GIT=$HOME/adafs/git

mkdir -p $GIT
cd $GIT

git clone git://git.mcs.anl.gov/bmi

cd $GIT
git clone --recurse-submodules https://github.com/mercury-hpc/mercury
git clone https://github.com/pmodels/argobots
cd $GIT/argobots
git checkout tags/v1.0a1

cd $GIT
git clone https://xgitlab.cels.anl.gov/sds/abt-snoozer
git clone https://xgitlab.cels.anl.gov/sds/margo
