# legup.tcl - LegUp configuration file for test suite
#
# These variables can be overridden by environment variables using the naming
# convention:
#
#       LEGUP_($VARIABLE_NAME)
#
# ie. to turn on pattern sharing:
#
#       export LEGUP_ENABLE_PATTERN_SHARING=1
#
# See Makefile.config for more examples
#

# set_project <family> <board> <project>
# Set the FPGA family, the board and the project. This includes the operation 
# characterization tcl file for the specified FPGA family, the board specific
# settings and the system parameters tcl file for the specified project
set CURRENT_PATH    [file dirname [info script]]
set BOARDS_PATH     $::CURRENT_PATH/../boards
proc set_project { family board project } {

    # Get characterization settings
    # Among other settings, the characterization file sets the 
    # parameter CLOCK_PERIOD, such as the following
    # set_parameter CLOCK_PERIOD 13
    # the clock period constraint in ns.
    set characterization_file $::BOARDS_PATH/$family/$board/$board\.tcl
    #puts stderr "Board's characterization file: $characterization_file\n";
    if { [file exists $characterization_file] == 0 } {
        puts stderr "Unrecognized Board ($family/$board)!\n";
    }
    source $characterization_file

    # Get project specific settings
    set project_info_file $::BOARDS_PATH/$family/$board/$project/legup\.tcl
    #puts stderr "Project info file: $project_info_file\n";
    if { [file exists $project_info_file] == 0 } {
        puts stderr "Unrecognized Project ($family/$board/$project)!\n";
    }
    source $project_info_file

    set_legup_config_board $board
    set ::BOARD $board

    set_parameter SYSTEM_PROJECT_NAME $project
}

# Set the default target project
# To change the project from a local config.tcl use:
#       source ../legup.tcl
#       set_project CycloneV DE1-SoC Tiger_SDRAM
# Changing the project also updates all associated family and board parameters
set_project CycloneV DE1-SoC Tiger_SDRAM
#set_project CycloneV DE1-SoC ARM_Simple_Hybrid_System
#set_project CycloneV SoCKit ARM_Simple_Hybrid_System
#set_project CycloneIV DE2-115 Tiger_SDRAM
#set_project CycloneII DE2 Tiger_SDRAM
#set_project CycloneII CycloneIIAuto Tiger_SDRAM
#set_project StratixV DE5-Net Tiger_DDR3
#set_project StratixIV DE4-230 Tiger_DDR2
#set_project StratixIV DE4-530 Tiger_DDR2

# Set the output path for all the LegUp generated files
# Only subdirectories of the current path are allowed
# The makefile will create the full path, while the clean target will remove it
set_legup_output_path output

# set to ensure that muxing will be placed on multipliers if max DSPs
# are exceeded (as opposed to performing multiplication with logic)
set_parameter RESTRICT_TO_MAXDSP 0

# Enable multi-pumping of multipliers that use DSPs
set_parameter MULTIPUMPING 0

# Override DSP infer estimation algorithm - assume multiply-by-constant
# always infers DSPs
set_parameter MULT_BY_CONST_INFER_DSP 0

# use local block rams for every array and remove global memory controller
# WARNING: only works with simple pointers
#set_parameter LOCAL_RAMS 1

# group all arrays in the memory controller into four RAMs (one for each
# bitwidth: 8, 16, 32, 64)
#set_parameter GROUP_RAMS 1

# when GROUP_RAMS is on, this option simplifies the address offset calculation.
# Calculate the offset for each array into the shared RAM to minimize addition.
# The offset must be a multiple of the size of the array in bytes (to allow an OR
# instead of an ADD): 
#       before: addr = baseaddr + offset
#       after:  addr = baseaddr OR offset
# the idea is that none of the lower bits of baseaddr should overlap with any
# bits of offset. This improves area and fmax (less adders) but at the cost of
# wasted empty memory inside the shared RAM
#set_parameter GROUP_RAMS_SIMPLE_OFFSET 1

# Use verilog to infer RAMs instead of instantiating a altsyncram module.
# Note: inferred RAMs don't support structs (no byte-enable) so having
# structs in your program forces this parameter to turn off
set_parameter INFERRED_RAMS 1

# Place constant arrays into RAMs instead of the default read-only memory
#set_parameter NO_ROMS 1

# Explicitly instantiate all multipliers as lpm_mult modules
#set_parameter EXPLICIT_LPM_MULTS 1

# Don't chain multipliers
#set_parameter MULTIPLIER_NO_CHAIN 1

################################################################################
# BEGIN PATTERN SHARING PARAMETERS
################################################################################

# Pattern sharing is an optimization which reduces circuit area by sharing 
# functional units and registers. Specifically it improves the logic density of 
# the design by combining many, under-utilized LUTs (e.g. 3-4 LUTs) into fewer, 
# larger LUTs (5-6 LUTs) and in the process reducing the total number of 
# registers.
#
# While pattern sharing reduces circuit area, experiments have shown that it
# can impact fmax positively but also (more commonly) negatively. While some 
# of this may be attributed to seed noise, the algorithm combines functional 
# units by adding logic (multiplexers) to the design so depending on the 
# circuit the critical path can become longer. On the other hand, improving 
# logic density and reducing the circuit size allows the design to be packed 
# more closely and this can reduce routing delays. 
 

# SET TO 0 TO DISABLE PATTERN SHARING
# (setting to 0 shares only dividers and remainders, as in LegUp 1.0)
# set_parameter ENABLE_PATTERN_SHARING 1


# The values of the remaining pattern sharing parameters below are ignored if 
# ENABLE_PATTERN_SHARING above is 0.


# Maximum pattern size to share. Setting to 0 will also disable pattern sharing.
set_parameter PS_MAX_SIZE 10

# Minimum pattern size to share. This is used because sharing is more beneficial
# for larger patterns (larger patterns have a smaller mux:instruction ratio) and
# sometimes sharing is only beneficial for patterns of a certain size or 
# greater. For example, in Cyclone II sharing small patterns (e.g. 2 adds) does 
# not reduce area.
set_parameter PS_MIN_SIZE 1

# Select which instructions to share in pattern sharing. Choices are:
#
#  -  Adders / Subtractors
#  -  Bitwise operations (AND, OR, XOR)
#  -  Shifts (logical shift Left/Right and arithmetic shift Right)
#
# If set, these instructions will be included in patterns and shared with 2-1
# muxing. Note that multipliers, dividers and remainders are not shared in 
# patterns because they should be shared with more than 2-1 muxing (if at all). 
# The bipartite binding algorithm is used for those instructions while pattern 
# sharing is used for the smaller instructions above.
set_parameter PATTERN_SHARE_SHIFT 1
if { [get_device_family] == "StratixIV" } {
    # These aren't worth sharing in CycloneII (4-LUT architectures)
    set_parameter PATTERN_SHARE_ADD 1
    set_parameter PATTERN_SHARE_SUB 1
    set_parameter PATTERN_SHARE_BITOPS 1
}

# Two operations will only be shared if the difference of their true bit widths
# is below this threshold: e.g. an 8-bit adder will not be shared with 
# a 32-bit adder unless BIT_DIFF_THRESHOLD >= 24
set_parameter PS_BIT_DIFF_THRESHOLD 10
set_parameter PS_BIT_DIFF_THRESHOLD_PREDS 30

# The minimum bit width of an instruction to consider
# (e.g. don't bother sharing 1 bit adders)
set_parameter PS_MIN_WIDTH 2

# write patterns to dot file
#set_parameter PS_WRITE_TO_DOT 1

# write patterns to verilog file
#set_parameter PS_WRITE_TO_VERILOG 1

################################################################################
# END PATTERN SHARING PARAMETERS
################################################################################

# MinimizeBitwidth parameters
#set to 1 to print bitwidth minimization stats
#set_parameter MB_PRINT_STATS 1
#set to filename from which to read initial data ranges.  If it's
#undefined, then no initial ranges are assumed
#set_parameter MB_RANGE_FILE "range.profile"
#max number of backward passes to execute (-1 for infinite)
set_parameter MB_MAX_BACK_PASSES -1
set_parameter MB_MINIMIZE_HW 0 


# Minimum pattern frequency written to dot/v file
#set_parameter FREQ_THRESHOLD 1

# disable register sharing based on live variable analysis
#set_parameter DISABLE_REG_SHARING 1

#
# Scheduling Variables
#


# Disable chaining of operations in a clock cycle. This will achieve the
# maximum amount of pipelining. 
# Note: this overrides CLOCK_PERIOD
#set_parameter SDC_NO_CHAINING 1

# Perform as-late-as-possible (ALAP) scheduling instead of as-soon-as-possible
# (ASAP).
#set_parameter SDC_ALAP 1

# Cause debugging information to be printed from the scheduler.
#set_parameter SDC_DEBUG 1

# Show some high level debug info from modulo scheduler. Like
# the final II and how many backtracking attempts
#set_parameter MODULO_DEBUG 1

# Push more multipliers into the same state for multi-pumping
#set_parameter SDC_MULTIPUMP 1

#
# Debugging
#

# turn off generating data flow graph dot files for every basic block
# these include the dfg.*.dot files and pipelineDFG.*.dot files
#set_parameter NO_DFG_DOT_FILES 1

# show dummy calls (like printf) in the dataflow dot graphs
#set_parameter DFG_SHOW_DUMMY_CALLS 1

# prepend every printf with the number of cycle elapsed
#set_parameter PRINTF_CYCLES 1

# print all signals to the verilog file even if they don't drive outputs
#set_parameter KEEP_SIGNALS_WITH_NO_FANOUT 1

# display cur_state on each cycle for each function
#set_parameter PRINT_STATES 1

# display the cycle time when each function starts and finishes
#set_parameter PRINT_FUNCTION_START_FINISH 1

# print statistics on the number of instructions and basic blocks in each loop/function
#set_parameter PRINT_BB_STATS 1

# turn off getelementptr instructions chaining
#set_parameter DONT_CHAIN_GET_ELEM_PTR 0

# include instruction labels in the state node labels in the FSM dot graph
#set_parameter INCLUDE_INST_IN_FSM_DOT_GRAPH 1

# Cycle the memory waitrequest signal: 5 cycles high, 1 cycle low
# This should have no impact on circuit functionality but just make the total
# cycle count about 5x higher
#set_parameter TEST_WAITREQUEST 1

# Disable assertions in Verilog output used to debug LegUp
#set_parameter VSIM_NO_ASSERT 1

# SDC resource constraints
set_parameter SDC_RES_CONSTRAINTS 1

# set scheduling resource constraints
# the resource name should match the device family operation database files:
#       boards/StratixIV/StratixIV.tcl
#       boards/CycloneII/CycloneII.tcl
# LegUp calculates the constraint on a functional unit by
# trying to find the most specific constraint (longest string match) that
# applies. For instance in the tcl file:
#      set_resource_constraint signed_divide_16 3
#      set_resource_constraint signed_divide 2
#      set_resource_constraint divide 1
# this is what will be returned:
#      get_resource_constraint(signed_divide_8)     = 2
#      get_resource_constraint(signed_divide_16)    = 3
#      get_resource_constraint(signed_divide_32)    = 2
#      get_resource_constraint(signed_divide_64)    = 2
#      get_resource_constraint(unsigned_divide_8)   = 1
#      get_resource_constraint(unsigned_divide_16)  = 1
#      get_resource_constraint(unsigned_divide_32)  = 1
#      get_resource_constraint(unsigned_divide_64)  = 1
# This also applies for memories. For instance, for the local memory 'shared'
# the functional unit name would be:
#      shared_mem_dual_port
# So the following tcl constraint would apply (if there was nothing more
# specific):
#      set_resource_constraint mem_dual_port 2
# Note: SDC resource constraints must be on for this to work
set_resource_constraint mem_dual_port 2
set_resource_constraint divide 1
set_resource_constraint modulus 1
#set_resource_constraint multiply 2
set_resource_constraint altfp_add 1
set_resource_constraint altfp_subtract 1
set_resource_constraint altfp_multiply 1
set_resource_constraint altfp_divide 1
# catchall for all other FP operations:
set_resource_constraint altfp 1


# specify the operation latency
# name should match the device family operation database file:
# boards/StratixIV/StratixIV.tcl or boards/CycloneII/CycloneII.tcl 
set_operation_latency altfp_add 14
set_operation_latency altfp_subtract 14
set_operation_latency altfp_multiply 11
set_operation_latency altfp_divide_32 33
set_operation_latency altfp_divide_64 61
set_operation_latency altfp_truncate_64 3
set_operation_latency altfp_extend_32 2
set_operation_latency altfp_fptosi 6
set_operation_latency altfp_sitofp 6
set_operation_latency signed_comp_o 1
set_operation_latency signed_comp_u 1
set_operation_latency reg 2

set_operation_latency mem_dual_port 2
set_operation_latency local_mem_dual_port 1

set_operation_latency multiply 1

# turn off operation sharing for hardware
# operations that are cheaper than multiplexing
# by default operation sharing is ON
# first argument is either -off or -on
# there is a special case for multipliers, see: RESTRICT_TO_MAXDSP 
# but set_operation_sharing overrides that parameter
# note: a constraint on "signed_add" will apply to: 
#   signed_add_8, signed_add_16, signed_add_32, signed_add_64,
#   unsigned_add_8, unsigned_add_16, unsigned_add_32, unsigned_add_64
set_operation_sharing -off signed_add 
set_operation_sharing -off signed_subtract
set_operation_sharing -off bitwise
set_operation_sharing -off shift
set_operation_sharing -off comp

# enable dual port binding
set_parameter DUAL_PORT_BINDING 1

# create load/store dependencies based on LLVM alias analysis
set_parameter ALIAS_ANALYSIS 1

################################################################################
# BEGIN MULTI-CYCLE PARAMETERS
################################################################################

# Set this to remove registers from data paths and instead print
# multi-cycle constraints
set_parameter MULTI_CYCLE_REMOVE_REG 0

# Print debugging statements
set_parameter MULTI_CYCLE_DEBUG 0

# Multi-cycle dividers. Todo: rather than fully multi-cycle, look at modifying
# the initiation interval (e.g. II of 2 rather than 1).
# Can do same with loop pipelining.
set_parameter MULTI_CYCLE_REMOVE_REG_DIVIDERS 0

# Duplicate load registers
set_parameter MULTI_CYCLE_DUPLICATE_LOAD_REG 0

# Remove registers from cmp instructions
set_parameter MULTI_CYCLE_REMOVE_CMP_REG 1

# Add -through constraints, which can improve some circuits
set_parameter MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS 0

# Also since .sdc constraints will be printed for certain registers, use the
# option below to additionally print .qsf constraints that will disable 
# merging for these registers. This seems like a bug in Quartus though, 
# because registers with timing constraints shouldn't be merged.
set_parameter MULTI_CYCLE_DISABLE_REG_MERGING 1


# Set LLVM_PROFILE to 1 in order to use profiling information in the scheduler. 
# Make sure to also set LLVM_PROFILE=1 in Makefile.config, which is required
# to gather the profiling data
set_parameter LLVM_PROFILE 0

# If LLVM_PROFILE is set, then schedules will be modified for all Basic Blocks 
# executed at or below the frequency threshold LLVM_PROFILE_MAX_BB_FREQ_TO_ALTER 
# Note that scheduling changes may also be affected by basic block length 
# (total #states) in addition to frequency.
set_parameter LLVM_PROFILE_MAX_BB_FREQ_TO_ALTER 100

# Below are the different scheduling changes supported for these infrequently
# executed basic blocks

# 1. Extend paths by a fixed number of cycles
# This change extends paths in infrequent BB by a fixed number of states.
# Note it only works if paths are multi-cycled, so MULTI_CYCLE_REMOVE_REG
# above must be set.
set_parameter LLVM_PROFILE_EXTRA_CYCLES 0

# 2. Adjust the target period for the basic block
# As an alternative to LLVM_PROFILE_EXTRA_CYCLES which simply extends paths
# in infrequent basic blocks by a fixed amount, LLVM_PROFILE_PERIOD_<DEVICE>
# can be used to change the target period for paths in infrequent blocks
# (e.g. in Cyclone II a 13ns constraint might be best, but infrequent blocks
# can use a 6ns constraint instead)
set_parameter LLVM_PROFILE_PERIOD_CII 13
set_parameter LLVM_PROFILE_PERIOD_SIV 6

################################################################################
# END MULTI-CYCLE PARAMETERS
################################################################################

# pipeline a loop:
#       loop_pipeline "loop_label" [ -ii <num> ] [-ignore-mem-deps]
# optional arguments:
#   -ii <num>
#       force a specific pipeline initiation interval
#   -ignore-mem-deps
#       ignore loop carried dependencies for all memory accesses in the loop
#loop_pipeline "loop1"
#loop_pipeline "loop2" -ii 1
#loop_pipeline "loop3" -ignore-mem-deps

# getelementptr and store instructions take an entire clock cycle
# turn off ignoring getelementptr and store instructions in timing report 
# set_parameter 	TIMING_NO_IGNORE_GETELEMENTPTR_AND_STORE 1

# number of paths to be printed in the timing report
set_parameter TIMING_NUM_PATHS 10

# turn off all loop pipelining
#set_parameter NO_LOOP_PIPELINING 1

# Specify the type of modulo scheduler to use for loop pipelining. There are
# three options:
#   SDC_BACKTRACKING
#       Default. Used SDC modulo scheduling with a backtracking mechanism to
#       resolve conflicting resource and recurrence constraints
#   SDC_GREEDY
#       SDC modulo scheduling using a greedy approach to resolving resource
#       constraints. This method may not achieve minimum II II for loops with
#       resource constraints and cross-iteration dependencies.
#   ITERATIVE
#       Classic iterative modulo scheduler approach using a list scheduler (no
#       operator chaining support)
set_parameter MODULO_SCHEDULER SDC_BACKTRACKING
#set_parameter MODULO_SCHEDULER SDC_GREEDY
#set_parameter MODULO_SCHEDULER ITERATIVE

# Solve the SDC problem incrementally by detecting negative cycle in
# the constraint graph. This is marginally faster than the LP solver
#set_parameter INCREMENTAL_SDC 1

# In greedy SDC modulo scheduling the order of scheduling instructions uses a
# priority function based on perturbation. Perturbation is calculated for each
# instruction by:
#    1) adding a GE constraint to the SDC formulation for that
#       instruction
#    2) resolving the SDC schedule
#    3) counting how many many other instructions are displaced from
#       their prior schedule
# Instruction with higher perturbation are scheduled with higher
# priority.  If this is off, instructions are scheduled randomly
set_parameter SDC_PRIORITY 1

# the budget ratio is a measure of backtracking effort in the SDC
# modulo scheduler. A low ratio will give up backtracking early, saving
# runtime but may not achieve the minimum II
set_parameter SDC_BACKTRACKING_BUDGET_RATIO 6

# The same as SDC_PRIORITY but for backtracking SDC modulo
# scheduling.  A priority function isn't strictly necessary due to
# backtracking but a decent scheduling order can really speed up
# scheduling by reducing the amount of backtracking. Without this on
# you might have to increase the
# SDC_BACKTRACKING_BUDGET_RATIO parameter to allow more
# backtracking.
set_parameter SDC_BACKTRACKING_PRIORITY 1

# By default during modulo scheduling we allow chaining to occur
# anywhere. This parameter turns off chaining for any operation not on
# a loop recurrence for maximum pipelining while still not impacting
# II. This parameter only works when SDC_BACKTRACKING is on
#set_parameter SDC_ONLY_CHAIN_CRITICAL 1

# restructure the loop body expression tree to reduce recurrence cycles that
# limit initiation interval when loop pipelining
#set_parameter RESTRUCTURE_LOOP_RECURRENCES 1

# turn on resource sharing inside a loop pipeline
set_parameter PIPELINE_RESOURCE_SHARING 1

# this option saves registers in the loop pipeline by only using
# registers on the pipeline stage boundaries. Instead of using a
# register for every single pipeline time step
set_parameter PIPELINE_SAVE_REG 1

# try to pipeline every loop in the program (if loop is supported by LegUp)
#set_parameter PIPELINE_ALL 1

# Print all the SDC constraints during modulo scheduling into
# the pipelining.legup.rpt file
#set_parameter DEBUG_SDC_CONSTRAINTS 1

# Parameters to control the Verilog generation for the FSM.  The first,
# CASE_FSM controls whether a case statement is used 
# instead of if statements.  
# The second parameter indicates whether casex(..) should be 
# used instead of case(...) This parameter is meaningless unless
# first parameter is 1.
set_parameter CASE_FSM 1
set_parameter CASEX 0

# Inspect HLS Debugger Configurations

#creates full debugging information for Inspect Debugger
set_parameter INSPECT_DEBUG 0
#creates partial debugging information for Inspect Debugger. This parameter
#should be set only when Inspect ON_CHIP_BUG_DETECT mode is required.
set_parameter INSPECT_ONCHIP_BUG_DETECT_DEBUG 0

set_parameter DEBUG_DB_HOST localhost
set_parameter DEBUG_DB_USER root
set_parameter DEBUG_DB_PASSWORD letmein

set_parameter DEBUG_DB_NAME legupDebug
set_parameter DEBUG_DB_SCRIPT_FILE $::CURRENT_PATH/createDebugDB.sql

set_parameter INSPECT_DEBUG_DB_NAME inspect_db
set_parameter INSPECT_DEBUG_DB_SCRIPT_FILE $::CURRENT_PATH/inspect_db.sql

# Divider module type
# WARNING: multicycling and bitwidth minimization have only been tested with
# altera dividers
# Turn this on to use generic dividers:
#set_parameter DIVIDER_MODULE "generic"
# Turn this on to use Altera-specific lpm_divide units:
set_parameter DIVIDER_MODULE "altera"

# turn on pass to combine basic blocks
# 1 to enable merging of all patterns
# 2 to enable only within loops
#set_combine_basicblock 1
