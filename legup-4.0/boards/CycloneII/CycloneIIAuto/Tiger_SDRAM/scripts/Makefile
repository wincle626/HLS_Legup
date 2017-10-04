# The makefile generates a legup system with the following configuration
# Board:		DE2
# Processor:	Tiger MIPS
# ICache:		Tiger's instruction cache
# DCache:		Direct-Mapped Write-Through (128 lines of 4 words)
# Memory:		SDRAM
# Other:		None

default:
	cp ../../../../legup_components.ipx .
	cp ../../../../legup_components.ipx ../.
	#qsys-script --script=gen_legup_system.tcl &> o_gen_legup_system.txt 2>&1
	qsys-script --script=gen_legup_system.tcl
	mv legup_system.qsys ../.
	cd ../ && qsys-generate ./legup_system.qsys --synthesis=VERILOG --testbench=STANDARD --testbench-simulation=VERILOG
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

