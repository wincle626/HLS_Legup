
source ../config.tcl

# resource constraints to make modulo scheduling difficult
set_operation_latency signed_multiply 18
set_resource_constraint multiply 3

# Runtime analysis - incremental SDC
set_parameter INCREMENTAL_SDC 1
