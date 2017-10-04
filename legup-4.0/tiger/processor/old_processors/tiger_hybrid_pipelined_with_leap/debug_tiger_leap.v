module debug_tiger_leap (
	input clk,
	input iCacheStall,
	input dCacheStall,
	input [31:0] pc,
	input [31:0] pc_r,
	input [31:0] pc_rr,
	input [31:0] pc_rrr,
	input [31:0] ins,
	input [31:0] ins_r,
	input [31:0] ins_rr,
	input insValid,
	input insValid_r,
	input insValid_rr
);
	`include "./tiger_defines.v"

	reg start;
	reg done;
	reg [63:0] count;
	reg [9:0] iCacheStall_count;
	reg [9:0] dCacheStall_count;

	initial start = 1'b0;
	initial done = 1'b0;
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
		if (iCacheStall == 1'b1) iCacheStall_count <= iCacheStall_count + 1;
		else iCacheStall_count <= 10'd0;
			
		/*
		if (iCacheStall_count == 10'd1000) begin
			$display("Instruction Cache is Stuck!\n");
			$finish;
		end
		*/
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
		if (pc == `STARTINGPC) begin
			start <= 1'b1;
		end
		if (start) begin
			count <= count + 1;
		end
		if (pc == 32'h10) begin
			if (start & ~done) begin
				done <= 1'b1;
				$display("counter = %d", count);
				`ifndef PROFILER_ON
					$finish;
				`endif
			end  
		end
	end

`ifdef PROFILER_ON
	// execution_trace.log
	integer log;
	reg program_started = 1'b0;
	initial begin
		log = $fopen ("execution_trace.log", "w");
		$fwrite (log, "    pc    |  pc_in   |   ins    |iVa|\n");
		$fclose (log);
	end
	always @ (posedge clk) begin
		program_started <= program_started | (pc == `STARTINGPC);
		log = $fopen ("execution_trace.log", "a");
		if (program_started | (pc == `STARTINGPC)) begin
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

endmodule
