# to run:
# quartus_sh -t setup_proj.tcl <family> <board> <verilog_name> <top_level_module> [<dbg_dir>]

set projname top.xise

if {[file exists $projname]} {
	file delete $projname
}

project new $projname

set family [lindex $argv 0]

set board  [lindex $argv 1]


set legupdir  [file normalize [file dirname [info script]]/../]

set xilinxIPDir $legupdir/ip/xilinx
#set search_path [file normalize [file dirname [info script]]/../tiger/processor/altera_libs]

#set board_qsf  $legupdir/boards/$family/$board/$board.qsf
set board_ucf  $legupdir/boards/$family/$board/$board.ucf
set board_xtcl  $legupdir/boards/$family/$board/$board.x.tcl

# Debugger files
if {[lindex $argv 4] == 1} {
	
	xfile add $legupdir/dbg/rtl/hlsd.v
	xfile add $legupdir/dbg/rtl/trigger.v
	xfile add $legupdir/dbg/rtl/trace.v
	xfile add $legupdir/dbg/rtl/comm.v
	xfile add $legupdir/dbg/rtl/uart_control.v
	xfile add $legupdir/dbg/rtl/uart_xilinx.vhd
	xfile add $legupdir/dbg/rtl/rams_xilinx.v

	if {$board eq "ML605"} {
		xfile add $legupdir/dbg/rtl/clockbuf_virtex6.v
	} else {
		puts "Board not supported for debugging"
		exit -1
	}
}

xfile add $legupdir/ip/libs/generic/div_unsigned.v
xfile add $legupdir/ip/libs/generic/div_signed.v 

#set_global_assignment -name SOURCE_FILE [lindex $quartus(args) 2].v
xfile add [lindex $argv 2].v

#set_global_assignment -name SDC_FILE [lindex $quartus(args) 2].sdc

#set_global_assignment -name TOP_LEVEL_ENTITY [lindex $quartus(args) 3]
project set top [lindex $argv 3]

# One-hot encoding is helpful for the trace scheduler logic
# Quartus seems to choose one-hot by default, but not ISE.
project set "FSM Encoding Algorithm" "One-Hot" -process "Synthesize - XST"

# XST was crashing without this - See http://forums.xilinx.com/t5/Synthesis/Synthesis-Error-INTERNAL-ERROR-Xst-cmain-c-3423-1-29-Process/td-p/290491
project set "Other XST Command Line Options" "-keep_hierarchy soft" -process "Synthesize - XST"

project set "Generate Detailed MAP Report" "true" -process "Map"

#set_global_assignment -name SEARCH_PATH $search_path

# first check if there is a .qsf file for the specified board
# if so, source it
if { [file exists $board_xtcl] } {
	source $board_xtcl
	puts "Using $board_xtcl"
} else {
# if neither a board or device .qsf file exists, print a warning
	puts "Error: $board_xtcl not found"
	exit -1
}

file copy -force $board_ucf "."
xfile add $board.ucf

# Xilinx support files needed for the UART.
if {[lindex $argv 4] == 1} {
	set isePath [file dirname [exec which ise]]
	set procCommonVhdlPath $isePath/../../../EDK/hw/XilinxProcessorIPLib/pcores/proc_common_v3_00_a/hdl/vhdl
	set procCommonVhdlPath [file normalize $procCommonVhdlPath]

	lib_vhdl new proc_common_v3_00_a

	foreach f [glob $procCommonVhdlPath/*.vhd] {
		xfile add $f
		lib_vhdl add_file proc_common_v3_00_a $f
	}
}

project close

