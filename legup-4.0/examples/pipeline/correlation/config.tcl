
source ../config.tcl

loop_pipeline "loop_1"
loop_pipeline "loop_2"
loop_pipeline "loop_3"
loop_pipeline "loop_4"
loop_pipeline "loop_5"
# isn't detected by LLVM as a loop:
#loop_pipeline "loop_6"
loop_pipeline "loop_7"
loop_pipeline "loop_8"
loop_pipeline "loop_9"



set_parameter LOCAL_RAMS 1
