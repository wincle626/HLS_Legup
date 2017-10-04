# This file defines the parameters of the system for use by LegUp
# Parameters are defined in <legup_root>/llvm/lib/Target/Verilog/LegupConfig.cpp


# Processor Settings
set_parameter SYSTEM_PROCESSOR_ARCHITECTURE ARMA9
set_parameter SYSTEM_PROCESSOR_NAME         Arm_A9_HPS
set_parameter SYSTEM_PROCESSOR_DATA_MASTER  h2f_axi_master

# Clock Settings
set_parameter SYSTEM_CLOCK_MODULE System_PLL
set_parameter SYSTEM_CLOCK_INTERFACE outclk0

# Reset Settings
set_parameter SYSTEM_RESET_MODULE Arm_A9_HPS
set_parameter SYSTEM_RESET_INTERFACE h2f_reset

# Memory Slave Settings
#set_parameter SYSTEM_MEMORY_MODULE      Arm_A9_HPS
#set_parameter SYSTEM_MEMORY_INTERFACE   f2h_axi_slave
set_parameter SYSTEM_MEMORY_MODULE      legup_hps_address_adaptor_0
set_parameter SYSTEM_MEMORY_INTERFACE   bridge

# Cache Settings
set_parameter SYSTEM_DATA_CACHE_TYPE legup_dm_wt_cache

# Memory Settings
set_parameter SYSTEM_MEMORY_BASE_ADDRESS        0x40000000
set_parameter SYSTEM_MEMORY_SIZE                0x04000000
set_parameter SYSTEM_MEMORY_WIDTH               2
set_parameter SYSTEM_MEMORY_SIM_INIT_FILE_TYPE  DAT
set_parameter SYSTEM_MEMORY_SIM_INIT_FILE_NAME  altera_sdram_partner_module.dat

# Parallel Core Settings
#set_parameter SYSTEM_BARRIER_BASE_ADDRESS   0x00800000
#set_parameter SYSTEM_MUTEX_BASE_ADDRESS     0x00800000


