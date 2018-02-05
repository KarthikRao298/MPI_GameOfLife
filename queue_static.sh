#!/bin/sh
# File Name       :queue_static.sh
# Description     :Script to execute the parallel game of life simulation on cluster
# Author          :Karthik Rao
# Date            :Nov 29 2017
# Version         :0.1


. ./params_static.sh

if [ ! -d ${RESULTDIR} ];
then
    mkdir ${RESULTDIR}
fi

#strong scaling

for INTENSITY in ${INTENSITIES};
do
    for N in ${NS};
    do	

	for PROC in ${PROCS}
	do
	
	    FILE=${RESULTDIR}/static${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		qsub -d $(pwd) -q mamba -l procs=${PROC} -v N=${N},PROC=${PROC} ./run_static.sh
	    fi

	done

    done
done



