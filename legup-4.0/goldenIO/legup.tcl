# legup.tcl - LegUp configuration file for test suite
#
# These variables can be overridden by environment variables using the naming
# convention:
#
#       LEGUP_($VARIABLE_NAME)
#
# ie. to turn off divider sharing:
#
#       export LEGUP_SHARE_DIV=0
#
# See Makefile.config for more examples
#

# if set, div/rem will be shared with any required mux width (as in Legup 1.0)
set_parameter SHARE_DIV 1
set_parameter SHARE_REM 1

# only turn on multiplier sharing with DSPs off
#set_parameter SHARE_MUL 1

# set to ensure that muxing will be placed on multipliers if max DSPs
# are exceeded (as opposed to performing multiplication with logic)
set_parameter RESTRICT_TO_MAXDSP 1

# Maximum chain size to consider. Setting to 0 uses Legup 1.0 original binding
# SET TO 0 TO DISABLE PATTERN SHARING
# (setting to 0 shares only dividers and remainders, as in LegUp 1.0)
set_parameter MAX_SIZE 10

# The settings below should all be nonzero, but can be disabled when debugging
# if set, these will be included in patterns and shared with 2-to-1 muxing
set_parameter SHARE_ADD 1
set_parameter SHARE_SUB 1
set_parameter SHARE_BITOPS 1
set_parameter SHARE_SHIFT 1

# Two operations will only be shared if the difference of their true bit widths
# is below this threshold: e.g. an 8-bit adder will not be shared with 
# a 32-bit adder unless BIT_DIFF_THRESHOLD >= 24
set_parameter BIT_DIFF_THRESHOLD 10
set_parameter BIT_DIFF_THRESHOLD_PREDS 30

# The minimum bit width of an instruction to consider
# (e.g. don't bother sharing 1 bit adders)
set_parameter MIN_WIDTH 2

# write patterns to dot file
#set_parameter WRITE_TO_DOT 1

# write patterns to verilog file
#set_parameter WRITE_TO_VERILOG 1

# Minimum pattern frequency written to dot/v file
#set_parameter FREQ_THRESHOLD 1

# disable register sharing based on live variable analysis
#set_parameter DISABLE_REG_SHARING 1

#
# Scheduling Variables
#

# Setting this environment variable to a particular integer value in ns will
# set the clock period constraint.
# WARNING: This gets overriden by the environment variable LEGUP_SDC_PERIOD in
# Makefile.config based on the target family. 
#set_parameter SDC_PERIOD 15

# Disable chaining of operations in a clock cycle. This will achieve the
# maximum amount of pipelining. 
# Note: this overrides SDC_PERIOD 
#set_parameter SDC_NO_CHAINING 1

# Perform as-late-as-possible (ALAP) scheduling instead of as-soon-as-possible
# (ASAP).
#set_parameter SDC_ALAP 1

# Cause debugging information to be printed from the scheduler.
#set_parameter SDC_DEBUG 1

# Disable SDC scheduling and use the original scheduling that was in the LegUp
# 1.0 release.
#set_parameter NO_SDC 1


