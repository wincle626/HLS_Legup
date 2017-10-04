#!/bin/bash
#this script submits a benchmark to execute on a scinet node

BENCHMARK=${1}

echo submitting ${BENCHMARK}
qsub -v BENCHMARK="${BENCHMARK}" scinet_jobs.sh
echo ${BENCHMARK} submitted!

