
source ../config.tcl

# we need to ignore the memory dependency for the prev_prev_row shift register:
# 		window[0][2] = prev_prev_row[prev_prev_row_index];
#		prev_prev_row[prev_prev_row_index] = prev_row_elem;
# We know that the prev_prev_row_index will change for the next iteration so there
# is no loop carried dependency here
loop_pipeline "loop" -ignore-mem-deps 

set_parameter LOCAL_RAMS 1
