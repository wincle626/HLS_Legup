#!/bin/bash
#this script generates all systems (single, pthreads, pthreads+openmp, etc) for a single benchmark
#then copies over all the systems for that benchmark to scinet and runs scinet_submit_single.sh
#which submits the job on scinet to simulate and synthesize all systems

if [ $# -eq 0 ]
  then
    echo "Name of benchmark needs to be given as an argument"
    exit 
fi

BENCHMARK=${1}
SCINET_DIR=/scratch/j/janders/jchoi/scinet

#delete existing benchmark directory
rm -rf scinet/${BENCHMARK}
#mkdir scinet

#make benchmark directory
mkdir -p scinet/${BENCHMARK}

#there is 1 single benchmark, 2 pthreads benchmarks, and 3 pthreads+openmp benchmarks
DIRS=('single' 'pthreads' 'pthreads' 'pthreads+openmp' 'pthreads+openmp' 'pthreads+openmp')
SUBDIRS=('' '' '_pipeline' '_2' '_3' '_4')
POSTFIX=('single' 'pthreads' 'pthreads_pipeline' 'hybrid_2' 'hybrid_3' 'hybrid_4')
SCINET_QUARTUS='/home/j/janders/acanis/altera/11.1/quartus/eda/sim_lib'

#loop through each case
for (( i = 0 ; i < ${#POSTFIX[@]} ; i++ ))
do
    #NAME="${BENCHMARK}""${POSTFIX[$i]}"
    NAME=${POSTFIX[$i]}
    DIR=${DIRS[$i]}
    SUBDIR="${BENCHMARK}""${SUBDIRS[$i]}"

    echo "Starting Generation!"
#    echo $DIR/${SUBDIR}
#    echo $NAME
    #generate system
    if [ -d "${DIR}/${SUBDIR}" ]; then
        cd ${DIR}/${SUBDIR}
    #    echo $PWD
        if [[ "${NAME}" == 'single' || "${NAME}" == 'pthreads' || "${NAME}" == 'pthreads_pipeline' ]]
            then 
                echo "Running hybrid flow for ${BENCHMARK} ${NAME}"
                make hybrid 
            else 
                echo "Running hybridomp flow for ${BENCHMARK} ${NAME}"
                make hybridomp
        fi
         
        echo "System Generation Complete!"

        #change path from mine to scinet quartus directory
        sed -i 's|^`include\s".*/altera_mf.v"|`include "'${SCINET_QUARTUS}'/altera_mf.v"|' tiger/tiger.v
        sed -i 's|^`include\s".*/220model.v"|`include "'${SCINET_QUARTUS}'/220model.v"|' tiger/tiger.v
        sed -i 's|^`include\s".*/sgate.v"|`include "'${SCINET_QUARTUS}'/sgate.v"|' tiger/tiger.v
        sed -i 's|^`include\s".*/altera_primitives.v"|`include "'${SCINET_QUARTUS}'/altera_primitives.v"|' tiger/tiger.v
        sed -i 's|^`include\s".*/stratixiv_atoms.v"|`include "'${SCINET_QUARTUS}'/stratixiv_atoms.v"|' tiger/tiger.v
        sed -i 's|^`include\s".*/stratixiii_atoms.v"|`include "'${SCINET_QUARTUS}'/stratixiii_atoms.v"|' tiger/tiger.v
        echo "Replacing Path Complete!"

        #tar the system and copy it
        #delete previous files
        rm -rf ${NAME}
        rm -f ${NAME}.tar.gz
        cp -r tiger ${NAME}
        tar cvzf ${NAME}.tar.gz ${NAME}
        cp ${NAME}.tar.gz ../../scinet/${BENCHMARK}
        cd ../..
    fi    
    echo "Copying Files Complete!"
done

#tar the entire benchmark directory and copy it over to scinet
cd scinet
tar cvzf ${BENCHMARK}.tar.gz ${BENCHMARK}
scp ${BENCHMARK}.tar.gz scinet:${SCINET_DIR} 

#run the script on scinet to submit jobs
#script is located on SCINET_DIR and called scinet_submit.sh
#ssh scinet 'bash -s' < ${SCINET_DIR}/scinet_submit_single.sh ${BENCHMARK}
#ssh into login node, then to scinet, go into 
ssh scinet "ssh gpc01 'cd ${SCINET_DIR};./scinet_submit_single.sh ${BENCHMARK}'"

#delete tar file for the entire benchmark
rm ${BENCHMARK}.tar.gz
cd ../


