`define STACK_ADDR	'hFFDFFC 	//'h800000 //
`define PROF_ADDR	'hFFE000 	//'h800004 //
`define S			32			// Address Stack Size
`define S2			5			// log2(stack depth)
`define N			16 			// Number of Functions
`define N2			4 			// log2(`N)

`define ICW			16			// instruction counter width
`define CCW			16			// cycle counter width
`define SCW			16			// stall cycle counter width

`define PW			22			// power counter individual width
`define PSW			28			// power stall counter width
`define PGROUPS		6			// number of groupings for power
`define PCW			(`PW*`PGROUPS+`PSW)	// power counter [total] width

`define PROF_TYPE "L"			// Choose profiler (L -- leap, s -- SnoopP)       
`define PROF_METHOD	"i" 		// Choose profiling method (i -- instruction count, c -- cycle count, s -- stall cycle count, p -- power count)       
`define CW ((`PROF_METHOD=="i") ? `ICW : (`PROF_METHOD=="c") ? `CCW : (`PROF_METHOD=="s") ? `SCW : (`PROF_METHOD=="p") ? `PCW : 0)
//`define CW64					// uncomment this line if ICW = 64

//synthesis read_comments_as_HDL off
// JAL: 0000 11ii iiii iiii iiii iiii iiii iiii	// THIS NEEDS TO BE 800000 for tiger, 80030000 in from sim, but shifted right 2 bits for int-addressable
//`define START_ADDR			32'b000011__0000_0011_0000_0000_0000_0000_00	// 'h80030000 = 'b1000_0000_|0000_0011_0000_0000_0000_0000|00
`define START_ADDR			32'b0000_11__0000_1000_0000_0000_0000_0000_00	// 'h00800000 = 'b0000_0000_|1000_0000_0000_0000_0000_0000|00	/// COMMENT BETTER
`define DO_HIER				1'b0

// addresses corresponding to the jal and ra of wrap.c (before/after call to main)
`define WRAP_MAIN_BEGIN		'h800018		// 0x800000 + 0x18
`define WRAP_MAIN_END		'h800074		// 0x800000 + 0x74

// power profiling constants
`define A 1
`define B 2
`define C 3
`define D 4
`define E 5
`define F 6
`define STALL 7

`define P_ADD   	`C
`define P_ADDI  	`D
`define P_ADDIU 	`D
`define P_ADDU  	`C
`define P_AND   	`E
`define P_ANDI  	`C
`define P_BEQ   	`B
`define P_BGEZ  	`B
`define P_BGEZAL	`B
`define P_BGTZ  	`B
`define P_BLEZ  	`B
`define P_BLTZ  	`B
`define P_BLTZAL	`B
`define P_BNE   	`C
`define P_DIV   	`A
`define P_DIVU  	`A
`define P_J     	`D
`define P_JAL   	`C
`define P_JR		`E
`define P_LB   		`E
`define P_LBU		`D
`define P_LH		`F
`define P_LHU		`E
`define P_LUI   	`D
`define P_LW    	`C
`define P_MFHI  	`A
`define P_MFLO  	`A
`define P_MULT  	`B
`define P_MULTU 	`B
`define P_NOP   	`E
`define P_OR    	`B
`define P_ORI   	`E
`define P_SB    	`F
`define P_SH		`F
`define P_SLL   	`D
`define P_SLLV  	`F
`define P_SLT   	`B
`define P_SLTI  	`A
`define P_SLTIU 	`B
`define P_SLTU  	`B
`define P_SRA   	`C
`define P_SRAV		`D
`define P_SRL  		`D
`define P_SRLV  	`D
`define P_SUB   	`D
`define P_SUBU  	`E
`define P_SW    	`D
`define P_XOR   	`B
`define P_XORI  	`B

`define P_ADD_ADDU	`C
`define P_DIV_DIVU	`A
`define P_ADDI_ADDIU `D
`define P_SB_SH		`F
