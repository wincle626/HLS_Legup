# Set the board specific settings
set_device_specs -Family CycloneII -Device EP2C35F672C6 -MaxLEs 33216 -MaxM4Ks 105 -MaxRAMBits 483840 -MaxDSPs 70        

set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
# Get family's characterization file
set characterization_file $::BOARDS_PATH/CycloneII/CycloneII.tcl
#puts stderr "Characterization file: $characterization_file\n";
if { [file exists $characterization_file] == 0 } {
    puts stderr "Can't find family's characterization file (CycloneII.tcl)!\n";
}
source $characterization_file

