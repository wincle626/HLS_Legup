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

module tiger_alu(
	input signed [31:0]	srca, srcb,	// 2 operands
	input  [4:0]		alucontrol, // What function to perform
	output [31:0]	    aluout		// Result of the function
);

	wire unsigned [31:0] srcau = srca;
	wire unsigned [31:0] srcbu = srcb;
	
	assign aluout	=	alucontrol == `ALU_ADD || alucontrol == (`ALU_ADD | `ALU_UNSIGNED)	?	srca + srcb
					:	alucontrol == `ALU_SUB || alucontrol == (`ALU_SUB | `ALU_UNSIGNED)	?	srca - srcb
					:	alucontrol == `ALU_AND												?	srca & srcb
					:	alucontrol == `ALU_OR												?	srca | srcb
					:	alucontrol == `ALU_XOR												?	srca ^ srcb
					:	alucontrol == `ALU_NOR												?	~(srca | srcb)
					:	alucontrol == `ALU_SLT												?	srca < srcb
					:	alucontrol == (`ALU_SLT | `ALU_UNSIGNED)							?	srcau < srcbu
					:	alucontrol == `ALU_LUI												?	{srcb[15:0], 16'b0}
					:																			32'hxxxx_xxxx;	
endmodule 