onerror {resume}
quietly WaveActivateNextPane {} 0

add wave -group tiger_top -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/*
add wave -group ins_cache -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/InsCache/*
add wave -group handler   -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/slave_handler/*
add wave -group jtag_mstr -noupdate -radix hexadecimal /tiger_tb/tiger_inst/*

# add wave -group pbridge_m -noupdate -radix hexadecimal /tiger_tb/tiger_inst/*
# add wave -group sdram     -noupdate -radix hexadecimal /tiger_tb/tiger_inst/*
# add wave -group fak_sdram -noupdate -radix hexadecimal /tiger_tb/tiger_inst/sdram_test_component_ram/*

add wave -group profiler  -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/*
add wave -group OpDecode        -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/od/*
add wave -group AddressHash     -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/ah/*
add wave -group AddressStack    -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/as/*
add wave -group CountingBlock   -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/cb/*
add wave -group CounterStorage  -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/cb/cs/*
add wave -group CounterStack    -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/cb/cstack/*
add wave -group IncCounter      -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/profiler/cb/ic/*


add wave -noupdate -label clk -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/clk
add wave -noupdate -label pc -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/pc
add wave -noupdate -label ins -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/ins
add wave -noupdate -label reset_n -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/reset
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/clk
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/reset
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_CACHE_address
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_CACHE_read
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_CACHE_write
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_CACHE_writedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_CACHE_readdata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_CACHE_waitrequest
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_instructionMaster_read
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_instructionMaster_address
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_instructionMaster_readdata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_instructionMaster_beginbursttransfer
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_instructionMaster_burstcount
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_instructionMaster_waitrequest
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avm_instructionMaster_readdatavalid
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_address
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_read
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_write
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_readdata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_writedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_readdatavalid
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_beginbursttransfer
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_burstcount
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_byteenable
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster0_waitrequest
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_address
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_beginbursttransfer
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_burstcount
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_byteenable
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_read
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_readdata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_readdatavalid
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_waitrequest
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_write
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avm_dataMaster1_writedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/iCacheStall
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/dCacheStall
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/iCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/dCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/canICacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/canDCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/pc
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/ins
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/memwrite
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/memwritedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/memread
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/mem16
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/mem8
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/memaddress
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/memreaddata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/memCanRead
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/memCanWrite
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/insValid
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/irq
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/irq_number
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avs_debugSlave_write
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avs_debugSlave_writedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/avs_debugSlave_irq
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/stall_cpu
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/cacheHit
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/start
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/count
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/iCacheStall_count
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/dCacheStall_count
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/clk
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/reset
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/iStall
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/dStall
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/stall_cpu
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/iCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/dCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/canICacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/canDCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/irq
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/irqNumber
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/pc
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/instrF
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memwrite
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memread
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/mem16
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/mem8
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memzerofill
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memaddress
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memwritedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memreaddata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memCanRead
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/memCanWrite
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/stallRqEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/exception
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/clearDe
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/stallDe
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/clearEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/stallEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/clearMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/stallMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/clearWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/stallWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/instrDe
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/controlDe
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/instrEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/controlEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/branchoutEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/rsEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/rtEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/CPOutEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/instrMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/controlMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/executeoutMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/branchoutMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/bottomaddressMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegEnMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegEnCopMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegNumMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegDataMA
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/instrWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/controlWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/MAOutWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/branchoutWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegDataWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegNumWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegEnWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/writeRegEnCopWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/rsExFF
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/rtExFF
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/clk
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/reset
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/stall
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/clear
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/irq
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/irqNumber
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/instr
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/writeRegEnWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/writeRegEnCopWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/writeRegNumWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/writeRegDataWB
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/exception
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/controlDe
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/instrEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/controlEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/rsEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/rtEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/CPOutEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/branchoutEx
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/nextpc
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/branchDelay
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/iCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/dCacheFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/controls
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/alucontrol
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/branchtype
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/destreg
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/control
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/i
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/cause
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/status
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/epc
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/epcDe
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/break
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/syscall
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/temp
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/rsFF
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/rtFF
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/clk
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/reset
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/stall
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/exception
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/instr
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/control
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/rs
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/rt
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/branchout
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/epc
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/branchDelay
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/nextpc
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/currentpc
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/branchtype
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/takebranch
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/signimmsh
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/tiger_top_0/core/de/b/pcplus4
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/csi_clockreset_clk
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/csi_clockreset_reset_n
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avs_CACHE0_begintransfer
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avs_CACHE0_read
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avs_CACHE0_write
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avs_CACHE0_writedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avs_CACHE0_readdata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/avs_CACHE0_waitrequest
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/cacheHit[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/cacheHit[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/cacheHit
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheAddress
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/blockWord
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/tag
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/memReadDataWord[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/memReadDataWord[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/memReadDataWord
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheWrite
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheClkEn
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheData
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheQ
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/cacheTag[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/cacheTag[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/cacheTag
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/validBit
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheQData
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheWriteData
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/writeData
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/savedTag
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/savedBlockWord
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/savedByte
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/fetchDone
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/bypassCache
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/address
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/writeDataWord
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/fetchData
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/fetchWord
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/state
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/state_64
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/savedMem16
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/savedMem8
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/flushAddr
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/clk
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/reset_n
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/memRead[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/memRead[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/memRead
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/memWrite
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/memAddress
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/memWriteData
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/memReadData[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/memReadData[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/memReadData
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/flush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/mem8
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/mem16
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/canRead
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/canWrite
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/canFlush
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/stall
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/stall_cpu_for_accel_reg[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/stall_cpu_for_accel_reg[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/stall_cpu_for_accel_reg
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/stall_cpu_nodly[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/stall_cpu_nodly[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/stall_cpu_nodly
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/stall_cpu
add wave -noupdate -radix hexadecimal -expand -subitemconfig {{/tiger_tb/tiger_inst/data_cache_0/stall_cpu_from_accel[1]} {-height 16 -radix hexadecimal} {/tiger_tb/tiger_inst/data_cache_0/stall_cpu_from_accel[0]} {-height 16 -radix hexadecimal}} /tiger_tb/tiger_inst/data_cache_0/stall_cpu_from_accel
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/unstall_cpu_from_accel
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/CACHE_begintransfer
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cache_Byteenable
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cache_Byteenable_32
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/CACHE_read
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/CACHE_readdata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/CACHE_waitrequest
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cache_word_Byteenable
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/CACHE_write
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/CACHE_writedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/cacheAddress
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_address
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_beginbursttransfer
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_burstcount
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_byteenable
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_byteenable_32
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_read
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_readdata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_readdatavalid
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_waitrequest
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_write
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_writedata
add wave -noupdate -radix hexadecimal /tiger_tb/tiger_inst/data_cache_0/dataMaster_writedata_32
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 8} {126380064 ps} 1} {{Cursor 5} {126382396 ps} 0}
configure wave -namecolwidth 546
configure wave -valuecolwidth 145
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
WaveRestoreZoom {126353636 ps} {126432364 ps}
