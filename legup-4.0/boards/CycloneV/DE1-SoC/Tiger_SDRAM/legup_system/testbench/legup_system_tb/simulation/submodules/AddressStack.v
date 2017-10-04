// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module AddressStack # (
	parameter N2 = 8,
	parameter S  = 32,
	parameter S2 = 5
	) (
	input clk,
	input reset,

	input call_ah_as,
	input retn_ah_as,
	input cnt_inc_ah_as,
	input [N2-1:0] funcNum_ah_as,

	output reg call_as_cb,
	output reg retn_as_cb,
	output reg cnt_inc_as_cb,
	output [N2-1:0] funcNum_as_cb,
	
	output reg exe_start_as,
	output reg exe_end_as
);

//+++++++++++++++++++++++++++++
// PIPELINING
//+++++++++++++++++++++++++++++
	reg call_r;
	reg retn_r;
	reg cnt_inc_r;
	reg exe_end_r;	// delay(pipeline) one cycle so that exe_end_as is aligned with the last write in CounterStorage

	always @ (posedge clk) begin
		if (reset) begin
			call_r    <= 1'b0;
			retn_r    <= 1'b0;
			cnt_inc_r <= 1'b0;
		
			call_as_cb    <= 1'b0;
			retn_as_cb    <= 1'b0;
			cnt_inc_as_cb <= 1'b0;
		end
		else begin
			call_r    <= call_ah_as;
			retn_r    <= retn_ah_as;
			cnt_inc_r <= cnt_inc_ah_as;
		
			call_as_cb    <= call_r & ~(exe_end_r | exe_end_as);
			retn_as_cb    <= retn_r & ~(exe_end_r | exe_end_as);
			cnt_inc_as_cb <= cnt_inc_r;
		end
	end

//+++++++++++++++++++++++++++++
// STACK
//+++++++++++++++++++++++++++++	
	wire [N2-1:0] stack_rd_data;
	reg  [N2-1:0] stack_wr_data;
	reg  [S2-1:0] stack_addr;
	reg  stack_wren;
	 
	assign funcNum_as_cb = stack_rd_data;
	
	always @ (posedge clk) begin
		if (reset) begin
			stack_wren <= 0;
			stack_wr_data <= {N2{1'b0}};
			stack_addr <= {S2{1'b1}};	// The first funcNum (first call) will be written to addr 0
		end
		else begin
			stack_wren <= call_ah_as;	// push on call - write stack only on call
			stack_wr_data <= funcNum_ah_as;
			if      (call_ah_as)	stack_addr <= stack_addr + 1'b1;	// push on call
			else if (retn_ah_as)	stack_addr <= stack_addr - 1'b1;	//  pop on retn
			else                	stack_addr <= stack_addr;
		end
	end
	
	// Just a ram blk used as stack; Nothing Fancy Here ~~~
	altsyncram stack (
		.clock0 (clk),
		.clocken0 (1'b1),
		.address_a (stack_addr),
		.wren_a    (stack_wren),
		.data_a    (stack_wr_data),
		.q_a       (stack_rd_data),
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
		stack.width_a = N2,
		stack.width_byteena_a = 1;

//+++++++++++++++++++++++++++++
// EXECUTION STATUS
//+++++++++++++++++++++++++++++
always @ (posedge clk) begin
	if (reset) begin
		exe_start_as <= 1'b0;
		exe_end_r    <= 1'b0;
		exe_end_as   <= 1'b0;
	end
	else begin
		// aligned with *_as_cb signals
		exe_start_as <= exe_start_as | call_r;                	// aligned with call_as_cb
		exe_end_r    <= exe_start_as & (&stack_addr) & retn_r;	// aligned with retn_as_cb
		exe_end_as   <= exe_end_r | exe_end_as;               	// aligned with last write in CounterStorage
	end
end

endmodule

