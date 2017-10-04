/*
  This work by Simon Moore and Gregory Chadwick is licenced under the
  Creative Commons Attribution-Non-Commercial-Share Alike 2.0
  UK: England & Wales License.

  To view a copy of this licence, visit:
     http://creativecommons.org/licenses/by-nc-sa/2.0/uk/
  or send a letter to:
     Creative Commons,
     171 Second Street,
     Suite 300,
     San Francisco,
     California 94105,
     USA.
*/

module tiger_top (
	input clk,
	input reset,

	// Instruction Master
	output	 	[31: 0]	avm_instrMaster_address,
	output	 				avm_instrMaster_read,
	output	 				avm_instrMaster_write,
	output	 	[31: 0]	avm_instrMaster_writedata,	
	output	 	[ 3: 0]	avm_instrMaster_byteenable,
	input			[31: 0]	avm_instrMaster_readdata,
	input						avm_instrMaster_waitrequest,
	input						avm_instrMaster_readdatavalid,

	// Data Master
	output 		[31: 0]	avm_dataMaster_address,
	output 					avm_dataMaster_read,
	output 					avm_dataMaster_write,
	output 		[31: 0]	avm_dataMaster_writedata,	
	output 		[ 3: 0]	avm_dataMaster_byteenable,
	input			[31: 0]	avm_dataMaster_readdata,
	input						avm_dataMaster_waitrequest,
	input						avm_dataMaster_readdatavalid
);

    parameter RESET_ADDRESS = 32'h00000000;


	reg						initial_instrMaster_stall;
	reg			[31: 0]	last_instrMaster_readdata;	
	reg			[31: 0]	last_dataMaster_readdata;

	wire						im_avalon_stall;
	wire			[31: 0]	pc;

	wire						dm_avalon_stall;
	wire						memwrite;
	wire						memread;
	wire						mem16;
	wire						mem8;
	wire			[31: 0]	memaddress;
	wire			[31: 0]	memwritedata;
	wire			[31: 0]	memreaddata;

	always @(posedge clk)
	begin
		if (reset)
			initial_instrMaster_stall <= 1'b1;
		else
			initial_instrMaster_stall <= 1'b0;
	end

	always @(posedge clk)
	begin
		if (reset)
		begin
			last_instrMaster_readdata <= 0;
			last_dataMaster_readdata <= 0;
		end
		else
		begin
			last_instrMaster_readdata <= avm_instrMaster_readdata;
			last_dataMaster_readdata <= avm_dataMaster_readdata;
		end
	end

//	assign memreaddata =	(avm_dataMaster_byteenable == 4'hF) ?	avm_dataMaster_readdata :
//								(avm_dataMaster_byteenable == 4'hC) ?	{16'h0000, avm_dataMaster_readdata[31:16]} :
//								(avm_dataMaster_byteenable == 4'h3) ?	{16'h0000, avm_dataMaster_readdata[15: 0]} :
//								(avm_dataMaster_byteenable == 4'h8) ?	{23'h000000, avm_dataMaster_readdata[31:24]} :
//								(avm_dataMaster_byteenable == 4'h4) ?	{23'h000000, avm_dataMaster_readdata[23:16]} :
//								(avm_dataMaster_byteenable == 4'h2) ?	{23'h000000, avm_dataMaster_readdata[15: 8]} :
//																					{23'h000000, avm_dataMaster_readdata[ 7: 0]};


	assign memreaddata =	
							(avm_dataMaster_readdatavalid) ?
								(avm_dataMaster_byteenable == 4'hF) ?	avm_dataMaster_readdata :
								(avm_dataMaster_byteenable == 4'hC) ?	{16'h0000, avm_dataMaster_readdata[31:16]} :
								(avm_dataMaster_byteenable == 4'h3) ?	{16'h0000, avm_dataMaster_readdata[15: 0]} :
								(avm_dataMaster_byteenable == 4'h8) ?	{23'h000000, avm_dataMaster_readdata[31:24]} :
								(avm_dataMaster_byteenable == 4'h4) ?	{23'h000000, avm_dataMaster_readdata[23:16]} :
								(avm_dataMaster_byteenable == 4'h2) ?	{23'h000000, avm_dataMaster_readdata[15: 8]} :
																					{23'h000000, avm_dataMaster_readdata[ 7: 0]}
							:
								(avm_dataMaster_byteenable == 4'hF) ?	last_dataMaster_readdata :
								(avm_dataMaster_byteenable == 4'hC) ?	{16'h0000, last_dataMaster_readdata[31:16]} :
								(avm_dataMaster_byteenable == 4'h3) ?	{16'h0000, last_dataMaster_readdata[15: 0]} :
								(avm_dataMaster_byteenable == 4'h8) ?	{23'h000000, last_dataMaster_readdata[31:24]} :
								(avm_dataMaster_byteenable == 4'h4) ?	{23'h000000, last_dataMaster_readdata[23:16]} :
								(avm_dataMaster_byteenable == 4'h2) ?	{23'h000000, last_dataMaster_readdata[15: 8]} :
																					{23'h000000, last_dataMaster_readdata[ 7: 0]};

	tiger_tiger #(
        .RESET_ADDRESS  (RESET_ADDRESS)
    ) core (
		.clk					(clk),
		.reset				(reset),
		.stall_cpu			(1'b0),

		// Instruction Master
		.iStall				(im_avalon_stall | initial_instrMaster_stall | reset),
		.pc					(pc),		// program counter
//		.instrF				(last_instrMaster_readdata),	// fetched instruction
		.instrF				(avm_instrMaster_readdata),	// fetched instruction
		.canICacheFlush	(1'b1),
		.iCacheFlush		(),

		// Data Master
		.dStall				(dm_avalon_stall),
		.memwrite			(memwrite),
		.memread				(memread),
		.mem16				(mem16),
		.mem8					(mem8), 
		// memory access mode outputs
		.memaddress			(memaddress),
		.memwritedata		(memwritedata),			// memory address and data outputs
		.memreaddata		(memreaddata),
		.memCanRead			(~(avm_dataMaster_waitrequest & (avm_dataMaster_read | avm_dataMaster_write))),
		.memCanWrite		(~(avm_dataMaster_waitrequest & (avm_dataMaster_read | avm_dataMaster_write))),
		.canDCacheFlush	(1'b1),
		.dCacheFlush		(),
		.memzerofill		(),
		
		// IRQ
		.irq					(0),
		.irqNumber			(0)
	);

	tiger_avalon_im im_avalon_controller (
		.clk									(clk),
		.reset								(reset),
	
		.memaddress							(pc),
		.memread								(1'b1),
		.memwrite							(1'b0),
		.memwritedata						(32'h00000000),
		.mem8									(1'b0),
		.mem16								(1'b0),
		.avalon_stall						(im_avalon_stall),

		.avm_procMaster_address			(avm_instrMaster_address),
		.avm_procMaster_read				(avm_instrMaster_read),
		.avm_procMaster_write			(avm_instrMaster_write),
		.avm_procMaster_writedata		(avm_instrMaster_writedata),	
		.avm_procMaster_byteenable		(avm_instrMaster_byteenable),
		.avm_procMaster_readdata		(avm_instrMaster_readdata),
		.avm_procMaster_waitrequest	(avm_instrMaster_waitrequest),
		.avm_procMaster_readdatavalid	(avm_instrMaster_readdatavalid)
	);
   
	tiger_avalon dm_avalon_controller (
		.clk									(clk),
		.reset								(reset),
	
		.memaddress							(memaddress),
		.memread								(memread),
		.memwrite							(memwrite),
		.memwritedata						(memwritedata),
		.mem8									(mem8),
		.mem16								(mem16),
		.avalon_stall						(dm_avalon_stall),

		.avm_procMaster_address			(avm_dataMaster_address),
		.avm_procMaster_read				(avm_dataMaster_read),
		.avm_procMaster_write			(avm_dataMaster_write),
		.avm_procMaster_writedata		(avm_dataMaster_writedata),	
		.avm_procMaster_byteenable		(avm_dataMaster_byteenable),
		.avm_procMaster_readdata		(avm_dataMaster_readdata),
		.avm_procMaster_waitrequest	(avm_dataMaster_waitrequest),
		.avm_procMaster_readdatavalid	(avm_dataMaster_readdatavalid)
	);

	// BF: Temporarily put this here until move to it's on component.
	// Taken from debug_tiger_leap.v
`define STARTINGPC 				32'h0080_0020
//`define FINISHPC 					32'h0080_0010

	reg start;
	reg done;
	reg [63:0] count;

	initial start = 1'b0;
	initial done = 1'b0;
	initial count = 64'd0;
	
	always @(posedge clk) begin
		//checking if PC goes undefined, if so quit simulation
		if (^pc === 1'bX) begin
			$display("PC is going undefined!\n");
			$finish;
		end

		//counting execution cycles 
		if (pc == `STARTINGPC) begin
			start <= 1'b1;
		end
		if (start) begin
			count <= count + 1;
		end
		if (pc == (RESET_ADDRESS + 32'h0000000C)) begin
			if (start & ~done) begin
				done <= 1'b1;
				$display("counter = %d", count);
				`ifndef PROFILER_ON
					$finish;
				`endif
			end  
		end
	end
   
endmodule

