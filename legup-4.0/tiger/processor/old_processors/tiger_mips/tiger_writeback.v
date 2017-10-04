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

module tiger_writeback(
	input clk,								// clock signal
	
	input [31:0] instr,						// current instruction
	input [`CONTROL_WIDTH] control,			// control signals
	input [31:0] branchout,					// the branch address to save if we are doing a link operation
	input [31:0] MAOut,						// result from the memory access stage
	
	output writeRegEn,						// output indicating if we wish to write to a register
	output writeRegEnCop,					// output indicating if we wish to write to a coprocessor register
	output [`REGNUM_WIDTH] writeRegNum,		// what register number to write to
	output [31:0] writeRegData				// data to store in the register
);

	assign writeRegEn = control[`CONTROL_REGWRITE];
	assign writeRegEnCop = control[`CONTROL_COPWRITE];
	assign writeRegNum = control[`CONTROL_WRITEREGNUM];
	assign writeRegData = MAOut;
endmodule 