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

module tiger_decode(
	input clk,								// clock signal
	input reset,							// reset signal
	input stall,							// stall signal
	input clear,							// clear signal
	
	input irq,								// interrupt signal
	input [5:0] irqNumber,

	input [31:0] instr,						// current instruction
	
	input writeRegEnWB,						// True if write back wants to write to a register
	input writeRegEnCopWB,					// Ture if write back wants to write a coprocessor register
	input [`REGNUM_WIDTH]writeRegNumWB,		// Register write back wants to write to
	input [31:0]writeRegDataWB,				// Data write back wants to write
	
	output exception,
	
	output [`CONTROL_WIDTH] controlDe,

	output reg [31:0] instrEx,				// output containing the current instruction
	output reg [`CONTROL_WIDTH] controlEx,	// output for the control signals
	output reg [31:0] rsEx,					// first of 2 operand outputs
	output reg [31:0] rtEx,					// second of 2 operand outputs
	output reg [31:0] CPOutEx,	            // Coprocessor register contents
			
	output [31:0] branchoutEx,				// branch address
					
	output [31:0] nextpc					// next instruction address
);
	wire branchDelay;
	
	wire iCacheFlush;
	wire dCacheFlush;
	
	// decoder module
	wire [15:0] controls;
	wire [4:0] alucontrol;
	wire [2:0] branchtype;
	wire [4:0] destreg;
	tiger_decoder d(
		.instr(instr),
 		.controls(controls),
		.alucontrol(alucontrol),
		.branchtype(branchtype),
		.destreg(destreg)
	);
	// set up the control signals
	wire [`CONTROL_WIDTH] control={iCacheFlush, dCacheFlush, controls, alucontrol, branchtype, destreg};
	assign controlDe = control;
	
	//register file - 31 registers (numbered 1 to 31) each 32 bits long
	reg [31:0] rf[31:1];
	
	reg [31:0] cause;
	reg [31:0] status;
	reg [31:0] epc;
	
	wire [31:0]epcDe;
	
	wire break = instr[31:26] == 6'b00_0000 && instr[5:0] == 6'b00_1101;
	wire syscall = instr[31:26] == 6'b00_0000 && instr[5:0] == 6'b00_1100;
	
	assign exception = (!status[0] && irq) || break || syscall;
	
	assign iCacheFlush = (writeRegEnCopWB && writeRegNumWB == 5'd3 && writeRegDataWB[0] == 1'b1);
	assign dCacheFlush = (writeRegEnCopWB && writeRegNumWB == 5'd3 && writeRegDataWB[1] == 1'b1);
		
	//If we're reading from $zero, register value is 0,
	//otherwise if it's a register WB is currently wanting to write
	//to pass the value straight through, otherwise use the value in the register file
	wire [31:0] rsFF = instr[25:21]==5'b0 ? 32'b0 
		: instr[25:21] == writeRegNumWB && writeRegEnWB ? writeRegDataWB
		: rf[instr[25:21]];
		
	//If we're reading from $zero, register value is 0,
	//otherwise if it's a register WB is currently wanting to write
	//to pass the value straight through, otherwise use the value in the register file
	wire [31:0] rtFF = instr[20:16]==5'b0 ? 32'b0 
		: instr[20:16] == writeRegNumWB && writeRegEnWB ? writeRegDataWB
		: rf[instr[20:16]];	
	
	always @(posedge clk)
	begin
		// if the register number is not 5'b00000 and register writeback from memory is enabled
		// then store the data from memory in the register file
		if(writeRegNumWB!=5'b0 && writeRegEnWB)
			rf[writeRegNumWB] <= writeRegDataWB;
		if(writeRegEnCopWB) begin
			case(writeRegNumWB)
				//5'b0_0000: cause <= writeRegDataWB;
				5'b0_0001: status <= writeRegDataWB;
				//5'b0_0010: epc <= writeRegDataWB;
			endcase
		end
		
		if( exception && !clear && !stall) begin
			if(irq)
				cause <= {branchDelay, 15'b0, irqNumber, 10'b0};
			else if(break)
				cause <= {branchDelay, 26'b0, 4'd9, 1'b0};
			else if(syscall)
				cause <= {branchDelay, 26'b0, 4'd8, 1'b0};
			else
				cause <= {branchDelay, 26'b0, 4'hf, 1'b0};
			
			status <= {status[31:1], 1'b1};
			epc <= epcDe;
		end
				
		if( reset ) begin
			instrEx 	<= 0;
			controlEx 	<= 0;
			rsEx		<= 0;
			rtEx 		<= 0;
			cause 		<= 0;
			status 		<= 0;
			epc			<= 0;
			
			// reset the stack pointer
			rf[29] <= 32'h0078_0000;
		end else if( stall ) begin
			//Stall instrEx
			//Stall controlEx
			//Stall rsEx
			//Stall rtEx
			//Stall CPOutEx
		end else if (clear || (exception && !stall)) begin
			instrEx		<= 0;
			controlEx	<= 0;
			rsEx		<= 0;
			rtEx		<= 0;
			CPOutEx 	<= 0;
		end else begin
			instrEx <= instr;
			controlEx <= control;
			rsEx <= rsFF;
			rtEx <= rtFF;
			
			CPOutEx <= instr[15:11] == 5'b0_0000 ? cause
				: instr[15:11] == 5'b0_0001 ? status
				: instr[15:11] == 5'b0_0010 ? epc
				: 5'bx_xxxx;
		end
	end
	
	tiger_branch b(
		.clk(clk),
		.reset(reset),
		.stall(stall || clear),

		.exception(exception),
		
		.instr(instr),
		.control(control),
		.rs(rsFF),
		.rt(rtFF),
		
		.branchout(branchoutEx),
		.epc(epcDe),
		.branchDelay(branchDelay),
		
		.nextpc(nextpc)
	);
	
endmodule 