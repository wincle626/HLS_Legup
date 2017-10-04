onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -expand -group Testbench /testbench/clk
add wave -noupdate -expand -group Testbench /testbench/reset
add wave -noupdate -expand -group {Sobel Top} /testbench/sobel/clk
add wave -noupdate -expand -group {Sobel Top} /testbench/sobel/reset
add wave -noupdate -expand -group {Sobel Top} /testbench/sobel/done
add wave -noupdate -expand -group {Sobel Top} -radix hexadecimal /testbench/sobel/errors
add wave -noupdate -expand -group {Sobel Top} /testbench/sobel/data_in_valid
add wave -noupdate -expand -group {Sobel Top} /testbench/sobel/data_out_valid
add wave -noupdate -expand -group {Sobel Top} -radix hexadecimal /testbench/sobel/input_image_address
add wave -noupdate -expand -group {Sobel Top} -radix unsigned /testbench/sobel/input_image_data
add wave -noupdate -expand -group {Sobel Top} -radix unsigned /testbench/sobel/sobel_image_data
add wave -noupdate -expand -group {Sobel Top} -radix hexadecimal /testbench/sobel/output_image_address
add wave -noupdate -expand -group {Sobel Top} -radix unsigned /testbench/sobel/output_image_data
add wave -noupdate -expand -group {Sobel Top} -radix hexadecimal /testbench/sobel/golden_image_address
add wave -noupdate -expand -group {Sobel Top} -radix unsigned /testbench/sobel/golden_image_data
add wave -noupdate -expand -group {Sobel Top} -radix unsigned /testbench/sobel/x_location
add wave -noupdate -expand -group {Sobel Top} -radix unsigned /testbench/sobel/y_location
add wave -noupdate -expand -group {Sobel Top} /testbench/sobel/non_border
add wave -noupdate -expand -group Sobel /testbench/sobel/sobel/clk
add wave -noupdate -expand -group Sobel /testbench/sobel/sobel/data_en
add wave -noupdate -expand -group Sobel -radix hexadecimal /testbench/sobel/sobel/data_in
add wave -noupdate -expand -group Sobel /testbench/sobel/sobel/reset
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/data_out
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/original_line_1
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/original_line_2
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/original_line_3
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gx_level_1
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gx_level_2
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gx_level_3
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gy_level_1
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gy_level_2
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gy_level_3
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gx_bounded
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/gy_bounded
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/g_bounded
add wave -noupdate -expand -group Sobel -radix unsigned /testbench/sobel/sobel/result
add wave -noupdate -expand -group Sobel -radix hexadecimal /testbench/sobel/sobel/shift_reg_out
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {22399 ps} 0}
configure wave -namecolwidth 301
configure wave -valuecolwidth 118
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
configure wave -timelineunits ns
update
WaveRestoreZoom {22336 ps} {22657 ps}
