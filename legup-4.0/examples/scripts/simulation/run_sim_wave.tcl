quit -sim

if {[file exists work]} {
	vdel -lib work -all;
}

if {![file exists work]} {
	vlib work;
}

vlog -novopt *.v
vlog -novopt ./submodules/*.v +incdir+./submodules
vlog -novopt -sv ./submodules/verbosity_pkg.sv +incdir+./submodules
vlog -novopt -sv ./submodules/*.sv +incdir+./submodules
vsim -novopt legup_system_tb

add wave -r /*

configure wave -signalnamewidth 2
configure wave -namecolwidth 300
configure wave -gridoffset 10000
property wave -radix hexadecimal *
update
WaveRestoreZoom {0 ps} {400 ns}

run 7000000000000000ns

