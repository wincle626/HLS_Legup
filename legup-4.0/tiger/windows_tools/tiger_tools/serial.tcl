if {$argc != 4} {
	puts "Need 4 arguments"
	exit
}

set usbblaster_name [lindex $argv 0]
set device_num [lindex $argv 1]
set instance [lindex $argv 2]
set port [lindex $argv 3]

set test_device [lindex [get_device_names -hardware_name $usbblaster_name] $device_num]

open_device -hardware_name $usbblaster_name -device_name $test_device

fconfigure stdin -blocking 0 \
	-buffering none \
	-encoding binary \
	-translation binary

set net [socket "localhost" $port]

fconfigure $net \
	-blocking 0 \
	-buffering none \
	-encoding binary \
	-translation binary

while {true} {
	if {[eof $net]} {
		exit	
	}
	
	device_lock -timeout 10000
		
	#See if there's anything to read
	device_virtual_ir_shift -instance_index $instance -ir_value 3 -no_captured_ir_value
	
	set readAmount [device_virtual_dr_shift -instance_index $instance -length 9 -dr_value \
		"000000000"]
				
	set readAmountInt 0

	#Convert binary number given to integer
	for {set i 0} {$i < [string length $readAmount]} {incr i} {
			set bit [string index $readAmount $i]
			if {$bit == 1} {
				set readAmountInt [expr {($readAmountInt * 2) + 1}]
			} else {
				set readAmountInt [expr {$readAmountInt * 2}]
			}
		}
		
	#If there is something to read
	if {$readAmountInt > 0} {
		#Print it to stdout
		for {set i 0} {$i < $readAmountInt} {incr i} {
			device_virtual_ir_shift -instance_index $instance -ir_value 2 -no_captured_ir_value
			set readChar [binary format H2 [device_virtual_dr_shift -instance_index $instance -length 8 \
				-dr_value "00" -value_in_hex]]
				
			puts -nonewline $net $readChar
		}
	}
	
	#Read from stdin
	set text [read $net 100]
	if {[string length $text] > 0} { #If we actually got something
		
		#Reverse the string we got (we need to send it in reverse)
		set reverseText ""
		for {set i [string length $text]} {$i > 0} {incr i -1} {
			set char [string index $text [expr {$i - 1}]]
			set reverseText "$reverseText$char"
		}
		
		#Convert it to hex for sending
		binary scan $reverseText H* textHex
		
		#Wait til there's space in the buffer to send it
		#puts "Waiting"
		
		device_unlock
		
		while {true} {
			device_lock -timeout 10000
			
			device_virtual_ir_shift -instance_index $instance -ir_value 1 -no_captured_ir_value
	
			set writeLeft [device_virtual_dr_shift -instance_index $instance -length 9 -dr_value \
				"000000000"]
				
			set writeLeftInt 0
			
			for {set i 0} {$i < [string length $writeLeft]} {incr i} {
				set bit [string index $writeLeft $i]
				if {$bit == 1} {
					set writeLeftInt [expr {($writeLeftInt * 2) + 1}]
				} else {
					set writeLeftInt [expr {$writeLeftInt * 2}]
				}
			}
			
			if {$writeLeftInt >= [string length $text]} {
				device_unlock
				break
			}
			
			device_unlock
		}
		
		device_lock -timeout 10000
		
		#puts "Waiting done"
		
		#Send it
		set textLength [expr {[string length $reverseText] * 8}]
		
		device_virtual_ir_shift -instance_index $instance -ir_value 0 -no_captured_ir_value
		
		device_virtual_dr_shift -instance_index $instance -length $textLength -value_in_hex \
			-dr_value $textHex -no_captured_dr_value
	}
	
	device_unlock
}

close_device