onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -label clk -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/clk
add wave -noupdate -label pc -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/core/pc
add wave -noupdate -label ins -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/ins
add wave -noupdate -label reset_n -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/reset
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/avm_instructionMaster_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/iCacheStall
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/dCacheStall
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avm_instructionMaster_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avm_instructionMaster_address
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avm_instructionMaster_readdata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avm_instructionMaster_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avm_instructionMaster_readdatavalid
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avs_debugSlave_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avs_debugSlave_writedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/avs_debugSlave_irq
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/memwrite
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/memread
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/mem16
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/mem8
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/memaddress
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/memwritedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/memreaddata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/memCanRead
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/memCanWrite
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/insValid
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/irq
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/irq_number
add wave -noupdate -label clk -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/clk
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/csi_clockreset_clk
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_sdram/clk
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/csi_clockreset_reset_n
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_address
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_writedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_byteenable
add wave -noupdate -radix unsigned /test_bench/DUT/the_data_cache_0/avm_dataMaster_readdata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_readdatavalid
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/avm_dataMaster_irq
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchDone_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/state_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheWrite_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQ_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel_for_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel_for_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel_until_fetchDone
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem8_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem16_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memReadData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memReadDataWord_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem_accelerator
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/irq
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/irq_number
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheClkEn
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/csi_clockreset_clk
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/csi_clockreset_reset_n
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelADDR_address
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelADDR_begintransfer
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelADDR_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelADDR_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelADDR_writedata
add wave -noupdate -radix unsigned /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelADDR_readdata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelADDR_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelDATA_begintransfer
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelDATA_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelDATA_writedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelDATA_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelSIZE_begintransfer
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelSIZE_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelSIZE_writedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avs_accelSIZE_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_address
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_writedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_byteenable
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_readdata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster_readdatavalid
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchWord_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/state_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchDone_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheHit_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/validBit_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_address
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_writedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_byteenable
add wave -noupdate -radix unsigned /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_readdata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_AccelMaster2_readdatavalid
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memAddress_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheHit_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheAddress_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/tag_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheWrite_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQ_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheTag_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQData_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchDone_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/address_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchWord_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/state_accel2
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_address
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_writedata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_byteenable
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_readdata
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_waitrequest
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_readdatavalid
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/avm_dataMaster_irq
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheHit
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheAddress
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/blockWord
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/tag
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memReadDataWord
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheWrite
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheClkEn
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheData
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQ
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheTag
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/validBit
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQData
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheWriteData
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/writeData
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedTag
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedBlockWord
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedByte
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchDone
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/bypassCache
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/address
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/writeDataWord
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchData
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchWord
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/state
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedMem16
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedMem8
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/flushAddr
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/clk
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/reset_n
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memRead
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memWrite
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memAddress
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memWriteData
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memReadData
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/flush
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem8
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem16
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/canRead
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/canWrite
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/canFlush
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheClkEn_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheClkEn_proc
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memRead_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memWrite_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memAddress_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memWriteData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memReadData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem8_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem16_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem64_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheHit_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheAddress_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/blockWord_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/tag_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/byte_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memReadDataWord_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheWrite_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQ_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheTag_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/validBit_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheQData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/cacheWriteData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/writeData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedTag_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedBlockWord_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedByte_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchDone_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel_for_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel_for_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/address_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/writeDataWord_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchData_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/fetchWord_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/state_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedMem64_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedMem16_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/savedMem8_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/flushAddr_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/mem_accelerator
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_cpu_for_accel_reg
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_cpu_for_accel
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_cpu_nodly
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel_until_fetchDone
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/write_after_read
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/read_after_write
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/state_64
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memRead_accel64
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memWrite_accel64
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_accel_64
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memRead_64
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memWrite_64
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/memReadData_accel_lo
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/waitrequest_32
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/counter_64
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/stall_condition

TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 8} {228612131 ps} 1} {{Cursor 5} {1161740836 ps} 1} {{Cursor 4} {701314913 ps} 0} {{Cursor 4} {228612000 ps} 0}
configure wave -namecolwidth 591
configure wave -valuecolwidth 376
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
