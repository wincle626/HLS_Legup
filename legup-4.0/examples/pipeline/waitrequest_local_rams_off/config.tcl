loop_pipeline "loop1"
loop_pipeline "loop2"
set_parameter PRINTF_CYCLES 1

# Cycle the memory waitrequest signal: 5 cycles high, 1 cycle low
# This should have no impact on circuit functionality but just make the total
# cycle count about 5x higher
set_parameter TEST_WAITREQUEST 1

