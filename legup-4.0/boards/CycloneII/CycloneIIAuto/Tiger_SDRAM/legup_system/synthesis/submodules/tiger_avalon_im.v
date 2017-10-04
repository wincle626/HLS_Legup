//This module contains the state machine to control processor for accessing components over Avalon (non-memory related)
module tiger_avalon_im (
	input clk,
	input reset,
	
	input [31:0] memaddress,
	input memread,
	input memwrite,
	input [31:0] memwritedata,
	input mem8,
	input mem16,
	output avalon_stall,

	output [31:0]avm_procMaster_address,
	output avm_procMaster_read,
	output avm_procMaster_write,
	output [31:0] avm_procMaster_writedata,	
	output [3:0] avm_procMaster_byteenable,
	input [31:0]avm_procMaster_readdata,
	input avm_procMaster_waitrequest,
	input avm_procMaster_readdatavalid
);

	reg outstanding_read;

	assign avalon_stall = ~avm_procMaster_readdatavalid;
//	assign avalon_stall = avm_procMaster_waitrequest | (outstanding_read & ~avm_procMaster_readdatavalid);

	assign avm_procMaster_address		= {memaddress[31:2], 2'h0};
//	assign avm_procMaster_address		= (outstanding_read) ? outstanding_address : memaddress;
	assign avm_procMaster_read			= memread & (~outstanding_read | avm_procMaster_readdatavalid);
	assign avm_procMaster_write		= memwrite;
	assign avm_procMaster_writedata	= memwritedata;
	assign avm_procMaster_byteenable = 4'hF;

	always @(posedge clk)
	begin
		if (reset)
			outstanding_read <= 1'b0;
		else if (avm_procMaster_readdatavalid & (~memread | avm_procMaster_waitrequest))
			outstanding_read <= 1'b0;
		else if (memread & ~avm_procMaster_waitrequest)
			outstanding_read <= 1'b1;
	end

endmodule
