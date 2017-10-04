# This file defines the parameters of the system for use by LegUp
# Parameters are defined in <legup_root>/llvm/lib/Target/Verilog/LegupConfig.cpp


# Processor Settings
set_parameter SYSTEM_PROCESSOR_ARCHITECTURE MIPSI
#set_parameter SYSTEM_PROCESSOR_NAME         Tiger_MIPS
#set_parameter SYSTEM_PROCESSOR_DATA_MASTER  data_master

# Cache Settings
set_parameter SYSTEM_DATA_CACHE_TYPE legup_dm_wt_cache

# Memory Settings
set_parameter SYSTEM_MEMORY_BASE_ADDRESS        0x00000000
set_parameter SYSTEM_MEMORY_SIZE                0x04000000
set_parameter SYSTEM_MEMORY_WIDTH               2
set_parameter SYSTEM_MEMORY_SIM_INIT_FILE_TYPE  DAT
set_parameter SYSTEM_MEMORY_SIM_INIT_FILE_NAME  altera_sdram_partner_module.dat

# Parallel Core Settings
#set_parameter SYSTEM_BARRIER_BASE_ADDRESS   0x00800000
#set_parameter SYSTEM_MUTEX_BASE_ADDRESS     0x00800000


