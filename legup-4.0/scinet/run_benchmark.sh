#!/bin/bash

HOME=$PWD
DIRS=( 
multipliers_pipeline_0/
multipliers_pipeline_1/
multipliers_pipeline_2/
multipliers_pipeline_3/
multipliers_pipeline_4/
multipliers_pipeline_5/
multipump_sdc_on_pipeline_0/
multipump_sdc_on_pipeline_1/
multipump_sdc_on_pipeline_2/
multipump_sdc_on_pipeline_3/
multipump_sdc_on_pipeline_4/
multipump_sdc_on_pipeline_5/
implicit_multipliers
explicit_multipliers
multipumping_sdc_off
multipumping_sdc_on
sharing_and_multipumping_sdc_off 
sharing_and_multipumping_sdc_on
sharing_multipliers
)

for (( a = 0 ; a < ${#DIRS[@]} ; a++ ))
do
		cd ${DIRS[$a]}/examples
        cp $HOME/legup/examples/benchmark.pl .
        ./benchmark.pl
        cd $HOME
		#cd ${DIRS[$a]}
        #rm -rf examples
        #rm -rf legup-multipump.*
        #cd $HOME
        #./launch.sh ${DIRS[$a]}
done
