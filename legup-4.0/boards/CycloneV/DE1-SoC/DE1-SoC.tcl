# Set the board specific settings
#set_device_specs -Family CycloneV -Device 5CSEMA5F31C6 -MaxALMs 32075 -MaxM10Ks 397 -MaxRAMBits 4065280 -MaxDSPs 87 #it is really MaxM10Ks
set_device_specs -Family CycloneV -Device 5CSEMA5F31C6 -MaxALMs 32075 -MaxM4Ks 397 -MaxRAMBits 4065280 -MaxDSPs 87

set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
# Get family's characterization file
set characterization_file $::BOARDS_PATH/CycloneV/CycloneV.tcl
#puts stderr "Characterization file: $characterization_file\n";
if { [file exists $characterization_file] == 0 } {
    puts stderr "Can't find family's characterization file (CycloneV.tcl)!\n";
}
source $characterization_file

