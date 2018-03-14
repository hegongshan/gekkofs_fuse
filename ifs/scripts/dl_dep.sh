#!/bin/bash

#set -x

COMMON_WGET_FLAGS="--no-verbose"
COMMON_GIT_FLAGS="--quiet --single-branch"

# Stop all backround jobs on interruption.
# "kill -- -$$" sends a SIGTERM to the whole process group,
# thus killing also descendants.
trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

clonedeps() {
    local FOLDER=$1
    local REPO=$2
    local COMMIT=$3
    local GIT_FLAGS=$4

    local ACTION

    if [ -d "${SOURCE}/${FOLDER}/.git" ]; then
        cd ${SOURCE}/${FOLDER} && git fetch -q || exit 1
        ACTION="Pulled"
    else
        git clone ${COMMON_GIT_FLAGS} ${GIT_FLAGS} -- "${REPO}" "${SOURCE}/${FOLDER}" > /dev/null || exit 1
        ACTION="Cloned"
    fi
    # fix the version
    cd "${SOURCE}/${FOLDER}" && git checkout -qf ${COMMIT} || exit 1
    echo "${ACTION} ${FOLDER} [$COMMIT]"
}

wgetdeps() {
    FOLDER=$1
    URL=$2
    if [ -d "${SOURCE}/${FOLDER}" ]; then
        rm -rf "${SOURCE}/${FOLDER}"
    else
        mkdir -p "${SOURCE}/${FOLDER}"
    fi
    cd ${SOURCE}
    FILENAME=$(basename $URL)
    if [ -f "${SOURCE}/$FILENAME" ]; then
        rm -f "${SOURCE}/$FILENAME"
    fi
    wget -q "$URL" || exit 1
    tar -xf "$FILENAME" --directory "${SOURCE}/${FOLDER}" --strip-components=1 || exit 1 
    rm -f "$FILENAME"
    echo "Downloaded ${FOLDER}"
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
SOURCE="$( readlink -f "${1}" )"

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

echo "Source path is set to  \"${SOURCE}\""

mkdir -p ${SOURCE}

# get cluster dependencies
if [[ ( "${CLUSTER}" == "mogon1" ) || ( "${CLUSTER}" == "fh2" ) ]]; then
    # get libtool for cci
    wgetdeps "libtool" "https://ftp.gnu.org/gnu/libtool/libtool-2.4.6.tar.gz" &
    # get libev for mercury
    wgetdeps "libev" "http://dist.schmorp.de/libev/libev-4.24.tar.gz" &
    # get gflags for rocksdb
    wgetdeps "gflags" "https://github.com/gflags/gflags/archive/v2.2.1.tar.gz" &
    # get zstd for fast compression in rocksdb
    wgetdeps "zstd" "https://github.com/facebook/zstd/archive/v1.3.2.tar.gz" &
    # get zlib for rocksdb
    wgetdeps "lz4" "https://github.com/lz4/lz4/archive/v1.8.0.tar.gz" &
	# get snappy for rocksdb
    wgetdeps "snappy" "https://github.com/google/snappy/archive/1.1.7.tar.gz" &
fi
#if [ "${CLUSTER}" == "fh2" ]; then
	# no distinct 3rd party software needed as of now.
#fi

# get BMI
if [ "${NA_LAYER}" == "bmi" ] || [ "${NA_LAYER}" == "all" ]; then
    clonedeps "bmi" "git://git.mcs.anl.gov/bmi" "2abbe991edc45b713e64c5fed78a20fdaddae59b" &
fi
# get CCI
if [ "${NA_LAYER}" == "cci" ] || [ "${NA_LAYER}" == "all" ]; then
    clonedeps "cci" "https://github.com/CCI/cci" "58fd58ea2aa60c116c2b77c5653ae36d854d78f2" &
fi
# get libfabric
if [ "${NA_LAYER}" == "ofi" ] || [ "${NA_LAYER}" == "all" ]; then
    wgetdeps "libfabric" "https://github.com/ofiwg/libfabric/archive/v1.5.3.tar.gz" &
fi
# get Mercury
clonedeps "mercury" "https://github.com/mercury-hpc/mercury" "c4faa382fd228c0b629c9164a984df1779089d3f"  "--recurse-submodules" &
# get Argobots
clonedeps "argobots" "https://github.com/carns/argobots.git" "78ceea28ed44faca12cf8ea7f5687b894c66a8c4" "-b dev-get-dev-basic" &
# get Argobots-snoozer
clonedeps "abt-snoozer" "https://xgitlab.cels.anl.gov/sds/abt-snoozer.git" "3d9240eda290bfb89f08a5673cebd888194a4bd7" &
# get Argobots-IO
#clonedeps "abt-io" "https://xgitlab.cels.anl.gov/sds/abt-io.git" "35f16da88a1c579ed4726bfa77daa1884829fc0c" &
# get Margo
clonedeps "margo" "https://xgitlab.cels.anl.gov/sds/margo.git" "72eec057314a4251d8658e03a18240275992e1ce" &
# get rocksdb
wgetdeps "rocksdb" "https://github.com/facebook/rocksdb/archive/v5.10.3.tar.gz" &

# Wait for all download to be completed 
wait
