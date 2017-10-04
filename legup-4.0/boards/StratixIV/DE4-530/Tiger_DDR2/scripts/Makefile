# The makefile generates a legup system with the following configuration
# Board:		DE4-530
# Processor:	Tiger MIPS
# ICache:		Tiger's instruction cache
# DCache:		Direct-Mapped Write-Through (128 lines of 4 words)
# Memory:		DDR2 SDRAM
# Other:		None

default: clean
	cp ../../../../legup_components.ipx .
	cp ../../../../legup_components.ipx ../.
	#qsys-script --script=gen_legup_system.tcl &> o_gen_legup_system.txt 2>&1
	qsys-script --script=gen_legup_system.tcl
	mv legup_system.qsys ../.
	cd ../ && qsys-generate ./legup_system.qsys --synthesis=VERILOG --testbench=STANDARD --testbench-simulation=VERILOG
	cp setup_sim.sh ../.
	cd ../ && ./setup_sim.sh
	rm -f ../setup_sim.sh
	rm -f legup_components.ipx
	rm -f ../legup_components.ipx
	# Remove unnecessary files
	rm -rf ../legup_system/testbench/aldec/
	rm -rf ../legup_system/testbench/cadence/
	rm -rf ../legup_system/testbench/mentor/
	rm -rf ../legup_system/testbench/synopsys/
	rm -f ../legup_system.cmp
	rm -f ../legup_system.html
	rm -f ../legup_system_generation.rpt
	rm -f ../legup_system_tb.csv
	rm -f ../legup_system_tb.spd

clean:
	rm -rf ../legup_system/*

.PHONY: default clean

