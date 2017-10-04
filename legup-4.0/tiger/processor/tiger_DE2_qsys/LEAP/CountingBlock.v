// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module CountingBlock # (
	parameter N = 256,
	parameter N2 = 8,
	parameter S  = 32,
	parameter S2 = 5,
	parameter CW = 32
	) (
	input clk,
	input reset,
	input do_hier,
	
	input exe_start_as,
	input [CW-1:0] init_count_on_jump,
	input call_as_cb,
	input retn_as_cb,
	input cnt_inc_as_cb,
	input [N2-1:0] funcNum_as_cb,

	input  [ N2-1:0] ext_addr,
	output [CW-1:0] ext_rd_data
);
wire [CW-1:0] count;   // counter value
wire [CW-1:0] prev_descendant_count_push;  // on call
wire [CW-1:0] prev_descendant_count_pop;   // on retn

CounterStorage cs (
	.clk (clk),
	.reset (reset),
	// config register
	.do_hier (do_hier),
	// from AddressStack
	.call_as_cb    (call_as_cb),
	.retn_as_cb    (retn_as_cb),
	.funcNum_as_cb (funcNum_as_cb),
	// from IncCounter
	.count (count),
	// from CounterStack
	.prev_descendant_count_pop  (prev_descendant_count_pop),
	.prev_descendant_count_push (prev_descendant_count_push),
	// Read request from external component i.e., JTAG_MASTER
	.ext_addr (ext_addr),
	.ext_rd_data (ext_rd_data)
);
	defparam cs .N  = N ;
	defparam cs .N2 = N2;

IncCounter ic (
	.clk (clk),
	.reset (reset),
	.exe_start_as  (exe_start_as),
	.init_count_on_jump (init_count_on_jump),
	.inc (cnt_inc_as_cb),
	.call_as_cb (call_as_cb),
	.retn_as_cb (retn_as_cb),
	.count (count)
);

CounterStack cstack (
	.clk (clk),
	.reset (reset),

	.call_as_cb (call_as_cb),
	.retn_as_cb (retn_as_cb),
	.prev_descendant_count_push (prev_descendant_count_push),
	.prev_descendant_count_pop (prev_descendant_count_pop)
);
endmodule
