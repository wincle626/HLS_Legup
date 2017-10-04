onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -format Logic -label clk /test_bench/DUT/the_tiger_top_0/tiger_top_0/clk
add wave -noupdate -format Logic -label reset_n /test_bench/DUT/the_tiger_top_0/tiger_top_0/reset
add wave -noupdate -format Literal -label pc -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/core/pc
add wave -noupdate -format Literal -label ins -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/ins
add wave -noupdate -format Literal -label dcache_state /test_bench/DUT/the_tiger_top_0/tiger_top_0/DataCache/state
add wave -noupdate -format Logic -label dcache_cacheHit /test_bench/DUT/the_tiger_top_0/tiger_top_0/DataCache/cacheHit
add wave -noupdate -format Literal -label icache_state -radix decimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/state
add wave -noupdate -format Logic -label icache_cacheHit /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheHit
add wave -noupdate -format Literal -label i_cacheAddress -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheAddress
add wave -noupdate -format Logic -label i_cacheClkEn /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheClkEn
add wave -noupdate -format Literal -label i_cacheData -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheData
add wave -noupdate -format Logic -label i_cacheHit /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheHit
add wave -noupdate -format Literal -label i_cacheQ -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheQ
add wave -noupdate -format Literal -label i_cacheQData -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheQData
add wave -noupdate -format Literal -label i_cacheTag -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheTag
add wave -noupdate -format Logic -label i_cacheWrite /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheWrite
add wave -noupdate -format Literal -label i_cacheWriteData -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheWriteData
add wave -noupdate -format Literal -label i_fetchData -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/fetchData
add wave -noupdate -format Logic -label i_fetchDone /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/fetchDone
add wave -noupdate -format Literal -label i_fetchWord -radix unsigned /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/fetchWord
add wave -noupdate -format Literal -label i_address -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/address
add wave -noupdate -format Literal -label i_memAddress -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/memAddress
add wave -noupdate -format Logic -label i_cacheClkEn /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheClkEn
add wave -noupdate -format Literal -label i_cacheData -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/cacheData
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1769578 ps} 0}
configure wave -namecolwidth 174
configure wave -valuecolwidth 243
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
WaveRestoreZoom {776979973 ps} {777743662 ps}
