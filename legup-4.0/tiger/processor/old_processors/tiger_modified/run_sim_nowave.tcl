quit -sim;

if {[file exists work]} {
	vdel -lib work -all;
}

if {![file exists work]} {
	vlib work;
}

vlog -novopt *.v;
vsim -novopt test_bench;

run 7000000000000000ns 
