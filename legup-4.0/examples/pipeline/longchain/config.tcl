
source ../config.tcl

loop_pipeline "loop"

set_parameter LOCAL_RAMS 1

# adder latency so restructuring affects cycles
set_operation_latency signed_add 1

# restructuring OFF: 124 cycles 
# restructuring ON: 34 cycles
set_parameter RESTRUCTURE_LOOP_RECURRENCES 1
