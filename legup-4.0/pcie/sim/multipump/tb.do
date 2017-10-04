vlib work
vlog *.v
vsim work.MP_2ports_tb
add wave -noupdate -radix hexadecimal /MP_2ports_tb/*
run 150
