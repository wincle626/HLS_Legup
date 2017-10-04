LEVEL = ../../examples
include ../../examples/Makefile.config

VSIMFLAG = -note 2009

VSIM_LOC = ${shell which vsim}
MODEL_BIN = ${shell dirname $(VSIM_LOC)}
MODEL_HOME = ${shell dirname $(MODEL_BIN)}

PROCESSOR_DIR = ${shell basename $(PROC_DIR)}
PWDIR = ${shell pwd}

DE2_PROC = tiger_DE2
DE4_PROC = tiger_DE4

# Configure hard-coded paths required by Tiger MIPS Verilog
all:
	./$(DE2_PROC)/clean
	echo "others = $(MODEL_HOME)/modelsim.ini" > $(DE2_PROC)/modelsim.ini.in
	#specifying paths to altera libraries in tiger.v
	sed -i 's|^`include\s".*/altera_mf.v"|`include "$(PWDIR)/altera_libs/altera_mf.v"|' $(DE2_PROC)/tiger.v
	sed -i 's|^`include\s".*/220model.v"|`include "$(PWDIR)/altera_libs/220model.v"|' $(DE2_PROC)/tiger.v
	sed -i 's|^`include\s".*/sgate.v"|`include "$(PWDIR)/altera_libs/sgate.v"|' $(DE2_PROC)/tiger.v
	./$(DE4_PROC)/clean
	echo "others = $(MODEL_HOME)/modelsim.ini" > $(DE4_PROC)/modelsim.ini.in
	#specifying paths to altera libraries in tiger.v
	sed -i 's|^`include\s".*/altera_mf.v"|`include "$(PWDIR)/altera_libs/altera_mf.v"|' $(DE4_PROC)/tiger.v
	sed -i 's|^`include\s".*/220model.v"|`include "$(PWDIR)/altera_libs/220model.v"|' $(DE4_PROC)/tiger.v
	sed -i 's|^`include\s".*/sgate.v"|`include "$(PWDIR)/altera_libs/sgate.v"|' $(DE4_PROC)/tiger.v
	sed -i 's|^`include\s".*/altera_primitives.v"|`include "$(PWDIR)/altera_libs/altera_primitives.v"|' $(DE4_PROC)/tiger.v
	sed -i 's|^`include\s".*/stratixiv_atoms.v"|`include "$(PWDIR)/altera_libs/stratixiv_atoms.v"|' $(DE4_PROC)/tiger.v
	sed -i 's|^`include\s".*/stratixiii_atoms.v"|`include "$(PWDIR)/altera_libs/stratixiii_atoms.v"|' $(DE4_PROC)/tiger.v

clean:
	./$(DE2_PROC)/clean
	rm -rf $(DE2_PROC)/modelsim.ini.in
	./$(DE4_PROC)/clean
	rm -rf $(DE4_PROC)/modelsim.ini.in
