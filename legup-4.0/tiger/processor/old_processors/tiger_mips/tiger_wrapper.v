
module tiger_wrapper (
	input  CLOCK_27,
	output [17:0] LEDR,
	output [8:0] LEDG,
	output [6:0] HEX0,
	output [6:0] HEX1,
	output [6:0] HEX2,
	output [6:0] HEX3,
	output [6:0] HEX4,
	output [6:0] HEX5,
	output [6:0] HEX6,
	output [6:0] HEX7,
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
	
	// SDRAM clock must lead CPU clock by 3ns
	pll25MHz pll25(.inclk0(CLOCK_27), .c0(CLOCK_25), .c1(DRAM_CLK));

	tiger tiger_sopc (
		// general
		.clk(CLOCK_25),
		.reset_n(1'b1),
		
		// LEDs
		.out_port_from_the_GreenLED(LEDG[8:0]),
		.out_port_from_the_RedLED(LEDR[17:0]),
		
		// Hex LEDs
		.HEX0_from_the_HexLED_0(HEX0),
		.HEX1_from_the_HexLED_0(HEX1),
		.HEX2_from_the_HexLED_0(HEX2),
		.HEX3_from_the_HexLED_0(HEX3),
		.HEX4_from_the_HexLED_0(HEX4),
		.HEX5_from_the_HexLED_0(HEX5),
		.HEX6_from_the_HexLED_0(HEX6),
		.HEX7_from_the_HexLED_0(HEX7),
		
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