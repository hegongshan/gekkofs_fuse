#!/bin/bash
################################################################################
# Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# SPDX-License-Identifier: MIT                                                 #
################################################################################

# optimal $GKFS_FIND_PROCESS is $GKFS_SERVERS+1 as we align servers and find processes if the $NUM_NODES are the same
# Output is saved to a file, so it can be processed by io500
srun --nvram-options=1LM:1980 -N $NUM_NODES -n $GKFS_FIND_PROCESS --export=ALL,PSM2_DEVICES=self,hfi,shm,PSM2_MULTIRAIL=1,PSM2_MULTI_EP=0,LD_PRELOAD=${GKFS_PRLD} -o find_${SLURM_JOB_ID}.txt $GKFS_FIND $@ -M $GKFS_MNT -S $GKFS_SERVERS
tail -n1 find_${SLURM_JOB_ID}.txt
exit 0

