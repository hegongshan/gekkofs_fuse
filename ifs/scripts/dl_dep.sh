#!/bin/bash

clonedeps() {
    FOLDER=$1
    CLONE=$2
    COMMIT=$3

    echo "#########################################################"
    echo "Cloning into ${SOURCE}/${FOLDER} ..."


    if [ -d "${SOURCE}/${FOLDER}" ]; then
        echo "${FOLDER} directory exists. Pulling instead."
        cd ${SOURCE}/${FOLDER} && git pull origin master &>> ${LOG}
    else
        cd ${SOURCE} && ${CLONE} &>> ${LOG}
    fi
    # fix the version
    cd ${SOURCE}/${FOLDER} && git checkout ${COMMIT} &>> ${LOG}
    echo "Done"
}

wgetdeps() {
    FOLDER=$1
    URL=$2
    echo "#########################################################"
    echo "Wgetting into ${SOURCE}/${FOLDER} ..."
    if [ -d "${SOURCE}/${FOLDER}" ]; then
        echo "${FOLDER} directory exists. Removing its content first."
        rm -rf ${SOURCE}/${FOLDER}/* &>> ${LOG}
    else
        mkdir ${SOURCE}/${FOLDER}
    fi
    cd ${SOURCE}
    FILENAME=$(basename $URL)
    if [ -f "${SOURCE}/$FILENAME" ]; then
        rm ${SOURCE}/$FILENAME
    fi
    wget $URL &>> ${LOG} || exit 1
    tar -xf $FILENAME --directory ${SOURCE}/${FOLDER} --strip-components=1 &>> ${LOG}
    rm $FILENAME
    echo "Done"
}

usage_short() {
	echo "
usage: dl_dep.sh [-h] [-n <NAPLUGIN>] [-c <CLUSTER>]
                    source_path
	"
}

help_msg() {

        usage_short
    echo "
This script gets all ADA-FS dependency sources (excluding the fs itself)

positional arguments:
        source_path              path where the dependency downloads are put


optional arguments:
        -h, --help              shows this help message and exits
        -n <NAPLUGIN>, --na <NAPLUGIN>
                                network layer that is used for communication. Valid: {bmi,cci,ofi,all}
                                defaults to 'all'
        -c <CLUSTER>, --cluster <CLUSTER>
                                additional configurations for specific compute clusters
                                supported clusters: {mogon1,fh2}
        "
}
CLUSTER=""
NA_LAYER=""

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case ${key} in
    -n|--na)
    NA_LAYER="$2"
    shift # past argument
    shift # past value
    ;;
    -c|--cluster)
    CLUSTER="$2"
    shift # past argument
    shift # past value
    ;;
    -h|--help)
    help_msg
    exit
    #shift # past argument
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

# positional arguments
if [[ -z ${1+x} ]]; then
    echo "Positional arguments missing."
    usage_short
    exit
fi
SOURCE=$1

LOG="/tmp/adafs_download_deps.log"
echo "" &> ${LOG}
# optional arguments
if [ "${NA_LAYER}" == "" ]; then
        echo "Defaulting NAPLUGIN to 'all'"
        NA_LAYER="all"
fi

# sanity checks
if [[ ( "${NA_LAYER}" == "cci" ) || ( "${NA_LAYER}" == "bmi" ) || ( "${NA_LAYER}" == "ofi" ) || ( "${NA_LAYER}" == "all" ) ]]; then
	echo NAPLUGIN = "${NA_LAYER}"
else
    echo "No valid plugin selected"
    usage_short
    exit
fi
if [[ "${CLUSTER}" != "" ]]; then
	if [[ ( "${CLUSTER}" == "mogon1" ) || ( "${CLUSTER}" == "fh2" ) ]]; then
		echo CLUSTER  = "${CLUSTER}"
    else
        echo "${CLUSTER} cluster configuration is invalid. Exiting ..."
        usage_short
        exit
    fi
else
    echo "No cluster configuration set."
fi

echo "Source path is set to '$1'"
echo "Download progress is logged at /tmp/adafs_download_deps.log"

mkdir -p ${SOURCE}

# get cluster dependencies
if [[ ( "${CLUSTER}" == "mogon1" ) || ( "${CLUSTER}" == "fh2" ) ]]; then
    # get libtool for cci
    wgetdeps "libtool" "https://ftp.gnu.org/gnu/libtool/libtool-2.4.6.tar.gz"
    # get libev for mercury
    wgetdeps "libev" "http://dist.schmorp.de/libev/libev-4.24.tar.gz"
    # get gflags for rocksdb
    wgetdeps "gflags" "https://github.com/gflags/gflags/archive/v2.2.1.tar.gz"
    # get zstd for fast compression in rocksdb
    wgetdeps "zstd" "https://github.com/facebook/zstd/archive/v1.3.2.tar.gz"
    # get zlib for rocksdb
    wgetdeps "lz4" "https://github.com/lz4/lz4/archive/v1.8.0.tar.gz"
	# get snappy for rocksdb
    wgetdeps "snappy" "https://github.com/google/snappy/archive/1.1.7.tar.gz"
fi
#if [ "${CLUSTER}" == "fh2" ]; then
	# no distinct 3rd party software needed as of now.
#fi

# get BMI
if [ "${NA_LAYER}" == "bmi" ] || [ "${NA_LAYER}" == "all" ]; then
    clonedeps "bmi" "git clone git://git.mcs.anl.gov/bmi" "2abbe991edc45b713e64c5fed78a20fdaddae59b"
fi
# get CCI
if [ "${NA_LAYER}" == "cci" ] || [ "${NA_LAYER}" == "all" ]; then
    clonedeps "cci" "git clone https://github.com/CCI/cci" "58fd58ea2aa60c116c2b77c5653ae36d854d78f2"
fi
# get libfabric
if [ "${NA_LAYER}" == "ofi" ] || [ "${NA_LAYER}" == "all" ]; then
    clonedeps "libfabric" "git clone https://github.com/ofiwg/libfabric" "tags/v1.5.3"
fi
# get Mercury
clonedeps "mercury" "git clone --recurse-submodules https://github.com/mercury-hpc/mercury" "6c82baf7819a553b6b8235fbe7c180989a1e17fe"
# get Argobots
clonedeps "argobots" "git clone -b dev-get-dev-basic https://github.com/carns/argobots.git" "78ceea28ed44faca12cf8ea7f5687b894c66a8c4"
# get Argobots-snoozer
clonedeps "abt-snoozer" "git clone https://xgitlab.cels.anl.gov/sds/abt-snoozer.git" "3d9240eda290bfb89f08a5673cebd888194a4bd7"
# get Margo
clonedeps "margo" "git clone https://xgitlab.cels.anl.gov/sds/margo.git" "3f9fe3a13392af1ba6df1b3d3bb16503da6b627d"
# get rocksdb
clonedeps "rocksdb" "git clone https://github.com/facebook/rocksdb" "tags/v5.10.2"

echo "Nothing left to do. Exiting."
