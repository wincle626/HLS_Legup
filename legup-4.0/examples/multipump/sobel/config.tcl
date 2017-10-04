# unroll flags are in ./Makefile

# shared settings
set_resource_constraint mem_dual_port 1
set_parameter DUAL_PORT_BINDING 0
set_parameter LOCAL_RAMS 1
set_parameter MULTIPLIER_NO_CHAIN 1
set_parameter ENABLE_PATTERN_SHARING 0
set_parameter MB_MINIMIZE_HW 0
set_parameter SDC_RES_CONSTRAINTS 0
set_parameter ALIAS_ANALYSIS 0

# original - 2 stage mul
#set_operation_latency signed_multiply 2
#set_operation_sharing -on signed_multiply

# sharing - 2 stage mul
#set_parameter SDC_RES_CONSTRAINTS 1
#set_resource_constraint multiply 1
#set_operation_latency signed_multiply 2
#set_operation_sharing -on signed_multiply

# multipumping - 3 stage mul
set_parameter MULTIPUMPING 1
set_operation_latency signed_multiply 3
