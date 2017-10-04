source ../../legup.tcl

set_parameter LOCAL_RAMS 1
set_parameter GROUP_RAMS 1
set_parameter GROUP_RAMS_SIMPLE_OFFSET 1
set_parameter CASE_FSM 1
set_parameter PRINT_FUNCTION_START_FINISH 1

set_operation_latency mem_dual_port 1

set_project StratixV DE5-Net Tiger_DDR3

# For MC Paths
#set_parameter MULTI_CYCLE_REMOVE_REG 1
#set_parameter MULTI_CYCLE_DISABLE_REG_MERGING 1
#set_parameter MULTI_CYCLE_DUPLICATE_LOAD_REG 1
#set_parameter MULTI_CYCLE_REMOVE_CMP_REG 1
#set_operation_latency local_mem_dual_port 2

# For MC Paths, based on SW profiling
#set_parameter LLVM_PROFILE 1
#set_parameter LLVM_PROFILE_MAX_BB_FREQ_TO_ALTER 1
#set_parameter LLVM_PROFILE_EXTRA_CYCLES 1

