
source ../config.tcl

#loop_pipeline "loop_3"
loop_pipeline "loop_3" -ignore-mem-deps 

# allow 2 sitofp operations in the pipeline to achieve II=1
set_resource_constraint altfp_sitofp 2

set_parameter LOCAL_RAMS 1
