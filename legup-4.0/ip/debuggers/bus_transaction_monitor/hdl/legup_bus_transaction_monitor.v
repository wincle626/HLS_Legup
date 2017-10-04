
module legup_bus_transaction_monitor #(
	parameter DW = 31, 
	parameter AW = 31, 
	parameter BE = 3,

	parameter NUM_READS					= 1000000,
	parameter NUM_WRITES					= 1000000,

	parameter CAPTURE_TRANSACTIONS	= 1,

	parameter READ_ADDRESS_FILENAME	= "read_addresses.dat",
	parameter READ_DATA_FILENAME		= "read_data.dat",
	parameter WRITE_ADDRESS_FILENAME	= "write_addresses.dat",
	parameter WRITE_DATA_FILENAME		= "write_data.dat"
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

reg		[31: 0]	read_address_file_handle;
reg		[31: 0]	read_data_file_handle;
reg		[31: 0]	write_address_file_handle;
reg		[31: 0]	write_data_file_handle;

reg		[31: 0]	bus_address_reads;
reg		[31: 0]	bus_data_reads;
reg		[31: 0]	bus_reads_addresses	[NUM_READS:0];
reg		[DW: 0]	bus_reads_data			[NUM_READS:0];
reg		[31: 0]	bus_writes;
reg		[31: 0]	bus_writes_addresses	[NUM_WRITES:0];
//reg		[31: 0]	bus_writes_addresses	[13:0];
reg		[DW: 0]	bus_writes_data 		[NUM_WRITES:0];

assign avs_slave_readdata				= avm_master_readdata;
assign avs_slave_readdatavalid		= avm_master_readdatavalid;
assign avs_slave_waitrequest			= avm_master_waitrequest;

assign avm_master_address				= avs_slave_address;
assign avm_master_byteenable			= avs_slave_byteenable;
assign avm_master_lock					= avs_slave_lock;
assign avm_master_read					= avs_slave_read;
assign avm_master_write					= avs_slave_write;
assign avm_master_writedata			= avs_slave_writedata;


initial
begin
	if (CAPTURE_TRANSACTIONS == 1)
	begin
		read_address_file_handle	= $fopen (READ_ADDRESS_FILENAME);
		read_data_file_handle		= $fopen (READ_DATA_FILENAME);
		write_address_file_handle	= $fopen (WRITE_ADDRESS_FILENAME);
		write_data_file_handle		= $fopen (WRITE_DATA_FILENAME);
	end
	else
	begin
		$readmemh(READ_ADDRESS_FILENAME,		bus_reads_addresses);
		$readmemh(READ_DATA_FILENAME,			bus_reads_data);
		$readmemh(WRITE_ADDRESS_FILENAME,	bus_writes_addresses);
		$readmemh(WRITE_DATA_FILENAME,		bus_writes_data);
	end

	bus_address_reads	<= 32'd0;
	bus_data_reads		<= 32'd0;
	bus_writes			<= 32'd0;
end
always @(posedge clk)
begin
	if (CAPTURE_TRANSACTIONS == 1)
	begin
		if (avs_slave_read & ~avm_master_waitrequest)
		begin
			$fwrite (read_address_file_handle, "%x\n", avs_slave_address);
			$fflush (read_address_file_handle);
			bus_address_reads <= bus_address_reads + 1;
		end
		if (avm_master_readdatavalid)
		begin
			$fwrite (read_data_file_handle,    "%x\n", avm_master_readdata);
			$fflush (read_data_file_handle);
			bus_data_reads <= bus_data_reads + 1;
		end

		if (avs_slave_write & ~avm_master_waitrequest)
		begin
			$fwrite (write_address_file_handle, "%x\n", avs_slave_address);
			$fwrite (write_data_file_handle,    "%x\n", avs_slave_writedata);
			$fflush (write_address_file_handle);
			$fflush (write_data_file_handle);
			bus_writes <= bus_writes + 1;
		end
	end
	else
	begin
		if (avs_slave_read & ~avm_master_waitrequest)
		begin
			if (bus_reads_addresses[bus_address_reads] != avs_slave_address) 
			begin
				$display ("Read %d has an address error !\n", bus_address_reads + 1);
				$display ("Expected transaction address: %08x", bus_reads_addresses[bus_address_reads]);
				$display ("Actual transaction address:   %08x\n", avs_slave_address);
				$stop;
			end
			bus_address_reads <= bus_address_reads + 1;
		end
		if (avm_master_readdatavalid)
		begin
			if (bus_reads_data[bus_data_reads] != avm_master_readdata) 
			begin
				$display ("Read %d has an data error !\n", bus_data_reads + 1);
				$display ("Expected transaction data:    %08x", bus_reads_data[bus_data_reads]);
				$display ("Actual transaction data:      %08x", avm_master_readdata);
				$stop;
			end
			bus_data_reads <= bus_data_reads + 1;
		end
		if (avs_slave_write & ~avm_master_waitrequest)
		begin
			if ((bus_writes_addresses[bus_writes] != avs_slave_address) || 
					(avs_slave_byteenable[0] & (bus_writes_data[bus_writes][ 7: 0] != avs_slave_writedata[ 7: 0])) ||
					(avs_slave_byteenable[1] & (bus_writes_data[bus_writes][15: 8] != avs_slave_writedata[15: 8])) ||
					(avs_slave_byteenable[2] & (bus_writes_data[bus_writes][23:16] != avs_slave_writedata[23:16])) ||
					(avs_slave_byteenable[3] & (bus_writes_data[bus_writes][31:24] != avs_slave_writedata[31:24]))) 
			begin
				$display ("Write %d has an error!\n", bus_writes + 1);
				$display ("Expected transaction address: %08x", bus_writes_addresses[bus_writes]);
				$display ("Actual transaction address:   %08x", avs_slave_address);
				$display ("Expected transaction data:    %08x", bus_writes_data[bus_writes]);
				$display ("Actual transaction data:      %08x", avs_slave_writedata);
				$display ("With byteenable:              %08x\n", avs_slave_byteenable);
				$stop;
			end
			bus_writes <= bus_writes + 1;
		end
	end
end


endmodule

