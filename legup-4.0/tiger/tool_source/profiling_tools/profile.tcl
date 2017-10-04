puts "\n\t+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

###############################################
### Argument checking
###############################################
# check whether the required files exist
if {[file exist "AH_reg.dat"] == 0} {
	puts "\t+\tFile AH_reg.dat does not exist."
	exit 1
}
if {[file exist "AH_tab.dat"] == 0} {
	puts "\t+\tFile AH_tab.dat does not exist."
	exit 1
}
if {[file exist "program.dat"] == 0} {
	puts "\t+\tFile program.dat does not exist."
	exit 1
}

# check arguments for profiling, use defaults if arguments not provided
set prof_target "0x00000001"
set prof_type   "0x00000000"
if {[llength $argv] == 1} {
	puts "\t+\tProfiling using default configuration parameters."
	puts "\t+\t\tProfiling Target : Number of Executed Instructions"
	puts "\t+\t\tProfiling Type   : Non-Hierarchical"
} elseif {[llength $argv] == 3} {
	set prof_target [lindex $argv 1]
	set prof_type   [lindex $argv 2]
	puts "\t+\tProfiling using user-specified configuration parameters."

	if {$prof_target eq "0x00000001"} {
		puts "\t+\t\tProfiling Target : Number of Executed Instructions"	
	} elseif {$prof_target eq "0x00000002"} {
		puts "\t+\t\tProfiling Target : Number of Cycles"	
	} elseif {$prof_target eq "0x00000004"} {
		puts "\t+\t\tProfiling Target : Number of Stall Cycles"
	} elseif {$prof_target eq "0x00000008"} {
		puts "\t+\t\tProfiling Target : Number of Instruction Stall Cycles"
	} elseif {$prof_target eq "0x00000010"} {
		puts "\t+\t\tProfiling Target : Number of Data Stall Cycles"
	} else {
		puts -nonewline "\t+\tError: Unknown Profiling Target - "
		puts $prof_target
		exit 1
	}
	
	if {$prof_type eq "0x00000000"} {
		puts "\t+\t\tProfiling Type   : Non-Hierarchical"	
	} elseif {$prof_type eq "0x00000001"} {
		puts "\t+\t\tProfiling Type   : Hierarchical"	
	} else {
		puts -nonewline "\t+\tError: Unknown Profiling Type - "
		puts $prof_type
		exit 1
	}
} else {
	puts "\t+\tError: Number of arguments is expected to be either 0 or 2"
	exit 1
}

set rpt_file [lindex $argv 0]
puts -nonewline "\t+\tProfiling results will be saved in "
puts $rpt_file

###############################################
### Find Jtag Master
###############################################
set master_paths [get_service_paths master]
if {[llength $master_paths] == 0} {
		puts "\t+\tError: No JTAG Master Node Found."
		exit 1
}
# assume we are using the first master service we find
set jtag_avl [lindex $master_paths 0]
open_service master $jtag_avl

###############################################
### Initialize Leap Profiler
###############################################

# assert reset register to hold tiger from running
master_write_32 $jtag_avl 0x02000300 0x00000001

# write address hash parameters for profiler
# registers:
set input [open "AH_reg.dat"]
set address 0x02000000
while {[gets $input line] >= 0} {
	master_write_32 $jtag_avl $address $line
	set address [expr {$address + 4}]
}
close $input

# tabs:
set input [open "AH_tab.dat"]
set address 0x02000100
if {[gets $input line] >= 0} {
	master_write_memory $jtag_avl $address $line
	set NumFuncs [llength [split $line " "]]
}
close $input

# configure profiler
master_write_32 $jtag_avl 0x02000200 $prof_target
master_write_32 $jtag_avl 0x02000204 $prof_type
master_write_32 $jtag_avl 0x02000208 0x00800020

###############################################
### Download Program Binary to SDRAM
###############################################
set input [open "program.dat"]
set address 0x00800000
while {[gets $input line] >= 0} {
	master_write_memory $jtag_avl $address $line
	set address [expr {$address + 256}]
}
close $input

###############################################
### Start Execution and Wait for Result
###############################################
# release reset register to start tiger
master_write_32 $jtag_avl 0x02000300 0x00000000

# read from profiler to know the status
puts -nonewline "\t+\tProgram Starting."
for {set data "0xfffffffe"} {$data eq "0xfffffffe"} {set data [master_read_32 $jtag_avl 0x02000000 1]} {
    puts -nonewline "."
}
puts "\n\t+\tProgram Execution Started"

puts -nonewline "\t+\tProgram Executing."
for {set data "0xffffffff"} {$data eq "0xffffffff"} {set data [master_read_32 $jtag_avl 0x02000000 1]} {
    puts -nonewline "."
}
puts "\n\t+\tProgram Execution Finished"

###############################################
### Retrieve Profiling Results
###############################################
set data [master_read_32 $jtag_avl 0x02000000 $NumFuncs]
set output [open $rpt_file w]
foreach value $data {
	puts $output $value
}
close $output
puts "\t+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
