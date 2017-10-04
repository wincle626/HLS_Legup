create_clock -period 1 -name clk [get_ports clk]
derive_pll_clocks
derive_clock_uncertainty
