// `include "tiger_dcache_parameters.v"
`define NUM_CACHE_PORTS 1
`define NUM_CACHE_WAYS 1

module tiger_dcache_av_1port (
	input						clk,
	input						reset_n,

	input			[30: 0]	avs_dcache_slave_0_address,
	input						avs_dcache_slave_0_begintransfer,
	input			[ 3: 0]	avs_dcache_slave_0_byteenable,
	input						avs_dcache_slave_0_read,
	input						avs_dcache_slave_0_write,
	input			[31: 0]	avs_dcache_slave_0_writedata,
	output		[31: 0]	avs_dcache_slave_0_readdata,
	output					avs_dcache_slave_0_readdatavalid,
	output					avs_dcache_slave_0_waitrequest,

	input			[31: 0]	avm_dcache_master_0_readdata,
	input						avm_dcache_master_0_readdatavalid,
	input						avm_dcache_master_0_waitrequest,
	output		[31: 0]	avm_dcache_master_0_address,
	output					avm_dcache_master_0_beginbursttransfer,
	output		[ 2: 0]	avm_dcache_master_0_burstcount,
	output		[ 3: 0]	avm_dcache_master_0_byteenable,
	output					avm_dcache_master_0_read,
	output					avm_dcache_master_0_write,
	output		[31: 0]	avm_dcache_master_0_writedata
);

reg						outstandingRead;
wire						waitrequest;
//wire						stall;
wire			[127:0]	writedata_0;
wire			[ 7: 0]	aso_proc_data;


assign avs_dcache_slave_0_readdatavalid	= ~avs_dcache_slave_0_waitrequest & outstandingRead & ~avs_dcache_slave_0_read;
assign avs_dcache_slave_0_waitrequest		= waitrequest | ~reset_n | aso_proc_data[4];
//assign writedata_0[31: 0]						= avs_dcache_slave_0_read ? {avs_dcache_slave_0_address[30:2], 2'h0} : avs_dcache_slave_0_address;
assign writedata_0[31: 0]						=  avs_dcache_slave_0_read ?		 				{avs_dcache_slave_0_address[30:2], 2'h0} :
															(avs_dcache_slave_0_byteenable == 4'hF) ? {avs_dcache_slave_0_address[30:2], 2'h0} :
															(avs_dcache_slave_0_byteenable == 4'h3) ? {avs_dcache_slave_0_address[30:2], 2'h0} :
															(avs_dcache_slave_0_byteenable == 4'hC) ? {avs_dcache_slave_0_address[30:2], 2'h2} :
															(avs_dcache_slave_0_byteenable == 4'h1) ? {avs_dcache_slave_0_address[30:2], 2'h0} :
															(avs_dcache_slave_0_byteenable == 4'h2) ? {avs_dcache_slave_0_address[30:2], 2'h1} :
															(avs_dcache_slave_0_byteenable == 4'h4) ? {avs_dcache_slave_0_address[30:2], 2'h2} :
																													{avs_dcache_slave_0_address[30:2], 2'h3};
//assign writedata_0[95:32]						= avs_dcache_slave_0_writedata;
assign writedata_0[95:32]						=  (avs_dcache_slave_0_byteenable == 4'hF) ? avs_dcache_slave_0_writedata :
															(avs_dcache_slave_0_byteenable == 4'h3) ? {2{avs_dcache_slave_0_writedata[15: 0]}} : 
															(avs_dcache_slave_0_byteenable == 4'hC) ? {2{avs_dcache_slave_0_writedata[31:16]}} : 
															(avs_dcache_slave_0_byteenable == 4'h1) ? {4{avs_dcache_slave_0_writedata[ 7: 0]}} : 
															(avs_dcache_slave_0_byteenable == 4'h2) ? {4{avs_dcache_slave_0_writedata[15: 8]}} : 
															(avs_dcache_slave_0_byteenable == 4'h4) ? {4{avs_dcache_slave_0_writedata[23:16]}} : 
																													{4{avs_dcache_slave_0_writedata[31:24]}};
assign writedata_0[96]							= avs_dcache_slave_0_write & ((avs_dcache_slave_0_byteenable == 4'h1) | (avs_dcache_slave_0_byteenable == 4'h2) | (avs_dcache_slave_0_byteenable == 4'h4) | (avs_dcache_slave_0_byteenable == 4'h8));
assign writedata_0[97]							= avs_dcache_slave_0_write & ((avs_dcache_slave_0_byteenable == 4'h3) | (avs_dcache_slave_0_byteenable == 4'hC));
assign writedata_0[98]							= 1'b0; //mem64 signal which doesn't exist for processor
assign writedata_0[99]							= 1'b0;
assign writedata_0[102:100]					= 0;
assign writedata_0[103]							= 1'b1;
assign writedata_0[127:104]					= 0;

always @(posedge clk)
begin
	if (~reset_n)
		outstandingRead	<= 1'b0;
	else if (avs_dcache_slave_0_read)
		outstandingRead	<= 1'b1;
	else if (~avs_dcache_slave_0_waitrequest)
		outstandingRead	<= 1'b0;
end

tiger_dcache tiger_dcache_inst (
	.csi_clockreset_clk						(clk),
	.csi_clockreset_reset_n					(reset_n),

	.aso_PROC_data								(aso_proc_data),

	.avs_CACHE0_begintransfer				(avs_dcache_slave_0_begintransfer),
	.avs_CACHE0_read							(avs_dcache_slave_0_read),
	.avs_CACHE0_write							(avs_dcache_slave_0_write),
	.avs_CACHE0_writedata					(writedata_0),
	.avs_CACHE0_readdata						(avs_dcache_slave_0_readdata),
	.avs_CACHE0_waitrequest					(waitrequest),

	.avm_dataMaster0_read					(avm_dcache_master_0_read),
	.avm_dataMaster0_write					(avm_dcache_master_0_write),
	.avm_dataMaster0_address				(avm_dcache_master_0_address),
	.avm_dataMaster0_writedata				(avm_dcache_master_0_writedata),
	.avm_dataMaster0_byteenable			(avm_dcache_master_0_byteenable),
	.avm_dataMaster0_readdata				(avm_dcache_master_0_readdata),
	.avm_dataMaster0_beginbursttransfer	(avm_dcache_master_0_beginbursttransfer),
	.avm_dataMaster0_burstcount			(avm_dcache_master_0_burstcount),	
	.avm_dataMaster0_waitrequest			(avm_dcache_master_0_waitrequest),
	.avm_dataMaster0_readdatavalid		(avm_dcache_master_0_readdatavalid)
);

endmodule

