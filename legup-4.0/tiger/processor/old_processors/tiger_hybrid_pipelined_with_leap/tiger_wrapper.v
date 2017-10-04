
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
	output UART_TXD,

	input  [0:0] KEY,
	output [1:0] LEDG,
	
	input   [2:0] SW,
	output [17:0] LEDR
);
	wire CLOCK_50i;
	
	// 50 MHz pll giving out -3ns phase shift for SDRAM clock
	pll50MHz pll50(.inclk0(CLOCK_50), .c0(CLOCK_50i), .c1(DRAM_CLK));

	tiger tiger_sopc (
		// general
		.clk(CLOCK_50i),
		.reset_n(KEY[0]),
		
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
		.txd_from_the_uart_0(UART_TXD),
		  
		// Program Execution Status Lights
        .coe_exe_end_from_the_tiger_top_0      (LEDG[0]),
        .coe_exe_start_from_the_tiger_top_0    (LEDG[1]),

		// Lights for Debug purpose
        .coe_debug_lights_from_the_tiger_top_0 (LEDR[17:0]),
        .coe_debug_select_to_the_tiger_top_0   (SW[2:0])
	);
	
endmodule 	
