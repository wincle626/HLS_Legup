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

`include "../../profiler/prof_chooser.v"

module tiger_top (
	input clk,
	input reset,
	
	output avm_dataMaster_read,
	output avm_dataMaster_write,
	output [31:0]avm_dataMaster_address,
	output [31:0]avm_dataMaster_writedata,
	output [3:0]avm_dataMaster_byteenable,
	input [31:0]avm_dataMaster_readdata,
	input avm_dataMaster_waitrequest,
	input avm_dataMaster_readdatavalid,
	input avm_dataMaster_irq,
	input [5:0]avm_dataMaster_irqnumber,
	
	output avm_instructionMaster_read,
	output [31:0]avm_instructionMaster_address,
	input [31:0]avm_instructionMaster_readdata,
	input avm_instructionMaster_waitrequest,
	input avm_instructionMaster_readdatavalid,
	
	input avs_debugSlave_write,
    input avs_debugSlave_writedata,
    output avs_debugSlave_irq
);
	
	wire iCacheStall;
	wire dCacheStall;
	wire iCacheFlush;
	wire dCacheFlush;
	wire canICacheFlush;
	wire canDCacheFlush;
	wire [31:0]pc;
	wire [31:0]ins;
	wire memwrite;
	wire memread;
	wire mem16;
	wire mem8;
	wire [31:0]memaddress;
	wire [31:0]memwritedata;
	wire [31:0]memreaddata;
	wire memCanRead;
	wire memCanWrite;
	wire insValid;
		
	tiger_debug debug_controller (
		.clk(clk),
		.reset_n(!reset),
		.avs_debugSlave_write(avs_debugSlave_write),
		.avs_debugSlave_writedata(avs_debugSlave_writedata),
		.avs_debugSlave_irq(avs_debugSlave_irq)
	);
	
	tiger_tiger core (
		.clk(clk),
		.reset(reset),
		.iStall(iCacheStall || !insValid),
		.dStall(dCacheStall),
		.irq(avm_dataMaster_irq),
		.irqNumber(avm_dataMaster_irqnumber),
		.pc(pc),		// program counter
		.instrF(ins),	// fetched instruction

		.memwrite(memwrite),
		.memread(memread),
		.mem16(mem16),
		.mem8(mem8), 
		// memory access mode outputs
		.memaddress(memaddress),
		.memwritedata(memwritedata),			// memory address and data outputs
		.memreaddata(memreaddata),
		.memCanRead(memCanRead),
		.memCanWrite(memCanWrite),
		.iCacheFlush(iCacheFlush),
		.dCacheFlush(dCacheFlush),
		.canICacheFlush(canICacheFlush),
		.canDCacheFlush(canDCacheFlush)
	);							   
	   
	Cache DataCache (
		.clk(clk),
		.reset_n(!reset),
		.memRead(memread),
		.memWrite(memwrite),
		.memAddress(memaddress),
		.memWriteData(memwritedata),
		.memReadData(memreaddata),
		.canRead(memCanRead),
		.canWrite(memCanWrite),
		.canFlush(canDCacheFlush),

		.mem8(mem8),
		.mem16(mem16),

		.stall(dCacheStall),
		.flush(dCacheFlush),

		.avm_dataMaster_read(avm_dataMaster_read),
		.avm_dataMaster_write(avm_dataMaster_write),
		.avm_dataMaster_address(avm_dataMaster_address),
		.avm_dataMaster_writedata(avm_dataMaster_writedata),
		.avm_dataMaster_byteenable(avm_dataMaster_byteenable),
		.avm_dataMaster_readdata(avm_dataMaster_readdata),
		.avm_dataMaster_waitrequest(avm_dataMaster_waitrequest),
		.avm_dataMaster_readdatavalid(avm_dataMaster_readdatavalid)
	);
				   				   
	Cache InsCache (
		.clk(clk),
		.reset_n(!reset),

		.memRead(!iCacheFlush),
		.memWrite(1'b0),
		.memAddress(pc),
		.memReadData(ins),
		.memWriteData(32'b0),
		.readDataValid(insValid),

		.canFlush(canICacheFlush),

		.mem8(1'b0),
		.mem16(1'b0),

		.stall(iCacheStall),
		.flush(iCacheFlush),

		.avm_dataMaster_read(avm_instructionMaster_read),
		.avm_dataMaster_address(avm_instructionMaster_address),
		.avm_dataMaster_readdata(avm_instructionMaster_readdata),
		.avm_dataMaster_waitrequest(avm_instructionMaster_waitrequest),
		.avm_dataMaster_readdatavalid(avm_instructionMaster_readdatavalid)
	);
	
		
	// instantiate a profiler
	profiler_chooser prof (
		.clk(clk),
		.reset(reset),
		.pc(pc),
		.instr(ins),
		.stall(iCacheStall|dCacheStall),
		
		// Avalon Profile Master ports
		.avm_profileMaster_read(avm_profileMaster_read),
		.avm_profileMaster_write(avm_profileMaster_write),
		.avm_profileMaster_address(avm_profileMaster_address),
		.avm_profileMaster_writedata(avm_profileMaster_writedata),
		.avm_profileMaster_byteenable(avm_profileMaster_byteenable),
		.avm_profileMaster_readdata(avm_profileMaster_readdata),
		.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest),
		.avm_profileMaster_readdatavalid(avm_profileMaster_readdatavalid)
	);
	
endmodule