
module tiger_wrapper (
	input  CLOCK_50,
	output [11:0] DRAM_ADDR,
	output DRAM_BA_0,
	output DRAM_BA_1,
	output DRAM_LDQM,
	output DRAM_UDQM,
	output DRAM_RAS_N,
	output DRAM_CAS_N,
	output DRAM_CKE,
	output DRAM_WE_N,
	output DRAM_CS_N,
	output DRAM_CLK,
	inout  [15:0] DRAM_DQ,
	input  UART_RXD,
	output UART_TXD
);
	wire CLOCK_50i;
	//wire CLK_F;
	
	// SDRAM clock must lead CPU clock by 3ns
	//pll25MHz pll25(.inclk0(CLOCK_27), .c0(CLOCK_25), .c1(DRAM_CLK));

	
	// 50 MHz pll giving out -3ns phase shift for SDRAM clock
	pll50MHz pll50(.inclk0(CLOCK_50), .c0(CLOCK_50i), .c1(DRAM_CLK));

	tiger tiger_sopc (
		// general
		//.clk(CLOCK_25),
		.clk(CLOCK_50i),
		//.clk_50(CLOCK_50),
		
		.reset_n(1'b1),
		
		// SDRAM
		.zs_addr_from_the_sdram(DRAM_ADDR),
		.zs_ba_from_the_sdram({DRAM_BA_1, DRAM_BA_0}),
		.zs_cas_n_from_the_sdram(DRAM_CAS_N),
		.zs_cke_from_the_sdram(DRAM_CKE),
		.zs_cs_n_from_the_sdram(DRAM_CS_N),
		.zs_dq_to_and_from_the_sdram(DRAM_DQ),
		.zs_dqm_from_the_sdram({DRAM_UDQM, DRAM_LDQM}),
		.zs_ras_n_from_the_sdram(DRAM_RAS_N),
		.zs_we_n_from_the_sdram(DRAM_WE_N),
		
		// UART
		.rxd_to_the_uart_0(UART_RXD),
        .txd_from_the_uart_0(UART_TXD)
	);
	
endmodule 	
