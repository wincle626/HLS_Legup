quit -sim
if {[file exists work]} {
	vdel -lib work -all;
}
if {![file exists work]} {
	vlib work;
}
vlog ../*.v
vlog *.v
vsim work.testbench -Lf 220model_ver -Lf altera_mf_ver -Lf verilog
do w.do
run 10000000
