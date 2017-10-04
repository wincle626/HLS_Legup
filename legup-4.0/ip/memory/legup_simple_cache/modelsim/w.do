onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider Testbench
add wave -noupdate -radix hexadecimal /cache_tb/s_waitrequest
add wave -noupdate -radix hexadecimal /cache_tb/m_read
add wave -noupdate -radix hexadecimal /cache_tb/m_write
add wave -noupdate -radix hexadecimal /cache_tb/clk
add wave -noupdate -radix hexadecimal /cache_tb/reset
add wave -noupdate -radix hexadecimal /cache_tb/s_rom_address
add wave -noupdate -radix hexadecimal /cache_tb/s_rom_data
add wave -noupdate -radix hexadecimal /cache_tb/m_rom_address
add wave -noupdate -radix hexadecimal /cache_tb/m_rom_data
add wave -noupdate -divider Cache
add wave -noupdate -radix hexadecimal /cache_tb/DUT/clk
add wave -noupdate -radix hexadecimal /cache_tb/DUT/reset
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_address
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_byteenable
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_read
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_write
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_writedata
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_readdata
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_readdatavalid
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avs_cache_waitrequest
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_readdata
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_readdatavalid
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_waitrequest
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_address
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_byteenable
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_read
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_write
add wave -noupdate -radix hexadecimal /cache_tb/DUT/avm_cache_writedata
add wave -noupdate -radix hexadecimal /cache_tb/DUT/last_avs_address
add wave -noupdate -radix hexadecimal /cache_tb/DUT/last_avs_read
add wave -noupdate -radix hexadecimal /cache_tb/DUT/cache_miss_state
add wave -noupdate -radix hexadecimal /cache_tb/DUT/port_a_data_out
add wave -noupdate -radix hexadecimal /cache_tb/DUT/port_a_tag_out
add wave -noupdate -radix hexadecimal /cache_tb/DUT/cache_hit
add wave -noupdate -radix hexadecimal /cache_tb/DUT/cache_miss
add wave -noupdate -radix hexadecimal /cache_tb/DUT/flush_address
add wave -noupdate /cache_tb/DUT/flush_complete
add wave -noupdate -divider {Tag Memory}
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/data_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/data_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/addr_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/addr_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/we_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/we_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/clk
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/q_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/q_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/tag_memory/ram
add wave -noupdate -divider {Data Memory}
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/data_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/data_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/addr_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/addr_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/we_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/we_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/clk
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/q_a
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/q_b
add wave -noupdate -radix hexadecimal /cache_tb/DUT/data_memory/ram
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {148559 ps} 0}
configure wave -namecolwidth 313
configure wave -valuecolwidth 132
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
WaveRestoreZoom {0 ps} {315358 ps}
