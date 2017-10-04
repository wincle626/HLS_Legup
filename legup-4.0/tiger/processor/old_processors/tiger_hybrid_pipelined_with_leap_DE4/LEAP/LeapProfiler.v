// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module LeapProfiler # (
	parameter N = 256,
	parameter N2 = 8,
	parameter S  = 32,
	parameter S2 = 5,
	parameter CW = 32
	) (
	input clk,
	input reset,
	input [31:0] pc_in,
	input [31:0] instr_in,
	input insValid_in,
	input istall_in,
	input dstall_in,

	output exe_start_as,
	output exe_end_as,

	input  [N2-1:0] lp_mm_addr,
	input           lp_mm_wren,
	input  [31  :0] lp_mm_wrdata,
	output [31  :0] lp_mm_rddata
	// lp_mm: write goes to AddressHash, read goes to CountingBlock | CounterStorage
	// lp_mm: Memory Mapping 10 bits address space on avalon bus - supports up to 2^8 functions
	//              module                 lp_mm_addr  lp_mm_wren
	//                 AH -         V1 - 8'b0000_0000 -   1   ----|
	//                 AH -   A1A2B1B2 - 8'b0000_0001 -   1   ----|==> lp_mm_addr[6:0] connects to < AddressHash|ram_wr_addr_ah[N2-2:0] >
	//                 AH -        tab - 8'b01tt_tttt -   1   ----|
	//   Increment Option -   inc_mask - 8'b1000_0000 -   1  -------|
	//   Do Hierarchical? -    do_hier - 8'b1000_0001 -   1  -------|===> handled at the top level LeapProfiler <Configure Options>
	//       Starting PC  -    init_pc - 8'b1000_0010 -   1  -------|
	//                 CB -    counter - 8'bcccc_cccc -   0  --------> lp_mm_addr[7:0] coonects to < Counting|CounterStorage|ram_wr_addr_ah[N2-1:0] >

);
//+++++++++++++++++++++++++++++
// Configure Options
//+++++++++++++++++++++++++++++
	//	inc_mask[4:0]	Increment Option  	init_count_on_jump
	//	  5'b00001    	 # of Instructions	    'b1
	//	  5'b00010    	 # of Cycles      	    'b1
	//	  5'b00100    	 # of Stalls      	    'b0
	//	  5'b01000   	 # of iStalls     	    'b0
	//	  5'b10000   	 # of dStalls     	    'b0
	reg [31:0] inc_mask;               // Increment Option - only 5 lsb are used; only 1 bit is expected to be asserted
	reg [CW-1:0] init_count_on_jump;   // For cycle and instruction profilings, init_count_on_jump should be 1. For stalls, should be 0
	reg do_hier;                       // Do Hierarchical
	reg [31:0] init_pc;                // Start profiling when pc_in == init_pc
	always @ (posedge clk) begin
		if (reset) begin
			inc_mask <= 32'b0;
			init_count_on_jump <= {(CW-1){1'b0}};
			do_hier <= 1'b0;
			init_pc <= 32'hbaddecaf;	// Initialize it with `STARTINGPC for on board system (NEED TO FIX THIS FOR DE4??)
		end
		else if (lp_mm_wren) begin
		//Address Mapping
			if (lp_mm_addr[N2-1] & ~(|lp_mm_addr[N2-2:0]) /*== 8'b1000_0000*/) begin
				inc_mask <= lp_mm_wrdata;
				init_count_on_jump <= |lp_mm_wrdata[1:0] ? { {(CW-2){1'b0}}, 1'b1} : {(CW-1){1'b0}};
			end
			else if (lp_mm_addr[N2-1] & ~(|lp_mm_addr[N2-2:1]) & lp_mm_addr[0] /*== 8'b1000_0001*/)
				do_hier <= lp_mm_wrdata [0];
			else if (lp_mm_addr[N2-1] & ~(|lp_mm_addr[N2-2:2]) & lp_mm_addr[1] & ~lp_mm_addr[0] /*== 8'b1000_0010*/)
				init_pc <= lp_mm_wrdata;
		end
	end

//+++++++++++++++++++++++++++++
// PIPELINING & BOOT_LOAD FILTERING
//+++++++++++++++++++++++++++++

	// Number of 'r's represents r th pipeline (convention throughtout the entire profiler code)
	//	i.e., r means first pipeline, rr means second pipeline, and etc

	reg profiling_started;	// local register; only used in the following always blk
	
	reg [31:0] pc_in_od, pc_r, pc_rr;
	reg [31:0] instr_in_od, instr_r, instr_rr;
	reg insValid_in_od, insValid_r, insValid_rr;
	reg cnt_inc_in_od, cnt_inc_r, cnt_inc_rr;

	always @ (posedge clk) begin
		if (reset | ( ~profiling_started & (pc_in != init_pc) ) ) begin
			profiling_started <= 1'b0;

			pc_r       <= 32'b0;
			instr_r    <= 32'b0;
			insValid_r <= 1'b0;
			cnt_inc_r  <= 1'b0;
			pc_rr       <= 32'b0;
			instr_rr    <= 32'b0;
			insValid_rr <= 1'b0;
			cnt_inc_rr  <= 1'b0;
			pc_in_od       <= 32'b0;
			instr_in_od    <= 32'b0;
			insValid_in_od <= 1'b0;
			cnt_inc_in_od  <= 1'b0;
		end
		else begin
		// 1st pipleline
			pc_r       <= pc_in;
			instr_r    <= instr_in;
			insValid_r <= insValid_in;
			// for cycle profiling, increment signal of counter is always 1; for instruction, increment signal is 1 when pc is changed
			cnt_inc_r  <=  | (inc_mask[4:0] & {dstall_in, istall_in, dstall_in|istall_in, 1'b1, |(pc_r^pc_in)});
		// 2nd pipleline
			pc_rr <= pc_r;
			cnt_inc_rr <= cnt_inc_r;
			instr_rr <= instr_r;
			insValid_rr <= insValid_r;
		// 3rd pipleline
			pc_in_od       <= pc_rr;
			cnt_inc_in_od  <= cnt_inc_rr;
			if ( ~profiling_started & (pc_in == init_pc) ) begin
				profiling_started <= 1'b1;
				// Insert a JAL instruction TWO CYCLE BEFORE pc_in_od becomes init_pc to pretend calling <main>
				instr_in_od    <= 32'h0c00_c003;
				insValid_in_od <= 1'b1;
			end
			else begin
				instr_in_od    <= instr_rr;
				insValid_in_od <= insValid_rr;
			end
		end
	end
	
//+++++++++++++++++++++++++++++
// OpDecode
//+++++++++++++++++++++++++++++
	wire [31:0] pc_od_ah;
	wire call_od_ah;
	wire retn_od_ah;
	wire cnt_inc_od_ah;

	OpDecode od (
		.clk (clk),
		.reset (reset),

		.instr_in_od (instr_in_od),
		.insValid_in_od (insValid_in_od),
		.pc_in_od (pc_in_od),
		.cnt_inc_in_od (cnt_inc_in_od),

		.pc_od_ah (pc_od_ah),
		.call_od_ah (call_od_ah),
		.retn_od_ah (retn_od_ah),
		.cnt_inc_od_ah (cnt_inc_od_ah)
	);

//+++++++++++++++++++++++++++++
// AddressHash
//+++++++++++++++++++++++++++++
	wire call_ah_as;
	wire retn_ah_as;
	wire cnt_inc_ah_as;
	wire [N2-1:0] funcNum_ah_as;
	
	AddressHash ah (
		.clk (clk),
		.reset (reset),

		.pc_od_ah (pc_od_ah),
		.call_od_ah (call_od_ah),
		.retn_od_ah (retn_od_ah),
		.cnt_inc_od_ah (cnt_inc_od_ah),

		.call_ah_as (call_ah_as),
		.retn_ah_as (retn_ah_as),
		.cnt_inc_ah_as (cnt_inc_ah_as),
		.funcNum_ah_as (funcNum_ah_as),
	
		.ram_wr_addr_ah (lp_mm_addr[N2-2:0] ),
		.ram_wr_data_ah (lp_mm_wrdata),
		.ram_wr_en_ah   (lp_mm_wren & ~lp_mm_addr[N2-1])
	);
	defparam ah .N  = N ;
	defparam ah .N2 = N2;

//+++++++++++++++++++++++++++++
// AddressStack
//+++++++++++++++++++++++++++++
	wire call_as_cb;
	wire retn_as_cb;
	wire cnt_inc_as_cb;
	wire [N2-1:0] funcNum_as_cb;
	
	AddressStack as (
		.clk (clk),
		.reset (reset),

		.call_ah_as (call_ah_as),
		.retn_ah_as (retn_ah_as),
		.cnt_inc_ah_as (cnt_inc_ah_as),
		.funcNum_ah_as (funcNum_ah_as),

		.call_as_cb (call_as_cb),
		.retn_as_cb (retn_as_cb),
		.cnt_inc_as_cb (cnt_inc_as_cb),
		.funcNum_as_cb (funcNum_as_cb),
	
		.exe_start_as (exe_start_as),
		.exe_end_as   (exe_end_as)
	);
	defparam as .N2 = N2;
	defparam as .S2 = S2;
	defparam as .S  = S;

//+++++++++++++++++++++++++++++
// CountingBlock
//+++++++++++++++++++++++++++++
	CountingBlock cb (
		.clk (clk),
		.reset (reset),
		.do_hier (do_hier),
		
		.exe_start_as (exe_start_as),
		.init_count_on_jump(init_count_on_jump),
		.call_as_cb (call_as_cb),
		.retn_as_cb (retn_as_cb),
		.cnt_inc_as_cb (cnt_inc_as_cb),
		.funcNum_as_cb (funcNum_as_cb),

		.ext_addr    (lp_mm_addr[N2-1:0]),
		.ext_rd_data (lp_mm_rddata)
	);
	defparam cb .N  = N ;
	defparam cb .N2 = N2;
	defparam cb .S  = S ;
	defparam cb .S2 = S2;
	defparam cb .CW = CW;
	
endmodule	
