# Set the board specific settings
set_device_specs -Family CycloneIV -Device EP4CE22F17C6N -MaxLEs 22320 -MaxM9KMemBlocks 66 -MaxRAMBits 608256 -MaxDSPs 132

set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
# Get family's characterization file
set characterization_file $::BOARDS_PATH/CycloneIV/CycloneIV.tcl
#puts stderr "Characterization file: $characterization_file\n";
if { [file exists $characterization_file] == 0 } {
    puts stderr "Can't find family's characterization file (CycloneIV.tcl)!\n";
}
source $characterization_file

