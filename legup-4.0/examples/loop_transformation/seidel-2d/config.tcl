# turn on loop pipelining for loop
loop_pipeline "l1"

set_parameter PRINTF_CYCLES 1

set_parameter LOCAL_RAMS 1
set_resource_constraint altfp_add 20
