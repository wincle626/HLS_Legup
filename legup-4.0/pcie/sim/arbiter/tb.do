vlib work
vlog *.v
vlog ../../pcie_tutorial/round_robin_arbiter.v
vsim -gN=10 work.arbiter_tb
run 300
