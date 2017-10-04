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

///////////////////////////////////////////////////////////
// MIPS SOFT PROCESSOR 
// TOP LEVEL
// 
// 5-stage pipeline:
// Fetch -> Decode -> Execute  -> MemoryAccess -> RegisterWriteBack
///////////////////////////////////////////////////////////

`include "tiger_defines.v"

module tiger_tiger(
	input clk,				// clock signal
	input reset,			// reset signal
	input iStall,			// instruction stall signal
	input dStall,			// data stall signal
	input stall_cpu, 		// stall cpu for accelerator

	/*output iCacheFlush,
	output dCacheFlush,
	
	input canICacheFlush,
	input canDCacheFlush,*/
	
	input irq,				// interrupt request signal
	input [5:0]irqNumber,
	
	output [31:0] pc,		// program counter
	input [31:0] instrF,	// fetched instruction

	output memwrite,memread,mem16,mem8,memzerofill, // memory access mode outputs
	output [31:0] memaddress,memwritedata,			// memory address and data outputs
	input [31:0] memreaddata,						// memory data input
	input memCanRead,
	input memCanWrite
);
	
	wire stallRqEx;
	wire exception;
	
	wire clearDe;
	wire stallDe;
	wire clearEx;
	wire stallEx;
	wire clearMA;
	wire stallMA;
	wire clearWB;
	wire stallWB;
	
	wire [31:0] instrDe;
	wire [`CONTROL_WIDTH] controlDe;

	wire [31:0] instrEx;
	wire [`CONTROL_WIDTH] controlEx;
	wire [31:0] branchoutEx;
	wire [31:0] rsEx;
	wire [31:0] rtEx;
	wire [31:0] CPOutEx;

	wire [31:0] instrMA;
	wire [`CONTROL_WIDTH] controlMA;
	wire [31:0] executeoutMA;
	wire [31:0] branchoutMA;
	wire [1:0] bottomaddressMA;
	
	wire writeRegEnMA;
	wire writeRegEnCopMA;
	wire [`REGNUM_WIDTH] writeRegNumMA;
	wire [31:0]writeRegDataMA;
	
	wire [31:0] instrWB;
	wire [`CONTROL_WIDTH] controlWB;
	wire [31:0] MAOutWB;
	wire [31:0] branchoutWB;
	wire [31:0] writeRegDataWB;
	wire [`REGNUM_WIDTH]writeRegNumWB;
	wire writeRegEnWB;
	wire writeRegEnCopWB;
		
	tiger_stalllogic sl(
		.controlDe(controlDe),
		.controlEx(controlEx),
		.instrDe(instrDe),
		.instrEx(instrEx),
		
		.writeRegNumMA(writeRegNumMA),
		.writeRegEnMA(writeRegEnMA),
		.writeRegEnCopMA(writeRegEnCopMA),
		
		.writeRegNumWB(writeRegNumWB),
		.writeRegEnWB(writeRegEnWB),
		.writeRegEnCopWB(writeRegEnCopWB),
		
		.stallRqEx(stallRqEx),
		.exception(exception),
		
		.iStall(iStall),
		.dStall(dStall),
		.stall_cpu(stall_cpu),
		
		.clearDe(clearDe), 
		.stallDe(stallDe),
		.clearEx(clearEx),
		.stallEx(stallEx),
		.clearMA(clearMA),
		.stallMA(stallMA),
		.clearWB(clearWB),
		.stallWB(stallWB)
	);
	
	// fetch stage
	tiger_fetch fe(
		.clk(clk),
		.reset(reset),
		.stall(stallDe),
		.clear(clearDe),
		
		.instr(instrF),

		.instrDE(instrDe)
	);

	// decode stage
	tiger_decode de(
		.clk(clk),
		.reset(reset),
		.stall(stallEx),
		.clear(clearEx),

		.irq(irq),
		.irqNumber(irqNumber),

		.instr(instrDe),
		.controlDe(controlDe),
		
		.writeRegEnWB(writeRegEnWB),
		.writeRegEnCopWB(writeRegEnCopWB),
		.writeRegNumWB(writeRegNumWB),
		.writeRegDataWB(writeRegDataWB),
		
		.exception(exception),
		
		.instrEx(instrEx),
		.controlEx(controlEx),
		.rsEx(rsEx),
		.rtEx(rtEx),
		.CPOutEx(CPOutEx),
		
		.branchoutEx(branchoutEx),

		.nextpc(pc)
	);
	
	// feed forward path for first operand
	wire [31:0]rsExFF;
	tiger_ff ff_for_rs(
		.regnum(instrEx[25:21]),
		.writereg1(writeRegNumMA),
		.writereg2(writeRegNumWB),
		.writeregen1(writeRegEnMA),
		.writeregen2(writeRegEnWB),
		.regdata(rsEx),
		.writeregdata1(writeRegDataMA),
		.writeregdata2(writeRegDataWB),
		.out(rsExFF)
	);
	
	// feed forward path for second operand
	wire [31:0]rtExFF;
	tiger_ff ff_for_rt(
		.regnum(instrEx[20:16]),
		.writereg1(writeRegNumMA),
		.writereg2(writeRegNumWB),
		.writeregen1(writeRegEnMA),
		.writeregen2(writeRegEnWB),
		.regdata(rtEx),
		.writeregdata1(writeRegDataMA),
		.writeregdata2(writeRegDataWB),
		.out(rtExFF)
	);
		
	// excecute stage
	tiger_execute ex(
		.clk(clk),
		.reset(reset),
		.stall(stallMA),
		.clear(clearMA),
		
		.instr(instrEx),
		.control(controlEx),
		.rs(rsExFF),
		.rt(rtExFF),
		.branchout(branchoutEx),
		.CPOut(CPOutEx),
		
		.instrMA(instrMA),
		.controlMA(controlMA),
		.executeoutMA(executeoutMA),
		.branchoutMA(branchoutMA),
		//.bottomaddressMA(bottomaddressMA),
		
		.stallRq(stallRqEx),
		
		.memread(memread),
		.mem16(mem16),
		.mem8(mem8),
		.memwrite(memwrite),
		.memaddress(memaddress),
		.memwritedata(memwritedata),
		.memCanRead(memCanRead),
		.memCanWrite(memCanWrite)
		//.iCacheFlush(iCacheFlush),
		//.dCacheFlush(dCacheFlush),
		//.canICacheFlush(canICacheFlush),
		//.canDCacheFlush(canDCacheFlush)
	);

	// memory access stage
	tiger_memoryaccess ma(
		.clk(clk),
		.reset(reset),
		.clear(clearWB),
		.stall(stallWB),
		
		//Pipeline registers in
		.instr(instrMA), //instruction
		.control(controlMA), //control signals
		.executeout(executeoutMA), //Output of the execute stage
		.branchout(branchoutMA), //PC value for use with b w/ link
		//Bottoms 2 bits of address, used if we're reading from memory
		//using a left or right hand read instruction
		//.bottomaddress(bottomaddressMA), 
		
		
		//Pipeline registers out
		.controlWB(controlWB),
		.MAOutWB(MAOutWB),
		.branchoutWB(branchoutWB),
		.instrWB(instrWB),
		
		//Read data from memory
		.memreaddata(memreaddata),
		
		//Signals used for controlling feed-forward and pipeline stall
		.writeRegEn(writeRegEnMA), //True if we will write to a register (in the WB stage)
		.writeRegEnCop(writeRegEnCopMA),
		.writeRegNum(writeRegNumMA), //Which register we will write to
		.writeRegData(writeRegDataMA) //What we would write to the register
	);
	
	// writeback stage
	tiger_writeback wb(
		.clk(clk),
		
		//Pipeline registers in		
		.instr(instrWB), //instruction
		.control(controlWB), //control signal
		.branchout(branchoutWB), //PC value for use with b w/ link
		.MAOut(MAOutWB), //Output of the memory access stage
		
		//Register write control
		.writeRegEn(writeRegEnWB), //True if we want to write to a register
		.writeRegEnCop(writeRegEnCopWB),
		.writeRegNum(writeRegNumWB), //Which register we want to write to
		.writeRegData(writeRegDataWB) //What data we want to write to it
	);

	
endmodule 
