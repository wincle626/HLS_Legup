source ../../../legup.tcl

set_parameter PRINTF_CYCLES 1
#set_operation_latency multiply 0

set_custom_verilog_function "custom_VGA_display" noMemory \
	input 0:0 VGA_REF_CLOCK_50 \
	output 0:0 VGA_CLK \
	output 0:0 VGA_HS  \
	output 0:0 VGA_VS  \
	output 0:0 VGA_BLANK_N \
	output 0:0 VGA_SYNC_N  \
	output 7:0 VGA_R \
	output 7:0 VGA_G \
	output 7:0 VGA_B

set_custom_verilog_file "../IPs/custom_VGA_display.v"
