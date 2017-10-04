# Set the board specific settings
set_device_specs -Family CycloneV -Device 5CSXFC6D6F31 -MaxALMs 41509 -MaxM4Ks 557 -MaxRAMBits 5703680 -MaxDSPs 112

set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
# Get family's characterization file
set characterization_file $::BOARDS_PATH/CycloneV/CycloneV.tcl
#puts stderr "Characterization file: $characterization_file\n";
if { [file exists $characterization_file] == 0 } {
    puts stderr "Can't find family's characterization file (CycloneV.tcl)!\n";
}
source $characterization_file

