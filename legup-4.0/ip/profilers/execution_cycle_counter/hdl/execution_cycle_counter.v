
module execution_cycle_counter #(
	parameter DW = 31, 
	parameter AW = 31, 
	parameter BE = 3,

	parameter START_ADDRESS	= 32'h00800020,
	parameter END_ADDRESS	= 32'h00000010
) (
	input						clk,
	input						reset, 

	input			[AW: 0]	avs_slave_address,
	input			[BE: 0]	avs_slave_byteenable,
	input						avs_slave_lock,
	input						avs_slave_read,
	input						avs_slave_write,
	input			[DW: 0]	avs_slave_writedata,
	output		[DW: 0]	avs_slave_readdata,
	output					avs_slave_readdatavalid,
	output					avs_slave_waitrequest,
	
	input			[DW: 0]	avm_master_readdata,
	input						avm_master_readdatavalid,
	input						avm_master_waitrequest,
	output		[31: 0]	avm_master_address,
	output		[BE: 0]	avm_master_byteenable,
	output					avm_master_lock,
	output					avm_master_read,
	output					avm_master_write,
	output		[DW: 0]	avm_master_writedata
);

reg					run_counter;
reg		[63: 0]	cycle_count;

assign avs_slave_readdata				= avm_master_readdata;
assign avs_slave_readdatavalid		= avm_master_readdatavalid;
assign avs_slave_waitrequest			= avm_master_waitrequest;

assign avm_master_address				= avs_slave_address;
assign avm_master_byteenable			= avs_slave_byteenable;
assign avm_master_lock					= avs_slave_lock;
assign avm_master_read					= avs_slave_read;
assign avm_master_write					= avs_slave_write;
assign avm_master_writedata			= avs_slave_writedata;

initial run_counter = 1'b0;
initial cycle_count = 64'd0;
always @(posedge clk)
begin
	if ((avs_slave_address == START_ADDRESS) & avs_slave_read)
		run_counter <= 1'b1;

	if (run_counter)
		cycle_count <= cycle_count + 1;

	if ((avs_slave_address == END_ADDRESS) & avs_slave_read & run_counter)
	begin
		$display("counter = %d", cycle_count);
		$stop;
	end
end

endmodule

