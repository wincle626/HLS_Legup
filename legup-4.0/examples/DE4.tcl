# used by Makefile.config if you use the FPGA_BOARD env variable:
#       FPGA_BOARD=DE4 make
source [file dirname [info script]]/legup.tcl
set_project StratixIV DE4-530 Tiger_DDR2
