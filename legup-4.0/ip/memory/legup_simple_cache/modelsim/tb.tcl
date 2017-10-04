quit -sim

if {[file exists work]} {
	vdel -lib work -all;
}

if {![file exists work]} {
	vlib work;
}

vlog *.v
vlog ../hdl/*.v
# vsim work.cache_tb
vsim cache_tb
do w.do
run 1000000

