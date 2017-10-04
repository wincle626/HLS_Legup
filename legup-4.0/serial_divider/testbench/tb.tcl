quit -sim
vlib work
set VERILOG_LIB_DIR "../../ip/libs/altera/"
set VERILOG_LIBS "$VERILOG_LIB_DIR/220model.v"
vlog $VERILOG_LIBS ../*.v *.v
vsim work.div_testbench 
do w.do
run 5000000
