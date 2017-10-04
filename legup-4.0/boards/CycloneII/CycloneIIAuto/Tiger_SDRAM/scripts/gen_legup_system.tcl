source ../../../../gen_systems_common.tcl

create_system legup_system

set_project_property DEVICE_FAMILY "Cyclone II"
set_project_property DEVICE "EP2C35F672C6"

###############################################################################
# Add Clock Source
add_instance clk clock_source

set_instance_parameter_value clk clockFrequency         50000000
set_instance_parameter_value clk clockFrequencyKnown    true
set_instance_parameter_value clk inputClockFrequency    "0"
set_instance_parameter_value clk resetSynchronousEdges  "NONE"

set_interface_property clk      EXPORT_OF clk.clk_in
set_interface_property reset    EXPORT_OF clk.clk_in_reset
###############################################################################


###############################################################################
# Add Tiger MIPS
add_instance Tiger_MIPS tiger_mips

set_instance_parameter_value Tiger_MIPS RESET_ADDRESS   0x00800000

add_connection clk.clk          Tiger_MIPS.clock
add_connection clk.clk_reset    Tiger_MIPS.reset
###############################################################################


###############################################################################
# Add Instruction Cache for the Tiger MIPS
add_instance Tiger_ICache tiger_icache

add_connection clk.clk                          Tiger_ICache.clock
add_connection clk.clk_reset                    Tiger_ICache.reset
add_connection Tiger_MIPS.instruction_master    Tiger_ICache.icache_slave

set_connection_parameter_value Tiger_MIPS.instruction_master/Tiger_ICache.icache_slave  baseAddress "0x00000000"

lock_avalon_base_address Tiger_ICache.icache_slave
###############################################################################


###############################################################################
# Add Data Cache
add_instance DCache legup_dm_wt_cache

add_connection clk.clk                  DCache.clk
add_connection clk.clk_reset            DCache.reset
add_connection Tiger_MIPS.data_master   DCache.cache_slave

set_connection_parameter_value Tiger_MIPS.data_master/DCache.cache_slave  baseAddress "0x00000000"

lock_avalon_base_address DCache.cache_slave
###############################################################################


###############################################################################
# Add JTAG Master for FPGA
add_instance JTAG_to_FPGA_Bridge altera_jtag_avalon_master

add_connection clk.clk          JTAG_to_FPGA_Bridge.clk
add_connection clk.clk_reset    JTAG_to_FPGA_Bridge.clk_reset
###############################################################################


###############################################################################
# Add SDRAM Controller
add_instance SDRAM altera_avalon_new_sdram_controller

set_instance_parameter_value SDRAM columnWidth              8
set_instance_parameter_value SDRAM rowWidth                 12
set_instance_parameter_value SDRAM dataWidth                16
set_instance_parameter_value SDRAM generateSimulationModel  true

set_interface_property sdram_wire EXPORT_OF SDRAM.wire

add_connection clk.clk                      SDRAM.clk
add_connection clk.clk_reset                SDRAM.reset
add_connection Tiger_ICache.icache_master   SDRAM.s1
add_connection DCache.cache_master          SDRAM.s1
add_connection JTAG_to_FPGA_Bridge.master   SDRAM.s1

set_connection_parameter_value Tiger_ICache.icache_master/SDRAM.s1  baseAddress "0x00800000"
set_connection_parameter_value DCache.cache_master/SDRAM.s1         baseAddress "0x00800000"
set_connection_parameter_value JTAG_to_FPGA_Bridge.master/SDRAM.s1  baseAddress "0x00800000"

lock_avalon_base_address SDRAM.s1
###############################################################################


###############################################################################
# Add JTAG UART
add_instance JTAG_UART altera_avalon_jtag_uart

add_connection clk.clk                      JTAG_UART.clk
add_connection clk.clk_reset                JTAG_UART.reset
add_connection Tiger_MIPS.data_master       JTAG_UART.avalon_jtag_slave
add_connection JTAG_to_FPGA_Bridge.master   JTAG_UART.avalon_jtag_slave

set_connection_parameter_value Tiger_MIPS.data_master/JTAG_UART.avalon_jtag_slave       baseAddress "0xFF201000"
set_connection_parameter_value JTAG_to_FPGA_Bridge.master/JTAG_UART.avalon_jtag_slave   baseAddress "0xFF201000"

#add_connection Tiger_MIPS.d_irq JTAG_UART.irq

#set_connection_parameter_value Tiger_MIPS.d_irq/JTAG_UART.irq irqNumber 8

lock_avalon_base_address JTAG_UART.avalon_jtag_slave
###############################################################################


###############################################################################
# Add UART
add_instance UART altera_avalon_uart

set_interface_property uart_wire EXPORT_OF UART.external_connection

add_connection clk.clk                  UART.clk
add_connection clk.clk_reset            UART.reset
add_connection Tiger_MIPS.data_master   UART.s1

set_connection_parameter_value Tiger_MIPS.data_master/UART.s1   baseAddress "0xF0001000"

#add_connection Tiger_MIPS.d_irq	UART.irq

#set_connection_parameter_value Tiger_MIPS.d_irq/UART.irq	irqNumber 8

lock_avalon_base_address UART.s1
###############################################################################


###############################################################################
# Add Cycle_Counters
#add_instance Cycle_Counters altera_avalon_uart
#
#add_connection clk.clk                  Cycle_Counters.clock
#add_connection clk.clk_reset            Cycle_Counters.reset
#add_connection Tiger_MIPS.data_master   Cycle_Counters.control
#add_connection Tiger_MIPS.data_master   Cycle_Counters.data
#
#set_connection_parameter_value Tiger_MIPS.data_master/Cycle_Counters.control    baseAddress "0xF0000010"
#set_connection_parameter_value Tiger_MIPS.data_master/Cycle_Counters.data       baseAddress "0xF0000030"
#
#lock_avalon_base_address Cycle_Counters.control
#lock_avalon_base_address Cycle_Counters.data
###############################################################################


###############################################################################
# Add End_Simulation
#add_instance End_Simulation altera_avalon_uart
#
#add_connection clk.clk                  End_Simulation.clock
#add_connection clk.clk_reset            End_Simulation.reset
#add_connection Tiger_MIPS.data_master   End_Simulation.control
#
#set_connection_parameter_value Tiger_MIPS.data_master/End_Simulation.control    baseAddress "0xF0000020"
#
#lock_avalon_base_address End_Simulation.s1
###############################################################################


save_system legup_system.qsys

