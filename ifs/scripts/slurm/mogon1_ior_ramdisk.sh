#!/bin/bash
# Slurm stuff

#SBATCH -J adafs_mdtest
#SBATCH -p nodeshort
#SBATCH -t 300
#SBATCH -A zdvresearch
#SBATCH --gres=ramdisk:20G

usage_short() {
        echo "
usage: mogon1_ior_ramdisk.sh [-h] [-n <PROC_PER_NODE>] [-b <BLOCKSIZE>] [-i <ITER>] [-Y] [-p]
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

        -n <PROC_PER_NODE>
                                number of processes per node
                                defaults to '16'
        -b <BLOCKSIZE>
                                total number of data written and read (use 1k, 1m, 1g, etc...)
                                defaults to '1m'
        -i <ITER>
                                number of iterations done around IOR
                                defaults to '1'
        -Y, --fsync
                                use fsync after writes
                                defaults to 'false'
        -p, --pretend
                                Pretend operation. Does not execute commands benchmark commands
                                This does start and stop the adafs daemon
        "
}

PROC_PER_NODE=16
ITER=1
BLOCKSIZE="1m"
FSYNC=false
PRETEND=false

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case ${key} in
    -n)
    PROC_PER_NODE="$2"
    shift # past argument
    shift # past value
    ;;
    -b)
    BLOCKSIZE="$2"
    shift # past argument
    shift # past value
    ;;
    -i)
    ITER="$2"
    shift # past argument
    shift # past value
    ;;
    -Y|--fsync)
    FSYNC=true
    shift # past argument
    ;;
    -p|--pretend)
    PRETEND=true
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
let MD_PROC_N=${NONODES}*16

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

BENCH_TMPL="mpiexec -np ${PROC_PER_NODE} --map-by node --hostfile ${HOSTFILE} -x LD_PRELOAD=/gpfs/fs2/project/zdvresearch/vef/fs/ifs/build/lib/libadafs_preload_client.so ior -a POSIX -i 1 -o ${WORKDIR} -b ${BLOCKSIZE} -F -w -r -W"

echo "#############"
echo "# 1. SEQUEL #"
echo "#############"
for TRANSFER in 4k 256k 512k 1m 2m 4m 8m 16m
do
    for i in {1..${ITER}}
    do
        CMD="${BENCH_TMPL} -t ${TRANSFER}"
        echo "## iteration $i"
        echo "## transfer size ${TRANSFER}"
        if [ "${FSYNC}" = true ] ; then
            CMD="${CMD} -Y"
            echo "## FSYNC on"
        fi
        echo "## Command ${CMD}"
        if [ "${PRETEND}" = true ] ; then
            eval ${CMD}
        fi
    done
done

echo "#############"
echo "# 2. RANDOM #"
echo "#############"
for TRANSFER in 4k 256k 512k 1m 2m 4m 8m 16m
do
    for i in {1..${ITER}}
    do
        CMD="${BENCH_TMPL} -t ${TRANSFER} -z"
        echo "## iteration $i"
        echo "## transfer size ${TRANSFER}"
        if [ "${FSYNC}" = true ] ; then
            CMD="${CMD} -Y"
            echo "## FSYNC on"
        fi
        echo "## Command ${CMD}"
        if [ "${PRETEND}" = true ] ; then
            eval ${CMD}
        fi
    done
done

# TODO 3. Striped later

echo "
############################################################################
############################### DAEMON STOP ############################### ############################################################################
"
# shut down adafs daemon on the nodes
python2 ${VEF_HOME}/ifs/scripts/shutdown_adafs.py -J ${SLURM_JOB_ID} ${VEF_HOME}/ifs/build/bin/adafs_daemon ${HOSTFILE}

# cleanup
rm ${HOSTFILE}
