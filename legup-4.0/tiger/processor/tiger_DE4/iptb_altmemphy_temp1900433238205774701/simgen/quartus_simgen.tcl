package require ::quartus::project
set project_name ddr2_phy_alt_mem_phy_seq_wrapper
if [catch {project_open $project_name}] {
        project_new $project_name
}
set_global_assignment -name "FAMILY" "Stratix IV"
export_assignments
set_global_assignment -name "COMPILER_SETTINGS" "ddr2_phy_alt_mem_phy_seq_wrapper"
set_global_assignment -name "SIMULATOR_SETTINGS" "ddr2_phy_alt_mem_phy_seq_wrapper"
set_global_assignment -name VERILOG_FILE "/home/choijon5/legup/tiger/processor/tiger_DE4_test/iptb_altmemphy_temp1900433238205774701/ddr2_phy_alt_mem_phy_seq_wrapper.v"
set_global_assignment -name VHDL_FILE "/home/choijon5/legup/tiger/processor/tiger_DE4_test/iptb_altmemphy_temp1900433238205774701/ddr2_phy_alt_mem_phy_seq.vhd"
set_global_assignment -name VERILOG_FILE "/home/choijon5/legup/tiger/processor/tiger_DE4_test/iptb_altmemphy_temp1900433238205774701/ddr2_phy.v"
set_global_assignment -name VERILOG_FILE "/home/choijon5/legup/tiger/processor/tiger_DE4_test/iptb_altmemphy_temp1900433238205774701/ddr2_phy_alt_mem_phy.v"
set_global_assignment -name VERILOG_FILE "/home/choijon5/legup/tiger/processor/tiger_DE4_test/iptb_altmemphy_temp1900433238205774701/ddr2_phy_alt_mem_phy_pll.v"
set_global_assignment -name USER_LIBRARIES "/home/choijon5/altera/11.1/ip/altera/ddr2_high_perf/lib"
set_global_assignment -name "STRATIX_OPTIMIZATION_TECHNIQUE" "SPEED"
set_global_assignment -name "DEVICE" "AUTO";
project_close
