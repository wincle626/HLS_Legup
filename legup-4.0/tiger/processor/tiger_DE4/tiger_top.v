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
`include "cache_parameters.v"

module tiger_top # (
	parameter prof_param_N2 = 8,
	parameter prof_param_S2 = 5,
	parameter prof_param_CW = 32
	) (
	input clk,
	input reset,
	
	//signals going to Data Cache
	output wire  [31:0] avm_CACHE_address,
	output wire         avm_CACHE_read,
	output wire         avm_CACHE_write,
	output wire [127:0] avm_CACHE_writedata,
	input       [127:0] avm_CACHE_readdata,
	input               avm_CACHE_waitrequest,

	input [7:0] asi_PROC_data,

	output [31:0] avm_procMaster_address,
	output        avm_procMaster_read,
	output        avm_procMaster_write,
	output [31:0] avm_procMaster_writedata,	
	output [ 3:0] avm_procMaster_byteenable,
	input  [31:0] avm_procMaster_readdata,
	input         avm_procMaster_waitrequest,
	input         avm_procMaster_readdatavalid,

	output        avm_instructionMaster_read,
	output [31:0] avm_instructionMaster_address,
	input  [`SDRAM_WIDTH-1:0]avm_instructionMaster_readdata,
	output        avm_instructionMaster_beginbursttransfer,
	output [`IBURSTCOUNTWIDTH-1:0]  avm_instructionMaster_burstcount,
	input         avm_instructionMaster_waitrequest,
	input         avm_instructionMaster_readdatavalid,
	
	// Profiling Status Signals
	output coe_exe_start,
	output coe_exe_end,
	
	// Debug ports
	input  [ 2:0] coe_debug_select,
	output [17:0] coe_debug_lights,
	
	// Avalon-MM Slave Interface
	input                     avs_leapSlave_chipselect,
	input [prof_param_N2-1:0] avs_leapSlave_address,
	input                     avs_leapSlave_read,
	input                     avs_leapSlave_write,
	input              [31:0] avs_leapSlave_writedata,
	output             [31:0] avs_leapSlave_readdata
);
	
	wire iCacheStall;
	wire dCacheStall;
	/*wire iCacheFlush;
	wire dCacheFlush;
	wire canICacheFlush;
	wire canDCacheFlush;*/
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
	
	// Soft Reset to Tiger (from slave_handler)
	wire tiger_soft_reset;

	assign bypassCache = memaddress[31];

	assign avm_CACHE_address = 0;
	assign avm_CACHE_read = memread && !bypassCache;
	assign avm_CACHE_write = memwrite && !bypassCache;
	assign avm_CACHE_writedata[31:0] = memaddress;
	assign avm_CACHE_writedata[95:32] = memwritedata;
	assign avm_CACHE_writedata[96] = mem8;
	assign avm_CACHE_writedata[97] = mem16;
	assign avm_CACHE_writedata[98] = 1'b0; //mem64 signal which doesn't exist for processor
	//assign avm_CACHE_writedata[99] = dCacheFlush;
	assign avm_CACHE_writedata[99] = 1'b0;
	assign avm_CACHE_writedata[102:100] = 0;

	//to indicate it's the processor, when processor port is shared with accelerators
	assign avm_CACHE_writedata[103] = 1'b1;
	assign avm_CACHE_writedata[127:104] = 0;

	assign memreaddata = avm_procMaster_readdatavalid? avm_procMaster_readdata : avm_CACHE_readdata[31:0];
	assign memCanRead = asi_PROC_data[1];
	assign memCanWrite = asi_PROC_data[2];
	//assign canDCacheFlush = asi_PROC_data[3];

	//assign dCacheStall = asi_PROC_data[4] || avalon_stall;	
	//avm_CACHE_waitrequest is used for when processor port to cache is shared with other accelerators
	assign dCacheStall = asi_PROC_data[4] || avalon_stall || avm_CACHE_waitrequest; 
	assign stall_cpu = asi_PROC_data[0];

	assign irq = 0;
	assign irq_number = 0;
	
	tiger_debug debug_controller (
		.clk(clk),
		.reset_n(!reset),
		.avs_debugSlave_write(avs_debugSlave_write),
		.avs_debugSlave_writedata(avs_debugSlave_writedata),
		.avs_debugSlave_irq(avs_debugSlave_irq)
	);
	
	tiger_tiger core (
		.clk(clk),
		.reset(reset | tiger_soft_reset),
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
		/*.iCacheFlush(iCacheFlush),
		.dCacheFlush(dCacheFlush),
		.canICacheFlush(canICacheFlush),
		.canDCacheFlush(canDCacheFlush),*/
		.memzerofill()
	);							   
   
	ins_cache InsCache (
		.clk(clk),
		.reset_n(!reset),

		//.PROC0_memRead(!iCacheFlush),
		.PROC0_memRead(1'b1),
		.PROC0_memAddress(pc),
		.PROC0_memReadData(ins),
		.PROC0_readDataValid(insValid),

		//.PROC0_canFlush(canICacheFlush),

		.PROC0_stall(iCacheStall),
		//.PROC0_flush(iCacheFlush),

		.avm_dataMaster0_read(avm_instructionMaster_read),
		.avm_dataMaster0_address(avm_instructionMaster_address),
		.avm_dataMaster0_readdata(avm_instructionMaster_readdata),
		.avm_dataMaster0_waitrequest(avm_instructionMaster_waitrequest),
		.avm_dataMaster0_readdatavalid(avm_instructionMaster_readdatavalid),
		.avm_dataMaster0_beginbursttransfer(avm_instructionMaster_beginbursttransfer),
		.avm_dataMaster0_burstcount(avm_instructionMaster_burstcount)
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

	// Leap MM Interface
	wire [prof_param_N2-1:0] lp_mm_addr;
	wire                     lp_mm_wren;
	wire              [31:0] lp_mm_wrdata;
	wire              [31:0] lp_mm_rddata;
	// Pipeline pc in order to make it aligned with processor's execution stage
	// Pipeline instr one cycle less than pc so that they are correspond to each other
	reg [31:0] pc_r, pc_rr, pc_rrr;
	reg [31:0] ins_r, ins_rr;
	reg insValid_r, insValid_rr;

`ifdef PROFILER_ON
	always @ (posedge clk) begin
		if (reset) begin
			pc_r   <= 32'b0;
			pc_rr  <= 32'b0;
			pc_rrr <= 32'b0;
			ins_r  <= 32'b0;
			ins_rr <= 32'b0;
			insValid_r  <= 1'b0;
			insValid_rr <= 1'b0;
		end
		else begin
			pc_r <= pc;
			if (|(pc_r ^ pc)) begin
				ins_r  <= ins;
				ins_rr <= ins_r;
				insValid_r  <= insValid;
				insValid_rr <= insValid_r;
				pc_rr  <= pc_r;
				pc_rrr <= pc_rr;
			end
		end
	end
	
	LeapProfiler profiler (
		.clk         (clk),
		.reset       (reset),
		.pc_in       (pc_rrr),
		.instr_in    (ins_rr),
		.insValid_in (insValid_rr),
		.istall_in   (iCacheStall || !insValid),
		.dstall_in   (dCacheStall ),

		.exe_start_as (coe_exe_start),
		.exe_end_as   (coe_exe_end),

		.lp_mm_addr   (lp_mm_addr),
		.lp_mm_wren   (lp_mm_wren),
		.lp_mm_wrdata (lp_mm_wrdata),
		.lp_mm_rddata (lp_mm_rddata)
	);
	defparam profiler. N  = {1'b1, {prof_param_N2{1'b0}} };
	defparam profiler. N2 = prof_param_N2;
	defparam profiler. S  = {1'b1, {prof_param_S2{1'b0}} };
	defparam profiler. S2 = prof_param_S2;
	defparam profiler. CW = prof_param_CW;

	assign coe_debug_lights [17] = tiger_soft_reset;
	assign coe_debug_lights [16] = coe_debug_select[2] ? dCacheStall : iCacheStall;
	assign coe_debug_lights [15:0] = coe_debug_select[1:0]==2'b00 ? pc_rrr[15:0] : (
	                                coe_debug_select[1:0]==2'b01 ? pc_rrr[31:16] : (
	                                    coe_debug_select[1:0]==2'b10 ? ins_rr[15:0] : ins_rr[31:16]
	                                )
	                             );
`endif

	tiger_leap_slave_handler slave_handler (
		.clk (clk),
		.reset (reset),
		.avs_leapSlave_chipselect (avs_leapSlave_chipselect),
		.avs_leapSlave_address    (avs_leapSlave_address),
		.avs_leapSlave_read       (avs_leapSlave_read),
		.avs_leapSlave_write      (avs_leapSlave_write),
		.avs_leapSlave_writedata  (avs_leapSlave_writedata),
		.avs_leapSlave_readdata   (avs_leapSlave_readdata),
		.lp_mm_addr   (lp_mm_addr),
		.lp_mm_wren   (lp_mm_wren),
		.lp_mm_wrdata (lp_mm_wrdata),
		.lp_mm_rddata (lp_mm_rddata),
		.exe_start_as (coe_exe_start),
		.exe_end_as   (coe_exe_end),
		.tiger_soft_reset (tiger_soft_reset)
	);
	defparam slave_handler .N2 = prof_param_N2;

	// synthesis translate_off
	debug_tiger_leap debug (
		.clk (clk),
		.iCacheStall (iCacheStall),
		.dCacheStall (dCacheStall),
		.pc     (pc),
		.pc_r   (pc_r),
		.pc_rr  (pc_rr),
		.pc_rrr (pc_rrr),
		.ins    (ins),
		.ins_r  (ins_r),
		.ins_rr (ins_rr),
		.insValid    (insValid),
		.insValid_r  (insValid_r),
		.insValid_rr (insValid_rr)
	);
	// synthesis translate_on


endmodule
