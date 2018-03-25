#!/bin/bash
# Slurm stuff

#SBATCH -J adafs_mdtest
#SBATCH -p nodeshort
#SBATCH -t 300
#SBATCH -A zdvresearch
#SBATCH --gres=ramdisk:16G

usage_short() {
        echo "
usage: mogon1_ior_ramdisk.sh [-h] [-n <PROC_PER_NODE>] [-b <BLOCKSIZE>] [-i <ITER>] [-Y] [-p]
                             [-t <TRANSFERSIZES>] [-s] [-r] [-v]
                             benchmark_dir+file_prefix
        "
}

help_msg() {

        usage_short
    echo "
This slurm batch script is for IOR testing adafs

positional arguments:
        benchmark_dir           benchmark workdir


optional arguments:
        -h, --help
                                shows this help message and exits

        -n <PROC_PER_NODE>, --nodes <PROC_PER_NODE>
                                number of processes per node
                                defaults to '16'
        -i <ITER>, --iterations <ITER>
                                number of iterations done around IOR
                                defaults to '1'
        -b <BLOCKSIZE>, --blocksize <BLOCKSIZE>
                                total number of data written and read (use 1k, 1m, 1g, etc...)
                                defaults to '16m'
        -t <TRANSFERSIZES>, --transfersizes <TRANSFERSIZES>
                                Sets the transfer sizes for the block sizes. Set a space separated list.
                                Each transfer size must be a multiple of the block size
                                Example: \"64m 32m 16m 8m 4m 2m 1m 512k 256k 128k 4k 1k\"
                                Defaults to example
        -s, --striping
                                Enable random striping for readback. A random seed of 42 is used.
        -r, --random
                                Enable random offsets for I/O
        -Y, --fsync
                                enable fsync after writes
                                defaults to 'false'
        -v, --verbose
                                enable ior verbosity
        -p, --pretend
                                Pretend operation. Does not execute commands benchmark commands
                                This does start and stop the adafs daemon
        "
}
# Set default values
PROC_PER_NODE=16
ITER=1
BLOCKSIZE="64m"
FSYNC=false
PRETEND=false
STRIPING=false
RANDOM=false
VERBOSE=""
TRANSFERSIZES="64m 32m 16m 8m 4m 2m 1m 512k 256k 128k 4k 1k"
START_TIME="$(date -u +%s)"

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case ${key} in
    -n|--nodes)
    PROC_PER_NODE="$2"
    shift # past argument
    shift # past value
    ;;
    -b|--blocksize)
    BLOCKSIZE="$2"
    shift # past argument
    shift # past value
    ;;
    -i|--iterations)
    ITER="$2"
    shift # past argument
    shift # past value
    ;;
    -t|--transfersizes)
    TRANSFERSIZES="$2"
    shift # past argument
    shift # past value
    ;;
    -Y|--fsync)
    FSYNC=true
    shift # past argument
    ;;
    -r|--random)
    RANDOM=true
    shift # past argument
    ;;
    -s|--striping)
    STRIPING=true
    shift # past argument
    ;;
    -p|--pretend)
    PRETEND=true
    shift # past argument
    ;;
    -v|--verbose)
    VERBOSE="-vv"
    shift # past argument
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

VEF_HOME="/home/vef"
HOSTFILE="${VEF_HOME}/jobdir/hostfile_${SLURM_JOB_ID}"
WORKDIR=$1
ROOTDIR="/localscratch/${SLURM_JOB_ID}/ramdisk"

# Load modules and set environment variables
PATH=$PATH:/home/vef/adafs/install/bin:/home/vef/.local/bin
C_INCLUDE_PATH=$C_INCLUDE_PATH:/home/vef/adafs/install/include
CPATH=$CPATH:/home/vef/adafs/install/include
CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:/home/vef/adafs/install/include
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/vef/adafs/install/lib
LIBRARY_PATH=$LIBRARY_PATH:/home/vef/adafs/install/lib
export PATH
export CPATH
export C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH
export LD_LIBRARY_PATH
export LIBRARY_PATH
export LDFLAGS='-L/home/vef/adafs/install/lib/'
export CPPFLAGS='-I/home/vef/adafs/install/include/'
module load devel/CMake/3.8.0
module load mpi/OpenMPI/2.0.2-GCC-6.3.0
module load devel/Boost/1.63.0-foss-2017a
export CC=$(which gcc)
export CXX=$(which g++)

# create a proper hostfile to run
srun -n ${SLURM_NNODES} hostname -s | sort -u > ${HOSTFILE} && sed -e 's/$/ max_slots=64/' -i ${HOSTFILE}

echo "Generated hostfile no of nodes:"
cat ${HOSTFILE} | wc -l

NONODES=$(cat ${HOSTFILE} | wc -l)
let IOR_PROC_N=${NONODES}*${PROC_PER_NODE}

echo "
############################################################################
############################### DAEMON START ############################### ############################################################################
"
# start adafs daemon on the nodes
python2 ${VEF_HOME}/ifs/scripts/startup_adafs.py -c -J ${SLURM_JOB_ID} --numactl "--cpunodebind=0,1 --membind=0,1" ${VEF_HOME}/ifs/build/bin/adafs_daemon ${ROOTDIR} ${WORKDIR} ${HOSTFILE}

# pssh to get logfiles. hostfile is created by startup script
${VEF_HOME}/.local/bin/pssh -O StrictHostKeyChecking=no -i -h /tmp/hostfile_pssh_${SLURM_JOB_ID} "tail /tmp/adafs_daemon.log"

echo "
############################################################################
############################ RUNNING BENCHMARK #############################
############################################################################
"
# Run benchmark

BENCH_TMPL="mpiexec -np ${IOR_PROC_N} --map-by node --hostfile ${HOSTFILE} -x LD_PRELOAD=/gpfs/fs2/project/zdvresearch/vef/fs/ifs/build/lib/libadafs_preload_client.so numactl --cpunodebind=2,3,4,5,6,7 --membind=2,3,4,5,6,7 /gpfs/fs1/home/vef/benchmarks/mogon1/ior/build/src/ior -a POSIX -i 1 -o ${WORKDIR} -b ${BLOCKSIZE} ${VERBOSE} -x -F -w -r -W"

echo "##########################"
echo "< 1. WARMUP              >"
echo "##########################"
for ((i=1;i<=3;i+=1))
do
    CMD="${BENCH_TMPL} -t 16m"
    echo "## Command ${CMD}"
    if [ "${PRETEND}" = false ] ; then
        eval ${CMD}
    fi
done
# Run experiments
echo "##########################"
echo "< 2. RUNNING EXPERIMENTS >"
echo "##########################"
# Some info output
if [ "${RANDOM}" = true ] ; then
    echo "## RANDOM I/O on"
fi
if [ "${STRIPING}" = true ] ; then
    echo "## STRIPING on"
fi
if [ "${FSYNC}" = true ] ; then
    echo "## FSYNC on"
fi
for TRANSFER in ${TRANSFERSIZES}
do
    echo "<new_transfer_size>;${TRANSFER}"
    for ((i=1;i<=${ITER};i+=1))
    do
        echo "<new_iteration>;$i"
        # build command from template and then execute it
        CMD="${BENCH_TMPL} -t ${TRANSFER}"
        echo "## iteration $i/${ITER} transfer size ${TRANSFER}"
        if [ "${RANDOM}" = true ] ; then
            CMD="${CMD} -z"
        fi
        if [ "${STRIPING}" = true ] ; then
            CMD="${CMD} -Z -X 42"
        fi
        if [ "${FSYNC}" = true ] ; then
            CMD="${CMD} -Y"
        fi
        echo "## Command ${CMD}"
        if [ "${PRETEND}" = false ] ; then
            eval ${CMD}
        fi
        echo "<finish_iteration>;$i"
        echo "### iteration $i/${ITER} done"
    done
    echo "<finish_transfer_size>;${TRANSFER}"
    echo "## new transfer size #################################"
done

echo "
############################################################################
############################### DAEMON STOP ############################### ############################################################################
"
END_TIME="$(date -u +%s)"
ELAPSED="$((${END_TIME}-${START_TIME}))"
MINUTES=$((${ELAPSED} / 60))
echo "##Elapsed time: ${MINUTES} minutes or ${ELAPSED} seconds elapsed for test set."
# shut down adafs daemon on the nodes
python2 ${VEF_HOME}/ifs/scripts/shutdown_adafs.py -J ${SLURM_JOB_ID} ${VEF_HOME}/ifs/build/bin/adafs_daemon ${HOSTFILE}

# cleanup
rm ${HOSTFILE}
