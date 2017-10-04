module pureHW_VGA (
		input CLOCK_50,
		input CLOCK2_50,
		output wire        VGA_CLK,                         //               vga.CLK
		output wire        VGA_HS,                          //                  .HS
		output wire        VGA_VS,                          //                  .VS
		output wire        VGA_BLANK_N,                       //                  .BLANK
		output wire        VGA_SYNC_N,                        //                  .SYNC
		output wire [7:0]  VGA_R,                           //                  .R
		output wire [7:0]  VGA_G,                           //                  .G
		output wire [7:0]  VGA_B,                           //                  .B
		input [1:0] KEY
		);

	wire reset = ~KEY[0];
	wire start = ~KEY[1];

	wire pll_locked, sys_clk;
	system_clk_pll clk50to100 (
		.refclk (CLOCK_50),   //  refclk.clk
		.rst (reset),      //   reset.reset
		.outclk_0 (sys_clk), // outclk0.clk
		.locked (pll_locked)
	);
	
	top DUT
	(
		.clk (sys_clk),
		.VGA_REF_CLOCK_50 ( CLOCK2_50 ),
		.VGA_CLK     (VGA_CLK),
		.VGA_HS      (VGA_HS),
		.VGA_VS      (VGA_VS),
		.VGA_BLANK_N (VGA_BLANK_N),
		.VGA_SYNC_N   (VGA_SYNC_N),
		.VGA_R       (VGA_R),
		.VGA_G       (VGA_G),
		.VGA_B       (VGA_B),
		.reset (reset | ~pll_locked),
		.start (start),
		.finish (),
		.return_val ()
	);


endmodule
