# Set the board specific settings
set_device_specs -Family StratixIV -Device EP4SGX530KH40C2 -MaxALMs 212480 -MaxM9KMemBlocks 1610 -MaxRAMBits 21233664 -MaxDSPs 128

set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
# Get family's characterization file
set characterization_file $::BOARDS_PATH/StratixIV/StratixIV.tcl
#puts stderr "Characterization file: $characterization_file\n";
if { [file exists $characterization_file] == 0 } {
    puts stderr "Can't find family's characterization file (StratixIV.tcl)!\n";
}
source $characterization_file

