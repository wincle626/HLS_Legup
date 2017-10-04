// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module IncCounter # (
	parameter CW = 32
	) (
	input clk,
	input reset,
	input exe_start_as,
	input [CW-1:0] init_count_on_jump,
	input inc,
	input call_as_cb,
	input retn_as_cb,
	output reg [CW-1:0] count
);	
	wire [CW-1:0] one = { {(CW-1){1'b0}}, 1'b1};	// nothing special, just 1, but avoids warning about truncated assignment
	always @(posedge clk) begin
		if      (reset | ~exe_start_as)  	count <= {CW{1'b0}};
		else if (call_as_cb | retn_as_cb)	count <= init_count_on_jump;
		else if (inc)                    	count <= count + one;
	end

// synthesis translate off
always @ (*)
if ( &count[CW-1 :1] ) begin
	$display ("Counter is saturated!!\n");
	$stop();
end
// synthesis translate on
endmodule

