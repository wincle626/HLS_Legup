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

module tiger_decoder(
	input [31:0]		instr,		// instruction to decode
	output reg [15:0]	controls,	// control signals
	output reg [4:0]	alucontrol,	// alu control code
	output reg [2:0]	branchtype,	// branch code
	output [4:0]		destreg		// destination register for result
);

	assign destreg = controls[5]	                ? 5'd31
				   : (controls[12] || controls[14])	? instr[20:16]
				   :				                  instr[15:11];
				
	wire [5:0] op = instr[31:26];
	wire [5:0] funct = instr[5:0];
	
	always @(*) begin
		// BEQ, BNE, BLEZ, BGTZ
		if (op[5:2] == 4'b0001) begin
			controls 	<= 16'b0000_1000_0000_0000;
			alucontrol 	<= `ALU_NONE;
			branchtype 	<= {op[1] | op[0], ~(op[1] ^ op[0]), ~op[0]};
		// BLTZ, BGEZ
		end else if (op == 6'b00_0001) begin
			controls 	<= 16'b0000_1000_0000_0000;
			alucontrol	<= `ALU_NONE;
			branchtype  <= {1'b0, instr[16], ~instr[16]};
		end else begin
			branchtype  <= `BR_NONE;
			// R-type instruction
			if (op == 6'b00_0000) begin
				controls	<= (funct == 6'b00_1000) ? 16'b0000_0000_1000_0000
							 : (funct == 6'b00_1001) ? 16'b0010_0000_1010_0000
							 : 						   16'b0010_0000_0000_0000;
				// ADD, ADD UNSIGNED, SUB, SUB UNSIGNED
				if (funct[5:2] == 4'b1000) begin
					alucontrol  <= {2'b01, funct[1], ~funct[1], funct[0]};
				end else if (funct[5:2] == 4'b0110) begin
					alucontrol  <= {1'b0, funct[1], {2{~funct[1]}}, funct[0]};
				end else if (funct[5:2] == 4'b1001) begin
					alucontrol  <= {funct[1], {3{~funct[1]}}, funct[0]};
				end else if (funct[5:2] == 4'b0100) begin
					alucontrol	<= {2'b00, funct[1], ~funct[1], funct[0]};
				end else if (funct[5:3] == 3'b000 && (funct[1] || ~funct[0])) begin
					alucontrol	<= {2'b11, funct[2:1], ~funct[0]};
				end else if (funct[5:1] == 5'b10101) begin
					alucontrol  <= {4'b1001, funct[0]};
				end else begin
					alucontrol	<= `ALU_NONE;
				end
			// ADDI, ADDIU, SLTI, SLTIU, ANDI, ORI, XORI, LUI
			end else if (op[5:3] == 3'b001) begin
				alucontrol  <= {op[1], ~op[1], op[2] & (op[0] | ~op[1]), ~op[2] | ~op[1], op[0] & (~op[2] | ~op[1])};
				controls	<= {2'b0, {2{1'b1}}, {5{1'b0}}, op[2] & (~op[1] | ~op[0]), {6{1'b0}}};
			// LB, LH, LWL, LW, LBU, LHU, LWR
			end else if (op[5:3] == 3'b100 && (~op[2] | ~op[1] | ~op[0])) begin
				alucontrol 	<= `ALU_ADD;
				controls	<= {2'b0, 7'b110_0100, op[2] & ~op[1], 1'b0, ~op[1] & op[0], ~op[1] & ~op[0], ~op[2] & op[1] & ~op[0], op[2] & op[1], 1'b0};
			// SB, SH, SW
			end else if (op[5:2] == 4'b1010 && (~op[1] | op[0])) begin
				alucontrol 	<= `ALU_ADD;
				controls 	<= {2'b0, 9'b0_1010_0000, ~op[1] & op[0], ~op[1] & ~op[0], 3'b000};
			// J, JAL
			end else if (op[5:1] == 5'b0_0001) begin
				alucontrol	<= `ALU_NONE;
				controls	<= {2'b0, op[0], 7'b000_0100, op[0], {5{1'b0}}};
			// ERET, JERET
			//end else if (op[5:1] == 5'b0_1000) begin
			//	alucontrol  <= `ALU_NONE;
			//	controls	<= op[0] 											? 16'b0000_0001_0000_0001
			//				 : {instr[25:21], instr[5:0]} == 11'b100_0001_1000 	? 16'b0000_0000_0000_0001
			//				 :													  16'b0000_0000_0000_0000;
			// MUL
			end else if (op == 6'b01_1100) begin
				alucontrol	<= (funct == 6'b00_0010) ? `ALU_MUL 			 : `ALU_NONE;
				controls	<= (funct == 6'b00_0010) ? 16'b0010_0000_0000_0000 : 16'b0000_0000_0000_0000;
			// MFC0, MTC0
			end else if (op == 6'b01_0000) begin
				alucontrol <= `ALU_NONE;
				controls   <= {instr[23], {2{~instr[23]}}, 13'b0_0000_0000};
			end else begin
				alucontrol	<= `ALU_NONE;
				controls	<= 16'b0000_0000_0000_0000;
			end
		end
	end
endmodule 