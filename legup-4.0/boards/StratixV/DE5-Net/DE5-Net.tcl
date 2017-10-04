# Set the board specific settings
#set_device_specs -Family StratixV -Device 5SGXEA7N2F45C2 -MaxALMs 234720 -MaxM20Ks 2560 -MaxRAMBits 520428800 -MaxDSPs 256 #MaxM20Ks
set_device_specs -Family StratixV -Device 5SGXEA7N2F45C2 -MaxALMs 234720 -MaxM4Ks 2560 -MaxRAMBits 520428800 -MaxDSPs 256

set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
# Get family's characterization file
set characterization_file $::BOARDS_PATH/StratixV/StratixV.tcl
#puts stderr "Characterization file: $characterization_file\n";
if { [file exists $characterization_file] == 0 } {
    puts stderr "Can't find family's characterization file (StratixV.tcl)!\n";
}
source $characterization_file

