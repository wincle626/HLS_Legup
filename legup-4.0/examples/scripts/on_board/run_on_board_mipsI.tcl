###############################################
### Print to JTAG UART Function
###############################################

proc jtag_uart { jtag_avl line } {
    foreach char [split $line "" ] {
        scan $char %c char
        master_write_8 $jtag_avl 0xFF201000 $char
    }
}


###############################################
### Start scipt
###############################################

puts "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

###############################################
### Process script arguments
###############################################
if {[llength $argv] == 1} {
	set mem_init_file [lindex $argv 0]
	puts "+ Initializing memory with the contents of $mem_init_file"
} else {
	puts "+ Error: Number of arguments must be 1"
	exit 1
}

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

###############################################
### Open Jtag Master
###############################################
open_service master $jtag_avl

###############################################
### Initialize Leap Profiler
###############################################

# assert reset register to hold tiger from running
master_write_32 $jtag_avl 0x80000300 0x00000001

# configure profiler
master_write_32 $jtag_avl 0x80000208 0x40000000

###############################################
### Download Program Binary to SDRAM
###############################################
set input [open $mem_init_file]
set address 0x40000000
while {[gets $input line] >= 0} {
    set line "0x$line"
    master_write_16 $jtag_avl $address $line
    set address [expr {$address + 2}]
}
close $input


###############################################
### Read SDRAM
###############################################
#set mem [master_read_32 $jtag_avl 0x40000020 4]
#foreach value $mem {
#	puts "+ $value"
#}


###############################################
### Start Execution
###############################################
jtag_uart $jtag_avl "\nStarting the program\n"

# release reset register to start tiger
master_write_32 $jtag_avl 0x80000300 0x00000000

###############################################
### Close Jtag Master
###############################################
close_service master $jtag_avl

puts "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

