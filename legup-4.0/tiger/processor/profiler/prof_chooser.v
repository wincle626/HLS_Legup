`include "prof_defines.v"

module profiler_chooser (
	input clk,
	input reset,
	input [25:0] pc,
	input [31:0] instr,
	input stall,
	
	// Avalon Profile Master ports
	output avm_profileMaster_read,
	output avm_profileMaster_write,
	output [31:0]avm_profileMaster_address,
	output [31:0]avm_profileMaster_writedata,
	output [3:0]avm_profileMaster_byteenable,
	input [31:0]avm_profileMaster_readdata,
	input avm_profileMaster_waitrequest,
	input avm_profileMaster_readdatavalid
);

	// init/retrieve signals
	reg init_start;
	wire init_done;
	reg retrieve_start;
	wire retrieve_done;
	always @(posedge(clk)) begin
		if (reset) begin
			init_start <= 1'b0;
			retrieve_start <= 1'b0;
		end else begin
			if (pc == `WRAP_MAIN_BEGIN) begin	
				init_start <= 1'b1;
			end else if (pc == `WRAP_MAIN_END) begin
				retrieve_start <= 1'b1;
			end else if (init_done) begin
				init_start <= 1'b0;
			end else if (retrieve_done) begin
				retrieve_start <= 1'b0;
			end else begin
				init_start <= init_start;
				retrieve_start <= retrieve_start;
			end
		end
	end
	
	//
	wire [`CW-1:0] rProfData;
	wire [`N2-1:0] rProfIndex;
	
	parameter PROF_TYPE = `PROF_TYPE;
	generate
		if (PROF_TYPE=="L") begin
			// LEAP -- sdram.dat needs to contain hash data
			leap leap_inst (
				.clk(clk),
				.glob_reset(reset),
				.pc_in(pc),
				.instr_in2(instr),
				.instrEx_in(instr),
				.istall_in(stall),
				.dstall_in(stall),
				.insValid_in(1'b1),
				
				
				// TESTER PORTS ONLY
				.init_start_in(init_start),
				.init_done(init_done),
				.retrieve_start_in(retrieve_start),
				.retrieve_done(retrieve_done),
				
				// Avalon Profile Master ports
				.avm_profileMaster_read(avm_profileMaster_read),
				.avm_profileMaster_write(avm_profileMaster_write),
				.avm_profileMaster_address(avm_profileMaster_address),
				.avm_profileMaster_writedata(avm_profileMaster_writedata),
				.avm_profileMaster_byteenable(avm_profileMaster_byteenable),
				.avm_profileMaster_readdata(avm_profileMaster_readdata),
				.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest),
				.avm_profileMaster_readdatavalid(avm_profileMaster_readdatavalid),
				
				.rProfData(rProfData),
				.rProfIndex(rProfIndex)
			);
		end else begin
			// SnoopP -- sdram.dat needs to contain function ranges
			SnoopP snoop (
				.clk(clk),
				.reset(reset),
				.pc(pc),
				.instr_in(instr),
				
				// TESTER PORTS ONLY
				.init_start(init_start),
				.init_done(init_done),
				.retrieve_start(retrieve_start),
				.retrieve_done(retrieve_done),
				
				// Avalon Profile Master ports
				.avm_profileMaster_read(avm_profileMaster_read),
				.avm_profileMaster_write(avm_profileMaster_write),
				.avm_profileMaster_address(avm_profileMaster_address),
				.avm_profileMaster_writedata(avm_profileMaster_writedata),
				//.avm_profileMaster_byteenable(avm_profileMaster_byteenable),
				.avm_profileMaster_readdata(avm_profileMaster_readdata),
				.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest),
				.avm_profileMaster_readdatavalid(avm_profileMaster_readdatavalid),
				
				
				.rProfData(rProfData),
				.rProfIndex(rProfIndex)
			);
			assign avm_profileMaster_byteenable = 'b1111;
		end
	endgenerate
endmodule
