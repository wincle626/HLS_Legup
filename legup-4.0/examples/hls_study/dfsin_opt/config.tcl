source ../config.tcl

set_parameter CASE_FSM 0
set_parameter LOCAL_RAMS 0
set_parameter PRINT_FUNCTION_START_FINISH 0

set_operation_latency mem_dual_port 2

set_operation_latency mem_dual_port 2

set_accelerator_function "sin_thread"
