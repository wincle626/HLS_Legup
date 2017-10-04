if {$argc != 2} {
	puts "Needs 2 arguments"
	exit
}

set usbblaster_name [lindex $argv 0]
set device_num [lindex $argv 1]

set test_device [lindex [get_device_names -hardware_name $usbblaster_name] $device_num]

open_device -hardware_name $usbblaster_name -device_name $test_device

device_lock -timeout 10000

device_virtual_ir_shift -instance_index 39 -ir_value 0 -no_captured_ir_value
device_virtual_dr_shift -instance_index 39 -length 1 -dr_value "1" -value_in_hex -no_captured_dr_value

device_unlock

close_device