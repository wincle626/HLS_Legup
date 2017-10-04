# TCL File Generated by University of Toronto's LegUp group
# DO NOT MODIFY

# +-----------------------------------
# | Specify required package(s) 
# | 
package require -exact qsys 13.0
# | 
# +-----------------------------------


# +-----------------------------------
# | module legup_av_width_adapter
# | 
set_module_property DESCRIPTION "Convert a 32-bit Avalon transfer to a 16-bit Avalon transfer"
set_module_property NAME legup_av_width_adapter
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property GROUP "LegUp/Interconnect/Adapters"
set_module_property AUTHOR "University of Toronto - LegUp Group"
set_module_property DISPLAY_NAME "Avalon Width Adapter"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE false
set_module_property ANALYZE_HDL AUTO
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
# | 
# +-----------------------------------

# +-----------------------------------
# | file sets
# | 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL legup_av_width_adapter
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file legup_av_width_adapter.v VERILOG PATH hdl/legup_av_width_adapter.v TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL legup_av_width_adapter
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file legup_av_width_adapter.v VERILOG PATH hdl/legup_av_width_adapter.v

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL legup_av_width_adapter
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file legup_av_width_adapter.v VERILOG PATH hdl/legup_av_width_adapter.v
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point clock
# | 
add_interface clock clock end
set_interface_property clock clockRate 0
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock SVD_ADDRESS_GROUP ""

add_interface_port clock clk clk Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point reset
# | 
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset reset reset Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point adapter_slave
# | 
add_interface adapter_slave avalon end
set_interface_property adapter_slave addressUnits SYMBOLS
set_interface_property adapter_slave associatedClock clock
set_interface_property adapter_slave associatedReset reset
set_interface_property adapter_slave bitsPerSymbol 8
set_interface_property adapter_slave burstOnBurstBoundariesOnly false
set_interface_property adapter_slave burstcountUnits WORDS
set_interface_property adapter_slave explicitAddressSpan 0
set_interface_property adapter_slave holdTime 0
set_interface_property adapter_slave linewrapBursts false
set_interface_property adapter_slave maximumPendingReadTransactions 8
set_interface_property adapter_slave readLatency 0
set_interface_property adapter_slave readWaitTime 1
set_interface_property adapter_slave setupTime 0
set_interface_property adapter_slave timingUnits Cycles
set_interface_property adapter_slave writeWaitTime 0
set_interface_property adapter_slave ENABLED true
set_interface_property adapter_slave EXPORT_OF ""
set_interface_property adapter_slave PORT_NAME_MAP ""
set_interface_property adapter_slave SVD_ADDRESS_GROUP ""

add_interface_port adapter_slave avs_width_adapter_address address Input 32
add_interface_port adapter_slave avs_width_adapter_byteenable byteenable Input 4
add_interface_port adapter_slave avs_width_adapter_read read Input 1
add_interface_port adapter_slave avs_width_adapter_write write Input 1
add_interface_port adapter_slave avs_width_adapter_writedata writedata Input 32
add_interface_port adapter_slave avs_width_adapter_readdata readdata Output 32
add_interface_port adapter_slave avs_width_adapter_readdatavalid readdatavalid Output 1
add_interface_port adapter_slave avs_width_adapter_waitrequest waitrequest Output 1
set_interface_assignment adapter_slave embeddedsw.configuration.isFlash 0
set_interface_assignment adapter_slave embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment adapter_slave embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment adapter_slave embeddedsw.configuration.isPrintableDevice 0
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point adapter_master
# | 
add_interface adapter_master avalon start
set_interface_property adapter_master addressUnits SYMBOLS
set_interface_property adapter_master associatedClock clock
set_interface_property adapter_master associatedReset reset
set_interface_property adapter_master bitsPerSymbol 8
set_interface_property adapter_master burstOnBurstBoundariesOnly false
set_interface_property adapter_master burstcountUnits WORDS
set_interface_property adapter_master doStreamReads false
set_interface_property adapter_master doStreamWrites false
set_interface_property adapter_master holdTime 0
set_interface_property adapter_master linewrapBursts false
set_interface_property adapter_master maximumPendingReadTransactions 0
set_interface_property adapter_master readLatency 0
set_interface_property adapter_master readWaitTime 1
set_interface_property adapter_master setupTime 0
set_interface_property adapter_master timingUnits Cycles
set_interface_property adapter_master writeWaitTime 0
set_interface_property adapter_master ENABLED true
set_interface_property adapter_master EXPORT_OF ""
set_interface_property adapter_master PORT_NAME_MAP ""
set_interface_property adapter_master SVD_ADDRESS_GROUP ""

add_interface_port adapter_master avm_width_adapter_readdata readdata Input 16
add_interface_port adapter_master avm_width_adapter_readdatavalid readdatavalid Input 1
add_interface_port adapter_master avm_width_adapter_waitrequest waitrequest Input 1
add_interface_port adapter_master avm_width_adapter_address address Output 32
add_interface_port adapter_master avm_width_adapter_byteenable byteenable Output 2
add_interface_port adapter_master avm_width_adapter_read read Output 1
add_interface_port adapter_master avm_width_adapter_write write Output 1
add_interface_port adapter_master avm_width_adapter_writedata writedata Output 16
# | 
# +-----------------------------------