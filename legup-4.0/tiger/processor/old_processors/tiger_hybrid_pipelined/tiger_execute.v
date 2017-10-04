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

module tiger_execute(
	input clk,										// clock signal
	input reset,									// reset signal
	input stall,									// stall signal
	input clear,									// clear signal
	
	input [31:0] instr,								// current instruction
	input [`CONTROL_WIDTH] control,					// control signals
	input [31:0] rs,								// first operand
	input [31:0] rt,								// second operand
	input [31:0] branchout,							// return branch address
	input [31:0] CPOut,								// coprocessor register value
		
	output reg [31:0] instrMA,						// output instruction
	output reg [`CONTROL_WIDTH] controlMA,			// output control signals
	output reg [31:0] executeoutMA,					// output execute unit result
	output reg [31:0] branchoutMA,					// output branch return address
	//output reg [1:0] bottomaddressMA,				// output bottom 2 bits of the address
	
	output stallRq,								// do we wish to stall the pipeline
	
	output memread,mem16,mem8,memwrite,	// what type of memory access do we require
	output [31:0] memaddress,memwritedata,			// address in memory to write to + the data to write
	output iCacheFlush, dCacheFlush,
	input memCanRead, memCanWrite,
	input canICacheFlush, canDCacheFlush
);
	// sign-extend the 2 operands and multiply them together to form a 64 bit number
	//wire [63:0] mult = {{32{rs[31] && !control[`CONTROL_ALUCONTROL_UNSIGNED]}}, rs}
	//					* {{32{rt[31] && !control[`CONTROL_ALUCONTROL_UNSIGNED]}}, rt};
	
	wire [63:0]mults;
	wire [63:0]multu;
	
	//tiger_mult ms(rs, rt, mults);
	tiger_mult ms(
	.clock (clk),
	.dataa (rs),
	.datab (rt),
	.result (mults)
	);
	
	//tiger_multu mu(rs, rt, multu);
	tiger_multu	mu(
	.clock (clk),
	.dataa (rs),
	.datab (rt),
	.result (multu)
	);					
	
	// countdown indicates how many clock cycles until the HI and LO registers are valid
	reg [3:0] countdown;
	reg source;
	reg divmul;
	
	// HI and LO registers
	reg [31:0] high, low;
	
	// perform a signed division on the 2 operands
	wire [31:0] divLO, divHI;
	tiger_div div(
		.clock(clk),
		.denom(rt),
		.numer(rs),
		.quotient(divLO),
		.remain(divHI)
	);
	
	// perform an unsigned division on the 2 operands
	wire [31:0] divuLO, divuHI;
	tiger_divu divu(
		.clock(clk),
		.denom(rt),
		.numer(rs),
		.quotient(divuLO),
		.remain(divuHI)
	);
	
	// alu unit
	wire [31:0] aluout;
	tiger_alu alu(
		.srca(rs), // first operand is always rs
		// second operand may be an immediate constant (so we sign-extend to 32 bits) or a second register
		.srcb(control[`CONTROL_USEIMM] ? {{16{instr[15] && (!control[`CONTROL_ZEROFILL] || control[`CONTROL_MEMREAD])}}, instr[15:0]} : rt),
		.alucontrol(control[`CONTROL_ALUCONTROL]),
		.aluout(aluout)
	);
	
	// shifter unit
	wire [31:0] shiftout;
	tiger_shifter shift(
		.src(rt),
		.amt(control[`CONTROL_ALUCONTROL_VARIABLE] ? rs[4:0] : instr[10:6]),
		.dir(control[`CONTROL_ALUCONTROL_RIGHT]),
		.alusigned(!control[`CONTROL_ALUCONTROL_UNSIGNED]),
		.shifted(shiftout)
	);
	
	// set the countdown
	wire [3:0] newcountdown1=reset ||
                             control[`CONTROL_ALUCONTROL] == `ALU_MTHI ||
                             control[`CONTROL_ALUCONTROL] == `ALU_MTLO                  ?   4'd0  // MTHI and MTLO only require a single cycle
                            :control[`CONTROL_ALUCONTROL] == `ALU_MULT ||
                             control[`CONTROL_ALUCONTROL] == (`ALU_MULT | `ALU_UNSIGNED)?	4'd4  // MULT requires 5 cycles
                            :control[`CONTROL_ALUCONTROL] == `ALU_DIV ||
                             control[`CONTROL_ALUCONTROL] == (`ALU_DIV | `ALU_UNSIGNED) ?	4'd15 // division requires 16 cycles to complete
                            :countdown>0												?	countdown-4'd1
                            :																4'd0;

	// if the pipeline is stalled we only decrement the countdown (if allowed)
	wire [3:0] newcountdown2=reset			?	4'd0
							:countdown>0	?	countdown-4'd1
							:					4'd0;
	
	// set the LO register		
	wire [31:0] newlow1= reset											?	32'd0
						:control[`CONTROL_ALUCONTROL] == `ALU_MTLO		?	rs			// move to LO
						:countdown==1									?
						 divmul == 1									?   (!source)?mults[31:0]:multu[31:0]
						:(!source ? divLO : divuLO)
						:													low;

	// the quotient of the division - used only when the pipeline is stalled
	wire [31:0] newlow2= reset											?	32'd0
						:countdown==1									?
						 divmul == 1									?   (!source)?mults[31:0]:multu[31:0]
						:(!source ? divLO : divuLO)
						:													low;
	
	// set the HI register 
	wire [31:0] newhigh1=reset											?	32'd0
						:control[`CONTROL_ALUCONTROL] == `ALU_MTHI		?	rs			// move to LO
						:countdown==1									?
						 divmul == 1									?   (!source)?mults[63:32]:multu[63:32]
						:(!source ? divHI : divuHI)
						:													high;

	// the remainder of the division - used only when the pipeline is stalled
	wire [31:0] newhigh2=reset											?	32'd0
						:countdown==1									?
						 divmul == 1									?   (!source)?mults[63:32]:multu[63:32]
						:(!source ? divHI : divuHI)
						:													high;

	always @(posedge clk)
	begin
		countdown <= !stall ? newcountdown1 : newcountdown2;
		low <= !stall ? newlow1 : newlow2;
		high <= !stall ? newhigh1 : newhigh2;

		if( reset || clear)	begin
			instrMA 	 <= 0;
			controlMA 	 <= 0;
			executeoutMA <= 0;
			branchoutMA  <= 0;
		end else if( stall ) begin
			//stall instrWB
			//stall controlWB
			//stall executeoutWB
			//stall branchoutWB
		end	else begin
			instrMA <= instr;
			controlMA <= control;
			executeoutMA <=  control[`CONTROL_ALUCONTROL] == `ALU_MUL		?	mults[31:0] // lower 32 bits of the multiplication
							:&control[`CONTROL_ALUCONTROL_SHIFT]			?	shiftout	// output of the shift unit
							:control[`CONTROL_ALUCONTROL] == `ALU_MFHI		?	high		// MFHI => contents of the HI register
							:control[`CONTROL_ALUCONTROL] == `ALU_MFLO		?	low			// MHLO => contents of the LO register
							:control[`CONTROL_MEML]||control[`CONTROL_MEMR]	?	rt			// second operand
							:control[`CONTROL_LINK] 						?   branchout   // return address for link operation
							:control[`CONTROL_COPREAD]						?   CPOut
							:control[`CONTROL_COPWRITE]						?   rt
							:													aluout;		// otherwise give the output of the ALU
			branchoutMA <= branchout;
			//bottomaddressMA <= aluout[1:0];
			
			if( control[`CONTROL_ALUCONTROL] == `ALU_DIV || control[`CONTROL_ALUCONTROL] == `ALU_MULT) begin
				source <= 0; // signed flag
			end	else if( control[`CONTROL_ALUCONTROL] == (`ALU_DIV | `ALU_UNSIGNED) || control[`CONTROL_ALUCONTROL] == (`ALU_MULT | `ALU_UNSIGNED)) begin
				source <= 1; // unsigned flag
			end
			if(control[`CONTROL_ALUCONTROL] == `ALU_DIV || control[`CONTROL_ALUCONTROL] == (`ALU_DIV | `ALU_UNSIGNED)) begin
				divmul <= 0; // division flag
			end	else if(control[`CONTROL_ALUCONTROL] == `ALU_MULT || control[`CONTROL_ALUCONTROL] == (`ALU_MULT | `ALU_UNSIGNED)) begin
				divmul <= 1; // multiplication flag
			end
		end
	end
	
	// we stall the pipeline if
	assign stallRq =
	(	
		// the current instruction is MFHI or MFLO and the countdown has yet to reach 0
		// (i.e. the contents of the registers are not valid at this point in time)
		(control[`CONTROL_ALUCONTROL] == `ALU_MFHI || control[`CONTROL_ALUCONTROL] == `ALU_MFLO)
		&& countdown > 0
	)
	|| // OR
	(!memCanRead && control[`CONTROL_MEMREAD]) //Can't read currently and we want to read
	|| // OR
	(!memCanWrite && control[`CONTROL_MEMWRITE]) //Can't write currently and we want to write
	||
	(!canICacheFlush && control[`CONTROL_ICACHEFLUSH])
	||
	(!canDCacheFlush && control[`CONTROL_DCACHEFLUSH]);
	

	// assign outputs appropriately
	assign memread		= clear || !memCanRead ? 1'b0 : control[`CONTROL_MEMREAD];
	assign memwrite		= clear || !memCanWrite ? 1'b0 : control[`CONTROL_MEMWRITE];
	assign mem16		= clear ? 1'b0 : control[`CONTROL_MEM16];
	assign mem8			= clear ? 1'b0 : control[`CONTROL_MEM8];
	assign memaddress	= clear ? 0 : aluout;
	assign memwritedata = clear ? 0 : rt;
	assign iCacheFlush  = clear || !canICacheFlush ? 1'b0 : control[`CONTROL_ICACHEFLUSH];
	assign dCacheFlush  = clear || !canDCacheFlush ? 1'b0 : control[`CONTROL_DCACHEFLUSH];
endmodule
