# used by Makefile.config if you use the FPGA_BOARD env variable:
#       FPGA_BOARD=DE2-115 make
source [file dirname [info script]]/legup.tcl
set_project CycloneIV DE2-115 Tiger_SDRAM
