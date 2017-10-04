quit -sim;

cp tiger_inst.v.copy tiger_inst.v

if {[file exists work]} {
	vdel -lib work -all;
}

if {![file exists work]} {
	vlib work;
}

vlog -novopt /home/legup/altera/13.0/quartus/eda/sim_lib/220model.v
vlog -novopt /home/legup/altera/13.0/quartus/eda/sim_lib/altera_mf.v
vlog -novopt /home/legup/altera/13.0/quartus/eda/sim_lib/sgate.v
vlog -novopt /home/legup/altera/13.0/quartus/eda/sim_lib/cycloneii_atoms.v

vlog -novopt *.v;
vlog -novopt ./tiger/testbench/tiger_tb/simulation/submodules/*.v;
vlog -novopt ./tiger/testbench/tiger_tb/simulation/submodules/verbosity_pkg.sv;
vlog -novopt ./tiger/testbench/tiger_tb/simulation/submodules/*.sv;
vlog -novopt ./tiger/testbench/tiger_tb/simulation/tiger_tb.v;

vsim -novopt tiger_tb;

do wave.do

run 7000000000000000ns 
