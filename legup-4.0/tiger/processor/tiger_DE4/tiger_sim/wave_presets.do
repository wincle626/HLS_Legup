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


# Display signals from module ddr2
add wave -noupdate -divider {ddr2}
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/pll_ref_clk
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_cke
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/soft_reset_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/global_reset_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/reset_phy_clk_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/reset_request_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/phy_clk
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_address
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_size
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_burstbegin
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_read_req
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_write_req
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_ready
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_wdata
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_be
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_rdata_valid
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/local_rdata
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_clk
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_cs_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_addr
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_ba
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_ras_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_cas_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_we_n
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_dm
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_dq
add wave -noupdate -format Logic -radix hexadecimal /test_bench/DUT/the_ddr2/mem_dqs


configure wave -justifyvalue right
configure wave -signalnamewidth 1
TreeUpdate [SetDefaultTree]