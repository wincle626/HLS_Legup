# used by Makefile.config if you use the FPGA_BOARD env variable:
#       FPGA_BOARD=DE2 make
source [file dirname [info script]]/legup.tcl
set_project CycloneII DE2 Tiger_SDRAM
