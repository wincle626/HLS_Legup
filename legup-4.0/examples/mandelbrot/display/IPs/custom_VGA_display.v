module custom_VGA_display (
		// standard LegUp accelerator interface
		input clk,
		input clk2x,
		input clk1x_follower,
		input reset,
		input start,
		output finish,
		// argument
		input [31:0] arg_i,
		input [31:0] arg_j,
		input [ 7:0] arg_color,

		// clock_50 for VGA pll
		input wire VGA_REF_CLOCK_50,
		// custom interface for VGA ports
		output wire        VGA_CLK,
		output wire        VGA_HS,
		output wire        VGA_VS,
		output wire        VGA_BLANK_N,
		output wire        VGA_SYNC_N,
		output wire [7:0]  VGA_R,
		output wire [7:0]  VGA_G,
		output wire [7:0]  VGA_B
		);

	wire	[ 7:0]  pr_data = arg_color;
	wire	[19:0]  pr_wraddress = { arg_i[9:0], arg_j[9:0] };
	reg  start_reg;
	always @ (posedge clk)
		start_reg <= start;
	wire	pr_wren = start; // & ~start_reg;

/* TESTING SECTION START */
/*
	reg	[7:0]  pr_data;
	reg	[19:0]  pr_wraddress;
	wire	pr_wren = 1;
	
	always @ (posedge clk) begin
		if (  reset | pr_wraddress == 1024*768 )
			pr_wraddress <= 20'b0;
		else
			pr_wraddress <= pr_wraddress + 20'b1;
	end
	
	always @ (*) begin
		if ( pr_wraddress[19:12] == 0 | 
		     pr_wraddress[19:12] == 191 |
			  pr_wraddress[ 9: 2] == 0	|
			  pr_wraddress[ 9: 2] == 255 )
			pr_data = 8'hFF;
		else
			pr_data = 8'b0;
	end
*/
/* TESTING SECTION END */

	// delay the finish signal for DC cycles
	parameter DC = 1;
	reg [31:0] cnt = 0;
	always @ (posedge clk)
		cnt <= (reset | pr_wren) ? 32'b0 : cnt + 32'b1;

	assign finish = (cnt == DC);
	
	VGA_with_RAM vwr (
		.sys_clk (clk),
		.VGA_REF_CLOCK_50 (VGA_REF_CLOCK_50),
		.reset (reset),
		// pixel ram write port
		.pr_data (pr_data),
		.pr_wraddress (pr_wraddress),
		.pr_wren (pr_wren),
		// output to vga adaptor
		.VGA_CLK (VGA_CLK),
		.VGA_HS (VGA_HS),
		.VGA_VS (VGA_VS),
		.VGA_BLANK_N (VGA_BLANK_N),
		.VGA_SYNC_N (VGA_SYNC_N),
		.VGA_R (VGA_R),
		.VGA_G (VGA_G),
		.VGA_B (VGA_B)
	);

endmodule
