onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider testbench
add wave -noupdate /div_testbench/run
add wave -noupdate /div_testbench/my_done
add wave -noupdate /div_testbench/steve_done
add wave -noupdate -radix hexadecimal /div_testbench/my_result
add wave -noupdate -radix hexadecimal /div_testbench/steve_result
add wave -noupdate -radix hexadecimal /div_testbench/lpm_result
add wave -noupdate /div_testbench/clk
add wave -noupdate /div_testbench/clken
add wave -noupdate /div_testbench/reset
add wave -noupdate -radix hexadecimal /div_testbench/rom_address
add wave -noupdate -radix hexadecimal /div_testbench/rom_data
add wave -noupdate -radix hexadecimal /div_testbench/operandA
add wave -noupdate -radix hexadecimal /div_testbench/operandB
add wave -noupdate /div_testbench/my_error
add wave -noupdate /div_testbench/steve_error
add wave -noupdate -divider {MY DUT}
add wave -noupdate /div_testbench/MY_DUT/clk
add wave -noupdate /div_testbench/MY_DUT/clken
add wave -noupdate /div_testbench/MY_DUT/resetn
add wave -noupdate /div_testbench/MY_DUT/go
add wave -noupdate -radix hexadecimal /div_testbench/MY_DUT/dividend
add wave -noupdate -radix hexadecimal /div_testbench/MY_DUT/divisor
add wave -noupdate -radix hexadecimal /div_testbench/MY_DUT/quotient
add wave -noupdate -radix hexadecimal /div_testbench/MY_DUT/remainder
add wave -noupdate /div_testbench/MY_DUT/done
add wave -noupdate /div_testbench/MY_DUT/y
add wave -noupdate /div_testbench/MY_DUT/Y
add wave -noupdate /div_testbench/MY_DUT/load_input_values
add wave -noupdate /div_testbench/MY_DUT/run_divider
add wave -noupdate -radix hexadecimal /div_testbench/MY_DUT/stored_divisor
add wave -noupdate /div_testbench/MY_DUT/count
add wave -noupdate /div_testbench/MY_DUT/count_is_zero
add wave -noupdate -radix hexadecimal /div_testbench/MY_DUT/sum
add wave -noupdate /div_testbench/MY_DUT/c_out
add wave -noupdate -divider DUT
add wave -noupdate /div_testbench/DUT/S1
add wave -noupdate /div_testbench/DUT/S2
add wave -noupdate /div_testbench/DUT/S3
add wave -noupdate /div_testbench/DUT/Clock
add wave -noupdate /div_testbench/DUT/Resetn
add wave -noupdate /div_testbench/DUT/s
add wave -noupdate /div_testbench/DUT/LA
add wave -noupdate /div_testbench/DUT/EB
add wave -noupdate -radix hexadecimal /div_testbench/DUT/DataA
add wave -noupdate -radix hexadecimal /div_testbench/DUT/DataB
add wave -noupdate -radix hexadecimal /div_testbench/DUT/R
add wave -noupdate -radix hexadecimal /div_testbench/DUT/Q
add wave -noupdate /div_testbench/DUT/Done
add wave -noupdate /div_testbench/DUT/Cout
add wave -noupdate /div_testbench/DUT/z
add wave -noupdate /div_testbench/DUT/R0
add wave -noupdate -radix hexadecimal /div_testbench/DUT/DataR
add wave -noupdate -radix hexadecimal /div_testbench/DUT/Sum
add wave -noupdate /div_testbench/DUT/y
add wave -noupdate /div_testbench/DUT/Y
add wave -noupdate -radix hexadecimal /div_testbench/DUT/A
add wave -noupdate -radix hexadecimal /div_testbench/DUT/B
add wave -noupdate -radix hexadecimal /div_testbench/DUT/Count
add wave -noupdate /div_testbench/DUT/EA
add wave -noupdate /div_testbench/DUT/Rsel
add wave -noupdate /div_testbench/DUT/LR
add wave -noupdate /div_testbench/DUT/ER
add wave -noupdate /div_testbench/DUT/ER0
add wave -noupdate /div_testbench/DUT/LC
add wave -noupdate /div_testbench/DUT/EC
add wave -noupdate /div_testbench/DUT/k
add wave -noupdate -divider Control
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/numer
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/denom
add wave -noupdate /div_testbench/LPM_DIVIDE_component/clock
add wave -noupdate /div_testbench/LPM_DIVIDE_component/aclr
add wave -noupdate /div_testbench/LPM_DIVIDE_component/clken
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/quotient
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/remain
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/quotient_pipe
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/remain_pipe
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/tmp_quotient
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/tmp_remain
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/not_numer
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/int_numer
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/not_denom
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/int_denom
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/t_numer
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/t_q
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/t_denom
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/t_r
add wave -noupdate /div_testbench/LPM_DIVIDE_component/sign_q
add wave -noupdate /div_testbench/LPM_DIVIDE_component/sign_r
add wave -noupdate /div_testbench/LPM_DIVIDE_component/sign_n
add wave -noupdate /div_testbench/LPM_DIVIDE_component/sign_d
add wave -noupdate -radix hexadecimal /div_testbench/LPM_DIVIDE_component/lpm_remainderpositive
add wave -noupdate /div_testbench/LPM_DIVIDE_component/i
add wave -noupdate /div_testbench/LPM_DIVIDE_component/rsig
add wave -noupdate /div_testbench/LPM_DIVIDE_component/pipe_ptr
add wave -noupdate /div_testbench/LPM_DIVIDE_component/i_aclr
add wave -noupdate /div_testbench/LPM_DIVIDE_component/i_clock
add wave -noupdate /div_testbench/LPM_DIVIDE_component/i_clken
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {209492 ps} 0}
configure wave -namecolwidth 367
configure wave -valuecolwidth 108
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {2385264 ps}
