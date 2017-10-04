set_parameter LOCAL_RAMS 1
#set_parameter GROUP_RAMS 1
#set_parameter GROUP_RAMS_SIMPLE_OFFSET 1
#set_parameter CASE_FSM 1

loop_pipeline "loop1"
loop_pipeline "loop2"
loop_pipeline "loop3"
loop_pipeline "loop4"
loop_pipeline "loop5"
loop_pipeline "loop6"

source ../config.tcl
