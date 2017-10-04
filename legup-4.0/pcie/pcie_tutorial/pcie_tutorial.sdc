# Clock constraints
create_clock -period "20.000 ns" -name {CLOCK_50} {CLOCK_50}

# Automatically constrain PLL and other generated clocks
derive_pll_clocks -create_base_clocks
