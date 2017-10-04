# Display signals from module sdram
add wave -noupdate -divider {sdram}
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/az_addr
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/az_be_n
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/az_cs
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/az_data
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/az_rd_n
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/az_wr_n
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/clk
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/za_data
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/za_valid
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/za_waitrequest
add wave -noupdate -format Literal -radix ascii /test_bench/DUT/the_sdram/CODE
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/zs_addr
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/zs_ba
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/zs_cs_n
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/zs_ras_n
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/zs_cas_n
add wave -noupdate -format Logic /test_bench/DUT/the_sdram/zs_we_n
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/zs_dq
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_sdram/zs_dqm


# Display signals from module uart_0
add wave -noupdate -divider {uart_0}
add wave -noupdate -divider {  Bus Interface}
add wave -noupdate -format Logic /test_bench/DUT/the_uart_0/chipselect
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_uart_0/address
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_uart_0/writedata
add wave -noupdate -format Literal -radix hexadecimal /test_bench/DUT/the_uart_0/readdata
add wave -noupdate -divider {  Internals}
add wave -noupdate -format Logic /test_bench/DUT/the_uart_0/tx_ready
add wave -noupdate -format Literal -radix ascii /test_bench/DUT/the_uart_0/tx_data
add wave -noupdate -format Logic /test_bench/DUT/the_uart_0/rx_char_ready
add wave -noupdate -format Literal -radix ascii /test_bench/DUT/the_uart_0/rx_data


configure wave -justifyvalue right
configure wave -signalnamewidth 1
TreeUpdate [SetDefaultTree]