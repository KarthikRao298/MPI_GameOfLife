#!/bin/sh
# File Name       :queue_sequential.sh
# Description     :Script to execute the sequential game of life simulation on cluster
# Author          :Karthik Rao
# Date            :Nov 29 2017
# Version         :0.1

qsub -q mamba -l procs=1 -d $(pwd) ./run_sequential.sh


