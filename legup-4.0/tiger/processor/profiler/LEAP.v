
//`include "../../profiler/prof_defines.v"
`include "../../profiler/prof_defines.v"

// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

 module leap (
	input clk,
	input glob_reset,
	input [25:0] pc_in,
	input [31:0] instr_in2,
	input [31:0] instrEx_in,
	input istall_in,
	input dstall_in,
	input insValid_in,
	
	// TESTER PORTS ONLY
	input init_start_in,
	output init_done,
	input retrieve_start_in,
	output retrieve_done,
	
	// Avalon Profile Master ports
	output avm_profileMaster_read,
	output avm_profileMaster_write,
	output [31:0]avm_profileMaster_address,
	output [31:0]avm_profileMaster_writedata,
	output [3:0]avm_profileMaster_byteenable,
	input [31:0]avm_profileMaster_readdata,
	input avm_profileMaster_waitrequest,
	input avm_profileMaster_readdatavalid,
	
	output [`CW-1:0] rProfData,
	output [`N2-1:0] rProfIndex
);

	reg [25:0] pc;
	reg [25:0] pc_minus_4;
	reg [31:0] instr_in;
	reg [31:0] instrEx;
	reg istall;
	reg dstall;
	reg insValid;
	reg init_start;
	reg retrieve_start;
	
	always @(posedge clk) begin
		if (glob_reset) begin
			pc <= 'b0;
			pc_minus_4 <= 'b0;
			instr_in <= 'b0;
			instrEx <= 'b0;
			istall <= 'b0;
			dstall <= 'b0;
			insValid <= 'b0;
			init_start <= 'b0;
			retrieve_start <= 'b0;
		end else begin
			pc <= pc_in;
			pc_minus_4 <= pc_in - 'h4;
			instr_in <= instr_in2;
			instrEx <= instrEx_in;
			istall <= istall_in;
			dstall <= dstall_in;
			insValid <= insValid_in;
			init_start <= init_start_in;
			retrieve_start <= retrieve_start_in;
		end
	end
	
	/*wire [25:0] pc = pc_in;
	wire [31:0] instr_in = instr_in2;
	wire [31:0] instrEx = instrEx_in;
	wire istall = istall_in;
	wire dstall = dstall_in;
	wire insValid = insValid_in;
	wire init_start = init_start_in;
	wire retrieve_start = retrieve_start_in;*/

	wire stall = istall | dstall;

	// wires for OpDecode
	wire [25:0] 	opTarget;
	wire [25:0] 	opJALrTarget;
	wire 			opCall;
	wire			opJALr;
	wire 			opRet;
	wire			opJALr_done;
	wire			opPC8;
	
	// wires for DataCounter
	wire [`CW-1:0]	dcCountOut;
	reg				dcPCdiff;
	
	// wires for CounterStorage
	wire [`N2-1:0]	csFuncNum;
	wire [`CW-1:0]  csCountIn;
	wire 			csRdReq;
	wire			csEmpty;
	wire			csSave;
	wire [`CW-1:0]	csSaveData;
	wire [`N2-1:0]	csNumFuncs;
	
	wire 			csRamWren;
	wire			csRamClkEn;
	wire [`N2-1:0]	csRamAddr;
	wire [`CW-1:0]	csRamWrData;
	wire [`CW-1:0]	csRamRdData;
	
	// wires for AddressStack
	wire [`N2-1:0]	asFuncNumOut;
	wire			asRamWren;
	wire			asRamClkEn;
	wire [`S2-1:0]	asRamAddr;
	wire [`N2-1:0]	asRamWrData;
	wire [`N2-1:0]	asRamRdData;
	
	// wires for HierarchyStack
	wire			hsStackRamWren;
	wire			hsRamClkEn;
	wire [`S2-1:0]	hsStackRamAddr;
	wire [`CW-1:0]	hsStackRamWrData;
	wire [`CW-1:0]	hsStackRamRdData;
	wire [`CW-1:0]	hsChildCount;
	
	// wires for AddressHash
	wire [25:0]		ahTarget;
	wire [`N2-1:0]	ahFuncNum;
	//wire			ahEnable;
	wire			ahJALr;
	
	wire [`N2-1:0]	ahRamAddr;
	wire [7:0]		ahRamRdData;
	wire			ahRamClkEn;
	
	// wires for DataStorage
	wire			dsHashRamWren;
	wire			dsHashRamClkEn;
	wire [`N2-1:0]	dsHashRamAddr;
	wire [7:0]		dsHashRamWrData;
	wire [7:0]		dsHashRamRdData;
	
	wire			dsStackRamWren;
	wire			dsStackRamClkEn;
	wire [`S2-1:0]	dsStackRamAddr;
	wire [`N2-1:0]	dsStackRamWrData;
	wire [`N2-1:0]	dsStackRamRdData;
	
	wire			dsStorageRamWren;
	wire			dsStorageRamClkEn;
	wire [`N2-1:0]	dsStorageRamAddr;
	wire [`CW-1:0]	dsStorageRamWrData;
	wire [`CW-1:0]	dsStorageRamRdData;

	// wires for Initializer
	wire			iInit;
	wire			iHashRamWren;
	wire [`N2-1:0]	iHashRamAddr;
	wire [7:0]		iHashRamWrData;
	wire [7:0]		iHashRamRdData;
	wire			init_hash_start;
	wire			init_hash_done;
	wire			hash_first_start;
	wire			init_avm_profileMaster_write;
	wire [31:0]		init_avm_profileMaster_address;
	wire [31:0]		init_avm_profileMaster_writedata;
	wire [3:0]		init_avm_profileMaster_byteenable;
	wire			init_reset;
	
	// wires for Retreiver
	wire 			rRetrieve;
//	wire [`CW-1:0]	rProfData;
//	wire [`N2-1:0]	rProfIndex;
	wire			retrieve_avm_profileMaster_write;
	wire [31:0]		retrieve_avm_profileMaster_address;
	wire [31:0]		retrieve_avm_profileMaster_writedata;
	wire [3:0]		retrieve_avm_profileMaster_byteenable;
		
	// general registers
	reg [25:0]		old_pc;
	reg [31:0]		old_instr;
	reg				pc_diff;
	reg				instr_diff;	// need this b/c sometimes pc changes but has to wait to read the instruction (so would otherwise miss the instruction)
	
	// Keep track of current function
	reg [`N2-1:0]	cur_func_reg;
	reg				cur_func_src;
	wire [`N2-1:0]	cur_func = (cur_func_src) ? ahFuncNum : cur_func_reg;
	reg [1:0] 		state;	
	reg [2:0]		delay_count;
	reg				jalr_prev;
	reg				saved_call;
	reg				hash_first_done;

	// combine global & init resets
	wire 			reset = glob_reset | init_reset;
	reg 			instr_init;
	reg 			store_first_call;
	wire [31:0]		instr;
	
	// 
	always @(posedge(clk)) begin
		jalr_prev <= opJALr;
	end
	
	// update pc_diff, store first jal
	always @(posedge(clk)) begin
		if (reset) begin	
			cur_func_reg <= 'h0;
			cur_func_src <= 1'b0;
			state <= 2'b00;
			hash_first_done <= 1'b0;
			delay_count <= 'b0;
			
			old_pc <= 'b0;
			old_instr <= 'b0;
			
			store_first_call <= 1'b0;
			instr_init <= 'b0;
		
		// initialize with hash of `START_ADDR
		end else if (init_start) begin
			// State 0: Wait for init_hash_done
			if (state <= 2'b00) begin
				if (hash_first_start & !hash_first_done) begin
					state <= 2'b01;
					delay_count <= 'b0;
					//pc_diff <= 1'b1;
					instr_init <= 1'b1; //`START_ADDR;		// pretend we start with a jump to 0x8003000 (GXemul) or 0x800000 (Tiger)
					//old_pc <= { instr[23:0], 2'b0 };
				end else if (hash_first_start & hash_first_done) begin
					hash_first_done <= 1'b0;
				end
			
			// State 1: Delay until hash finished (2 cycles)
			end else if (state == 2'b01) begin
				pc_diff <= 1'b0;
			
				// stay in this state until delay finished
				if (delay_count < 3'b111) begin			// 2 cycles
					delay_count <= delay_count + 1'b1;
				
				end else if (delay_count < 3'b011) begin	// 2 cycles
					instr_init <= 1'b0;
					delay_count <= delay_count + 1'b1;
				
				// update cur_func
				end else begin
					instr_init <= 32'b0;
					hash_first_done <= 1'b1;
					store_first_call <= 1'b1;
					$display("initializing cur_func = %h", ahFuncNum);
					cur_func_reg <= ahFuncNum;
					state <= 2'b10;
					//pc_diff <= 1'b1;	// set pc_diff so first instr will count
				end
			end else if (state == 2'b10) begin
				store_first_call <= 1'b0;
				state <= 2'b00;
			end
		
		// set/clear pc_diff signal so modules know when PC changes
		end else begin
			pc_diff <= (pc != old_pc);	
			instr_diff <= 0;//(instr != old_instr);
			old_pc <= pc;
			old_instr <= instr;
		
			// State 0: Wait for ret/call
			if (state <= 2'b00) begin
				if (opCall | opRet) begin
					state <= 2'b01;
					delay_count <= opCall ? 'b01 : 'b10;
					saved_call <= opCall;
					//cur_func_src <= opCall;
				
				// Handle JALr differently
				end else if (!opJALr & jalr_prev) begin	// use jalr_prev to indicate we WERE in jalr -- once opJALr goes low, that means the target is ready
					state <= 2'b01;
					delay_count <= 1'b0;
					saved_call <= 1'b1;
				end	
				
			// State 1: Delay until hash finished (2 cycles)
			end else if (state == 2'b01) begin
				// stay in this state until delay finished
				if (delay_count < 3'b10) begin		// 2 cycles
					cur_func_src <= 1'b1; 			// make mux use ahFuncNum (for bypass of call-then-ret issue)
					delay_count <= delay_count + 1'b1;
				
				// update cur_func
				end else begin
					if (opRet)	cur_func_reg <= asFuncNumOut;
					else		cur_func_reg <= saved_call ? ahFuncNum : asFuncNumOut;
					cur_func_src <= 1'b0;	// make mux use cur_func_reg, not directly ahFuncNum (for bypass of call-then-ret issue)
					delay_count <= 'b0;
					
					// this is a bypass condition to avoid the case where a hash is available in the same cycle that another call happens
					if (!opCall & !opRet) state <= 2'b00;
					else				  state <= 2'b10;
				end				
			
			// State 2: Wait until call/ret is low
			end else if (state == 2'b10) begin
				if (!opCall & !opRet) state <= 2'b00;
			end			
		end
	end
				

	// Make sure calls arrive at the right time to the counter & counter storage modules (for cases when it stalls between the jal instr and the actual jump)
	wire call_jalr = opCall | opJALr;// (!opJALr & jalr_prev);
	wire call_jalr_pc;	// wire for cycles, reg for power
	reg call_jalr_pc_state;
	reg [31:0] call_jalr_target;
	always @(posedge(clk)) begin
		if (reset) begin
			//call_jalr_pc <= 1'b0;	// for power
			call_jalr_pc_state <= 1'b0;
			call_jalr_target <= 'b0;
		end else begin
			if (call_jalr & hash_first_done) begin
				call_jalr_pc_state <= 1'b1;
				call_jalr_target <= opTarget;		// for cycles
				//call_jalr_target <= opTarget + 'h8;	// for power
			end
			if (call_jalr_pc_state & (pc == call_jalr_target | (opJALr & !opPC8))) begin
				//call_jalr_pc <= 1'b1;	// for power
				call_jalr_pc_state <= 1'b0;
			end
			//if (call_jalr_pc & !call_jalr_pc_state) call_jalr_pc <= 1'b0;	// for power
		end
	end
	assign call_jalr_pc = (call_jalr_pc_state & (pc == call_jalr_target | (opJALr & !opPC8)));	// for cycles
	
	// Make sure returns arrive at the right time to the counter & counter storage modules (for cases when it stalls between the ret instr and the actual jump)
	wire ret_pc;
	reg ret_pc_state;
	reg [31:0] ret_pc_val;
	reg [31:0] ret_instr_val;
	always @(posedge(clk)) begin
		if (reset) begin
			ret_pc_state <= 1'b0;
			ret_pc_val <= 'b0;
			ret_instr_val <= 'b0;
		end else begin
			if (opRet & hash_first_done) begin
				ret_pc_state <= 1'b1;
				ret_pc_val <= pc;				// for cycles
				//ret_instr_val <= instr_in;	// for power;
			end
			if (ret_pc_state & (pc != ret_pc_val) & (pc_minus_4 != ret_pc_val)) begin	// for cycles
			//if (ret_pc_state & (instrEx == ret_instr_val)) begin				// for power
				ret_pc_state <= 1'b0;
			end
		end
	end
	assign ret_pc = (ret_pc_state & (pc != ret_pc_val) & (pc_minus_4 != ret_pc_val));	// +4 to make sure its not just the branch delay slot	----- for cycles
	//assign ret_pc = (ret_pc_state & (instrEx == ret_instr_val));	// for power
	
	// Make sure counter_storage uses correct "cur_func" by storing it
	reg [`N2-1:0] cur_func_pc;
	always @(posedge(clk)) begin
		if (reset) begin
			cur_func_pc <= 'b0;
		end else begin
			if (!ret_pc_state & !call_jalr_pc_state) begin	// if neither call/ret are waiting for the actual jump, keep track of current function
				cur_func_pc <= (delay_count == 3'b010) ? ahFuncNum : cur_func;	// a second bypass to have hash value go straight to counter_storage so it can read the data a cycle earlier
			end
		end
	end	
	
	// New way of doing "pc_diff" -- just send prev_pc to instr_counter and have it compare
	reg [31:0] prev_pc;
	always @(posedge(clk)) begin
		if (reset) begin
			prev_pc <= 'b0;
		end else begin
			prev_pc <= pc;
		end
	end
	

	// For cycles: instr_in, for power: instrEx 
	// Note: use this, don't just pass instrEx to powerCounter, b/c you want ret/call to be based on this too so 
	// 		instructions in execute stage, which is what we're measuring for power, are attributed to the right function.
	//		otherwise, they could still be running after a jump is decoded but not performed.
	assign instr = instr_in;//(init_start & !init_done) ? instr_init : instr_in;
	
	// keep track of total cycles & stalls
	reg [31:0] tCycle;
	reg [31:0] tStall;
	reg count_oncea, count_onceb;
	always @(posedge(clk)) begin
		if (reset) begin
			tCycle <= 'b0;
			tStall <= 'b0;
			count_oncea <= 1'b0;
			count_onceb <= 1'b0;
		end else if (retrieve_start & ~count_oncea) begin
			$display("_Counted Cycles: %5d, Counted Stalls: %5d", tCycle, tStall);
			count_oncea <= 1'b1;
		end else if (retrieve_done & ~count_onceb) begin
			$display("Counted Cycles: %5d, Counted Stalls: %5d", tCycle, tStall);
			count_onceb <= 1'b1;
		end else begin
			tCycle <= tCycle + 1'b1;
			if (stall) tStall <= tStall + 1'b1;
		end
	end
	
	// Connect modules
	assign ahTarget				= opTarget;
	//assign ahEnable			= opCall | (!opJALr & jalr_prev);
	assign csCountIn			= dcCountOut;
		
	assign dsHashRamWren		= iHashRamWren;			// Hash RAM is only written by Initializer
	assign dsHashRamAddr		= (init_start & !init_done & !init_hash_start & !init_hash_done & !hash_first_start & !hash_first_done) ? iHashRamAddr : ahRamAddr;
	assign ahRamRdData			= dsHashRamRdData;
	assign dsHashRamWrData		= iHashRamWrData ;
	assign ahJALr				= opJALr_done;
	
	assign dsStackRamWren		= asRamWren;	// Stack RAM is only accessed by addressStack
	assign dsStackRamAddr		= asRamAddr;
	assign asRamRdData			= dsStackRamRdData;
	assign dsStackRamWrData		= asRamWrData;
		
	assign dsStorageRamWren		= csRamWren;	// Storage RAM is only written by counterStorage
	assign dsStorageRamAddr		= (retrieve_start & !retrieve_done) ? rProfIndex : csRamAddr;
	assign csRamRdData			= dsStorageRamRdData;										//////////// ISSUE-- this is where to combine different counters
	assign rProfData			= dsStorageRamRdData;
	assign dsStorageRamWrData	= csRamWrData;												//////////// ISSUE-- this is where to combine different counters

	// mux avalon signals
	assign avm_profileMaster_write = (init_start & !init_done) ? init_avm_profileMaster_write : retrieve_avm_profileMaster_write;
	assign avm_profileMaster_address = (init_start & !init_done) ? init_avm_profileMaster_address : retrieve_avm_profileMaster_address;
	assign avm_profileMaster_writedata = (init_start & !init_done) ? init_avm_profileMaster_writedata : retrieve_avm_profileMaster_writedata;
	assign avm_profileMaster_byteenable = (init_start & !init_done) ? init_avm_profileMaster_byteenable : retrieve_avm_profileMaster_byteenable;
	
	// mux clkEn signals
	assign dsHashRamClkEn = (init_start & !init_done) ? 1'b1 : ahRamClkEn;
	assign dsStackRamClkEn = asRamClkEn;
	assign dsStorageRamClkEn = (retrieve_start & !retrieve_done) ? 1'b1 : csRamClkEn;
	assign dsHierRamClkEn = hsRamClkEn;
	
	// fix pc_diff signal for instruction counter (to count first instruction properly) -- only hold for half cycle
	/*always @(posedge(clk) or negedge(clk)) begin
		if (clk) begin	// posedge
			icPCdiff <= pc_diff;
		end else begin	// negedge
			icPCdiff <= 1'b0;
		end
	end*/


	// Instantiate the modules
	OpDecode decoder (
		.clk(clk),
		.reset(reset),
		//.pc_diff(pc_diff),
		.pc(pc),
		.pc_nolag(pc_in),
		.instr(instr),
		.instr_nolag(instr_in2),
		.insValid(insValid),	// to make sure PC is valid (odd case in jpeg & dfsin where cache misses but the temp (and invalid) data read happnes to be a 'jr ra' and prof screws up
		.ret(opRet),
		.call(opCall),
		.jalr(opJALr),
		.jalr_done(opJALr_done),
		.target(opTarget),
		.jalr_target(opJALrTarget),
		.pc_within_8(opPC8)
	);
	
	parameter PROF_METHOD = `PROF_METHOD;
	generate
		if (PROF_METHOD=="i") begin
			InstructionCounter instr_counter (
				.clk(clk),
				.reset(reset),
				//.pc_diff(pc_diff),
				.prev_pc(prev_pc),
				.pc(pc),
				.ret(ret_pc),
				.call(call_jalr_pc),					/// use jalr and add reset to 2 or something?
				.count_out(dcCountOut),
				.init_start(init_start)
			);
		end else if (PROF_METHOD=="c") begin		
			CycleCounter cycle_counter (
				.clk(clk),
				.reset(reset),
				.ret(ret_pc),
				.call(call_jalr_pc),					/// use jalr and add reset to 2 or something?
				.count_out(dcCountOut),
				.init_start(init_start)
			);
		end else if (PROF_METHOD=="s") begin
			StallCounter stall_counter (
				.clk(clk),
				.reset(reset),
				.stall(stall),
				.ret(ret_pc),
				.call(call_jalr_pc),					/// use jalr and add reset to 2 or something?
				.count_out(dcCountOut),
				.init_start(init_start)
			);
		end else if (PROF_METHOD=="p") begin
			PowerCounter power_counter (
				.clk(clk),
				.reset(reset),
				.pc_diff(pc_diff),
				.stall(stall),
				.instr_in(instr),
				.ret(ret_pc),
				.call(call_jalr_pc),					/// use jalr and add reset to 2 or something?
				.count_out(dcCountOut),
				.init_start(init_start)
			);
		end
	endgenerate
	
	CounterStorage counter_storage (
		.clk(clk),
		.reset(reset),
		.call(call_jalr_pc),					// jalr must be used to know when to store
		.ret(ret_pc),
		.cur_func_num(cur_func_pc),
		.count(csCountIn),
		.child_count(hsChildCount),
		.init_start(init_start),
		.pc_diff(pc_diff),
		
		// For accessing RAM
		.ram_wren(csRamWren),
		//.ram_clken(csRamClkEn),
		.ram_addr(csRamAddr),
		.ram_wr_data(csRamWrData),
		.ram_rd_data(csRamRdData),
		
		// temp..
		.retrieve_done(retrieve_done)
	);

	AddressStack addr_stack (
		.clk(clk),
		.reset(reset),
		.ret(opRet),
		.call(call_jalr),				// jalr must be used
		.func_num_in(cur_func),
		.func_num_out(asFuncNumOut),
		.pc_diff(pc_diff),
		.init_start(init_start),
		.store_first_call(store_first_call),
		
		// For accessing RAM
		.ram_wren(asRamWren),
		//.ram_clken(asRamClkEn),
		.ram_addr(asRamAddr),
		.ram_wr_data(asRamWrData),
		.ram_rd_data(asRamRdData)
	);

	parameter DO_HIER = `DO_HIER;
	generate
	if (DO_HIER) begin
		HierarchyStack hier_stack (
			.clk(clk),
			.reset(reset),
			.ret(ret_pc),
			.call(call_jalr_pc),				// can i ignore jalr??
			.child_count(hsChildCount),
			.count_in(dcCountOut),
			//.pc_diff(pc_diff),
			.init_start(init_start),
			
			// For accessing RAM
			.ram_wren(hsStackRamWren),
			//.ram_clken(hsStackClkEn),
			.ram_addr(hsStackRamAddr),
			.ram_wr_data(hsStackRamWrData),
			.ram_rd_data(hsStackRamRdData)
		);
	end
	endgenerate
	
	AddressHash addr_hash (
		.clk(clk),
		.reset(reset),
		.instr_no_lag(instr_in2),
		.instrValid_no_lag(insValid_in),
		.addr_in(opJALrTarget),
		.funcNum(ahFuncNum),
		//.enbl_in(call_jalr),		// jalr must be used
		.jalr(ahJALr),
		
		.init_start(init_hash_start),
		.init_done(init_hash_done),
		.instr_init(instr_init),
		
		// For accessing RAM
		.ram_addr(ahRamAddr),
		.ram_rd_data(ahRamRdData)
		//.ram_clken(ahRamClkEn)
	);

	DataStorage data_storage (
		.clk(clk),
		.reset(reset),
		.init_start(init_start),
		.retrieve_start(retrieve_start),
		
		.hash_wren(dsHashRamWren),
		.hash_addr(dsHashRamAddr),
		.hash_wr_data(dsHashRamWrData),
		.hash_rd_data(dsHashRamRdData),
		
		.addr_stack_wren(dsStackRamWren),
		.addr_stack_addr(dsStackRamAddr),
		.addr_stack_wr_data(dsStackRamWrData),
		.addr_stack_rd_data(dsStackRamRdData),
		
		.hier_stack_wren(hsStackRamWren),
		.hier_stack_addr(hsStackRamAddr),
		.hier_stack_wr_data(hsStackRamWrData),
		.hier_stack_rd_data(hsStackRamRdData),
		
 		.storage_wren(dsStorageRamWren),
		.storage_addr(dsStorageRamAddr),
		.storage_wr_data(dsStorageRamWrData),
		.storage_rd_data(dsStorageRamRdData)
	);
	
	vInitializer init (
		.clk(clk),
		.reset(glob_reset),
		.reset_out(init_reset),
		.pc(pc),
		
		.init_start(init_start),
		.init_done(init_done),
		.init_hash_start(init_hash_start),
		.init_hash_done(init_hash_done),
		.hash_first_start(hash_first_start),
		.hash_first_done(hash_first_done),
		
		.hashRdData(dsHashRamRdData),		// temp for debugging
		.hashWren(iHashRamWren),
		.hashAddr(iHashRamAddr),
		.hashWrData(iHashRamWrData),
		
		//Avalon Bus side signals
		.avm_profileMaster_read(avm_profileMaster_read),
		.avm_profileMaster_write(init_avm_profileMaster_write),
		.avm_profileMaster_address(init_avm_profileMaster_address),
		.avm_profileMaster_writedata(init_avm_profileMaster_writedata),
		.avm_profileMaster_byteenable(init_avm_profileMaster_byteenable),
		.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest),
		.avm_profileMaster_readdata(avm_profileMaster_readdata),
		.avm_profileMaster_readdatavalid(avm_profileMaster_readdatavalid)
	);

	generate
		if (PROF_METHOD!="p") begin
			vRetriever retrieve (
				.clk(clk),
				.reset(reset),
				.pc(pc),
				.retrieve_start(retrieve_start),
				.retrieve_done(retrieve_done),
				.prof_data(rProfData),
				.prof_index(rProfIndex),
				
				//Avalon Bus side signals
				.avm_profileMaster_write(retrieve_avm_profileMaster_write),
				.avm_profileMaster_address(retrieve_avm_profileMaster_address),
				.avm_profileMaster_writedata(retrieve_avm_profileMaster_writedata),
				.avm_profileMaster_byteenable(retrieve_avm_profileMaster_byteenable),
				.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest)
			);
		end else begin
			vPowerRetriever retrieve (
				.clk(clk),
				.reset(reset),
				.pc(pc),
				.retrieve_start(retrieve_start),
				.retrieve_done(retrieve_done),
				.prof_data(rProfData),
				.prof_index(rProfIndex),
				
				//Avalon Bus side signals
				.avm_profileMaster_write(retrieve_avm_profileMaster_write),
				.avm_profileMaster_address(retrieve_avm_profileMaster_address),
				.avm_profileMaster_writedata(retrieve_avm_profileMaster_writedata),
				.avm_profileMaster_byteenable(retrieve_avm_profileMaster_byteenable),
				.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest)
			);
		end
	endgenerate
endmodule


module OpDecode (
	input clk,
	input reset,
	//input pc_diff,		// can remove
	input [25:0] pc,
	input [25:0] pc_nolag,
	input [31:0] instr,
	input [31:0] instr_nolag,
	input insValid,
	output  ret,
	output  call,
	output reg jalr,
	output reg jalr_done,
	output [25:0] target,
	output [25:0] jalr_target,
	output reg pc_within_8
);
	
	//wire pc_nolag_diff = (pc_nolag != pc);
	reg [25:0] pc_jalr;
	//wire 
	//assign pc_within_8 = (pc == pc_jalr) | (pc == (pc_jalr+'h4));	// make this a smaller comparator (only need to compare 2 bits of each pc)
	
	reg [2:0] pc_diff_count;
	
	wire tCall = insValid & (instr[31:26] == 6'b00_0011);
	wire tRet = insValid & (instr[31:0] == 32'b0000_0011_1110_0000_0000_0000_0000_1000);	// $s = $ra = 31
	wire tJALR = (instr_nolag[31:26] == 6'b0 & instr_nolag[20:16] == 5'b0 & instr_nolag[10:6] == 5'b0 & instr_nolag[5:0] == 6'b00_1001);
	
	wire [25:0] tTarget = { instr[23:0], 2'b0 };//tCall ? { instr[23:0], 2'b0 } : 	// JAL
	//					  tRet  ? 26'b0 : 		// JR ra -- since we'll be using addr stack, this is irrelevant
	//					  26'b0;
	
	assign call = tCall;
	assign ret = tRet;
	assign target = tTarget;
	assign jalr_target = pc_jalr;
	
	always @(posedge(clk)) begin
		if (reset) begin
			jalr_done <= 1'b0;
			pc_diff_count <= 'b0;
			jalr <= 1'b0;
			pc_jalr <= 'b0;
		end else begin
			if (jalr_done) begin
				if (pc_diff_count < 3'b010) pc_diff_count <= pc_diff_count + 1'b1;
				else begin
					pc_diff_count <= 'b0;
					jalr_done <= 1'b0;
				end
			
			end else if (tJALR) begin
				pc_diff_count <= 'b1;
				pc_jalr <= pc_nolag;
				jalr <= 1'b1;
				jalr_done <= 1'b0;
			
			end else if (jalr & !pc_within_8) begin
				pc_diff_count <= 'b0;
				pc_jalr <= pc;
				jalr <= 1'b0;
				jalr_done <= 1'b1;
			end
		end
		
		pc_within_8 <= (pc_nolag == pc) | (pc_nolag == pc + 'h4);
	end

	//always @(target) $display("instr = %h, call = %b, ret = %b, target = %h", instr, call, ret, target);
endmodule

module InstructionCounter (
	input clk,
	input reset,
	//input pc_diff,
	input [31:0] prev_pc,
	input [31:0] pc,
	input ret,
	input call,
	output [`CW-1:0] count_out,
	input init_start
);

	reg [`CW-1:0] counter;
	assign count_out = counter;
	wire pc_diff = (prev_pc != pc);
	wire [31:0] counter_plus_one = counter + 1'b1;
	
	always @(posedge(clk)) begin
		if (init_start | call | ret) begin	
			counter <= 'b1;
		end else if (reset) begin
			counter <= 'b0;
		end	else if (pc_diff) begin
			counter <= counter_plus_one;//counter + 1'b1;
		end
	end
endmodule

module CycleCounter (
	input clk,
	input reset,
	input ret,
	input call,
	output [`CW-1:0] count_out,
	input init_start
);

	reg [`CW-1:0] counter;
	assign count_out = counter;
	wire [31:0] counter_plus_one = counter + 1'b1;
	
	always @(posedge(clk)) begin
		if (init_start | call | ret) begin	
			counter <= 'b1;
		end else if (reset) begin
			counter <= 'b0;
		end	else begin
			counter <= counter_plus_one;//counter + 1'b1;
		end
	end
endmodule

module StallCounter (
	input clk,
	input reset,
	input stall,
	input ret,
	input call,
	output [`CW-1:0] count_out,
	input init_start
);

// issue -- setting counter to 1 on call/return assumes first instruction always stalls, which isn't always true.

	reg [`CW-1:0] counter;
	assign count_out = counter;
	/*reg prev_stall;
	always @(posedge(clk)) begin
		if (reset) prev_stall <= 1'b0;
		else	   prev_stall <= stall;
	end*/
	wire [31:0] counter_plus_one = counter + 1'b1;
	
	always @(posedge(clk)) begin
		if (init_start | call | ret) begin	
			counter <= stall;
		end else if (reset) begin
			counter <= 'b0;
		end	else if (stall) begin
			counter <= counter_plus_one;//counter + 1'b1;
		end
	end
endmodule

module PowerCounter (	//omega
	input clk,
	input reset,
	input pc_diff,
	input stall,
	input [31:0] instr_in,
	input ret,
	input call,
	output [`CW-1:0] count_out,
	input init_start
);

	// This assumes 6 groups + stalls (not parameterized)
	reg [`PW-1:0] A;
	reg [`PW-1:0] B;
	reg [`PW-1:0] C;
	reg [`PW-1:0] D;
	reg [`PW-1:0] E;
	reg [`PW-1:0] F;
	reg [`PSW-1:0] STALL;
	reg [`PW-1:0] UNKNOWN;
	assign count_out = { A,B,C,D,E,F,STALL };
	
	wire [5:0] op = instr_in[31:26];
	wire [5:0] funct = instr_in[5:0];
	reg [3:0] cur_group;

	always @(posedge(clk)) begin
		if (reset) begin
			A <= 'b0;
			B <= 'b0;
			C <= 'b0;
			D <= 'b0;
			E <= 'b0;
			F <= 'b0;
			STALL <= 'b0;
			UNKNOWN <= 'b0;
			cur_group <= 'b0;
		end	else begin
			cur_group = 'b0;
			
			// PC_DIFF
			if (~stall & pc_diff) begin //(pc_diff) begin	//
				// NOP
				if (instr_in == 'b0) begin
					cur_group = `P_NOP;								// NOP			- 0000 0000 0000 0000 0000 0000 0000 0000
				
				// BEQ, BNE, BLEZ, BGTZ
				end else if (op[5:2] == 4'b0001) begin
					if (op[1:0] == 2'b00) cur_group = `P_BEQ;		// BEQ
					if (op[1:0] == 2'b01) cur_group = `P_BNE;		// BNE
					if (op[1:0] == 2'b10) cur_group = `P_BLEZ;		// BLEZ
					if (op[1:0] == 2'b11) cur_group = `P_BGTZ;		// BGTZ
					
				// BLTZ, BGEZ, BGEZAL, BLTZAL
				end else if (op == 6'b000001) begin
					if (instr_in[16] == 1'b0)	cur_group = `P_BLTZ;	// BLTZ, BLTZAL	- 0000 01ss sss0 0000 iiii iiii iiii iiii
					else				  		cur_group = `P_BGEZ;	// BGEZ, BGEZAL - 0000 01ss sss0 0001 iiii iiii iiii iiii
			
				// R-type instruction
				end else begin
					if (op == 'b000000) begin
						// ADD, ADDU, SUB, SUBU
						if (funct[5:2] == 4'b1000) begin
							if (funct[1] == 1'b0)	  cur_group = `P_ADD_ADDU;	// ADD, ADDU	- 0000 00ss ssst tttt dddd d000 0010 0000
							else begin
								if (funct[0] == 1'b0) cur_group = `P_SUB;		// SUB
								else				  cur_group = `P_SUBU;		// SUBU
							end
						
						// MUL, MULU, DIV, DIVU
						end else if (funct[5:2] == 4'b0110) begin
							if (funct[1:0] == 2'b00)	cur_group = `P_MULT;		// MULT
							if (funct[1:0] == 2'b01)	cur_group = `P_MULTU;		// MULTU
							if (funct[1] == 1'b1)		cur_group = `P_DIV_DIVU;	// DIV, DIVU	- 0000 00ss ssst tttt 0000 0000 0001 1010

						// AND, OR, XOR
						end else if (funct[5:2] == 4'b1001) begin
							if (funct[1:0] == 2'b00)	cur_group = `P_AND;		// AND
							if (funct[1:0] == 2'b01)	cur_group = `P_OR;		// OR
							if (funct[1:0] == 2'b10)	cur_group = `P_XOR;		// XOR
			
						// MFHI, MFLO, 
						end else if (funct[5:2] == 'b0100) begin
							if (funct[1] == 1'b0)	cur_group = `P_MFHI;		// MFHI
							else					cur_group = `P_MFLO;		// MFLO
							
						// SLL, SRA, SRL, SLLV, SRLV
						end else if (funct[5:3] == 3'b000 && (funct[1] || ~funct[0])) begin
							if (funct[2:0] == 2'b000)	cur_group = `P_SLL;		// SLL,
							if (funct[2:0] == 2'b100)	cur_group = `P_SLLV;	// SLLV
							if (funct[1:0] == 2'b11)	cur_group = `P_SRA;		// SRA
							if (funct[2:0] == 3'b010)	cur_group = `P_SRL;		// SRL
							if (funct[2:0] == 3'b110)	cur_group = `P_SRLV;	// SRLV
	
						// SLT, SLTU
						end else if (funct[5:1] == 5'b10101) begin
							if (funct[0] == 1'b0)	cur_group = `P_SLT;			// SLT			0000 00ss ssst tttt dddd d000 0010 1010
							else					cur_group = `P_SLTU;		// SLTU			0000 00ss ssst tttt dddd d000 0010 1011
						
						// JR, JALR
						end else if (funct[5:1] == 5'b00100) begin
							cur_group = `P_JR;									// JR, JALR		- 0000 00ss sss0 0000 0000 0000 0000 1000
							
						end else begin
							UNKNOWN <= UNKNOWN + 1'b1;
						end

					// ADDI, ADDIU, SLTI, SLTIU, ANDI, ORI, XORI, LUI
					end else if (op[5:3] == 3'b001) begin
						if      (op[2:0] == 3'b101)	cur_group = `P_ORI;			// ORI
						else if (op[2:0] == 3'b111)	cur_group = `P_LUI;			// LUI
						else if (op[2:1] == 2'b00)	cur_group = `P_ADDI_ADDIU;	// ADDI, ADDIU
						else if (op[2:1] == 2'b10)	cur_group = `P_ANDI;		// ANDI
						else if (op[2:0] == 3'b010)	cur_group = `P_SLTI;		// SLTI
						else if (op[1:0] == 2'b11)	cur_group = `P_SLTIU;		// SLTIU
						else if (op[1:0] == 2'b10)	cur_group = `P_XORI;		// XORI
						
					// LBU, LHU
					end else if (op[5:1] == 'b10010) begin
						if (op[0] == 1'b0)	cur_group = `P_LBU;				// LBU
						else				cur_group = `P_LHU;				// LHU
						
					// LH
					end else if (op == 'b100001) begin
						cur_group = `P_LH;									// LH
						
					// LB, LH, LW, SB, SH, SW
					end else if (op[5:4] == 'b10 && ~op[2]) begin
						if (op[1:0] == 2'b11) begin	
							if (op[3] == 1'b0)	cur_group = `P_LW;			// LW
							else				cur_group = `P_SW;			// SW
						end else begin
							if (op[3] == 1'b0)	cur_group = `P_LB;			// LB
							else				cur_group = `P_SB_SH;		// SB, SH
						end
						
					// J, JAL
					end else if (op[5:1] == 'b00001) begin
						if (op[0] == 1'b0)	cur_group = `P_J;				// J
						else				cur_group = `P_JAL;				// JAL

					end else begin
						//UNKNOWN <= UNKNOWN + 1'b1;
					end
				end
			
			// Cache Stall
			end else if (stall) begin
				cur_group = `STALL;
			
			// Pipeline Stall
			end	else begin	
				cur_group = `STALL;
			end
			
			if (init_start | call | ret) begin
				A <= (cur_group == `A);
				B <= (cur_group == `B);
				C <= (cur_group == `C);
				D <= (cur_group == `D);
				E <= (cur_group == `E);
				F <= (cur_group == `F);
				STALL <= (cur_group == `STALL);
				//UNKNOWN <= 'b0;
				
			end else begin
				case (cur_group)
					`A:	A <= A + 1'b1;
					`B:	B <= B + 1'b1;
					`C:	C <= C + 1'b1;
					`D:	D <= D + 1'b1;
					`E:	E <= E + 1'b1;
					`F:	F <= F + 1'b1;
					`STALL:	STALL <= STALL + 1'b1;
					default: UNKNOWN <= UNKNOWN + 1'b1;
				endcase
			end		
		end
	end
endmodule

module CounterStorage (
	// NEED TO THINK ABOUT SATURATION!- need to shift input count as well as the storage RAM
	input clk,
	input reset,
	input call,
	input ret,
	input [`N2-1:0] cur_func_num,
	input [`CW-1:0] count,
	input [`CW-1:0] child_count,
	input init_start,
	input pc_diff,
	
	// For accessing RAM
	output reg ram_wren,
	//output reg ram_clken,
	output reg [`N2-1:0] ram_addr,
	output reg [`CW-1:0] ram_wr_data,
	input [`CW-1:0] ram_rd_data,
	
	
	// temp..
	input retrieve_done
);

	reg main_func;
	reg [1:0] state;
	wire call_ret;
	reg prev_call_ret;
	wire [`CW-1:0] mod_count = count;// + 1'b1;	// +1 to account for the call/ret instruction						///////////// ISSUE FOR MULTIPLE WIDTHS!
	reg [`CW-1:0] prev_child_count;
	
	assign call_ret = (`DO_HIER) ? ret : (ret | call);	// store only on ret if doing hierarchichal
	
	// temp..
	reg [31:0] ctotal;
	reg [31:0] stotal;
	reg display_done;
	reg [27:0] ram_wr_data_disp;
	reg [`CW-1:0] rd_data;
	reg [`CW-1:0] wr_data;
		
	// extend ram_addr during wren so we don't store to the wrong entry (for power)
	//reg [31:0] delayed_func_num;		// power
	always @(posedge(clk)) begin
		//delayed_func_num <= cur_func_num;	// power
		//assign ram_addr = ram_wren ? delayed_func_num : cur_func_num;		// power
		ram_addr <= cur_func_num;	// cycles
		
		if (`PROF_METHOD != "p")
			if (`DO_HIER)	ram_wr_data <= ram_rd_data + wr_data + prev_child_count;	// don't want to use ram_rd_data here -- want to use rd_data to cut critical path
			else			ram_wr_data <= ram_rd_data + wr_data;
		else begin
			ram_wr_data <= { rd_data[159:138] + wr_data[159:138], rd_data[137:116] + wr_data[137:116], rd_data[115:94] + wr_data[115:94], rd_data[93:72] + wr_data[93:72], rd_data[71:50] + wr_data[71:50], rd_data[49:28] + wr_data[49:28], rd_data[27:0] + wr_data[27:0] };
		end
		//ram_wr_data <= csa_adder_result;
	end
	//csa_adder csa_adder_inst (rd_data, wr_data, csa_adder_result);
	
	// whenever call or ret go high, load old value and add count to it
	always @(posedge(clk)) begin
		if (reset | init_start) begin
			ram_wren <= 1'b0;
			//ram_clken <= 1'b0;
			prev_call_ret <= 1'b0;
			state <= 2'b00;
			prev_child_count <= 'b0;
			main_func <= (`DO_HIER) ? 1'b0 : 1'b1;	// main_func used to increment the very first call by 1 if pc_diff is still high, because this means the count didn't propagate
								//	- this is because the pc_diff for the previous instruction always counts for this, except for the first one @ CPI=1
			
		end else begin
			if (state == 2'b00) begin
				prev_call_ret <= call_ret;
				prev_child_count <= child_count;
			
				// if we have a *new* call or ret, read from RAM @ cur_func_num (should be already there)
				if (call_ret & !prev_call_ret) begin
					if (`PROF_METHOD != "p")	wr_data <= mod_count + (pc_diff & main_func);	//
					
					//rd_data <= ram_rd_data;		// want to use rd_data to cut critical path
					
					//ram_wren <= 1'b1;
					state <= 2'b01;
					main_func <= 1'b0;	//	
					
				// if not a call or ret, stop the write!
				end //else begin
					ram_wren <= 1'b0;
				//end	
			end	else if (state == 2'b01) begin
				ram_wren <= 1'b1;
				state <= 2'b00;
			end
		end
	end
	
	/*always @(posedge(clk)) begin
		if (reset | init_start) begin
			ram_wren <= 1'b0;
			//ram_clken <= 1'b0;
			prev_call_ret <= 1'b0;
			//state <= 2'b00;
			prev_child_count <= 'b0;
			main_func <= 1'b1;	// main_func used to increment the very first call by 1 if pc_diff is still high, because this means the count didn't propagate
								//	- this is because the pc_diff for the previous instruction always counts for this, except for the first one @ CPI=1
			
		end else begin
			if (state == 2'b00) begin
				prev_call_ret <= call_ret;
				prev_child_count <= child_count;
			
				// if we have a *new* call or ret, read from RAM @ cur_func_num (should be already there)
				if (call_ret & !prev_call_ret) begin
					if (`DO_HIER) begin
						ram_wr_data <= mod_count + ram_rd_data + prev_child_count + (pc_diff & main_func);	//					///////////// ISSUE FOR MULTIPLE WIDTHS!
					end else begin
						ram_wr_data <= mod_count + ram_rd_data + (pc_diff & main_func);	//										///////////// ISSUE FOR MULTIPLE WIDTHS!
					end
					
					ram_wren <= 1'b1;
					//ram_clken <= 1'b1;
					state <= 2'b00;	// why do i have a state variable... there are no other states...
					main_func <= 1'b0;	//	
					
				// if not a call or ret, stop the write!
				end else begin
					ram_wren <= 1'b0;
					//ram_clken <= 1'b0;
				end	
			end	else if (state == 2'b01) begin
				
		end
	end*/
	
endmodule

module AddressStack (
	input clk,
	input reset,
	input ret,
	input call,
	input [`N2-1:0] func_num_in,
	output [`N2-1:0] func_num_out,
	input pc_diff,
	input init_start,
	input store_first_call,
	
	// For accessing RAM
	output reg ram_wren,
	//output reg ram_clken,
	output reg [`S2-1:0] ram_addr,
	output [`N2-1:0] ram_wr_data,
	input [`N2-1:0] ram_rd_data
);

	reg [`S2-1:0] sp;	// stack pointer
	reg init_call_done;	// so we only do a call to that function once
	
	// Assign wires for RAM
	assign ram_wr_data = func_num_in;
	assign func_num_out = ram_rd_data;
	
	reg state;

	always @(posedge(clk)) begin
		if (reset | (init_start & !store_first_call & !init_call_done)) begin
			sp <= 'b0;
			ram_wren <= 1'b0;
			//ram_clken <= 1'b1;	// not sure how to disable clock and enable it in time
			init_call_done <= 1'b0;
			state <= 1'b0;

		end else if (state == 1'b0) begin
			// pop on a return -- decrement sp then return value @ new sp
			if (ret) begin	//pc_diff & 
				sp <= sp - 1'b1;
				ram_addr <= sp - 1'b1;
				ram_wren <= 1'b0;
				//$display("sp = %d", sp-1);
				state <= 1'b1;
			
			// push on call (when ready) -- store value @ current sp then increment sp
			end else if (call) begin	// & (pc_diff | (store_first_call & !init_call_done))
				sp <= sp + 1'b1;
				ram_addr <= sp;
				ram_wren <= 1'b1;
				init_call_done <= 1'b1;
				//$display("sp = %d", sp+1);
				state <= 1'b1;
			
			// if not call or ret, stop write
			end else begin
				ram_wren <= 1'b0;
				ram_addr <= sp-1'b1;
			end
		end else if (state == 1'b1) begin
			if (!call & !ret) state <= 1'b0;
			ram_wren <= 1'b0;
		end
	end
endmodule

module HierarchyStack (
	input clk,
	input reset,
	input ret,
	input call,
	output reg [`CW-1:0] child_count,
	input [`CW-1:0] count_in,
	//input pc_diff,
	input init_start,
	
	// For accessing RAM
	output reg ram_wren,
	//output reg ram_clken,
	output reg [`S2-1:0] ram_addr,
	output reg [`CW-1:0] ram_wr_data,
	input [`CW-1:0] ram_rd_data
);

	reg [`S2-1:0] sp;	// stack pointer
	reg [1:0] state;
	reg prev_ret;
	reg [`CW-1:0] prev_count;
	reg [`CW-1:0] rd_data;
	
	//wire [`CW-1:0] update_child_count = child_count + ram_rd_data;
	
	// grab the counter value before it's reset @ ret/call!
	always @(posedge(clk)) begin
		if (reset | init_start) begin
			sp <= 'b0;
			ram_wren <= 1'b0;
			//ram_clken <= 1'b1;	// not sure how to disable clock and enable it in time
			child_count <= 'b0;
			state <= 2'b00;
			prev_count <= 'b0;

		end else if (state == 2'b00) begin
			// pop on a return -- decrement sp then return value @ new sp
			if (ret) begin
				sp <= sp - 1'b1;
				ram_addr <= sp - 1'b1;
				ram_wren <= 1'b0;
				prev_ret <= 1'b1;
				state <= 2'b01;
				child_count <= child_count + count_in;
				rd_data <= ram_rd_data;
			
			// push on call (when ready) -- store value @ current sp then increment sp
			end else if (call) begin
				sp <= sp + 1'b1;
				ram_addr <= sp;
				ram_wren <= 1'b1;
				prev_ret <= 1'b0;
				state <= 2'b01;
				ram_wr_data <= child_count + count_in;
			
			// if not call or ret, stop write
			end else begin
				ram_wren <= 1'b0;
				ram_addr <= sp - 1'b1;
			end
		end else if (state == 2'b01) begin
			ram_wren <= 1'b0;
			state <= 2'b00;
			if (prev_ret) child_count <= child_count + rd_data;
			else		  child_count <= 'b0;
		end
	end
endmodule
	
module AddressHash (
	input clk,
	input reset,
	input [31:0] instr_no_lag,
	input instrValid_no_lag,
	input [25:0] addr_in,			// JALr target address
	output [`N2-1:0] funcNum,
	//input enbl_in,					// not used
	input jalr,
	
	input init_start,
	output reg init_done,
	input instr_init,
	
	// For accessing RAM
	output [`N2-1:0] ram_addr,
	input [7:0] ram_rd_data
	//output reg ram_clken
);

	reg [31:0]	V1;
	reg [7:0]	B1;
	reg [7:0]	B2;
	reg [7:0]	A1;
	reg [7:0]	A2;

	reg [31:0] 	a;
	wire [31:0]	b;
	wire [7:0] 	tab_b;
	reg [25:0] 	val;
	reg [`N2-1:0] 	rsl;
	
	reg [3:0] 	state;
	reg [3:0]	iState;
	reg [3:0]   next_state;
	reg [`N2-1:0] init_addr;
	reg [1:0]	rd_cnt;

	assign funcNum = rsl[`N2-1:0];
	assign ram_addr = (init_start & !init_done) ? init_addr : b[`N2-1:0];
	assign tab_b = ram_rd_data;
	
	// decode non-lagged instr
	wire call = (instrValid_no_lag & (instr_no_lag[31:26] == 6'b00_0011));
	//wire [25:0] addr = call ? {6'b0, instr_no_lag[23:0], 2'b0 } : 32'h00800000;
	wire [25:0] addr = call ? jalr ? addr_in : {6'b0, instr_no_lag[23:0], 2'b0 } : 32'h00800000;
	//wire [25:0] addr = jalr ? addr_in : call ? {6'b0, instr_no_lag[23:0], 2'b0 } : 32'h00800000;
	//wire [25:0] addr = jalr ? addr_in : 32'h00800000;
	
	// initialize hash parameters
	always @(posedge(clk)) begin
		if (reset) begin
			iState <= 4'b0000;
			init_done <= 1'b0;
		end else if (init_start & !init_done) begin
			// State 0: Start reading V1
			if (iState == 4'b0000) begin
				init_addr <= `N;
				iState <= 4'b1111;
				next_state <= 4'b0001;
				rd_cnt <= 2'b00;
				
			// State 1: Read next byte of V1
			end else if (iState == 4'b0001) begin
				init_addr <= init_addr + 1'b1;
				rd_cnt <= rd_cnt + 1'b1;
				
				case (rd_cnt)
					2'b00:	V1[31:24] <= ram_rd_data;
					2'b01:	V1[23:16] <= ram_rd_data;
					2'b10:	V1[15:8]  <= ram_rd_data;
					2'b11:	V1[7 :0]  <= ram_rd_data;
				endcase
				
				if (rd_cnt == 2'b11) next_state <= 4'b0010;
				else				 next_state <= 4'b0001;
				
				iState <= 4'b1111;	// go to intermediate state to wait for read data
			
			// State 2: Read A/B
			end else if (iState == 4'b0010) begin
				init_addr <= init_addr + 1'b1;
				rd_cnt <= rd_cnt + 1'b1;
				
				case (rd_cnt)
					2'b00:	A1 <= ram_rd_data;
					2'b01:	A2 <= ram_rd_data;
					2'b10:	B1  <= ram_rd_data;
					2'b11:	B2  <= ram_rd_data;
				endcase
				
				if (rd_cnt == 2'b11) next_state <= 4'b0011;
				else				 next_state <= 4'b0010;
				
				iState <= 4'b1111;	// go to intermediate state to wait for read data
			
			// State 3: Hash initialization is done		
			end else if (iState == 4'b0011) begin
				$display("V1 = %h, A1 = %d, A2 = %d, B1 = %d, B2 = %h", V1, A1, A2, B1, B2);
				init_done <= 1'b1;
				iState <= 4'b0000;
			
			// Delay one cycle for RAM to read data
			end else if (iState == 4'b1111) begin
				iState <= next_state;
			end
				
		end else if (!init_start & init_done) begin	// clear init_done at end of handshake
			init_done <= 1'b0;
		end
	end
	
	wire enbl = instr_init | call | jalr;
	assign b = (val >> B1) & B2;
	
	// state machine for hashing
	always @(posedge(clk)) begin
		if (reset) begin
			state <= 3'b000;
			//ram_clken <= 1'b0;
		end else begin
			if (state == 3'b000) begin
				if (enbl) begin
					//$display("V1 = %h, A1 = %h, A2 = %h, B1 = %h, B2 = %h\n", V1, A1, A2, B1, B2);
													//$display("\naddr = %h", addr);
					val = addr + V1;				//$display("val = %h", val);
					val = val + (val << 8);			//$display("val = %h", val);
					val = val ^ (val >> 4);			//$display("val = %h", val);
			
					state <= 3'b001;
				end else begin
					state <= state;
				end
			end else if (state == 3'b001) begin
				//b = (val >> B1) & B2;			$display("b = %h", b);*/
				a = (val + (val << A1));		//$display("a = %h", a);
				state <= 3'b010;
			
			end else if (state == 3'b010) begin		//$display("tab_b = %h", tab_b);
				rsl = ((a >> A2) ^ tab_b);			//$display("rsl = %h", rsl);
				state <= 3'b000;
			end
		end
	end
endmodule

module DataStorage (
	input clk,
	input reset,
	input init_start,
	input retrieve_start,
	
	// Hash I/Os
	input hash_wren,
	//input hash_clken,
	input [`N2-1:0] hash_addr,
	input [7:0] hash_wr_data,
	output [7:0] hash_rd_data,

	// Address Stack I/Os
	input addr_stack_wren,
	//input addr_stack_clken,
	input [`S2-1:0] addr_stack_addr,
	input [`N2-1:0] addr_stack_wr_data,
	output [`N2-1:0] addr_stack_rd_data,
	
	// Hierarchy Stack I/Os
	input hier_stack_wren,
	//input hier_stack_clken,
	input [`S2-1:0] hier_stack_addr,
	input [`CW-1:0] hier_stack_wr_data,
	output [`CW-1:0] hier_stack_rd_data,
	
	// Storage I/Os
	input storage_wren,
	//input storage_clken,
	input [`N2-1:0] storage_addr,
	input [`CW-1:0] storage_wr_data,
	output [`CW-1:0] storage_rd_data
);
	//always @(hier_stack_addr or hier_stack_rd_data or hier_stack_wr_data)
	//	$display("hier_stack_addr = %h, hier_stack_rd_data = %h, hier_stack_wr_data = %h", hier_stack_addr, hier_stack_rd_data, hier_stack_wr_data);

	// 9bit Address for indexing into M4K (2^9 = 512, 512*9 = 4096)
	wire [8:0] hash_9bit_addr  = { 1'b0, {(8-`N2){1'b0}}, hash_addr [`N2-1:0] };
	wire [8:0] addr_stack_9bit_addr = { 1'b1, {(8-`S2){1'b0}}, addr_stack_addr[`S2-1:0] };
	
	// Extend Stack Data for 8bits
	wire [7:0] addr_stack_8bit_wr_data = { {(8-`N2){1'b0}}, addr_stack_wr_data[`N2-1:0] };
	wire [7:0] addr_stack_8bit_rd_data;
	assign addr_stack_rd_data = addr_stack_8bit_rd_data[`N2-1:0];
	
	reg init_done;
	
	// muxing wires for resetting storage RAM
	wire storage_wren_mux;
	wire [`N2-1:0] storage_addr_mux;
	wire [`CW-1:0] storage_wr_data_mux;
	reg [`N2-1:0] reset_cnt;
	
	assign storage_wren_mux = (init_start & !init_done) ? 1'b1 : (retrieve_start) ? 1'b0 : storage_wren;
	assign storage_addr_mux = (init_start & !init_done) ? reset_cnt : storage_addr;
	assign storage_wr_data_mux = (init_start & !init_done) ? 'b0 : storage_wr_data;
	
	// state machine to reset all storage RAM data
	always @(posedge(clk)) begin
		if (reset) begin
			init_done <= 1'b0;
			reset_cnt <= 'b0;
		end else if (init_start & !init_done) begin
			if (reset_cnt < `N-1)	reset_cnt <= reset_cnt + 1'b1;
			else					init_done <= 1'b1;
		end else if (!init_start & init_done) begin
			init_done <= 1'b0;
			reset_cnt <= 'b0;
		end
	end
	
	// EVENTUALLY NEED TO TRY TO MERGE HIERARCHY STACK INTO LAST QUARTER OF HASH_STACK
	//		- will need to use 64-bit wide RAM with byte-enables
	altsyncram hier_stack (
		.wren_a (hier_stack_wren),
		.clock0 (clk),
		.clocken0 (hier_stack_clken),
		.address_a (hier_stack_addr),
		.data_a (hier_stack_wr_data),
		.q_a (hier_stack_rd_data),
		.aclr0 (1'b0),
		.aclr1 (1'b0),
		.address_b (1'b1),
		.addressstall_a (1'b0),
		.addressstall_b (1'b0),
		.byteena_a (1'b1),
		.byteena_b (1'b1),
		.clock1 (1'b1),
		.clocken1 (1'b1),
		.clocken2 (1'b1),
		.clocken3 (1'b1),
		.data_b (1'b1),
		.eccstatus (),
		.q_b (),
		.rden_a (1'b1),
		.rden_b (1'b1),
		.wren_b (1'b0)
	);
	defparam
		hier_stack.clock_enable_input_a = "BYPASS",
		hier_stack.clock_enable_output_a = "BYPASS",
		hier_stack.intended_device_family = "Cyclone II",
		hier_stack.lpm_hint = "ENABLE_RUNTIME_MOD=NO",
		hier_stack.lpm_type = "altsyncram",
		hier_stack.numwords_a = `S,
		hier_stack.operation_mode = "SINGLE_PORT",
		hier_stack.outdata_aclr_a = "NONE",
		hier_stack.outdata_reg_a = "UNREGISTERED",
		hier_stack.power_up_uninitialized = "FALSE",
		hier_stack.ram_block_type = "M4K",
		hier_stack.widthad_a = `S2,
		hier_stack.width_a = `CW,
		hier_stack.width_byteena_a = 1;
				
	// Instantiate the Hash Array & Stack using AltSyncRam
	altsyncram hash_stack (
		// Hash Signals (Port A)
		.wren_a (hash_wren),
		.address_a (hash_9bit_addr),
		.data_a (hash_wr_data),
		.q_a (hash_rd_data),
		.byteena_a (1'b1),
				
		// Stack Signals (Port B)
		.wren_b (addr_stack_wren),
		.address_b (addr_stack_9bit_addr),
		.data_b (addr_stack_8bit_wr_data),
		.q_b (addr_stack_8bit_rd_data),
		.byteena_b (1'b1),
		
		// Common Signals
		.clock0 (clk),	
		.clocken0 (1'b1),//hash_clken | addr_stack_clken),		//////////
		.aclr0 (1'b0),
		.aclr1 (1'b0),
		.addressstall_a (1'b0),
		.addressstall_b (1'b0),
		.clock1 (1'b1),
		.clocken1 (1'b1),
		.clocken2 (1'b1),
		.clocken3 (1'b1),
		.eccstatus (),
		.rden_a (1'b1),
		.rden_b (1'b1)
	);
	defparam
		hash_stack.address_reg_b = "CLOCK0",
		hash_stack.clock_enable_input_a = "BYPASS",
		hash_stack.clock_enable_input_b = "BYPASS",
		hash_stack.clock_enable_output_a = "BYPASS",
		hash_stack.clock_enable_output_b = "BYPASS",
		hash_stack.indata_reg_b = "CLOCK0",
		hash_stack.intended_device_family = "Cyclone II",
		hash_stack.lpm_type = "altsyncram",
		hash_stack.numwords_a = 512,
		hash_stack.numwords_b = 512,
		hash_stack.operation_mode = "BIDIR_DUAL_PORT",
		hash_stack.outdata_aclr_a = "NONE",
		hash_stack.outdata_aclr_b = "NONE",
		hash_stack.outdata_reg_a = "UNREGISTERED",
		hash_stack.outdata_reg_b = "UNREGISTERED",
		hash_stack.power_up_uninitialized = "FALSE",
		hash_stack.read_during_write_mode_mixed_ports = "DONT_CARE",
		hash_stack.widthad_a = 9,	// (2^9)*8 = 4096.  Just use the whole M4K(s)
		hash_stack.widthad_b = 9,	// (2^9)*8 = 4096.  Just use the whole M4K(s)
		hash_stack.width_a = 8,		// if N <= 256, widthad can be 8.  if more will need separate widths for each port
		hash_stack.width_b = 8,		// if N <= 256, widthad can be 8.  if more will need separate widths for each port
		hash_stack.width_byteena_a = 1,
		hash_stack.width_byteena_b = 1,
		hash_stack.wrcontrol_wraddress_reg_b = "CLOCK0";


	// Instantiate the Storage RAM
	altsyncram storage (
		.wren_a (storage_wren_mux),
		.clock0 (clk),
		.clocken0 (1'b1), //storage_clken),
		.address_a (storage_addr_mux),
		.data_a (storage_wr_data_mux),
		.q_a (storage_rd_data),
		.aclr0 (1'b0),
		.aclr1 (1'b0),
		.address_b (1'b1),
		.addressstall_a (1'b0),
		.addressstall_b (1'b0),
		.byteena_a (1'b1),
		.byteena_b (1'b1),
		.clock1 (1'b1),
		.clocken1 (1'b1),
		.clocken2 (1'b1),
		.clocken3 (1'b1),
		.data_b (1'b1),
		.eccstatus (),
		.q_b (),
		.rden_a (1'b1),
		.rden_b (1'b1),
		.wren_b (1'b0)
	);
	defparam
		storage.clock_enable_input_a = "BYPASS",
		storage.clock_enable_output_a = "BYPASS",
		storage.intended_device_family = "Cyclone II",
		storage.lpm_hint = "ENABLE_RUNTIME_MOD=NO",
		storage.lpm_type = "altsyncram",
		storage.numwords_a = `N,
		storage.operation_mode = "SINGLE_PORT",
		storage.outdata_aclr_a = "NONE",
		storage.outdata_reg_a = "UNREGISTERED",
		storage.power_up_uninitialized = "FALSE",
		storage.ram_block_type = "M4K",
		storage.widthad_a = `N2,
		storage.width_a = `CW,
		storage.width_byteena_a = 1;
endmodule

module vInitializer (
	input clk,
	input reset,
	output reset_out,
	input [25:0] pc,
	
	input init_start,
	output reg init_done,
	output reg init_hash_start,
	input init_hash_done,
	output reg hash_first_start,
	input hash_first_done,
	
	input [7:0] hashRdData,	// temp for debugging
	output reg hashWren,
	output reg [`N2-1:0] hashAddr,
	output reg [7:0] hashWrData,
	
	//Avalon Bus side signals
	output reg avm_profileMaster_read,
	output reg avm_profileMaster_write,
	output reg [31:0]avm_profileMaster_address,
	output reg [31:0]avm_profileMaster_writedata,
	output reg [3:0]avm_profileMaster_byteenable,
	input avm_profileMaster_waitrequest,
	input [31:0]avm_profileMaster_readdata,
	input avm_profileMaster_readdatavalid
);
	/* What to do...
	 *	- Go when PC > 0x800000 (ie PC[23] == 1, also use a Done signal
	 *	- Stall processor (or have it poll on a mem-mapped addr in sdram?)	-- poll b/c this is processor-independent
	 *	- reset address stack's SP (send reset signal?)
	 *	- reset instr. counter's counter (send reset signal?)
	 *	- flush buffer (if it still exists)
	 *	- reset counter storage's RAM (aClr signal?)
	 *	- store each value for tab[] into RAM
	 *	- store V1/A1/A2/B1/B2 into address hash
	 *	- Unstall processor (or write to mem-mapped addr in sdram?)	-- poll b/c this is processor-independent
	 */

	reg [2:0] state;
	reg [2:0] next_state;
	reg [25:0] sdram_addr;
	reg [`N2:0] read_count;
	reg [2:0] hashByte;
	reg [31:0] hashWrData32;
	reg needData;
	
	assign reset_out = (init_start & !init_done & state==3'b000);

	// Read each value from counter storage's RAM, write to SDRAM
	always @(posedge(clk)) begin
		if (reset) begin
			state <= 3'b000;
			avm_profileMaster_write <= 1'b0;
			avm_profileMaster_read <= 1'b0;
			init_done <= 1'b0;
			init_hash_start <= 1'b0;
			hash_first_start <= 1'b0;
			hashByte <= 3'b000;
			needData <= 1'b0;	// so we can start the next sdram read immediately
		end else begin
			if (init_start & !init_done) begin
				// State 0: Reset profiler
				if (state == 3'b000) begin
					// get sdram read ready
					avm_profileMaster_byteenable <= 4'b1111;
					sdram_addr <= `PROF_ADDR + `N;
					
					// get ready to read V1/A/B from SDRAM
					avm_profileMaster_address <= `PROF_ADDR + `N;
					avm_profileMaster_read <= 1'b1;
					read_count <= 'b0;
					needData <= 1'b1;		// this isn't working as well as i'd like -- fix eventually
											// idea is to start the next read to sdram immediately so theres never a wait after writing to RAM
					
					state <= 3'b001;
				
				// State 1: Read V1/A1/A2/B1/B2 from SDRAM, write to hash RAM
				end else if (state == 3'b001) begin
					if (!avm_profileMaster_waitrequest) begin
						if (needData) begin
							avm_profileMaster_read <= 1'b1;
							avm_profileMaster_address <= avm_profileMaster_address + 'h4;
							needData <= 1'b0;
						end else begin						
							avm_profileMaster_read <= 1'b0;
						end
					end		
				
				
					if (avm_profileMaster_readdatavalid) begin
						//$display("V1/A/B (%d) = %h at address %h", read_count, avm_profileMaster_readdata, sdram_addr);
						
						// prepare write-out to hash BRAM
						state <= 3'b111;
						hashAddr <= read_count + `N;
						hashByte <= 3'b001;		// since i'm passing in the first byte now, don't re-send it next cycle
						hashWrData32 <= avm_profileMaster_readdata;
						hashWrData <= avm_profileMaster_readdata[31:24];
						read_count <= read_count + 'h4;
						sdram_addr <= sdram_addr + 26'h4;	// this variable is only used for debug output
						
						// stop after V1/A/B are fully read & written
						if (read_count >= 'h8) begin		// `N-4 b/c when reading at 28, we read the 28th, 29th, 30th, and 31st bits so we're done
							read_count <= 'b0;
							hashWren <= 1'b0;
							next_state <= 3'b010;
						end else begin
							hashWren <= 1'b1;
							next_state <= 3'b001;
							avm_profileMaster_read <= 1'b1;	// get next sdram read started
						end
						
						if (read_count >= 'h4) needData <= 1'b0;
						else				   needData <= 1'b1;
					end
					
				// State 2: Initialize Hash (Put V1/A/B into registers)
				end else if (state == 3'b010) begin
					if (!init_hash_done) begin
						init_hash_start <= 1'b1;
						//avm_profileMaster_read <= 1'b0;
					end else if (!avm_profileMaster_waitrequest) begin	
						init_hash_start <= 1'b0;
						avm_profileMaster_read <= 1'b1;
						avm_profileMaster_address <= `PROF_ADDR;
						sdram_addr <= `PROF_ADDR;
						state <= 3'b011;
					end
					
				// State 3: Read tab[] from sdram, put to hash RAM
				end else if (state == 3'b011) begin
					// if we're not done getting tab[], keep reading
					if (!avm_profileMaster_waitrequest) begin
						if (needData) begin
							avm_profileMaster_read <= 1'b1;
							avm_profileMaster_address <= avm_profileMaster_address + 'h4;
							needData <= 1'b0;
						end else begin					
							avm_profileMaster_read <= 1'b0;
						end
					end				
					
					if (avm_profileMaster_readdatavalid) begin
						//$display("Tab[%d] = %h at address %h", read_count, avm_profileMaster_readdata, sdram_addr);
						
						// prepare write-out to hash BRAM
						state <= 3'b111;
						hashAddr <= read_count[`N2-1:0];
						hashByte <= 3'b001;		// since i'm passing in the first byte now, don't re-send it next cycle
						hashWrData32 <= avm_profileMaster_readdata;
						hashWrData <= avm_profileMaster_readdata[31:24];
						sdram_addr <= sdram_addr + 26'h4;	// this variable is only used for debug output
						
						// if we've written all 4 bytes AND we're done reading from sdram, start reading V1
						if (read_count >= (`N-4)) begin		// `N-4 b/c when reading at 28, we read the 28th, 29th, 30th, and 31st bits so we're done
							read_count <= 'b0;
							//avm_profileMaster_read <= 1'b0;
							hashWren <= 1'b0;
							next_state <= 3'b100;
						end else begin	// otherwise, come back here when bram's done
							//avm_profileMaster_read <= 1'b1;
							read_count <= read_count + 'h4;
							hashWren <= 1'b1;
							next_state <= 3'b011;
							needData <= 1'b1;
						end
					end
				
				// State 4: Add first function (main/wrapper) to stack
				end else if (state == 3'b100) begin
					//if (!avm_profileMaster_waitrequest) avm_profileMaster_read <= 1'b0;
					hash_first_start <= 1'b1;
					if (hash_first_done) begin
						hash_first_start <= 1'b0;
						state <= 3'b101;
					end
					
				// State 5: Tell processor to continue
				end else if (state == 3'b101) begin
					if (!avm_profileMaster_waitrequest) begin		// shouldn't need this if statement... was complaining though
						avm_profileMaster_read <= 1'b0;
						avm_profileMaster_write <= 1'b1;
						avm_profileMaster_address <= `STACK_ADDR;
						avm_profileMaster_writedata <= 32'hDEADBEEF;
						state <= 3'b110;
					end						

				// State 6: Wait until write finished
				end else if (state == 3'b110) begin
					if (!avm_profileMaster_waitrequest) begin
						avm_profileMaster_write <= 1'b0;
						if (pc >= `WRAP_MAIN_BEGIN & pc <= `WRAP_MAIN_END) begin
							init_done <= 1'b1;
							state <= 3'b000;
						end
					end
					
					if (avm_profileMaster_waitrequest === 1'bx) begin	
						$display("avm_profileMaster_waitrequest = X");
						$stop;
					end
				
				// State 7: Write the 4 bytes to the hash RAM	
				end else if (state == 3'b111) begin
					// if we haven't written all 4 bytes, keep going
					if (hashByte < 3'b100) begin
						hashByte <= hashByte + 1'b1;
						hashAddr <= hashAddr + 1'b1;
										
						// choose byte to send					
						case (hashByte)
							//3'b000: hashWrData <= avm_profileMaster_readdata[31:24];
							3'b001: hashWrData <= hashWrData32[23:16];
							3'b010: hashWrData <= hashWrData32[15:8];
							3'b011: hashWrData <= hashWrData32[7:0];
						endcase
						
					end else begin
						hashWren <= 1'b0;
						state <= next_state;
						//if (next_state != 'b100) avm_profileMaster_read <= 1'b1;
					end
				end

			end else if (!init_start & init_done) begin
				init_done <= 1'b0;	// reset init_done so handshaking can occur again
			end
		end
	end	
endmodule

module vRetriever (
	input clk,
	input reset,
	input [25:0] pc,
	input retrieve_start,
	output reg retrieve_done,
	input [`CW-1:0] prof_data,
	output reg [`N2-1:0] prof_index,
	
	//Avalon Bus side signals
	output reg avm_profileMaster_write,
	output reg [31:0]avm_profileMaster_address,
	output reg [31:0]avm_profileMaster_writedata,
	output reg [3:0]avm_profileMaster_byteenable,
	input avm_profileMaster_waitrequest
);

	reg [2:0] state;
	reg [`N2:0] prof_count;		// used b/c prof_index can only go up to `N-1, so is always < `N (circular)
	
	// Read each value from counter storage's RAM, write to SDRAM
	always @(posedge(clk)) begin
		if (reset) begin
			retrieve_done <= 1'b0;
			state <= 3'b000;
			prof_index <= 'b0;
			avm_profileMaster_write <= 1'b0;
		end else begin
			if (retrieve_start & !retrieve_done) begin
				//$display("state = %d, CW = %d", state, `CW);
				// State 0: Setup muxes
				if (state == 3'b000) begin
					avm_profileMaster_byteenable <= 4'b1111;
					prof_index <= 'b0;
					prof_count <= 'b0;
					state <= 3'b001;
					$display("Profile Results:");
				end
				
				// State 1: Prepare/write data to sdram	-- PROBLEM, 64 bits will require 2 writes/reads
				else if (state == 3'b001) begin
					if(!avm_profileMaster_waitrequest) begin
						if (prof_count < `N) begin
							`ifdef CW64
								avm_profileMaster_address <= `PROF_ADDR + { prof_index, 3'b000 }; //{ PROF_ADDR[31:`N2+2], prof_index, 2'b0 };
							`else
								avm_profileMaster_address <= `PROF_ADDR + { prof_index, 2'b00 }; //{ PROF_ADDR[31:`N2+2], prof_index, 2'b0 };
							`endif
							avm_profileMaster_write <= 1'b1;
							avm_profileMaster_writedata <= prof_data;

							prof_index <= prof_index + 1'b1;							
							prof_count <= prof_count + 1'b1;	
							$display("%4d %4d", prof_index, prof_data);
							`ifdef CW64
								$display("CW > 32...");
								state <= 3'b011;
							`endif
							
						end else begin
							avm_profileMaster_address <= `STACK_ADDR;
							avm_profileMaster_write <= 1'b1;
							avm_profileMaster_writedata <= 32'hCADFEED;
							state <= 3'b010;
						end
					end
				end 
				
				// State 2: Wait until unstall write is finished			
				else if (state == 3'b010) begin
					if(!avm_profileMaster_waitrequest) begin
						avm_profileMaster_write <= 0;
						state <= 3'b000;
						retrieve_done <= 1'b1;
						$display("Done");
						#1000000 $stop;		// uncomment this to have results in modelsim waveform right at end
					end
				end
				
				// State 3: Write out top 32 bits for a 64-bit counter
				`ifdef CW64
				else if (`CW>32 & state == 3'b011) begin
					if(!avm_profileMaster_waitrequest) begin
						avm_profileMaster_address <= `PROF_ADDR + { prof_index, 3'b100 }; //{ PROF_ADDR[31:`N2+2], prof_index, 2'b0 };
						avm_profileMaster_write <= 1'b1;
						avm_profileMaster_writedata <= prof_data[63:32];
						
						state <= 3'b001;
						//$display("prof_data = %4d  @  %4d", prof_data, prof_index);
					end
				end
				`endif

			end else if (!retrieve_start & retrieve_done) begin
				retrieve_done <= 1'b0;	// reset retrieve_done so handshaking can occur again
			end
		end
	end	
endmodule

/*/ Retrieve Power Results (assuming entry size of 160=32x5 bits)
module vPowerRetriever (	// comment out when not in use to avoid compiler errors
	input clk,
	input reset,
	input [25:0] pc,
	input retrieve_start,
	output reg retrieve_done,
	input [`CW-1:0] prof_data,
	output reg [`N2-1:0] prof_index,
	
	//Avalon Bus side signals
	output reg avm_profileMaster_write,
	output reg [31:0]avm_profileMaster_address,
	output reg [31:0]avm_profileMaster_writedata,
	output reg [3:0]avm_profileMaster_byteenable,
	input avm_profileMaster_waitrequest
);

parameter PROF_METHOD = `PROF_METHOD;
generate
if (PROF_METHOD=="p") begin
	// for output
	reg [31:0] ctotal;
	reg [31:0] stotal;

	reg [2:0] state;
	reg [`N2:0] prof_count;		// used b/c prof_index can only go up to `N-1, so is always < `N (circular)
	reg [31:0] prof_addr;		// this is to just add 32bits to the address instead of doing shifting/etc.
	reg [2:0] repeat_count;		// used to count out all 32-bit words to write to sdram
	reg [159:0] prof_data_writeout;	// so we can start the next RAM read and not lose the current data
	
	// Read each value from counter storage's RAM, write to SDRAM
	always @(posedge(clk)) begin
		if (reset) begin
			retrieve_done <= 1'b0;
			state <= 3'b000;
			prof_index <= 'b0;
			avm_profileMaster_write <= 1'b0;
		end else begin
			if (retrieve_start & !retrieve_done) begin
				//$display("state = %d, CW = %d", state, `CW);
				// State 0: Setup muxes
				if (state == 3'b000) begin
					avm_profileMaster_byteenable <= 4'b1111;
					prof_index <= 'b0;
					prof_count <= 'b0;
					prof_addr <= `PROF_ADDR;
					state <= 3'b001;
					ctotal <= 'b0;
					stotal <= 'b0;
					$display("Profile Results:");
				end
				
				// State 1: Prepare/write data to sdram	-- PROBLEM, 64 bits will require 2 writes/reads
				else if (state == 3'b001) begin
					if(!avm_profileMaster_waitrequest) begin
						if (prof_count < `N) begin
							avm_profileMaster_address <=  prof_addr;
							avm_profileMaster_write <= 1'b1;
							avm_profileMaster_writedata <= prof_data[159:128];	// just at intervals of 32 for sdram block size
							prof_data_writeout <= prof_data;

							prof_addr <= prof_addr + 32'h4;		// 4 bytes = 32 bits = one sdram entry
							prof_index <= prof_index + 1'b1;	// start the next RAM read so its ready in time
							repeat_count <= 'b0;
							
							$display("%4d %4d|%4d|%4d|%4d|%4d|%4d|%4d     %5d", prof_index, prof_data[159:138], prof_data[137:116], prof_data[115:94], prof_data[93:72], prof_data[71:50], prof_data[49:28], prof_data[27:0], (prof_data[159:138] + prof_data[137:116] + prof_data[115:94] + prof_data[93:72] + prof_data[71:50] + prof_data[49:28]));
							ctotal <= ctotal + (prof_data[159:138] + prof_data[137:116] + prof_data[115:94] + prof_data[93:72] + prof_data[71:50] + prof_data[49:28]);
							stotal <= stotal + prof_data[27:0];
							state <= 3'b011;
							
						end else begin
							$display("Total Cycles: %5d, Total Stalls: %5d", ctotal, stotal);
							avm_profileMaster_address <= `STACK_ADDR;
							avm_profileMaster_write <= 1'b1;
							avm_profileMaster_writedata <= 32'hCADFEED;
							state <= 3'b010;
						end
					end
				end 
				
				// State 2: Wait until unstall write is finished			
				else if (state == 3'b010) begin
					if(!avm_profileMaster_waitrequest) begin
						avm_profileMaster_write <= 0;
						state <= 3'b000;
						retrieve_done <= 1'b1;
						$display("Done");
						#1000000 $finish;		// uncomment this to have results in modelsim waveform right at end
					end
				end
				
				// State 3: Write all 160 bits to SDRAM
				else if (state == 3'b011) begin
					if(!avm_profileMaster_waitrequest) begin
						if (repeat_count < 3'b100) begin
							avm_profileMaster_address <= prof_addr;
							avm_profileMaster_write <= 1'b1;
							prof_addr <= prof_addr + 32'h4;
							
							// choose word to send					
							case (repeat_count)
								3'b000: avm_profileMaster_writedata <= prof_data_writeout[127:96];
								3'b001: avm_profileMaster_writedata <= prof_data_writeout[95:64];
								3'b010: avm_profileMaster_writedata <= prof_data_writeout[63:32];
								3'b011: avm_profileMaster_writedata <= prof_data_writeout[31:0];
							endcase
							repeat_count <= repeat_count + 1'b1;
						
						end else begin
							prof_count <= prof_count + 1'b1;		
							avm_profileMaster_write <= 1'b0;
							state <= 3'b001;
						end
					end
				end

			end else if (!retrieve_start & retrieve_done) begin
				retrieve_done <= 1'b0;	// reset retrieve_done so handshaking can occur again
			end
		end
	end
end
endgenerate
endmodule
*/