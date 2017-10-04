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

module tiger_memoryaccess(
	input clk,							// clock signal
	input reset,
	input clear,
	input stall,
	
	input [31:0] instr,					// current instruction
	input [`CONTROL_WIDTH] control,		// control signals
	input [31:0] executeout,			// output of the execute stage
	input [31:0] branchout,
	
	output reg [`CONTROL_WIDTH] controlWB,
	output reg [31:0] MAOutWB,
	output reg [31:0] branchoutWB,
	output reg [31:0] instrWB,
	
	//input [1:0] bottomaddress,			// bottom bits of the address
	input [31:0] memreaddata,			// data read from memory
	
	//true if we wish to write to the register given in writeRegNum
	//the write will actually be done in the following WB stage
	output writeRegEn,
	//the number of the register we're going write to (in the WB stage)
	output [`REGNUM_WIDTH] writeRegNum,
	
	//true if we wish to write to the coprocessor register given in writeRegNum
	//the write will actually be done in the following WB stage
	output writeRegEnCop,
	
	//if we are going to write to a register, what data we would write
	//note that this is the output from the execute stage
	//if we are reading from memory, the memory read data will
	//be put into the next pipeline register at the next clock
	output [31:0] writeRegData
);
	
	assign writeRegEn = control[`CONTROL_REGWRITE];
	assign writeRegEnCop = control[`CONTROL_COPWRITE];
	assign writeRegNum = control[`CONTROL_WRITEREGNUM];
	assign writeRegData = executeout;

	wire [31:0]memData;
	
	assign memData = memreaddata;
	
	/*assign memregdata = control[`CONTROL_MEML] ?	// left hand side of the bus
			 bottomaddress==2'd0 ?	memreaddata
			:bottomaddress==2'd1 ?	{memreaddata[23:0],executeout[7:0]}
			:bottomaddress==2'd2 ?	{memreaddata[15:0],executeout[15:0]}
			:bottomaddress==2'd3 ?	{memreaddata[7:0],executeout[23:0]}
			:						32'hXXXX_XXXX
		:control[`CONTROL_MEMR] ?	// right hand side of the bus
			 bottomaddress==2'd0 ?	{executeout[31:8],memreaddata[31:24]}
			:bottomaddress==2'd1 ?	{executeout[31:16],memreaddata[31:16]}
			:bottomaddress==2'd2 ?	{executeout[31:24],memreaddata[31:8]}
			:bottomaddress==2'd3 ?	memreaddata
			:						32'hXXXX_XXXX
		:memreaddata;*/
	
	
	//Pipeline, on the positive clock edge move everything to the next pipeline stage
	always @(posedge clk) begin	
		if (reset || clear) begin
			controlWB	<= 0;
			MAOutWB<= 0;
			branchoutWB <= 0;
			instrWB <= 0;
		end else if (stall) begin
			//stall controlWB
			//stall MAOutWB
			//stall branchoutWB
			//stall instrWB
		end else begin
			controlWB	<= control;
			MAOutWB <= control[`CONTROL_MEMREAD] ? 
					(!control[`CONTROL_ZEROFILL] && control[`CONTROL_MEM8] ? {{24{memData[7]}}, memData[7 : 0]}
					 : !control[`CONTROL_ZEROFILL] && control[`CONTROL_MEM16] ? {{16{memData[15]}}, memData[15 : 0]}
					 : memData)
				: executeout;
			branchoutWB <= branchout;
			instrWB <= instr;
		end
	end
endmodule 