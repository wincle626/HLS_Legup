# used by Makefile.config if you use the FPGA_BOARD env variable:
#       FPGA_BOARD=DE5-Net make
source [file dirname [info script]]/legup.tcl
set_project StratixV DE5-Net Tiger_DDR3
