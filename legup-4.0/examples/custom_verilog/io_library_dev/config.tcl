source ../../legup.tcl
set_project CycloneIV DE2-115 Tiger_SDRAM
set_custom_verilog_function "boardPutChar" UART_BYTE_OUT 7:0 output UART_START_SEND 0:0 output UART_RESPONSE 1:0 input KEY 3:0 input LEDR 17:0 output
set_custom_verilog_file "stdio.v"
set_custom_verilog_file "topLevel.v"
set_custom_top_level_module "test"

