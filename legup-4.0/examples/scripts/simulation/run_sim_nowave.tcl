quit -sim

if {[file exists work]} {
	vdel -lib work -all;
}

if {![file exists work]} {
	vlib work;
}

vlog *.v
vlog ./submodules/*.v +incdir+./submodules
vlog -sv ./submodules/verbosity_pkg.sv +incdir+./submodules
vlog -sv ./submodules/*.sv +incdir+./submodules
vsim legup_system_tb

run 7000000000000000ns

