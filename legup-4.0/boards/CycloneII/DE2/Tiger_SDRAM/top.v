module DE2 (
	//Simple
	input  			CLOCK_50,
	input 	[0:0] 	KEY,

	//UART
	input  			UART_RXD,
	output 			UART_TXD,

	//Memory (SDRAM)
	output 	[11:0] 	DRAM_ADDR,
	output 			DRAM_BA_0,
	output 			DRAM_BA_1,
	output 			DRAM_LDQM,
	output 			DRAM_UDQM,
	output 			DRAM_RAS_N,
	output 			DRAM_CAS_N,
	output 			DRAM_CKE,
	output			DRAM_WE_N,
	output 			DRAM_CS_N,
	output 			DRAM_CLK,
	inout  	[15:0] 	DRAM_DQ
);

	wire CLOCK_50i;
	
	// 50 MHz pll giving out -3ns phase shift for SDRAM clock
//	pll50MHz pll50(.inclk0(CLOCK_50), .c0(CLOCK_50i), .c1(DRAM_CLK));

	legup_system legup_system_qsys(
		// clock and reset
		.clk_clk(CLOCK_50),                        //                        clk.clk
		.reset_reset_n(KEY[0]),                     //                      reset.reset_n
		
		// SDRAM
		.sdram_wire_addr(DRAM_ADDR),                //                 sdram_wire.addr
		.sdram_wire_ba({DRAM_BA_1, DRAM_BA_0}),                   //                           .ba
		.sdram_wire_cas_n(DRAM_CAS_N),              //                           .cas_n
		.sdram_wire_cke(DRAM_CKE),                  //                           .cke
		.sdram_wire_cs_n(DRAM_CS_N),                //                           .cs_n
		.sdram_wire_dq(DRAM_DQ),                    //                           .dq
		.sdram_wire_dqm({DRAM_UDQM,DRAM_LDQM}),     //                           .dqm
		.sdram_wire_ras_n(DRAM_RAS_N),               //                           .ras_n
		.sdram_wire_we_n(DRAM_WE_N),                //                           .we_n
		
		// UART
		.uart_wire_rxd(UART_RXD),  // uart_0_external_connection.rxd
		.uart_wire_txd(UART_TXD)   //                           .txd
	);

endmodule
