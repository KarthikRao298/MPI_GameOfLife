#!/bin/sh
# File Name       :run_sequential.sh
# Description     :Script to compile and execute the sequential game of life simulation
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
    

make sequential

. ./params_static.sh

for intensity in $INTENSITIES;
do
    for n in $NS;
    do
	FILE=${RESULTDIR}/sequential_${n}

	if [ ! -f ${FILE} ]
	then
	    ./sequential ${n} 2>${RESULTDIR}/sequential_${n}  >/dev/null
	fi
    done
done
