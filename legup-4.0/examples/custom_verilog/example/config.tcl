source ../../legup.tcl
set_project CycloneIV DE2-115 Tiger_SDRAM

set_custom_verilog_function "boardPutChar" noMemory \
                                           output 7:0 UART_BYTE_OUT \
                                           output 0:0 UART_START_SEND \
                                           input 1:0 UART_RESPONSE \
                                           input 3:0 KEY \
                                           output 17:0 LEDR

set_custom_verilog_function "boardGetChar" noMemory \
                                           input 7:0 UART_BYTE_IN \
                                           input 0:0 UART_START_RECEIVE  \
                                           input 1:0 UART_RESPONSE \
                                           output 17:0 LEDR

set_custom_verilog_function "updateCharactersEntered" noMemory \
                                                      output 6:0 HEX0 \
                                                      output 6:0 HEX1 \
                                                      output 6:0 HEX2 \
                                                      output 6:0 HEX3

set_custom_verilog_file "stdio.v"
set_custom_verilog_file "topLevel.v"
set_custom_top_level_module "io_top"
