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

module tiger_decoder2(
	input [31:0]		instr,		// instruction input
	output reg [13:0]	controls,	// general control signals
	output reg [4:0]	alucontrol, // alu operation type
	output reg [2:0]	branchtype, // what type of branch we are performing
	output [4:0]		destreg		// destination register
);

  assign destreg =    controls[5]		?	5'd31			// LINK
					: controls[12]		?	instr[20:16]	// USEIMM
					:						instr[15:11];

  // opcode
  wire [5:0] op = instr[31:26];
  // function
  wire [5:0] funct = instr[5:0];

  // zero extend this operation
  parameter ZEROX=		14'b00_0000_0100_0000;
  // ask for 16 bits from the bus
  parameter MEM16=		14'b00_0000_0001_0000;
  // ask for 8 bits from the bus
  parameter MEM8=		14'b00_0000_0000_1000;
  // ask for left from the bus
  parameter MEML=		14'b00_0000_0000_0100;
  // ask for right from the bus
  parameter MEMR=		14'b00_0000_0000_0010;

  // do a link in addition to another instruction, by ensuring
  // regwrite is high to store to register $31 and the link flag
  parameter LINK=		14'b10_0000_0010_0000;

  // set the regwrite flag to store result in a register
  parameter ALU=		14'b10_0000_0000_0000;
  // set the regjump flag to do a register jump
  parameter REGJUMP=	14'b00_0000_1000_0000;
  // set the branch flag to do a branch
  parameter BRANCH=		14'b00_1000_0000_0000;
  // set the jump flag to do a jump
  parameter JUMP=		14'b00_0001_0000_0000;

  // set the regwrite flag to store results in a register
  // and the useimm flag to use the 16bit immediate as an operand
  parameter IMMOP=		14'b11_0000_0000_0000;

  // set the irqreturn flag to return from the interrupt
  parameter ERET=		14'b00_0000_0000_0001;

  // set regwrite to store memory to a register									
  // set useimm to calculate the address
  // set memread to do a memory read
  parameter LOAD=		14'b11_0010_0000_0000;

  // set useimm to calculate the address
  // set memwrite to do a memory write
  parameter STORE=		14'b01_0100_0000_0000;

  parameter UNKNOWN=	14'b00_0000_0000_0000;


  always @(*)
    case(op)
      6'b000000:
		begin
			if( funct == 6'b001000 )
				controls <= REGJUMP;
			else if( funct == 6'b001001 )
				controls <= REGJUMP | LINK;
			else
				controls <= ALU;
			case(funct)          // RTYPE
				 6'b010000: alucontrol <= `ALU_MFHI;
				 6'b010001: alucontrol <= `ALU_MTHI;
				 6'b010010: alucontrol <= `ALU_MFLO;
				 6'b010011: alucontrol <= `ALU_MTLO;
				
				 6'b011000: alucontrol <= `ALU_MULT;
				 6'b011001: alucontrol <= `ALU_MULT | `ALU_UNSIGNED;
				
				 6'b011010: alucontrol <= `ALU_DIV;
				 6'b011011: alucontrol <= `ALU_DIV | `ALU_UNSIGNED;
				
				 6'b100000: alucontrol <= `ALU_ADD;
				 6'b100001: alucontrol <= `ALU_ADD | `ALU_UNSIGNED;
				 6'b100010: alucontrol <= `ALU_SUB;
				 6'b100011: alucontrol <= `ALU_SUB | `ALU_UNSIGNED;

				 6'b100100: alucontrol <= `ALU_AND;
				 6'b100101: alucontrol <= `ALU_OR;
				 6'b100110: alucontrol <= `ALU_XOR;
				 6'b100111: alucontrol <= `ALU_NOR;

				 6'b101010: alucontrol <= `ALU_SLT;
				 6'b101011: alucontrol <= `ALU_SLT | `ALU_UNSIGNED;

				 6'b000000: alucontrol <= `ALU_SL | `ALU_UNSIGNED;
				 6'b000010: alucontrol <= `ALU_SR | `ALU_UNSIGNED;
				 6'b000011: alucontrol <= `ALU_SR;

				 6'b000100: alucontrol <= `ALU_SLV | `ALU_UNSIGNED;
				 6'b000110: alucontrol <= `ALU_SRV | `ALU_UNSIGNED;
				 6'b000111: alucontrol <= `ALU_SRV;
	
        	 default:   alucontrol <= `ALU_NONE;
	        endcase
			branchtype <= `BR_NONE;
		end

	  // BLTZ or BGEZ
      6'b000001: begin
			controls <= BRANCH;
			alucontrol <= `ALU_NONE;
			if( instr[16] == 0 )
				branchtype <= `BR_LTZ;
			else
				branchtype <= `BR_GEZ;
		end
		
	  // J
      6'b000010: begin
			controls <= JUMP;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_NONE;
		end

	  // JAL
	  6'b000011: begin
			controls <= JUMP | LINK;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_NONE;
		end
		
	  // BEQ
      6'b000100: begin
			controls <= BRANCH;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_EQ;
		end

	  // BNE
      6'b000101: begin
			controls <= BRANCH;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_NE;
		end

	  // BLEZ
      6'b000110: begin
			controls <= BRANCH;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_LEZ;
		end

	  // BGTZ
      6'b000111: begin
			controls <= BRANCH;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_GTZ;
		end

	  // ADDI
      6'b001000: begin
			controls <= IMMOP;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // ADDIU
      6'b001001: begin
			controls <= IMMOP;
			alucontrol <= `ALU_ADD | `ALU_UNSIGNED;
			branchtype <= `BR_NONE;
		end
		
	  // SLTI
      6'b001010: begin
			controls <= IMMOP;
			alucontrol <= `ALU_SLT;
			branchtype <= `BR_NONE;
		end

      // SLTIU
      6'b001011: begin
			controls <= IMMOP;
			alucontrol <= `ALU_SLT | `ALU_UNSIGNED;
			branchtype <= `BR_NONE;
		end

	  // ANDI
      6'b001100: begin
			controls <= IMMOP | ZEROX;
			alucontrol <= `ALU_AND;
			branchtype <= `BR_NONE;
		end

	  // ORI
      6'b001101: begin
			controls <= IMMOP | ZEROX;
			alucontrol <= `ALU_OR;
			branchtype <= `BR_NONE;
		end
	
	  // XORI
      6'b001110: begin
			controls <= IMMOP | ZEROX;
			alucontrol <= `ALU_XOR;
			branchtype <= `BR_NONE;
		end

	  // LUI
	  6'b001111: begin
			controls <= IMMOP;
			alucontrol <= `ALU_LUI; 
			branchtype <= `BR_NONE;
		end
		
	  // ERET (possibly)
	  6'b010000: begin
			if( {instr[25:21], instr[5:0]} == 11'b10000_011000 )
				controls <= ERET;
			else
				controls <= UNKNOWN;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_NONE;
		end

	  // JERET
	  6'b010001: begin
			controls <= ERET | JUMP;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_NONE;
		end

	  // MUL (possibly)
	  6'b011100: begin
			if( funct == 6'b000010 )
			begin
				controls <= ALU;
				alucontrol <= `ALU_MUL;
			end
			else
			begin
				controls <= UNKNOWN;
				alucontrol <= `ALU_NONE;
			end
			branchtype <= `BR_NONE;
		end

	  // LB
      6'b100000: begin
			controls <= LOAD | MEM8;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // LH
      6'b100001: begin
			controls <= LOAD | MEM16;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end
		
	  // LWL
      6'b100010: begin
			controls <= LOAD | MEML;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // LW
      6'b100011: begin
			controls <= LOAD;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // LBU
      6'b100100: begin
			controls <= LOAD | MEM8 | ZEROX;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // LHU
      6'b100101: begin
			controls <= LOAD | MEM16 | ZEROX;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // LWR
      6'b100110: begin
			controls <= LOAD | MEMR;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // SB
      6'b101000: begin
			controls <= STORE | MEM8;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // SH
      6'b101001: begin
			controls <= STORE | MEM16;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

	  // SW
      6'b101011: begin
			controls <= STORE;
			alucontrol <= `ALU_ADD;
			branchtype <= `BR_NONE;
		end

      default:
		begin
			controls <= UNKNOWN;
			alucontrol <= `ALU_NONE;
			branchtype <= `BR_NONE;
		end
    endcase
endmodule 