# used by Makefile.config if you use the FPGA_BOARD env variable:
#       FPGA_BOARD=DE1-SoC-ARMA9 make
source [file dirname [info script]]/legup.tcl
set_project CycloneV SoCKit ARM_Simple_Hybrid_System
