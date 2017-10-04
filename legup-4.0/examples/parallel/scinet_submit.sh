#!/bin/bash
#this script submits each benchmark to execute on a scinet node

BENCHMARKS=( mandelbrot )

#iterate over benchmarks and submit jobs
for (( a = 0 ; a < ${#BENCHMARKS[@]} ; a++ ))
do
	echo ${BENCHMARKS[$a]}
	qsub -v BENCHMARK="${BENCHMARKS[$a]}" scinet_jobs.sh&
	sleep 10 
done
