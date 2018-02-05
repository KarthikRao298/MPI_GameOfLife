#!/bin/sh
# File Name       :
# Description     :Script to execute the parallel version of game of life simulation
# Author          :Karthik Rao
# Date            :Nov 29 2017
# Version         :0.1

RESULTDIR=result/
h=`hostname`

if [ "$h" = "mba-i1.uncc.edu"  ];
then
    echo Do not run this on the headnode of the cluster, use qsub!
    exit 1
fi

if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi
   
mpirun ./game ${N} 2> ${RESULTDIR}/static${N}_${PROC} 1>${RESULTDIR}/static${N}_${PROC}.txt

