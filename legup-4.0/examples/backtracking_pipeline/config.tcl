source ../../legup.tcl

# turn on loop pipelining for loop
loop_pipeline "loop"

set_parameter PRINTF_CYCLES 1

set_parameter LOCAL_RAMS 1

# Zhang
#set_parameter MODULO_SCHEDULER SDC_GREEDY

# Backtracking
set_parameter MODULO_SCHEDULER SDC_BACKTRACKING

# Backtracking + Restructuring
#set_parameter MODULO_SCHEDULER SDC_BACKTRACKING
#set_parameter RESTRUCTURE_LOOP_RECURRENCES 1

# Runtime analysis - incremental SDC
#set_parameter INCREMENTAL_SDC 1

set_operation_latency multiply 0

set_project StratixIV DE4-530 Tiger_DDR2
