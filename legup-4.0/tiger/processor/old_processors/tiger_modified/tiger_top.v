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

module tiger_top (
	input clk,
	input reset,
	
	//signals going to Data Cache

	output wire [71:0] aso_TigertoCache_data,
	input [39:0] asi_CachetoTiger_data,

	output avm_instructionMaster_read,
	output [31:0]avm_instructionMaster_address,
	input [31:0]avm_instructionMaster_readdata,
	output avm_instructionMaster_beginbursttransfer,
	output[2:0]avm_instructionMaster_burstcount,
	input avm_instructionMaster_waitrequest,
	input avm_instructionMaster_readdatavalid
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
	wire irq;
	wire [5:0]irq_number;

	wire avs_debugSlave_write;
	wire avs_debugSlave_writedata;
	wire avs_debugSlave_irq;

	wire stall_cpu;

	assign aso_TigertoCache_data[0] = memread;
	assign aso_TigertoCache_data[1] = memwrite;
	assign aso_TigertoCache_data[33:2] = memaddress;
	assign aso_TigertoCache_data[65:34] = memwritedata;
	assign aso_TigertoCache_data[66] = dCacheFlush;
	assign aso_TigertoCache_data[67] = mem8;
	assign aso_TigertoCache_data[68] = mem16;
	assign aso_TigertoCache_data[71:69] = 3'b0;
 
	assign memreaddata = asi_CachetoTiger_data[31:0];
	assign memCanRead = asi_CachetoTiger_data[32];
	assign memCanWrite = asi_CachetoTiger_data[33];
	assign canDCacheFlush = asi_CachetoTiger_data[34];
	assign dCacheStall = asi_CachetoTiger_data[35];
	assign stall_cpu = asi_CachetoTiger_data[36];
	assign irq = 0;
	assign irq_number = 0;

	// synthesis translate_off
/*
	reg eight;
	initial eight=1'b0;
	always @(pc) begin
		if (pc == 'ha88) begin
			if (eight == 1'b0) eight <= 1'b1;
			else		   $finish;
		end
	end*/

	reg start;
	reg [63:0] count;
	initial start = 1'b0;
	initial count = 64'd0;
	always @(posedge clk) begin
		if (pc == 32'h800000) begin
			start <= 1'b1;
//			$display("START!");
		end

		if (start) begin
			count <= count + 1;
		end
		
//		$monitor ("counter = %d", count);
		if (pc == 32'hac0) begin
			if (start) begin
			$display("counter = %d", count);
			$finish;
			end  
		end
	end
	// synthesis translate_on
	
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
		.stall_cpu(stall_cpu),
		.iStall(iCacheStall || !insValid),
		.dStall(dCacheStall),
		.irq(irq),
		.irqNumber(irq_number),
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
		.canDCacheFlush(canDCacheFlush),
		.memzerofill()
	);							   
   
	ins_cache InsCache (
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
		.avm_dataMaster_readdatavalid(avm_instructionMaster_readdatavalid),
		.avm_dataMaster_beginbursttransfer(avm_instructionMaster_beginbursttransfer),
		.avm_dataMaster_burstcount(avm_instructionMaster_burstcount),

		.canRead(),
		.canWrite(),
		.avm_dataMaster_write(),
		.avm_dataMaster_writedata(),
		.avm_dataMaster_byteenable()
	);
	
endmodule
