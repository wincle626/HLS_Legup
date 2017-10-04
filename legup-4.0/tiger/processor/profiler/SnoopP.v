`include "../../profiler/prof_defines.v"

// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

 module SnoopP (
	input clk,
	input reset,
	input [25:0] pc,
	input [31:0] instr_in,
	input stall,
	
	// TESTER PORTS ONLY
	input init_start,
	output init_done,
	input retrieve_start,
	output retrieve_done,
	
	// Avalon Profile Master ports
	output avm_profileMaster_read,
	output avm_profileMaster_write,
	output [31:0]avm_profileMaster_address,
	output [31:0]avm_profileMaster_writedata,
	output [3:0] avm_profileMaster_byteenable,
	input [31:0]avm_profileMaster_readdata,
	input avm_profileMaster_waitrequest,
	input avm_profileMaster_readdatavalid,
	
	output [`CW-1:0] rProfData,
	output [`N2-1:0] rProfIndex
);

	wire [`N2-1:0] 	addr_a;
	wire [`N2-1:0] 	count_a;
	wire [25:0] 	addr_lo_data;
	wire [25:0] 	addr_hi_data;
	wire [`CW-1:0]	count_data;
	wire			prof_init_start;
	wire			prof_init_done;
	

	assign rProfData = count_data;
	assign rProfIndex = count_a;
	
	// mux avalon signals
	wire			init_avm_profileMaster_write;
	wire [31:0]		init_avm_profileMaster_address;
	wire [31:0]		init_avm_profileMaster_writedata;
	wire			retrieve_avm_profileMaster_write;
	wire [31:0]		retrieve_avm_profileMaster_address;
	wire [31:0]		retrieve_avm_profileMaster_writedata;
		
	assign avm_profileMaster_write = (init_start & !init_done) ? init_avm_profileMaster_write : retrieve_avm_profileMaster_write;
	assign avm_profileMaster_address = (init_start & !init_done) ? init_avm_profileMaster_address : retrieve_avm_profileMaster_address;
	assign avm_profileMaster_writedata = (init_start & !init_done) ? init_avm_profileMaster_writedata : retrieve_avm_profileMaster_writedata;

	Profiler profile (
		.clk(clk),
		.reset(reset),
		.pc(pc),
		.stall(stall),
		
		.addr_a(addr_a),
		.count_a(count_a),
		
		.addr_lo_data(addr_lo_data),
		.addr_hi_data(addr_hi_data),
		.count_data(count_data),
		
		.global_init(init_start | retrieve_start),
		.init_start(prof_init_start),
		.init_done(prof_init_done)
	);
		
	sInitializer init (
		.clk(clk),
		.reset(reset),
		.pc(pc),
		
		.init_start(init_start),
		.init_done(init_done),
		
		.addr_a(addr_a),
		.addr_lo_data(addr_lo_data),
		.addr_hi_data(addr_hi_data),
		
		.prof_init_start(prof_init_start),
		.prof_init_done(prof_init_done),

		//Avalon Bus side signals
		.avm_profileMaster_read(avm_profileMaster_read),
		.avm_profileMaster_write(init_avm_profileMaster_write),
		.avm_profileMaster_address(init_avm_profileMaster_address),
		.avm_profileMaster_writedata(init_avm_profileMaster_writedata),
		.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest),
		.avm_profileMaster_readdata(avm_profileMaster_readdata),
		.avm_profileMaster_readdatavalid(avm_profileMaster_readdatavalid)
	);

	sRetriever retrieve (
		.clk(clk),
		.reset(reset),
		.pc(pc),
		
		.retrieve_start(retrieve_start),
		.retrieve_done(retrieve_done),
		
		.count_a(count_a),
		.count_data(count_data),
		
		//Avalon Bus side signals
		.avm_profileMaster_write(retrieve_avm_profileMaster_write),
		.avm_profileMaster_address(retrieve_avm_profileMaster_address),
		.avm_profileMaster_writedata(retrieve_avm_profileMaster_writedata),
		.avm_profileMaster_waitrequest(avm_profileMaster_waitrequest)
	);

endmodule

module Profiler (
	input clk,
	input reset,
	input [25:0] pc,
	input stall,
	
	// addresses to index into register arrays
	input [`N2-1:0] addr_a,
	input [`N2-1:0] count_a,
	
	// values returned from register arrays
	input [25:0] addr_lo_data,
	input [25:0] addr_hi_data,
	output [`CW-1:0] count_data,
	
	// init handshaking signals
	input global_init,	// to tell if the system is initializing
	input init_start,	// tell if new data is ready to write into reg's
	output reg init_done
);

	// Register arrays to store addr_lo/addr_hi/counters
	reg [25:0] addr_lo [`N-1:0];
	reg [25:0] addr_hi [`N-1:0];
	reg [`CW-1:0] count [`N-1:0];
	assign count_data = count[count_a];
	
	// initialize addr ranges
	always @(posedge(clk)) begin
		if (reset) begin
			init_done <= 1'b0;
		end else begin
			if (init_start & !init_done) begin
				//$display("addr_a = %h, addr_lo_data = %h, addr_hi_data = %h", addr_a, addr_lo_data, addr_hi_data);
				addr_lo[addr_a] <= addr_lo_data;
				addr_hi[addr_a] <= addr_hi_data;
				init_done <= 1'b1;
			end else if (!init_start & init_done) begin
				init_done <= 1'b0;
			end
		end
	end

	// setup each addr range profiler
	integer i;
	reg [`CW-1:0] count_val;
	reg [`N2-1:0] count_cur;
	always @(posedge(clk)) begin
		if (reset) begin
			// Reset Counters
			for (i=0; i<`N; i=i+1)
				count[i] <= 'b0;
	
		end else begin
			if (!global_init) begin
				if (stall | (`PROF_METHOD!="s")) begin	// only count on stalls (or always if not measuring stalls)
					for (i=0; i<`N; i=i+1) begin
						if (pc >= addr_lo[i] & pc <= addr_hi[i]) begin	// reading addr_**_data from init reg arrays
							count[i] <= count[i] + 1'b1;
							count_val <= count[i] + 1'b1;
							count_cur <= i;
						end
					end
				end
			end
		end
	end

endmodule

module sInitializer ( // for sdram
	input clk,
	input reset,
	input [25:0] pc,
	
	input init_start,
	output reg init_done,
	
	output reg [`N2-1:0] addr_a,
	output reg [25:0] addr_lo_data,
	output reg [25:0] addr_hi_data,
	
	output reg prof_init_start,
	input prof_init_done,
	
	//Avalon Bus side signals
	output reg avm_profileMaster_read,
	output reg avm_profileMaster_write,
	output reg [31:0]avm_profileMaster_address,
	output reg [31:0]avm_profileMaster_writedata,
	input avm_profileMaster_waitrequest,
	input [31:0]avm_profileMaster_readdata,
	input avm_profileMaster_readdatavalid
);

	reg [2:0] state;
	reg [2:0] next_state;
	reg [25:0] sdram_addr;
	reg [`N2+1:0] read_count;
	wire [`N2-1:0] reg_addr = read_count[`N2:1];	// use to divide by 2 since lo and hi reg's alternate
	
	// Read each value from counter storage's RAM, write to SDRAM
	always @(posedge(clk)) begin
		if (reset) begin
			state <= 3'b000;
			avm_profileMaster_write <= 1'b0;
			avm_profileMaster_read <= 1'b0;
			avm_profileMaster_writedata <= 'b0;
			init_done <= 1'b0;
			prof_init_start <= 1'b0;
		end else begin
			if (init_start & !init_done) begin
				// State 0: Reset profiler
				if (state == 3'b000) begin
					// get ready to read ranges from SDRAM
					avm_profileMaster_address <= `PROF_ADDR;
					sdram_addr <= `PROF_ADDR;
					avm_profileMaster_read <= 1'b1;
					prof_init_start <= 1'b1;
					
					read_count <= 'b0;
					state <= 3'b001;
					
				end else if (state == 3'b001) begin
					// Handshaking with profiler module
					if (prof_init_start & prof_init_done)
						prof_init_start <= 1'b0;
				
					//If wait request is low we can give another address to read from
					if(!avm_profileMaster_waitrequest) begin
						//If we've given address for all the blocks we want, stop reading
						if (avm_profileMaster_address == `PROF_ADDR + (2*4*`N))
							avm_profileMaster_read <= 1'b0;
						else
							avm_profileMaster_address <= avm_profileMaster_address + 4'h4;
					end
					
					//If we have valid data
					if(avm_profileMaster_readdatavalid) begin
						// if we've read all addr_lo/addr_hi, tell processor to unstall
						if (read_count == (2*`N)) begin
							avm_profileMaster_read <= 1'b0;
							avm_profileMaster_address <= `STACK_ADDR;
							avm_profileMaster_writedata <= 32'hDEADBEEF;
							state <= 3'b011;
						end else begin
							read_count <= read_count + 1'b1;
							sdram_addr <= sdram_addr + 4'h4;	// this variable is only used for debug output
							
							// store data
							if (read_count[0]) begin
								//$display("addr_hi[%d] = %h at address %h", reg_addr, avm_profileMaster_readdata, sdram_addr);
								addr_hi_data <= avm_profileMaster_readdata; // store to addr_lo registers
								addr_a <= reg_addr;
								prof_init_start <= 1'b1;
							end else begin
								//$display("addr_lo[%d] = %h at address %h", reg_addr, avm_profileMaster_readdata, sdram_addr);
								addr_lo_data <= avm_profileMaster_readdata; // store to addr_lo registers
							end
						end
					end
				
				// State 3: Tell processor to continue
				end else if (state == 3'b011) begin
					if (!avm_profileMaster_waitrequest) begin
						avm_profileMaster_write <= 1'b1;
						state <= 3'b100;
					end
					
				// State 4: Wait until write finished
				end else if (state == 3'b100) begin
					if (!avm_profileMaster_waitrequest) begin
						avm_profileMaster_write <= 1'b0;
						init_done <= 1'b1;
						state <= 3'b000;
					end
				end

			end else if (!init_start & init_done) begin
				init_done <= 1'b0;	// reset init_done so handshaking can occur again
			end
		end
	end	
endmodule

module sRetriever (
	input clk,
	input reset,
	input [25:0] pc,
	
	input retrieve_start,
	output reg retrieve_done,
	
	output reg [`N2-1:0] count_a,
	input [`CW-1:0] count_data,
	
	//Avalon Bus side signals
	output reg avm_profileMaster_write,
	output reg [31:0]avm_profileMaster_address,
	output reg [31:0]avm_profileMaster_writedata,
	input avm_profileMaster_waitrequest
);
	
	reg [2:0] state;
	reg [`N2:0] prof_count;		// used b/c prof_index can only go up to `N-1, so is always < `N (circular)
	
	// Read each value from counter storage's RAM, write to SDRAM
	always @(posedge(clk)) begin
		if (reset) begin
			retrieve_done <= 1'b0;
			state <= 3'b000;
			count_a <= 'b0;
			avm_profileMaster_write <= 1'b0;
		end else begin
			if (retrieve_start & !retrieve_done) begin
				// State 0: Setup muxes
				if (state == 3'b000) begin
					count_a <= 'h0;
					prof_count <= 'b0;
					state <= 3'b001;
				end
				
				// State 1: Prepare/write data to sdram
				else if (state == 3'b001) begin
					if(!avm_profileMaster_waitrequest) begin
						if (prof_count < `N) begin
							`ifdef CW64
								avm_profileMaster_address <= `PROF_ADDR + { count_a, 2'b000 };
							`else
								avm_profileMaster_address <= `PROF_ADDR + { count_a, 2'b00 };
							`endif
							
							avm_profileMaster_write <= 1'b1;
							avm_profileMaster_writedata <= count_data;

							count_a <= count_a + 1'b1;							
							prof_count <= prof_count + 1'b1;	
							$display("prof_data = %4d  @  %4d", count_data, count_a);
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
						#1000000 $finish;		// uncomment this to have results in modelsim waveform right at end
					end
				end
				
				// State 3: Write out top 32 bits for a 64-bit counter
				`ifdef CW64
				else if (`CW>32 & state == 3'b011) begin
					if(!avm_profileMaster_waitrequest) begin
						avm_profileMaster_address <= `PROF_ADDR + { count_a, 3'b100 };
						avm_profileMaster_write <= 1'b1;
						avm_profileMaster_writedata <= count_data[63:32];
						
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
