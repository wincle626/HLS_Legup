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
	output wire [31:0] avm_CACHE_address,
	output wire avm_CACHE_read,
	output wire avm_CACHE_write,
	output wire [127:0]avm_CACHE_writedata,
	input  [127:0]avm_CACHE_readdata,
	input avm_CACHE_waitrequest,

	input [7:0] asi_PROC_data,

	output [31:0]avm_procMaster_address,
	output avm_procMaster_read,
	output avm_procMaster_write,
	output [31:0] avm_procMaster_writedata,	
	output [3:0] avm_procMaster_byteenable,
	input [31:0]avm_procMaster_readdata,
	input avm_procMaster_waitrequest,
	input avm_procMaster_readdatavalid,

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
	wire cacheHit;
	wire bypassCache;
	wire avalon_stall;

	assign bypassCache = memaddress[31];

	assign avm_CACHE_address = 0;
	assign avm_CACHE_read = memread && !bypassCache;
	assign avm_CACHE_write = memwrite && !bypassCache;
	assign avm_CACHE_writedata[31:0] = memaddress;
	assign avm_CACHE_writedata[95:32] = memwritedata;
	assign avm_CACHE_writedata[96] = mem8;
	assign avm_CACHE_writedata[97] = mem16;
	assign avm_CACHE_writedata[98] = 1'b0; //mem64 signal which doesn't exist for processor
	assign avm_CACHE_writedata[99] = dCacheFlush;
	assign avm_CACHE_writedata[102:100] = 0;

	//to indicate it's the processor, when processor port is shared with accelerators
	assign avm_CACHE_writedata[103] = 1'b1;
	assign avm_CACHE_writedata[127:104] = 0;

	assign memreaddata = avm_procMaster_readdatavalid? avm_procMaster_readdata : avm_CACHE_readdata[31:0];
	assign memCanRead = asi_PROC_data[1];
	assign memCanWrite = asi_PROC_data[2];
	assign canDCacheFlush = asi_PROC_data[3];

	//assign dCacheStall = asi_PROC_data[4] || avalon_stall;	
	//avm_CACHE_waitrequest is used for when processor port to cache is shared with other accelerators
	assign dCacheStall = asi_PROC_data[4] || avalon_stall || avm_CACHE_waitrequest; 
	assign stall_cpu = asi_PROC_data[0];

	assign irq = 0;
	assign irq_number = 0;

	// Used for Debugging
	// synthesis translate_off
	reg start;
	reg [63:0] count;
	reg [9:0] iCacheStall_count;
	reg [9:0] dCacheStall_count;

	initial start = 1'b0;
	initial count = 64'd0;
	initial iCacheStall_count = 10'd0;
	initial dCacheStall_count = 10'd0;
	always @(posedge clk) begin

		//checking if PC goes undefined, if so quit simulation
		if (^pc === 1'bX) begin
			$display("PC is going undefined!\n");
			$finish;
		end

		//checking if icache gets stuck (should never be stalled for more than 1,000 cycles at once), if so quit simulation
		if (iCacheStall == 1'b1) begin
			iCacheStall_count <= iCacheStall_count + 1;
		end
		else begin
			iCacheStall_count <= 10'd0;
		end

		if (iCacheStall_count == 10'd1000) begin
//			$display("Instruction Cache is Stuck!\n");
			//$finish;
		end

		//checking if icache gets stuck (should never be stalled for more than 1,000 cycles at once), if so quit simulation
		if (dCacheStall == 1'b1) begin
			dCacheStall_count <= dCacheStall_count + 1;
		end
		else begin
			dCacheStall_count <= 10'd0;
		end

		if (dCacheStall_count == 10'd1000) begin
			$display("Data Cache is Stuck!\n");
//			$finish;
		end

		//counting execution cycles 
		if (pc == 32'h800000) begin
			start <= 1'b1;
		end
		if (start) begin
			count <= count + 1;
		end
//		if (pc == 32'hac0) begin
		if (pc == 32'h10) begin
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
		.memAddress(pc),
		.memReadData(ins),
		.readDataValid(insValid),

		.canFlush(canICacheFlush),

		.stall(iCacheStall),
		.flush(iCacheFlush),

		.avm_dataMaster_read(avm_instructionMaster_read),
		.avm_dataMaster_address(avm_instructionMaster_address),
		.avm_dataMaster_readdata(avm_instructionMaster_readdata),
		.avm_dataMaster_waitrequest(avm_instructionMaster_waitrequest),
		.avm_dataMaster_readdatavalid(avm_instructionMaster_readdatavalid),
		.avm_dataMaster_beginbursttransfer(avm_instructionMaster_beginbursttransfer),
		.avm_dataMaster_burstcount(avm_instructionMaster_burstcount)
	);

	//this module is used to control when processor needs to access memory-mapped components
	tiger_avalon avalon_controller (
		.clk(clk),
		.reset(reset),
	
		.memaddress(memaddress),
		.memread(memread && bypassCache),
		.memwrite(memwrite && bypassCache),
		.memwritedata(memwritedata),
		.mem8(mem8),
		.mem16(mem16),
		.avalon_stall(avalon_stall),

		.avm_procMaster_address(avm_procMaster_address),
		.avm_procMaster_read(avm_procMaster_read),
		.avm_procMaster_write(avm_procMaster_write),
		.avm_procMaster_writedata(avm_procMaster_writedata),	
		.avm_procMaster_byteenable(avm_procMaster_byteenable),
		.avm_procMaster_readdata(avm_procMaster_readdata),
		.avm_procMaster_waitrequest(avm_procMaster_waitrequest),
		.avm_procMaster_readdatavalid(avm_procMaster_readdatavalid)
	);

	
endmodule
