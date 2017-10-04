package require -exact qsys 13.0
load_system legup_system.qsys
add_instance cycle_counters cycle_counters
add_connection clk_0.clk cycle_counters.clock
add_connection clk_0.clk_reset cycle_counters.reset
add_connection tiger_mips.data_master  cycle_counters.control
add_connection tiger_mips.data_master  cycle_counters.data

set_connection_parameter_value tiger_mips.data_master/cycle_counters.control baseAddress "0xF0000010"
set_connection_parameter_value tiger_mips.data_master/cycle_counters.data baseAddress "0xF0000030"

save_system

