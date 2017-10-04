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

// enable profiling
// `define PROFILER_ON 1'b1

// default addresses
`define EXCEPTION_HANDLER_ADDR	32'h0000_0A00
`define BOOT_ADDR				32'h0080_0000		// starts at 0000_0000 b/c thats the addr of onchip_mem
`define STARTINGPC 				32'h0080_0020
`define FINISHPC 				32'h0080_0010

`define REGNUM_WIDTH		4:0
 
 // CONTROL is a set of signals, 'CONTROL_WIDTH wide
 // each define starting with 'CONTROL declares what 
 // each bit of the control signal represents
`define CONTROL_WIDTH		30:0
`define CONTROL_ALUCONTROL	12:8
`define CONTROL_BRANCHTYPE	7:5
`define CONTROL_WRITEREGNUM	4:0

`define BR_NONE			3'b000
`define BR_LTZ			3'b001
`define BR_GEZ			3'b010
`define BR_EQ			3'b011
`define BR_NE			3'b100
`define BR_LEZ			3'b101
`define BR_GTZ			3'b110

`define CONTROL_ALUCONTROL_UNSIGNED	8
`define CONTROL_ALUCONTROL_RIGHT	9
`define CONTROL_ALUCONTROL_VARIABLE	10
`define CONTROL_ALUCONTROL_SHIFT	12:11

 // ALU control signals
`define ALU_UNSIGNED	5'b0000_1

`define	ALU_NONE		5'b0000_0	// unsigned version needed
`define ALU_MFHI		5'b0001_0
`define ALU_MTHI		5'b0001_1
`define ALU_MFLO		5'b0010_0
`define ALU_MTLO		5'b0010_1
`define ALU_MULT		5'b0011_0	// unsigned version needed
`define ALU_DIV			5'b0100_0	// unsigned version needed
`define ALU_ADD			5'b0101_0	// unsigned version needed
`define ALU_SUB			5'b0110_0	// unsigned version needed
`define ALU_AND			5'b0111_0
`define ALU_OR			5'b0111_1
`define ALU_XOR			5'b1000_0
`define ALU_NOR			5'b1000_1
`define ALU_SLT			5'b1001_0	// unsigned version needed
`define ALU_LUI			5'b1010_0
// 1010_1 is unused
`define ALU_MUL			5'b1011_0	// unsigned version needed
`define ALU_SL			5'b1100_0	// unsigned version needed
`define ALU_SR			5'b1101_0	// unsigned version needed
`define ALU_SLV			5'b1110_0	// unsigned version needed
`define ALU_SRV			5'b1111_0	// unsigned version needed


`define CONTROL_IRQRETURN	13
`define CONTROL_MEMR		14
`define CONTROL_MEML		15
`define CONTROL_MEM8		16
`define CONTROL_MEM16		17
`define CONTROL_LINK		18
`define CONTROL_ZEROFILL	19
`define CONTROL_REGJUMP		20
`define CONTROL_JUMP		21
`define CONTROL_MEMREAD		22
`define CONTROL_MEMWRITE	23
`define CONTROL_BRANCH		24
`define CONTROL_USEIMM		25
`define CONTROL_REGWRITE	26
`define CONTROL_COPREAD     27
`define CONTROL_COPWRITE    28
`define CONTROL_DCACHEFLUSH 29
`define CONTROL_ICACHEFLUSH 30
