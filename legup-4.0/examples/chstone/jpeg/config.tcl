source ../config.tcl

# temporary: some getelmentptr instructions have
# negative offsets which are not handled properly
# by the OR gate
set_parameter GROUP_RAMS_SIMPLE_OFFSET 0
