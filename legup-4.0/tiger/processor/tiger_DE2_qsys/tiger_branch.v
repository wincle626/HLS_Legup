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
`include "tiger_defines.v"

module tiger_branch(
	input clk,						// clock signal
	input reset,					// reset signal
	input stall,					// stall signal

	input exception,				// interrupt request signal
	
	input [31:0] instr,				// current instruction
	input [`CONTROL_WIDTH] control,	// control signals
	input [31:0] rs,				// first operand
	input [31:0] rt,				// second operand
	
	output reg [31:0] branchout,	// return address if we are doing a branch with link operation
	output [31:0]epc,
	output reg branchDelay,
	
	output [31:0] nextpc			// address of the next instruction to load
);

	reg [31:0] currentpc;
	   
	wire [2:0] branchtype = control[`CONTROL_BRANCHTYPE];
	
	wire takebranch =  control[`CONTROL_BRANCH]						 // take the branch if:
					&&
					(  (branchtype == `BR_LTZ && rs[31])			 // we have a branch if < 0 instruction and the MSB of the first operand is set
					|| (branchtype == `BR_GEZ && !rs[31])			 // we have a branch if >= 0 instruction and the MSB of the first operand is not set
					|| (branchtype == `BR_EQ  && rs == rt)			 // we have a branch if equal isntruction and both operands are equal to each other
					|| (branchtype == `BR_NE  && rs != rt)			 // we have a branch if not equal instruction and both operands are not equal
					|| (branchtype == `BR_LEZ && rs[31])			 // we have a branch if <= 0 instruction and either the MSB is set
					|| (branchtype == `BR_LEZ && rs == 0)			 // or the register equals 0
					|| (branchtype == `BR_GTZ && !rs[31] && rs != 0) // we have a branch if > 0 instruction, and the MSB is not set, and the register does not equal 0
					);
					
	// sign extend the immediate constant (used for a relative jump here) and shift 2 places to the left
	wire [31:0] signimmsh = {{14{instr[15]}}, instr[15:0], 2'b00};
	wire [31:0] pcplus4 = currentpc + 4;
	
	assign nextpc	=	reset							?	`BOOT_ADDR							// reset the start address
					:	stall							?	currentpc								// we are stalled so do not change the current address
					:   exception						?   `EXCEPTION_HANDLER_ADDR
					:	takebranch						?	currentpc + signimmsh					// perform a relative branch
					:	control[`CONTROL_JUMP]			?	{currentpc[31:28], instr[25:0], 2'b00}	// perform an immediate jump
					:	control[`CONTROL_REGJUMP]		?	rs										// perform a jump to the address contained in operand 1
					:										pcplus4;								// if a jump is not being performed, advance the pc by 4
	
	//if we're in a branch delay slot the exception program counter needs to
	//point to the branch rather than the instruction in the delay slot
	//otherwise we need to point to the instruction where the exception occured
	assign epc = branchDelay ? branchout - 8
					: currentpc - 4;
	
	always @(posedge clk)
	begin
		currentpc <= nextpc;
		
		if((takebranch || control[`CONTROL_JUMP] || control[`CONTROL_REGJUMP]) && !stall)
			branchDelay <= 1;
		else if(!stall)
			branchDelay <= 0;
		
		if( reset )	begin
			branchDelay <= 0;
		end else if( !stall ) begin
			// set the return branch address
			branchout <= pcplus4;
		end
	end
endmodule 
