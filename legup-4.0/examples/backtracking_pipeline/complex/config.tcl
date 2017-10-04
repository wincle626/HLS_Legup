
source ../config.tcl

# resource constraints to make modulo scheduling difficult
set_resource_constraint multiply 3
set_operation_latency multiply 18

set_parameter SDC_PRIORITY 0
