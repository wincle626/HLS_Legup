quit -sim;

cp tiger_inst.v.copy tiger_inst.v

if {[file exists work]} {
	vdel -lib work -all;
}

if {![file exists work]} {
	vlib work;
}

vlog -novopt *.v;
vlog multiported_caches/MP_4ports/*.v;
vlog multiported_caches/LVT_4ports/*.v;

vsim -novopt test_bench;
do wave.do

run 7000000000000000ns 
