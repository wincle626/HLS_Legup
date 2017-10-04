package require -exact qsys 13.0
load_system legup_system.qsys
add_instance end_simulation end_simulation
add_connection clk_0.clk end_simulation.clock
add_connection clk_0.clk_reset end_simulation.reset
add_connection tiger_mips.data_master  end_simulation.control

set_connection_parameter_value tiger_mips.data_master/end_simulation.control baseAddress "0xF0000020"

save_system

