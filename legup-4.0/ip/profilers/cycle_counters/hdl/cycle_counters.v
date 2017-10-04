
module cycle_counters #(
	// Avalon interconnect parameters
//	parameter DW = 31, 	// Number of data bits minus 1
	parameter AW = 0, 	// Number of address bits minus 1
//	parameter BE = 3,		// Number of byte enable bits minus 1

	// Module parameters
	parameter CYCLE_COUNTERS	= 2	// The number of cycle counters
) (
	input						clk,
	input						reset, 

	input						avs_control_address,
	input						avs_control_read,
	input						avs_control_write,
	input			[31: 0]	avs_control_writedata,
	output		[31: 0]	avs_control_readdata,

	input			[AW: 0]	avs_data_address,
	input			[ 7: 0]	avs_data_byteenable,
	input						avs_data_read,
	input						avs_data_write,
	input			[63: 0]	avs_data_writedata,
	output reg	[63: 0]	avs_data_readdata
);

localparam CC = CYCLE_COUNTERS - 1;		// The number of cycle counters minus 1

reg		[CC: 0]	run_counters;
reg		[63: 0]	cycle_counts	[CC: 0];

wire		[CC: 0]	reset_counters;
//wire		[CC: 0]	set_counter;

genvar	g;
integer	i;

assign avs_control_readdata = run_counters;

always @(posedge clk)
begin
	avs_data_readdata <= cycle_counts[avs_data_address];
end

initial run_counters = {CYCLE_COUNTERS{1'b0}};
always @(posedge clk)
begin
//	if ((avs_control_address == 0) & avs_slave_write & avs_control_byteenable[0])
//		run_counter[ 7: 0] <= avs_control_writedata[ 7: 0];
//
//	if ((avs_control_address == 0) & avs_slave_write & avs_control_byteenable[1])
//		run_counter[15: 8] <= avs_control_writedata[15: 8];
//
//	if ((avs_control_address == 0) & avs_slave_write & avs_control_byteenable[2])
//		run_counter[23:16] <= avs_control_writedata[23:16];
//
//	if ((avs_control_address == 0) & avs_slave_write & avs_control_byteenable[3])
//		run_counter[31:24] <= avs_control_writedata[31:24];

	if ((avs_control_address == 0) & avs_control_write)
	begin
		$display("Write to run counters");
		for (i = 0; i < CYCLE_COUNTERS; i = i + 1)
			$display("counter %d = %d", i, cycle_counts[i]);

		run_counters <= avs_control_writedata;
	end
end

generate
	for (g = 0; g < CYCLE_COUNTERS; g = g + 1)
	begin : CHECK_FOR_INITIATE_COUNTER_RESET
		assign reset_counters[g] = (avs_control_address == 1) & avs_control_writedata[g] & avs_control_write;
	end
endgenerate

initial
begin
	for (i = 0; i < CYCLE_COUNTERS; i = i+ 1)
		cycle_counts[i] = 64'd0;
end
always @(posedge clk)
begin
	for (i = 0; i < CYCLE_COUNTERS; i = i+ 1)
	begin
		if (reset_counters[i])
			cycle_counts[i] <= 64'd0;
		else if (run_counters[i])
			cycle_counts[i] <= cycle_counts[i] + 64'd1;
	end
end

endmodule

