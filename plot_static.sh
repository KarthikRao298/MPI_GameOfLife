#!/bin/sh
# File Name       :plot_static.sh
# Description     :Script to plot the speedup of parallel game of life simulation
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
	FILE=${RESULTDIR}/sequential_${N}
	if [ ! -f ${FILE} ]
	then
	    echo missing sequential result file "${FILE}". Have you run queue_sequential and waited for completion?
	fi

	seqtime=$(cat ${RESULTDIR}/sequential_${N})
	
	for PROC in ${PROCS}
	do
	
	    FILE=${RESULTDIR}/static${N}_${PROC}
	    
	    if [ ! -f ${FILE} ]
	    then
		echo missing static result file "${FILE}". Have you run queue_static and waited for completion?
	    fi

	    partime=$(cat ${RESULTDIR}/static${N}_${PROC})
	    
	    echo ${PROC} ${seqtime} ${partime}
	done > ${RESULTDIR}/speedup_static_ni_${N}


	GNUPLOTSTRONG="${GNUPLOTSTRONG} set title 'strong scaling. n=${N} i=${INTENSITY}'; plot '${RESULTDIR}/speedup_static_ni_${N}' u 1:(\$2/\$3);"
    done
done

gnuplot <<EOF
set terminal pdf
set output 'static_sched_plots.pdf'

set style data linespoints

set key top left

set xlabel 'proc'
set ylabel 'speedup'

${GNUPLOTSTRONG}
EOF


