source ../../../../gen_systems_common.tcl

set memory_base 0x40000000
set prog_start  0x40000020

create_system legup_system

set_project_property DEVICE_FAMILY "Stratix V"
set_project_property DEVICE "5SGXEA7N2F45C2"

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

set_instance_parameter_value Tiger_MIPS RESET_ADDRESS   $memory_base
###############################################################################


###############################################################################
# Add Leap Profiler
add_instance Leap_Profiler leap_profiler

set_instance_parameter_value Leap_Profiler STARTING_PC   $prog_start

set_interface_property leap_profiling_signals   EXPORT_OF Leap_Profiler.profiling_signals
set_interface_property leap_debug_port          EXPORT_OF Leap_Profiler.debug_port

add_connection Leap_Profiler.leap_processor_reset   Tiger_MIPS.reset
add_connection Tiger_MIPS.instruction_master        Leap_Profiler.from_cpu

set_connection_parameter_value Tiger_MIPS.instruction_master/Leap_Profiler.from_cpu baseAddress "0x00000000"

lock_avalon_base_address Leap_Profiler.from_cpu
###############################################################################


###############################################################################
# Add Leap's Simulation Controller
add_instance Leap_Sim_Control leap_sim_controller

set_instance_parameter_value Leap_Sim_Control STARTING_PC   $prog_start

add_connection Leap_Sim_Control.bridge_master   Leap_Profiler.leapslave

set_connection_parameter_value Leap_Sim_Control.bridge_master/Leap_Profiler.leapslave  baseAddress "0x00000000"

lock_avalon_base_address Leap_Profiler.leapslave
###############################################################################


###############################################################################
# Add Instruction Cache for the Tiger MIPS
add_instance Tiger_ICache tiger_icache

add_connection Leap_Profiler.to_memory  Tiger_ICache.icache_slave

set_connection_parameter_value Leap_Profiler.to_memory/Tiger_ICache.icache_slave  baseAddress "0x00000000"

lock_avalon_base_address Tiger_ICache.icache_slave
###############################################################################


###############################################################################
# Add Data Cache
add_instance DCache legup_dm_wt_cache

add_connection Tiger_MIPS.data_master   DCache.cache_slave

set_connection_parameter_value Tiger_MIPS.data_master/DCache.cache_slave  baseAddress "0x00000000"

lock_avalon_base_address DCache.cache_slave
###############################################################################


###############################################################################
# Add JTAG Master for FPGA
add_instance JTAG_to_FPGA_Bridge altera_jtag_avalon_master

add_connection JTAG_to_FPGA_Bridge.master   Leap_Sim_Control.bridge_slave

set_connection_parameter_value JTAG_to_FPGA_Bridge.master/Leap_Sim_Control.bridge_slave  baseAddress "0x80000000"

lock_avalon_base_address Leap_Sim_Control.bridge_slave
###############################################################################


###############################################################################
# Add DDR3 SDRAM Controller
add_instance DDR3_SDRAM altera_mem_if_ddr3_emif

set_instance_parameter_value DDR3_SDRAM MEM_CLK_FREQ        800
set_instance_parameter_value DDR3_SDRAM MEM_CLK_FREQ_MAX    800
set_instance_parameter_value DDR3_SDRAM REF_CLK_FREQ        50.0
set_instance_parameter_value DDR3_SDRAM MEM_VENDOR          Micron
set_instance_parameter_value DDR3_SDRAM MEM_FORMAT          UNBUFFERED
set_instance_parameter_value DDR3_SDRAM MEM_DQ_WIDTH        64
set_instance_parameter_value DDR3_SDRAM MEM_ROW_ADDR_WIDTH  14
set_instance_parameter_value DDR3_SDRAM MEM_COL_ADDR_WIDTH  10
set_instance_parameter_value DDR3_SDRAM MEM_TCL             11
set_instance_parameter_value DDR3_SDRAM MEM_DRV_STR         "RZQ/7"
set_instance_parameter_value DDR3_SDRAM MEM_RTT_NOM         "RZQ/6"
set_instance_parameter_value DDR3_SDRAM MEM_WTCL            8
set_instance_parameter_value DDR3_SDRAM MEM_RTT_WR          "RZQ/4"

set_instance_parameter_value DDR3_SDRAM TIMING_TIS	        170
set_instance_parameter_value DDR3_SDRAM TIMING_TIH	        120
set_instance_parameter_value DDR3_SDRAM TIMING_TDS	        10
set_instance_parameter_value DDR3_SDRAM TIMING_TDH	        45
set_instance_parameter_value DDR3_SDRAM TIMING_TDQSQ	    100
set_instance_parameter_value DDR3_SDRAM TIMING_TQH	        0.38
set_instance_parameter_value DDR3_SDRAM TIMING_TDQSCK	    255
set_instance_parameter_value DDR3_SDRAM TIMING_TDQSCKDS	    450
set_instance_parameter_value DDR3_SDRAM TIMING_TDQSCKDM	    900
set_instance_parameter_value DDR3_SDRAM TIMING_TDQSCKDL	    1200
set_instance_parameter_value DDR3_SDRAM TIMING_TDQSS	    0.27
set_instance_parameter_value DDR3_SDRAM TIMING_TQSH	        0.4
set_instance_parameter_value DDR3_SDRAM TIMING_TDSH	        0.18
set_instance_parameter_value DDR3_SDRAM TIMING_TDSS	        0.18

set_instance_parameter_value DDR3_SDRAM MEM_TINIT_US		500
set_instance_parameter_value DDR3_SDRAM MEM_TMRD_CK	    	4
set_instance_parameter_value DDR3_SDRAM MEM_TRAS_NS	    	35.0
set_instance_parameter_value DDR3_SDRAM MEM_TRCD_NS	    	13.75
set_instance_parameter_value DDR3_SDRAM MEM_TRP_NS	    	13.75
set_instance_parameter_value DDR3_SDRAM MEM_TREFI_US		7.8
set_instance_parameter_value DDR3_SDRAM MEM_TRFC_NS	    	160.0
set_instance_parameter_value DDR3_SDRAM MEM_TWR_NS	    	15.0
set_instance_parameter_value DDR3_SDRAM MEM_TWTR	    	4
set_instance_parameter_value DDR3_SDRAM MEM_TFAW_NS		    30.0
set_instance_parameter_value DDR3_SDRAM MEM_TRRD_NS	    	6.0
set_instance_parameter_value DDR3_SDRAM MEM_TRTP_NS		    7.5

set_instance_parameter_value DDR3_SDRAM RATE		        Quarter
set_instance_parameter_value DDR3_SDRAM POWER_OF_TWO_BUS	true
set_instance_parameter_value DDR3_SDRAM USER_DEBUG_LEVEL	0
set_instance_parameter_value DDR3_SDRAM USER_DEBUG_LEVEL	0
set_instance_parameter_value DDR3_SDRAM USER_DEBUG_LEVEL	0

set_instance_parameter_value DDR3_SDRAM PACKAGE_DESKEW		                true
set_instance_parameter_value DDR3_SDRAM AC_PACKAGE_DESKEW		            true
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_MAX_CK_DELAY		    1.78
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_MAX_DQS_DELAY		    0.7
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_SKEW_CKDQS_DIMM_MIN	0.06
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_SKEW_CKDQS_DIMM_MAX	1.29
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_SKEW_BETWEEN_DIMMS		0.05
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_SKEW_WITHIN_DQS		0.0090
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_SKEW_BETWEEN_DQS		0.1
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_DQ_TO_DQS_SKEW		    0.0020
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_AC_SKEW		        0.05
set_instance_parameter_value DDR3_SDRAM TIMING_BOARD_AC_TO_CK_SKEW		    0.012


set_interface_property ddr3_memory  EXPORT_OF DDR3_SDRAM.memory
set_interface_property ddr3_status  EXPORT_OF DDR3_SDRAM.status
set_interface_property ddr3_oct     EXPORT_OF DDR3_SDRAM.oct

add_connection clk.clk                      DDR3_SDRAM.pll_ref_clk
add_connection clk.clk_reset                DDR3_SDRAM.global_reset
add_connection clk.clk_reset                DDR3_SDRAM.soft_reset
add_connection Tiger_ICache.icache_master   DDR3_SDRAM.avl
add_connection DCache.cache_master          DDR3_SDRAM.avl
add_connection JTAG_to_FPGA_Bridge.master   DDR3_SDRAM.avl

set_connection_parameter_value Tiger_ICache.icache_master/DDR3_SDRAM.avl    baseAddress "0x00000000"
set_connection_parameter_value DCache.cache_master/DDR3_SDRAM.avl           baseAddress "0x00000000"
set_connection_parameter_value JTAG_to_FPGA_Bridge.master/DDR3_SDRAM.avl    baseAddress "0x00000000"

lock_avalon_base_address DDR3_SDRAM.avl

# Use the DDR3 SDRAM's clock for the system
add_connection DDR3_SDRAM.afi_clk       Tiger_MIPS.clock
add_connection DDR3_SDRAM.afi_reset     Tiger_MIPS.reset
add_connection DDR3_SDRAM.afi_clk       Leap_Profiler.clock
add_connection DDR3_SDRAM.afi_reset     Leap_Profiler.reset
add_connection DDR3_SDRAM.afi_clk       Leap_Sim_Control.clock
add_connection DDR3_SDRAM.afi_reset     Leap_Sim_Control.reset
add_connection DDR3_SDRAM.afi_clk       Tiger_ICache.clock
add_connection DDR3_SDRAM.afi_reset     Tiger_ICache.reset
add_connection DDR3_SDRAM.afi_clk       DCache.clk
add_connection DDR3_SDRAM.afi_reset     DCache.reset
add_connection DDR3_SDRAM.afi_clk       JTAG_to_FPGA_Bridge.clk
add_connection DDR3_SDRAM.afi_reset     JTAG_to_FPGA_Bridge.clk_reset
###############################################################################


###############################################################################
# Add JTAG UART
add_instance JTAG_UART altera_avalon_jtag_uart

add_connection DDR3_SDRAM.afi_clk           JTAG_UART.clk
add_connection DDR3_SDRAM.afi_reset         JTAG_UART.reset
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

add_connection DDR3_SDRAM.afi_clk       UART.clk
add_connection DDR3_SDRAM.afi_reset     UART.reset
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

