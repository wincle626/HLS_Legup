quit -sim;
vsim -novopt +acc=rn test_bench 
do t_wave.do;

run 700000ns