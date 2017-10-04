`include "./tiger_defines.v"

module tiger_leap_slave_handler # (
	parameter N2 = 8
	) (
	input clk,
	input reset,
	// Avalon-MM Slave Interface
	input [N2-1:0] avs_leapSlave_address,
	input          avs_leapSlave_read,
	input          avs_leapSlave_write,
	input   [31:0] avs_leapSlave_writedata,
	output  [31:0] avs_leapSlave_readdata,
	
	// Leap MM Interface
	output [N2-1:0] lp_mm_addr,
	output          lp_mm_wren,
	output [31  :0] lp_mm_wrdata,
	input  [31  :0] lp_mm_rddata,
	
	// Status Bit of Program Execution (from Leap|AddressStack)
	input exe_start_as,
	input exe_end_as,
	
	// Soft Reset to Tiger
	output reg tiger_soft_reset
);
	 /***********************************************************************************************
	 *                                slave_address  slave_write
	 *       Soft Reset -  reset_arr - 8'b1100_0000 -   1  ---------------> Self clear
	 *               AH -         V1 - 8'b0000_0000 -   1   --------|
	 *               AH -   A1A2B1B2 - 8'b0000_0001 -   1   --------|
	 *               AH -        tab - 8'b01tt_tttt -   1   --------|
	 * Increment Option -   inc_mask - 8'b1000_0000 -   1  ---------|==> LEAP
	 * Do Hierarchical? -    do_hier - 8'b1000_0001 -   1  ---------|
	 *     Starting PC  -    init_pc - 8'b1000_0010 -   1  ---------|
	 *               CB -    counter - 8'bcccc_cccc -   0  ---------|
	 *
	 * !!! PROTOCOL:
	 *       | exe_start_as | exe_end_as |   readdata    |
	 *       |       0      |     0      | 32'hffff_fffe |  program execution is NOT YET STARTED from profiler's view ( several cycles latency from tiger_top )
	 *       |       1      |     0      | 32'hffff_ffff |  program execution is STARTED but not yet FINISHED from profiler's view
	 *       |       1      |     1      | lp_mm_rddata  |  program execution is FINISHED. readdata contains the value of the reading counter.
	 *
	 ***********************************************************************************************/

//+++++++++++++++++++++++++++++
// Leap MM Interface
//+++++++++++++++++++++++++++++
	//assign avs_leapSlave_readdata = lp_mm_rddata;
	assign avs_leapSlave_readdata = exe_start_as ? (exe_end_as ? lp_mm_rddata : 32'hffff_ffff) : 32'hffff_fffe;
	
	assign lp_mm_addr   = avs_leapSlave_address;
	assign lp_mm_wren   = avs_leapSlave_write;
	assign lp_mm_wrdata = avs_leapSlave_writedata;
	
//+++++++++++++++++++++++++++++
// Soft Reset to Tiger
//+++++++++++++++++++++++++++++
	always @ (posedge clk) begin
		if (reset)
			tiger_soft_reset <= 1'b1;
		else if ( avs_leapSlave_write & ( &avs_leapSlave_address[N2-1 -: 2] & ~(|avs_leapSlave_address[N2-3:0]) ) )  //8'b1100_0000
			tiger_soft_reset <= avs_leapSlave_writedata[0];
	end
	
//+++++++++++++++++++++++++++++
// Force Slave Interface
//	- For Simulation
//+++++++++++++++++++++++++++++

// synthesis translate off
	parameter SIMULATE_WITHOUT_JTAG_MASTER = 1'b0;

	parameter ACTUAL_FUNC_NUM  = 16;
	// Hash parameters
	parameter tab = 128'h0001000007090b0c0004050e00080d0d;
	parameter V1 = 32'h447499e7;
	parameter A1 = 8'd5;
	parameter A2 = 8'd28;
	parameter B1 = 8'd8;
	parameter B2 = 8'hf;
	parameter CNT_INC_OPTION = 32'b00001;
	parameter DO_HIER_OPTION = 32'b0;
	parameter STARTING_PC    = `STARTINGPC;

	integer i, log;
	initial begin
		if ( SIMULATE_WITHOUT_JTAG_MASTER ) begin
			force avs_leapSlave_write      = 1'b0;
			force avs_leapSlave_address    = {(N2){1'b0}};
			force avs_leapSlave_writedata  = 32'b0;

			for (i=0;i<100;i=i+1) @ (posedge clk);

			@ (posedge clk) begin		// Reset Tiger
				$display ("************************\n*** Resetting Tiger at %x\n************************", $time);
				force avs_leapSlave_address   = {2'b11, {(N2-2){1'b0}} };
				force avs_leapSlave_writedata = 32'h1;
				force avs_leapSlave_write      = 1'b1;
			end
		//++++++++++  initialize profiler  ++++++++++
			$display ("************************\n*** Forcing Tiger_Leap_Slave Behavior \n************************");
			@ (posedge clk) begin		// write V1
				force avs_leapSlave_address    = {(N2){1'b0}};
				force avs_leapSlave_writedata  = V1;
			end
			@ (posedge clk) begin		// write A1,A2,B1,B2
				force avs_leapSlave_address   = {{(N2-1){1'b0}}, 1'b1};
				force avs_leapSlave_writedata = {A1,A2,B1,B2};
			end
			for (i=-1 ; i< ACTUAL_FUNC_NUM/4; i=i+1) begin	// write tab
				@ (posedge clk) begin
					force avs_leapSlave_address   = {2'b01,{(N2-2){1'b0}}} + i;
					force avs_leapSlave_writedata = tab[32*(i+1)-1 -: 32];
				end
			end
			@ (posedge clk) begin		// Setup Counter Increment Option
				force avs_leapSlave_address   = {1'b1, {(N2-1){1'b0}} };
				force avs_leapSlave_writedata = CNT_INC_OPTION;
			end
			@ (posedge clk) begin		// Setup Hierarchical Profiling Option
				force avs_leapSlave_address   = {1'b1, {(N2-2){1'b0}},1'b1};
				force avs_leapSlave_writedata = DO_HIER_OPTION;
			end
			@ (posedge clk) begin		// Tell profiler which instruction calls <main>
				force avs_leapSlave_address   = {1'b1, {(N2-3){1'b0}},2'b10};  
				force avs_leapSlave_writedata = STARTING_PC;
			end
			@ (posedge clk) begin		// Release Reset of Tiger
				$display ("************************\n*** Releasing reset of Tiger at %x\n************************", $time);
				force avs_leapSlave_address   = {2'b11, {(N2-2){1'b0}} };
				force avs_leapSlave_writedata = 32'h0;
			end
			@ (posedge clk) force avs_leapSlave_write = 1'b0;
		
		//++++++++++  monitor execution status  ++++++++++
			@ (posedge exe_start_as)
				$display ("************************\n*** Profiling Started.\n************************");
			@ (posedge exe_end_as);
			@ (posedge clk)
				$display ("************************\n*** Profiling Finished.\n************************");
		//++++++++++  retrieve counter values  ++++++++++
			log = $fopen ("profiling_result.log", "w");
			$fwrite (log, "############################\n####  Profiling Result  ####\n############################\n");
			for (i=-1 ; i<= ACTUAL_FUNC_NUM+1; i=i+1) begin
				@ (posedge clk) begin
					force avs_leapSlave_address = i;
				end
				if (i>=1)  $fwrite (log, "FuncNum-%2h  Data-%8d\n", i-1, avs_leapSlave_readdata);
			end
			$fclose(log);

			$stop();
		end
	end
// synthesis translate on
endmodule
