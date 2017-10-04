// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on


// enable profiling
// `define PROFILER_ON 1'b1

module LeapTop # (
	parameter prof_param_N2 = 8,
	parameter prof_param_S2 = 5,
	parameter prof_param_CW = 32,

    parameter STARTING_PC    = 32'h0080_0020
) (
	input clk,
	input reset,

	// Slave Port - From CPU
	input	[31: 0]	avs_from_cpu_address,
	input			avs_from_cpu_read,
	input			avs_from_cpu_write,
	input	[31: 0]	avs_from_cpu_writedata,	
	input	[ 3: 0]	avs_from_cpu_byteenable,
	output	[31: 0]	avs_from_cpu_readdata,
	output			avs_from_cpu_waitrequest,
	output			avs_from_cpu_readdatavalid,

	// Master Port - To I$/Memory
	output	[31: 0]	avs_to_memory_address,
	output			avs_to_memory_read,
	output			avs_to_memory_write,
	output	[31: 0]	avs_to_memory_writedata,	
	output	[ 3: 0]	avs_to_memory_byteenable,
	input	[31: 0]	avs_to_memory_readdata,
	input			avs_to_memory_waitrequest,
	input			avs_to_memory_readdatavalid,

    // Leap Reset Port
    output          tiger_soft_reset,

	// Profiling Status Signals
	output coe_exe_start,
	output coe_exe_end,

    // Debug ports
	input  [ 2:0] coe_debug_select,
	output [17:0] coe_debug_lights,

    // Avalon-MM Slave Interface
	input  [29:0] avs_leapSlave_address,
	input         avs_leapSlave_read,
	input         avs_leapSlave_write,
	input  [31:0] avs_leapSlave_writedata,
	output [31:0] avs_leapSlave_readdata
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


    // 
   	// Slave Port - From CPU
	assign avs_from_cpu_readdata        = avs_to_memory_readdata;
	assign avs_from_cpu_waitrequest     = avs_to_memory_waitrequest;
	assign avs_from_cpu_readdatavalid   = avs_to_memory_readdatavalid;

	// Master Port - To I$/Memory
	assign avs_to_memory_address        = avs_from_cpu_address;
	assign avs_to_memory_read           = avs_from_cpu_read;
	assign avs_to_memory_write          = avs_from_cpu_write;
	assign avs_to_memory_writedata      = avs_from_cpu_write;
	assign avs_to_memory_byteenable     = avs_from_cpu_byteenable;

    wire   [31:0]  pc           = avs_from_cpu_address;
    wire   [31:0]  ins          = avs_to_memory_readdata;
    wire           insValid     = avs_to_memory_readdatavalid;
    wire           iCacheStall  = 1'b0;
    wire           dCacheStall  = 1'b0;

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

	// execution_trace.log
	integer log;
	reg program_started = 1'b0;
	initial begin
		log = $fopen ("execution_trace.log", "w");
		$fwrite (log, "    pc    |  pc_in   |   ins    |iVa|\n");
		$fclose (log);
	end
	always @ (posedge clk) begin
		program_started <= program_started | (pc == STARTING_PC);
		log = $fopen ("execution_trace.log", "a");
		if (program_started | (pc == STARTING_PC)) begin
			$fwrite (log, " %x | %x | %x | %b |\n", pc_rr, pc_rrr, ins_rr, insValid_rr);
		end
		$fclose(log);
		if ((|(pc^pc_r)) & ~insValid) begin
			$display ("Warning: insValid is 0 when new pc shows up...");
			$display ("\tAn Valid Instruction may be missed by profiler.");
			$display ("\t| %x | %x | %x | %b |", pc, pc_r, ins, insValid);
		end
	end
`endif

	tiger_leap_slave_handler slave_handler (
		.clk (clk),
		.reset (reset),
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

endmodule	
