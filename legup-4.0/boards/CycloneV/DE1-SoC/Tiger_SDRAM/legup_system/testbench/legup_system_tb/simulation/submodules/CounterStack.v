// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module CounterStack # (
	parameter S  = 32,
	parameter S2 = 5,
	parameter CW = 32
	) (
	input clk,
	input reset,

	input call_as_cb,
	input retn_as_cb,
	input  [CW-1:0] prev_descendant_count_push,
	output [CW-1:0] prev_descendant_count_pop
);

//+++++++++++++++++++++++++++++
// STACK
//+++++++++++++++++++++++++++++	
	reg  [S2-1:0] stack_addr;
	wire [CW-1:0] stack_rd_data;
	wire [CW-1:0] stack_wr_data;
	wire  stack_wren;

	// prev_descendant_count_pop always holds the previous counting value (before the last call to descendant) of CURRENT function
	assign prev_descendant_count_pop = stack_rd_data;	 
	assign stack_wr_data = retn_as_cb ? {CW{1'b0}} : prev_descendant_count_push;	// !! CLEAR the data on retn
	assign stack_wren = call_as_cb | retn_as_cb;	// write on call, clear on retn

	// stack_addr always changes @ the next cycle after a write
	always @ (posedge clk) begin
		if (reset) begin
			stack_addr <= {S2{1'b0}};	// The first call will be written to addr 0
		end
		else begin
			if      (call_as_cb)	stack_addr <= stack_addr + 1'b1;	// push on call
			else if (retn_as_cb)	stack_addr <= stack_addr - 1'b1;	//  pop on retn
			else                	stack_addr <= stack_addr;
		end
	end
	
	altsyncram stack (
		.clock0 (clk),
		.clocken0 (1'b1),
		.address_a (stack_addr),
		.wren_a (stack_wren),
		.data_a (stack_wr_data),
		.q_a (stack_rd_data),
		.aclr0 (1'b0),
		.aclr1 (1'b0),
		.address_b (1'b0),
		.addressstall_a (1'b0),
		.addressstall_b (1'b0),
		.byteena_a (1'b1),
		.byteena_b (1'b1),
		.clock1 (1'b1),
		.clocken1 (1'b1),
		.clocken2 (1'b1),
		.clocken3 (1'b1),
		.data_b (1'b0),
		.eccstatus (),
		.q_b (),
		.rden_a (1'b1),
		.rden_b (1'b0),
		.wren_b (1'b0)
	);
	defparam
		stack.clock_enable_input_a = "BYPASS",
		stack.clock_enable_output_a = "BYPASS",
		stack.intended_device_family = "Cyclone II",
		stack.lpm_hint = "ENABLE_RUNTIME_MOD=NO",
		stack.lpm_type = "altsyncram",
		stack.numwords_a = S,
		stack.operation_mode = "SINGLE_PORT",
		stack.outdata_aclr_a = "NONE",
		stack.outdata_reg_a = "UNREGISTERED",
		stack.power_up_uninitialized = "FALSE",
		stack.ram_block_type = "M4K",
		stack.widthad_a = S2,
		stack.width_a = CW,
		stack.width_byteena_a = 1;

endmodule

