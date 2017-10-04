#!/bin/bash
# MOAB/Torque submission script for multiple, dynamically-run 
# serial jobs on SciNet GPC
#
#PBS -l nodes=1:ppn=8,walltime=02:55:00
#PBS -m e
#PBS -N legup-multipump


#module load Xlibraries/X11-32
#module load extras

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
#ssh -N -f -L 7325:localhost:7325 gpc01
#ssh -N -f -L 7327:localhost:7327 gpc01
ssh -N -f -L 6056:localhost:6056 -L 7056:localhost:7056 -L 8056:localhost:8056 -L 16056:localhost:16056 -L 17056:localhost:17056 -L 18056:localhost:18056 gpc01

#Track how long everything takes.
echo "STARTING ${BENCHMARK} SUBMIT SCRIPT"
date

#trap the termination signal, and call the function 'trap_term' when 
# that happens, so results may be saved.
trap "trap_term" TERM

if [ $TESTING -eq 1 ]
then
RAM=/scratch/j/janders/${USER}/andrew/ram
OUTPUT_DIR=/scratch/j/janders/${USER}/andrew/output
else
OUTPUT_DIR=$PBS_O_WORKDIR
RAM=/dev/shm/${USER}
fi
echo "RAM dir: $RAM"
mkdir -v -p $RAM

# copy legup binaries and benchmark to Ramdisk
cd $OUTPUT_DIR/legup
cp -v -r --parents * $RAM

cd $RAM/examples/$BENCHMARK
make cleanall
make FAMILY=StratixIV
make v FAMILY=StratixIV
make p FAMILY=StratixIV
make f FAMILY=StratixIV

# copy back results
mkdir -p $OUTPUT_DIR
cd $RAM
cp -v --parents examples/* $OUTPUT_DIR
cp -v -r --parents examples/$BENCHMARK $OUTPUT_DIR

# Run jobs scripts in parallel  
# COMMANDS ARE ASSUMED TO BE SCRIPTS CALLED job*
#find -name 'job*' | parallel -j 2

# Clean up /dev/shm
if [ $TESTING -ne 1 ]
then
rm -rf $RAM
fi

echo "FINISHED ${BENCHMARK} SUBMIT SCRIPT"
date
