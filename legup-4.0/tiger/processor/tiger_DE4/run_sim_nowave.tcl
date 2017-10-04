quit -sim;

if {[file exists work]} {
	vdel -lib work -all;
}

if {![file exists work]} {
	vlib work;
}

vlog *.v;
vlog multiported_caches/MP_4ports/*.v;
vlog multiported_caches/LVT_4ports/*.v;

vsim test_bench;

run 7000000000000000ns 
