source ../../legup.tcl

set_custom_verilog_function "assignSwitchesToLEDsInverted" noMemory \
                             output 7:2 LEDR \
                             input 7:2 SW \
                             input 3:0 KEY
set_custom_verilog_function "assignSwitchesToLEDs" noMemory \
                             output 5:0 LEDR \
                             input 5:0 SW \
                             input 3:0 KEY

set_custom_verilog_file "assignSwitchesToLEDs.v"
set_custom_verilog_file "testbench.v"
set_custom_test_bench_module "tester"
