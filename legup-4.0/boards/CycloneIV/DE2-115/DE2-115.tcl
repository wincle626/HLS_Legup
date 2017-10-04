# Set the board specific settings
set_device_specs -Family CycloneIV -Device EP4CE115F29C7 -MaxLEs 114480 -MaxM9KMemBlocks 432 -MaxRAMBits 3981312 -MaxDSPs 532

set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
# Get family's characterization file
set characterization_file $::BOARDS_PATH/CycloneIV/CycloneIV.tcl
#puts stderr "Characterization file: $characterization_file\n";
if { [file exists $characterization_file] == 0 } {
    puts stderr "Can't find family's characterization file (CycloneIV.tcl)!\n";
}
source $characterization_file

