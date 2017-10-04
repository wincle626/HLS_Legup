#!/bin/bash
# MOAB/Torque submission script for multiple, dynamically-run 
# serial jobs on SciNet GPC
#
#PBS -l nodes=1:ppn=8,walltime=30:00:00
#PBS -N legup

#this script generates all job scripts for a benchmark and runs them on a single scinet node
#Each job script simulates and synthesizes a type of system for a benchmark

INPUT_DIR=/scratch/j/janders/${USER}/scinet
OUTPUT_DIR=/scratch/j/janders/${USER}/output

#Benchmark name is given when submitting the job
#BENCHMARK=${1}

#write job script which untars file, simulate then synthesize, and copy back results
#Parameter $1=type of system (single, pthreads, etc)
function write_job_script {

    NAME=${1}
    INPUT_FILE="${NAME}"".tar.gz"
    JOB_SCRIPT="job""_""${BENCHMARK}""_""${NAME}"

	#create output directory to copy back to results to
	mkdir -p ${OUTPUT_DIR}/${BENCHMARK}/${NAME}
	
    echo "tar xvzf ${INPUT_FILE}" > ${JOB_SCRIPT}.sh
	echo "rm ${INPUT_FILE}" >> ${JOB_SCRIPT}.sh
	echo "cd ${NAME}" >> ${JOB_SCRIPT}.sh

	echo "echo STARTING MODELSIM ${BENCHMARK} ${NAME}" >> ${JOB_SCRIPT}.sh

	echo "./simulate" >> ${JOB_SCRIPT}.sh

	echo "echo FINISHED MODELSIM ${BENCHMARK} ${NAME}" >> ${JOB_SCRIPT}.sh

	#copy Modelsim results back 
	echo "cp transcript ${OUTPUT_DIR}/${BENCHMARK}/${NAME}" >> ${JOB_SCRIPT}.sh

	echo "echo STARTING QUARTUS ${BENCHMARK} ${NAME}" >> ${JOB_SCRIPT}.sh

	#run Quartus
	echo "quartus_sh --64bit --flow compile tiger_top" >> ${JOB_SCRIPT}.sh
	#run report timing to get critical path
 	#echo "quartus_sta --do_report_timing tiger_top.qpf" >> ${JOB_SCRIPT}.sh
		
	echo "echo FINISHED QUARTUS ${BENCHMARK} ${NAME}" >> ${JOB_SCRIPT}.sh

	#copy Quartus results back
	echo "cp *.summary ${OUTPUT_DIR}/${BENCHMARK}/${NAME}" >> ${JOB_SCRIPT}.sh
	echo "cp *.rpt ${OUTPUT_DIR}/${BENCHMARK}/${NAME}" >> ${JOB_SCRIPT}.sh

	echo "cd .." >> ${JOB_SCRIPT}.sh
    #all work is done at this point

    #copy system back for analyzing critical path, etc
    #tar the benchmark to copy back
	echo "tar cvzf ${INPUT_FILE} ${NAME}" >> ${JOB_SCRIPT}.sh
	echo "cp ${INPUT_FILE} ${OUTPUT_DIR}/${BENCHMARK}/${NAME}" >> ${JOB_SCRIPT}.sh

	#delete system directory to free memory
	echo "rm -rf ${NAME}" >> ${JOB_SCRIPT}.sh

	cp ${JOB_SCRIPT}.sh ${OUTPUT_DIR}/${BENCHMARK}/${NAME}
	#set permission to file
	chmod u=rwx ${JOB_SCRIPT}.sh
}


function cleanup_ramdisk {
    echo -n "Cleaning up ramdisk directory /dev/shm/j/janders/${USER} on "
    date
    rm -rf /dev/shm/${USER}
    echo -n "done at "
    date
}
 
function trap_term {
    echo -n "Trapped term (soft kill) signal on "
    date
#   save_results
    cleanup_ramdisk
    exit
}







#Setup Licenses
# set up quartus license
ssh -N -f -L 1802:ra.eecg.utoronto.ca:1802 datamover1
ssh -N -f -L 27002:ra.eecg.utoronto.ca:27002 datamover1

# set up modelsim license
# run the following ssh commands first on gpc01, which can connect to
# seth, which forwards the license port from picton
ssh -N -f -L 6056:localhost:6056 -L 7056:localhost:7056 -L 8056:localhost:8056 -L 16056:localhost:16056 -L 17056:localhost:17056 -L 18056:localhost:18056 gpc01


#Track how long everything takes.
echo "STARTING ${BENCHMARK} SUBMIT SCRIPT"
date

#Copy benchmarks to Ramdisk
mkdir -p /dev/shm/${USER}

#copy over benchmark directory to ramdisk
#each node will run one benchmark for all 6 cases
cp ${INPUT_DIR}/${BENCHMARK}.tar.gz /dev/shm/${USER}/

#go into ramdisk directory and into the benchmark directory
cd /dev/shm/${USER}

tar xvzf ${BENCHMARK}.tar.gz
cd ${BENCHMARK}


#there is 1 single benchmark, 2 pthreads benchmarks, and 3 pthreads+openmp benchmarks
POSTFIX=('single' 'pthreads' 'pthreads_pipeline' 'hybrid_2' 'hybrid_3' 'hybrid_4')

#loop through each case
for (( i = 0 ; i < ${#POSTFIX[@]} ; i++ ))
do
    NAME=${POSTFIX[$i]}
    FILE="${NAME}"".tar.gz"
    if [ -f ${FILE} ]; then
        write_job_script ${NAME}
    fi	
done

#trap the termination signal, and call the function 'trap_term' when 
# that happens, so results may be saved.
trap "trap_term" TERM

# Run jobs scripts in parallel  
# COMMANDS ARE ASSUMED TO BE SCRIPTS CALLED job*
find -name 'job*' | parallel -j 4

# Clean up /dev/shm
rm -rf /dev/shm/${USER}

echo "FINISHED ${BENCHMARK} SUBMIT SCRIPT"
date
