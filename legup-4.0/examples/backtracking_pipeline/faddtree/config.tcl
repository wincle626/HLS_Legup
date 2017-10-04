
source ../config.tcl

# resource constraints to make modulo scheduling difficult
set_resource_constraint altfp_add 1
set_operation_latency altfp_add 13

# Runtime analysis - incremental SDC
set_parameter INCREMENTAL_SDC 0
