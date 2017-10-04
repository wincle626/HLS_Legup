/*
  This work by Simon Moore and Gregory Chadwick is licenced under the
  Creative Commons Attribution-Non-Commercial-Share Alike 2.0
  UK: England & Wales License.

  To view a copy of this licence, visit:
     http://creativecommons.org/licenses/by-nc-sa/2.0/uk/
  or send a letter to:
     Creative Commons,
     171 Second Street,
     Suite 300,
     San Francisco,
     California 94105,
     USA.
*/
`include "../tiger_defines.v"

module	tiger_fetch(
	input clk,					// clock signal
	input reset,				// reset signal
	input stall,				// whether this unit is stalled on the pipeline
	input clear,				// clear signal

	input [31:0] instr,			// instruction loaded from memory

	output reg [31:0] instrDE	// output instruction
);
	
	always @(posedge clk)
	begin
		if( reset || clear) begin
			instrDE <= 0;
		end else if( stall ) begin
			//Stall instrDE
		end else begin
			instrDE <= instr;
		end
	end
endmodule 