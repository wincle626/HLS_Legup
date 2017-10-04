module VGA_with_RAM (
		input sys_clk,
		input VGA_REF_CLOCK_50,
		input reset,
		// pixel ram write port
		input	[ 7:0]  pr_data,
		input	[19:0]  pr_wraddress,
		input	 pr_wren,
		// output to vga adaptor
		output wire        VGA_CLK,
		output wire        VGA_HS,
		output wire        VGA_VS,
		output wire        VGA_BLANK_N,
		output wire        VGA_SYNC_N,
		output wire [7:0]  VGA_R,
		output wire [7:0]  VGA_G,
		output wire [7:0]  VGA_B
		);
	
	wire pr_q;
	
	wire [31:0] pixel_dma_master_address;
	wire        pixel_dma_master_lock;
	wire        pixel_dma_master_read;
	reg         pixel_dma_master_waitrequest = 1'b0;
	reg         pixel_dma_master_readdatavalid;
	wire  [15:0] pixel_dma_master_readdata = (pr_q==1'b0) ? 16'b0 : 16'hFFFF;
	
	reg read_reg;	// the pixel_RAM takes two cycles to read
	always @ (posedge sys_clk) begin
		read_reg <= pixel_dma_master_read;
		pixel_dma_master_readdatavalid <= read_reg;
	end
	
	pixel_RAM PR (
		.data  (pr_data[0]),
		.rdaddress  (pixel_dma_master_address [20:1]),
		.rdclock  (sys_clk),
		.wraddress  (pr_wraddress),
		.wrclock  (sys_clk),
		.wren (pr_wren),
		.q (pr_q)
	);

	VGA_Subsystem vga_subsystem (
		.sys_clk_clk                        (sys_clk),
		.sys_reset_reset_n                  (~reset),
		.vga_CLK                            (VGA_CLK),
		.vga_HS                             (VGA_HS),
		.vga_VS                             (VGA_VS),
		.vga_BLANK                          (VGA_BLANK_N),
		.vga_SYNC                           (VGA_SYNC_N),
		.vga_R                              (VGA_R),
		.vga_G                              (VGA_G),
		.vga_B                              (VGA_B),
		.pixel_dma_master_readdatavalid     (pixel_dma_master_readdatavalid),
		.pixel_dma_master_waitrequest       (pixel_dma_master_waitrequest),
		.pixel_dma_master_address           (pixel_dma_master_address),
		.pixel_dma_master_lock              (pixel_dma_master_lock),
		.pixel_dma_master_read              (pixel_dma_master_read),
		.pixel_dma_master_readdata          (pixel_dma_master_readdata),
		.pixel_dma_control_slave_address    (),
		.pixel_dma_control_slave_byteenable (),
		.pixel_dma_control_slave_read       (1'b0),
		.pixel_dma_control_slave_write      (1'b0),
		.pixel_dma_control_slave_writedata  (32'b0),
		.pixel_dma_control_slave_readdata   (),
		.vga_pll_ref_clk_clk				(VGA_REF_CLOCK_50),
		.vga_pll_ref_reset_reset			(reset)
//		.pixel_dma_control_slave_address    (mm_interconnect_1_vga_subsystem_pixel_dma_control_slave_address),
//		.pixel_dma_control_slave_byteenable (mm_interconnect_1_vga_subsystem_pixel_dma_control_slave_byteenable),
//		.pixel_dma_control_slave_read       (mm_interconnect_1_vga_subsystem_pixel_dma_control_slave_read),
//		.pixel_dma_control_slave_write      (mm_interconnect_1_vga_subsystem_pixel_dma_control_slave_write),
//		.pixel_dma_control_slave_writedata  (mm_interconnect_1_vga_subsystem_pixel_dma_control_slave_writedata),
//		.pixel_dma_control_slave_readdata   (mm_interconnect_1_vga_subsystem_pixel_dma_control_slave_readdata),
	);
	
endmodule
