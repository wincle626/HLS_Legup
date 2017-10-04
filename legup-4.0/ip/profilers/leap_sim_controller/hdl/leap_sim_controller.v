// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on


// enable profiling
// `define PROFILER_ON 1'b1

module leap_sim_controller # ( 
	// test bench parameters - to be written to profiler's registers	
	parameter CNT_INC_OPTION = 32'b00010,
	parameter DO_HIER_OPTION = 32'b1,
	parameter STARTING_PC    = 32'h0080_0020,

	parameter SIMULATE_WITHOUT_HOST_PC = 1'b1,

	parameter N2 = 8,
	parameter ACTUAL_FUNC_NUM = 64,
	// Hash parameters
	parameter tab = 512'h000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a291e1a1309,
	parameter V1 = 32'h62d6701e,
	parameter A1 = 8'd10,
	parameter A2 = 8'd26,
	parameter B1 = 8'd13,
	parameter B2 = 8'h7
) (
    input           clk,
    input           reset,

    input  [N2-1:0] avs_bridge_slave_address,
    input  [ 3: 0]  avs_bridge_slave_byteenable,
    input           avs_bridge_slave_read,
    input           avs_bridge_slave_write,
    input  [31: 0]  avs_bridge_slave_writedata,
    output [31: 0]  avs_bridge_slave_readdata,
    output          avs_bridge_slave_waitrequest,

    input  [31: 0]  avm_bridge_master_readdata,
    input           avm_bridge_master_waitrequest,
    output [31: 0]  avm_bridge_master_address,
    output [ 3: 0]  avm_bridge_master_byteenable,
    output          avm_bridge_master_read,
    output          avm_bridge_master_write,
    output [31: 0]  avm_bridge_master_writedata
);

//+++++++++++++++++++++++++++++
// Bridge the Slave and Master
// Interfaces
//+++++++++++++++++++++++++++++
assign avs_bridge_slave_readdata        = avm_bridge_master_readdata;
assign avs_bridge_slave_waitrequest     = avm_bridge_master_waitrequest;

assign avm_bridge_master_address        = {avs_bridge_slave_address, 2'h0};
assign avm_bridge_master_byteenable     = avs_bridge_slave_byteenable;
assign avm_bridge_master_read           = avs_bridge_slave_read;
assign avm_bridge_master_write          = avs_bridge_slave_write;
assign avm_bridge_master_writedata      = avs_bridge_slave_writedata;



//+++++++++++++++++++++++++++++
// Force Slave Interface
//+++++++++++++++++++++++++++++

// synthesis translate off
	integer i, log;

    initial begin
		force avm_bridge_master_read       = 1'b0;
		force avm_bridge_master_write      = 1'b0;
		force avm_bridge_master_address    = 32'b0;
		force avm_bridge_master_writedata  = 32'b0;

		for (i=0;i<100;i=i+1) @ (posedge clk);
		
		// Reset Tiger
		$display ("************************\n*** Resetting Tiger at %x\n************************", $time);
		force avm_bridge_master_address   = {2'b11, {(N2-2){1'b0}}, 2'b0};
		force avm_bridge_master_writedata = 32'h1;
		force avm_bridge_master_write      = 1'b1;
        // Wait until waitrequest go low.
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);



	`ifdef PROFILER_ON
		//++++++++++  initialize profiler  ++++++++++
		$display ("************************\n*** Initializing the Leap Profiler \n************************");
        // write V1
		force avm_bridge_master_address    = { {(N2){1'b0}}, 2'b0};
		force avm_bridge_master_writedata  = V1;
        // Wait until waitrequest go low.
        @(posedge clk);
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);

		// write A1,A2,B1,B2
		force avm_bridge_master_address   = {{(N2-1){1'b0}}, 1'b1, 2'b0};
		force avm_bridge_master_writedata = {A1,A2,B1,B2};
        // Wait until waitrequest go low.
        @(posedge clk);
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);

        for (i=0 ; i< ACTUAL_FUNC_NUM/4; i=i+1) begin	// write tab
			force avm_bridge_master_address   = {2'b01, {(N2-2){1'b0}}, 2'b0} + i*4;
			force avm_bridge_master_writedata = tab[32*(i+1)-1 -: 32];
            // Wait until waitrequest go low.
            @(posedge clk);
    		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);
		end

		// Setup Counter Increment Option
		force avm_bridge_master_address   = {1'b1, {(N2-1){1'b0}}, 2'b0};
		force avm_bridge_master_writedata = CNT_INC_OPTION;
        // Wait until waitrequest go low.
        @(posedge clk);
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);

		// Setup Hierarchical Profiling Option
		force avm_bridge_master_address   = {1'b1, {(N2-2){1'b0}}, 1'b1, 2'b0};
		force avm_bridge_master_writedata = DO_HIER_OPTION;
        // Wait until waitrequest go low.
        @(posedge clk);
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);

		// Tell profiler the starting pc
		force avm_bridge_master_address   = {1'b1, {(N2-3){1'b0}}, 2'b10, 2'b0};
		force avm_bridge_master_writedata = STARTING_PC;
        // Wait until waitrequest go low.
        @(posedge clk);
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);
	`endif

		$display ("************************\n*** Releasing reset of Tiger at %x\n************************", $time);
    	// Release Tiger from Reset
		force avm_bridge_master_address   = {2'b11, {(N2-2){1'b0}}, 2'b0};
		force avm_bridge_master_writedata = 32'h0;
        // Wait until waitrequest go low.
        @(posedge clk);
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);
		force avm_bridge_master_write = 1'b0;

        @(posedge clk);

	`ifdef PROFILER_ON
		//++++++++++  monitor execution status  ++++++++++
		force avm_bridge_master_read = 1'b1;
		force avm_bridge_master_address = 32'h0000_0000;

		@(posedge clk); // readdata is valid at this clock cycle
		while ( avm_bridge_master_readdata == 32'hffff_fffe) @(posedge clk);
        // Wait until waitrequest go low.
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);

        @(posedge clk);
		$display ("************************\n*** Profiling Started.\n************************");
		while ( avm_bridge_master_readdata == 32'hffff_ffff) @(posedge clk);
        // Wait until waitrequest go low.
		while ( avm_bridge_master_waitrequest == 1'b1) @(posedge clk);
		force avm_bridge_master_read = 1'b0;
		$display ("************************\n*** Profiling Finished.\n************************");

        // When a few clock cycles than get the 
        for (i=0;i<10;i=i+1) @ (posedge clk);

        //++++++++++  retrieve counter values  ++++++++++
		$display ("************************\n*** Retrieving Result.\n************************");
		log = $fopen ("profiling_result.log", "w");
		$fwrite (log, "############################\n####  Profiling Result  ####\n############################\n");

   		for (i=0 ; i< ACTUAL_FUNC_NUM; ) begin
			force avm_bridge_master_read = 1'b1;
			force avm_bridge_master_address = i*4;
			@ (posedge clk) begin
				if ( avm_bridge_master_waitrequest == 0) begin
					$fwrite (log, "FuncNum-%2h  Data-%8d\n", i, avm_bridge_master_readdata);
					i=i+1;
				end
			end
		end
		force avm_bridge_master_read = 1'b0;
		$fclose(log);

		@ (posedge clk) force avm_bridge_master_read = 1'b0;
		$stop();
	`endif
	end
// synthesis translate on

endmodule
