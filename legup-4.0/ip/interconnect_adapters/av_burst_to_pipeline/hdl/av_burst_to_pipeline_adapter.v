
module av_burst_to_pipeline_adapter (
	input 				clk,
	input 				reset,

	input					avs_read,
	input					avs_write,
	input 	[31: 0]	avs_address,
	input 	[31: 0]	avs_writedata,
	input 	[ 3: 0]	avs_byteenable,
	output	[31: 0]	avs_readdata,
	input					avs_beginbursttransfer,
	input	 	[ 2: 0]	avs_burstcount,	
	output				avs_waitrequest,
	output				avs_readdatavalid,

	output				avm_read,
	output				avm_write,
	output	[31: 0]	avm_address,
	output	[31: 0]	avm_writedata,
	output	[ 3: 0]	avm_byteenable,
	input		[31: 0]	avm_readdata,
	output				avm_lock,
//	output	[ 2: 0]	avm_burstcount,
	input					avm_waitrequest,
	input					avm_readdatavalid
);

reg				handling_burst;
reg				pending_burst;
reg	[ 2:0]	burst_length;
reg	[ 2:0]	next_burst_length;
reg	[ 2:0]	burst_count;
reg	[31:0]	burst_address;

// To Slave
assign avs_readdata 			= avm_readdata;
assign avs_waitrequest		= (avs_read | avs_write) & ~handling_burst ? avm_waitrequest : 1'b1;
assign avs_readdatavalid	= avm_readdatavalid;

// To Master
assign avm_read				= (handling_burst) ? 1'b1 : avs_read; 
assign avm_write				= (handling_burst) ? 1'b0 : avs_write;
assign avm_address			= (handling_burst) ? burst_address : avs_address;
assign avm_writedata			= avs_writedata;
assign avm_byteenable		= (handling_burst) ? 4'hF : avs_byteenable;
//assign avm_lock				= 1'b0; // (handling_burst) ? 1'b1 : avs_read | avs_write;
assign avm_lock				= (handling_burst) ? 1'b1 : avs_read;

always @(posedge clk)
begin
	if (reset)
		handling_burst <= 1'b0;
//	else if (~avs_waitrequest & (pending_burst | avs_beginbursttransfer))
	else if (~avs_waitrequest & (pending_burst | (avs_beginbursttransfer & avs_read)))
		handling_burst <= 1'b1;
	else if ((burst_length == burst_count) & ~avm_waitrequest)
		handling_burst <= 1'b0;
end

always @(posedge clk)
begin
	if (reset)
		pending_burst <= 1'b0;
	else if (~avs_waitrequest)
		pending_burst <= 1'b0;
//	else if ((avs_read | avs_write) & avs_beginbursttransfer)
	else if (avs_read & avs_beginbursttransfer)
		pending_burst <= 1'b1;
end

always @(posedge clk)
begin
	if (reset)
		burst_length <= 3'h0;
	else if (~avs_waitrequest & avs_beginbursttransfer)
		burst_length <= avs_burstcount;
	else if (~avs_waitrequest & pending_burst)
		burst_length <= next_burst_length;
end

always @(posedge clk)
begin
	if (reset)
		next_burst_length <= 3'h0;
	else if ((avs_read | avs_write) & avs_beginbursttransfer)
		next_burst_length <= avs_burstcount;
end

always @(posedge clk)
begin
	if (reset)
		burst_count <= 3'h0;
	else if (~avs_waitrequest & pending_burst)
		burst_count <= 3'h2;
	else if (~avs_waitrequest & avs_beginbursttransfer)
		burst_count <= 3'h2;
	else if (handling_burst & ~avm_waitrequest)
		burst_count <= burst_count + 3'h1;
end

always @(posedge clk)
begin
	if (reset)
		burst_address <= 32'h00000000;
	else if (~avs_waitrequest & pending_burst)
		burst_address <= avs_address + 32'h00000004;
	else if (~avs_waitrequest & avs_beginbursttransfer)
		burst_address <= avs_address + 32'h00000004;
	else if (handling_burst & ~avm_waitrequest)
		burst_address <= burst_address + 32'h00000004;
end

endmodule

