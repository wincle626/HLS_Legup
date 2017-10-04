`include "./tiger_icache_parameters.v"

module tiger_icache_av_1port (
	input						clk,
	input						reset_n,

	input			[29: 0]	avs_icache_slave_address,
	input						avs_icache_slave_read,
	output		[31: 0]	avs_icache_slave_readdata,
	output					avs_icache_slave_readdatavalid,
	output					avs_icache_slave_waitrequest,

	input		[`SDRAM_WIDTH-1:0]		avm_icache_master_readdata,
	input						avm_icache_master_readdatavalid,
	input						avm_icache_master_waitrequest,
	output		[31: 0]	avm_icache_master_address,
	output					avm_icache_master_beginbursttransfer,
	output	[`IBURSTCOUNTWIDTH-1:0]	avm_icache_master_burstcount,
	output					avm_icache_master_read
);

reg						outstandingRead;
wire						readDataValid;
wire						stall;

assign avs_icache_slave_readdatavalid	= readDataValid & outstandingRead;
assign avs_icache_slave_waitrequest		= stall | (outstandingRead & ~avs_icache_slave_readdatavalid) | ~reset_n;

always @(posedge clk)
begin
	if (~reset_n)
		outstandingRead	<= 1'b0;
	else if (avs_icache_slave_read)
		outstandingRead	<= 1'b1;
	else if (avs_icache_slave_readdatavalid)
		outstandingRead	<= 1'b0;
end

tiger_icache tiger_icache_inst (
	.clk											(clk),
	.reset_n										(reset_n),

	//CPU side memory control signals
	.PROC0_memRead								(avs_icache_slave_read | outstandingRead),
	.PROC0_memAddress							({avs_icache_slave_address, 2'h0}),
	.PROC0_memReadData						(avs_icache_slave_readdata),

	.PROC0_flush								(1'b0),

	.PROC0_canFlush							(),

	//True if the data on memReadData is valid (i.e. data we've just read from the cache)
	.PROC0_readDataValid						(readDataValid),

	//CPU pipeline stall
	.PROC0_stall								(stall),

	//Avalon Bus side signals
	.avm_dataMaster0_read					(avm_icache_master_read),
	.avm_dataMaster0_address				(avm_icache_master_address),
	.avm_dataMaster0_beginbursttransfer	(avm_icache_master_beginbursttransfer),
	.avm_dataMaster0_burstcount			(avm_icache_master_burstcount),
	.avm_dataMaster0_readdata				(avm_icache_master_readdata),
	.avm_dataMaster0_waitrequest			(avm_icache_master_waitrequest & avm_icache_master_read),
	.avm_dataMaster0_readdatavalid		(avm_icache_master_readdatavalid)
);

endmodule

