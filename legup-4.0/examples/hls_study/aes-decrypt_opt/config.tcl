source ../config.tcl
set_memory_global Sbox
set_memory_global word
set_memory_global statemt
set_memory_global key

set_parameter PRINTF_CYCLES 1

set_parameter CLOCK_PERIOD 1

# watch out -- aes has multiple .c files
#set_parameter PIPELINE_ALL 1
