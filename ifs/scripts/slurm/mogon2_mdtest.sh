#!/bin/bash
# Slurm stuff

#SBATCH -J adafs_mdtest
#SBATCH -p parallel
#SBATCH -t 300
#SBATCH -A m2_zdvresearch

usage_short() {
        echo "
usage: adafs_mdtest.sh [-h] [-n <MD_PROC_N>] [-i <MD_ITER>] [-I <NUM_ITEMS>] [-u]
                    benchmark_dir
        "
}

help_msg() {

        usage_short
    echo "
This slurm batch script is for mdtesting adafs

positional arguments:
        benchmark_dir           path where the dependency downloads are put


optional arguments:
        -h, --help
                                shows this help message and exits
        -n <MD_PROC_N>
                                number of processes used in mdtest
                                defaults to '1'
        -i <MD_ITER>
                                number of iterations done in mdtest
                                defaults to '1'
        -I <NUM_ITEMS>
                                number of files per process in mdtest
                                defaults to '500000'
        -u, --unique
                                use if files should be placed in a unique directory per-process in mdtest
        "
}

MD_PROC_N=16
MD_ITER=1
MD_ITEMS="500000"
MD_UNIQUE=""
START_TIME="$(date -u +%s)"

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case ${key} in
    -n)
    MD_PROC_N="$2"
    shift # past argument
    shift # past value
    ;;
    -i)
    MD_ITER="$2"
    shift # past argument
    shift # past value
    ;;
    -I)
    MD_ITEMS="$2"
    shift # past argument
    shift # past value
    ;;
    -u|--unique)
    MD_UNIQUE="-u"
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
MD_DIR=$1
ROOTDIR="/localscratch/${SLURM_JOB_ID}"

# Load modules and set environment variables
PATH=$PATH:/home/vef/adafs_m2/install/bin:/home/vef/.local/bin
C_INCLUDE_PATH=$C_INCLUDE_PATH:/home/vef/adafs_m2/install/include
CPATH=$CPATH:/home/vef/adafs_m2/install/include
CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:/home/vef/adafs_m2/install/include
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/vef/adafs_m2/install/lib
LIBRARY_PATH=$LIBRARY_PATH:/home/vef/adafs_m2/install/lib
export PATH
export CPATH
export C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH
export LD_LIBRARY_PATH
export LIBRARY_PATH
export LDFLAGS='-L/home/vef/adafs_m2/install/lib/'
export CPPFLAGS='-I/home/vef/adafs_m2/install/include/'
module load devel/CMake/3.7.2
module load devel/Boost/1.65.1-foss-2017a
export CC=$(which gcc)
export CXX=$(which g++)

# printing stuff
echo "files per process: ${MD_ITEMS}"

# create a proper hostfile to run
srun -n ${SLURM_NNODES} hostname -s | sort -u > ${HOSTFILE} && sed -e 's/$/ max_slots=64/' -i ${HOSTFILE}

echo "Generated hostfile no of nodes:"
cat ${HOSTFILE} | wc -l

NONODES=$(cat ${HOSTFILE} | wc -l)
let MD_PROC_N=${NONODES}*16


echo "Shutting down adafs_daemons that might be running"
python2 ${VEF_HOME}/ifs_m2/scripts/shutdown_adafs.py -J ${SLURM_JOB_ID} ${VEF_HOME}/ifs_m2/build/bin/adafs_daemon ${HOSTFILE} -9

echo "
############################################################################
############################### DAEMON START ############################### ############################################################################
"
# start adafs daemon on the nodes
python2 ${VEF_HOME}/ifs_m2/scripts/startup_adafs.py -c -J ${SLURM_JOB_ID} --numactl "--cpunodebind=0 --membind=0" ${VEF_HOME}/ifs_m2/build/bin/adafs_daemon ${ROOTDIR} ${MD_DIR} ${HOSTFILE}

#echo "logfiles:"
#cat /tmp/adafs_daemon.log
# pssh to get logfiles. hostfile is created by startup script
${VEF_HOME}/.local/bin/pssh -O StrictHostKeyChecking=no -i -h /tmp/hostfile_pssh_${SLURM_JOB_ID} "tail /tmp/adafs_daemon.log"

echo "
############################################################################
############################ RUNNING BENCHMARK #############################
############################################################################
"
# Run benchmark
BENCHCMD="mpiexec -np ${MD_PROC_N} --map-by node --hostfile ${HOSTFILE} --mca mtl ^psm2,ofi -x LD_PRELOAD=/lustre/project/zdvresearch/vef/fs/ifs/build/lib/libadafs_preload_client.so numactl --cpunodebind=1 --membind=1 /lustre/miifs01/project/zdvresearch/vef/benchmarks/ior/build/src/mdtest -z 0 -b 1 -i ${MD_ITER} -d ${MD_DIR} -F -I ${MD_ITEMS} -C -r -T -v 1 ${MD_UNIQUE}"

eval ${BENCHCMD}

echo "
############################################################################
############################### DAEMON STOP ############################### ############################################################################
"
END_TIME="$(date -u +%s)"
ELAPSED="$((${END_TIME}-${START_TIME}))"
MINUTES=$((${ELAPSED} / 60))
echo "##Elapsed time: ${MINUTES} minutes or ${ELAPSED} seconds elapsed for test set."
# shut down adafs daemon on the nodes
python2 ${VEF_HOME}/ifs_m2/scripts/shutdown_adafs.py -J ${SLURM_JOB_ID} ${VEF_HOME}/ifs_m2/build/bin/adafs_daemon ${HOSTFILE} -9

# cleanup
rm ${HOSTFILE}
