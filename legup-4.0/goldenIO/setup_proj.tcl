# to run:
# quartus_sh -t setup_proj.tcl <family> <verilog_name>

project_new top -overwrite

set_global_assignment -name FAMILY [lindex $quartus(args) 0]
set_global_assignment -name DEVICE AUTO

# Target: DE2 Board
#set_global_assignment -name FAMILY CycloneII
#set_global_assignment -name DEVICE EP2C35F672C6

# Target: StratixII
#set_global_assignment -name FAMILY StratixII
# fix Stratix II no fit on gsm
#set_global_assignment -name DEVICE EP2S60F484C3

set_global_assignment -name TOP_LEVEL_ENTITY top
set_global_assignment -name SOURCE_FILE [lindex $quartus(args) 1].v
set_global_assignment -name SDC_FILE [lindex $quartus(args) 1].sdc
set_global_assignment -name NUMBER_OF_PATHS_TO_REPORT 100000

# Create timing assignments
create_base_clock -fmax "1000 MHz" -target clk clk

# turn small rams into logic
#set_global_assignment -name AUTO_RAM_TO_LCELL_CONVERSION ON

# prevent DSP blocks from being used
#set_global_assignment -name DSP_BLOCK_BALANCING "LOGIC ELEMENTS"

# minimize circuit area when packing
#set_global_assignment -name INI_VARS "fit_pack_for_density_light=on; fit_report_lab_usage_stats=on"

# extra synthesis options
#set_global_assignment -name SMART_RECOMPILE ON
#set_global_assignment -name OPTIMIZE_MULTI_CORNER_TIMING ON
#set_global_assignment -name PHYSICAL_SYNTHESIS_COMBO_LOGIC ON
#set_global_assignment -name PHYSICAL_SYNTHESIS_REGISTER_RETIMING ON
#set_global_assignment -name PHYSICAL_SYNTHESIS_REGISTER_DUPLICATION ON
#set_global_assignment -name PHYSICAL_SYNTHESIS_ASYNCHRONOUS_SIGNAL_PIPELINING ON
#set_global_assignment -name PHYSICAL_SYNTHESIS_COMBO_LOGIC_FOR_AREA OFF
#set_global_assignment -name PHYSICAL_SYNTHESIS_MAP_LOGIC_TO_MEMORY_FOR_AREA OFF
#set_global_assignment -name PHYSICAL_SYNTHESIS_EFFORT EXTRA
#set_global_assignment -name NUM_PARALLEL_PROCESSORS ALL
#set_global_assignment -name FLOW_ENABLE_IO_ASSIGNMENT_ANALYSIS ON
#set_global_assignment -name CYCLONEII_OPTIMIZATION_TECHNIQUE SPEED
#set_global_assignment -name SYNTH_TIMING_DRIVEN_SYNTHESIS ON
#set_global_assignment -name ADV_NETLIST_OPT_SYNTH_WYSIWYG_REMAP ON
#set_global_assignment -name OPTIMIZE_POWER_DURING_FITTING "NORMAL COMPILATION"
#set_global_assignment -name ROUTER_TIMING_OPTIMIZATION_LEVEL MAXIMUM
#set_global_assignment -name PLACEMENT_EFFORT_MULTIPLIER 4
#set_global_assignment -name ROUTER_EFFORT_MULTIPLIER 4
#set_global_assignment -name FINAL_PLACEMENT_OPTIMIZATION ALWAYS
#set_global_assignment -name FITTER_AGGRESSIVE_ROUTABILITY_OPTIMIZATION AUTOMATICALLY
#set_global_assignment -name ENABLE_DRC_SETTINGS ON
#set_global_assignment -name SAVE_DISK_SPACE OFF
#set_global_assignment -name MUX_RESTRUCTURE OFF
#set_global_assignment -name OPTIMIZE_POWER_DURING_SYNTHESIS "NORMAL COMPILATION"
#set_global_assignment -name STATE_MACHINE_PROCESSING AUTO
#set_global_assignment -name PARALLEL_SYNTHESIS ON
#set_global_assignment -name AUTO_PACKED_REGISTERS_STRATIXII NORMAL
#set_global_assignment -name AUTO_PACKED_REGISTERS_MAXII NORMAL
#set_global_assignment -name AUTO_PACKED_REGISTERS_CYCLONE NORMAL
#set_global_assignment -name AUTO_PACKED_REGISTERS NORMAL
#set_global_assignment -name AUTO_PACKED_REGISTERS_STRATIX NORMAL
#set_global_assignment -name PRE_MAPPING_RESYNTHESIS ON


project_close
