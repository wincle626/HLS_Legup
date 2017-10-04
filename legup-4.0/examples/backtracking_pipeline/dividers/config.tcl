
source ../config.tcl

# resource constraints to make modulo scheduling difficult
set_resource_constraint divide 4
set_operation_latency divide 64

