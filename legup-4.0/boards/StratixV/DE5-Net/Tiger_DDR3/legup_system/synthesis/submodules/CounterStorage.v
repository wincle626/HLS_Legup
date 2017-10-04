// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module CounterStorage # (
	parameter N = 256,
	parameter N2 = 8,
	parameter CW = 32
	) (
	input clk,
	input reset,
	input do_hier,
	// from AddressStack
	input call_as_cb,
	input retn_as_cb,
	input [N2-1:0] funcNum_as_cb,
	// from Counters
	input [CW-1:0] count,
	// from CounterStorage
	input [CW-1:0] prev_descendant_count_pop,
	output [CW-1:0] prev_descendant_count_push,
	
	// Read request from external component i.e., JTAG_MASTER
	input  [N2-1:0] ext_addr,
	output [CW-1:0] ext_rd_data
);

//+++++++++++++++++++++++++++++
// STACK & STORAGE
//+++++++++++++++++++++++++++++
	// prev_descendant_count is the value BEFORE calling the last descendant of CURRENT function
	// descendant_count      is the value of             the last descendant of CURRENT function; or 0 in non-hierarchical mode
	// count                 is the value of CURRENT function AFTER the last retn from descendent
	/*
		For instance, func A calls func B like the following:
		  Cycle   Instr
			1. instr of A
			2. instr of A - calling B
			3. instr of A - delay slot, 32'b0
			4. instr of B
			5. instr of B
			6. instr of B - return to A
			7. instr of B - delay slot, 32'0
			8. instr of A
			9. instr of A
		at cycle 9, prev_descendant_count = 3, descendant_count = 4, count = 2
	*/		
	reg [CW-1:0] descendant_count, descendant_count_r, count_r, prev_descendant_count_pop_r;
	reg [N2-1:0] funcNum_r;
	
	reg  storage_wren;
	reg  [N2-1:0] storage_addr;
	wire [CW-1:0] storage_wr_data;
	wire [CW-1:0] storage_rd_data;
	
	always @ (posedge clk) begin
		if (reset) begin
			funcNum_r                   <= { N2{1'b0}};
			descendant_count            <= {CW{1'b0}};
			descendant_count_r          <= {CW{1'b0}};
			count_r                     <= {CW{1'b0}};
			prev_descendant_count_pop_r <= {CW{1'b0}};
		end
		else begin
			funcNum_r                   <= funcNum_as_cb;                                      //------|
			count_r                     <= count;                                              //------|==\ counter_storage_ram write
			descendant_count_r          <= descendant_count;                                   //------|==/   at the next cycle of call/ret
			prev_descendant_count_pop_r <= do_hier ? prev_descendant_count_pop : {CW{1'b0}};   //------|
			
			if      (call_as_cb)	        descendant_count <= {CW{1'b0}};	// clear on call; this value is added into prev_descendant_count
			else if (retn_as_cb & do_hier)	descendant_count <= prev_descendant_count_pop + descendant_count + count;
		end
	end

	assign prev_descendant_count_push =        prev_descendant_count_pop   + descendant_count   + count  ;
	assign storage_wr_data = storage_rd_data + prev_descendant_count_pop_r + descendant_count_r + count_r;
	always @ (posedge clk) begin
		if (reset) begin
			storage_wren <= 1'b0;
			storage_addr <= { N2{1'b0}};
		end
		else begin
			storage_wren <= do_hier ? retn_as_cb : (call_as_cb | retn_as_cb);	// if do_hier, only write on retn
			storage_addr <= funcNum_r;
		end
	end
//+++++++++++++++++++++++++++++
// RAM
//+++++++++++++++++++++++++++++	
	altsyncram	counter_storage_ram (
		// Internal Signals (Port A)
				.wren_a    (storage_wren),
				.address_a (storage_addr),
				.data_a    (storage_wr_data),
				.q_a       (storage_rd_data),
				.addressstall_a (1'b0),
				.byteena_a (1'b1),
		// External Signals (Port B)
				.wren_b    (1'b0),
				.address_b (ext_addr),
				.data_b    ({CW{1'b0}}),
				.q_b       (ext_rd_data),
				.aclr0 (1'b0),
				.aclr1 (1'b0),
				.addressstall_b (1'b0),
				.byteena_b (1'b1),
		// Common Signals
				.clock0 (clk),
				.clock1 (1'b1),
				.clocken0 (1'b1),
				.clocken1 (1'b1),
				.clocken2 (1'b1),
				.clocken3 (1'b1),
				.eccstatus (),
				.rden_a (1'b1),
				.rden_b (1'b1));
	defparam
		counter_storage_ram.address_reg_b = "CLOCK0",
		counter_storage_ram.clock_enable_input_a = "BYPASS",
		counter_storage_ram.clock_enable_input_b = "BYPASS",
		counter_storage_ram.clock_enable_output_a = "BYPASS",
		counter_storage_ram.clock_enable_output_b = "BYPASS",
		counter_storage_ram.indata_reg_b = "CLOCK0",
		counter_storage_ram.intended_device_family = "Cyclone II",
		counter_storage_ram.lpm_type = "altsyncram",
		counter_storage_ram.numwords_a = N,
		counter_storage_ram.numwords_b = N,
		counter_storage_ram.operation_mode = "BIDIR_DUAL_PORT",
		counter_storage_ram.outdata_aclr_a = "NONE",
		counter_storage_ram.outdata_aclr_b = "NONE",
		counter_storage_ram.outdata_reg_a = "UNREGISTERED",
		counter_storage_ram.outdata_reg_b = "UNREGISTERED",
		counter_storage_ram.power_up_uninitialized = "FALSE",
		counter_storage_ram.read_during_write_mode_mixed_ports = "DONT_CARE",
		counter_storage_ram.widthad_a = N2,
		counter_storage_ram.widthad_b = N2,
		counter_storage_ram.width_a = CW,
		counter_storage_ram.width_b = CW,
		counter_storage_ram.width_byteena_a = 1,
		counter_storage_ram.width_byteena_b = 1,
		counter_storage_ram.wrcontrol_wraddress_reg_b = "CLOCK0";

//+++++++++++++++++++++++++++++
// DEBUG
//+++++++++++++++++++++++++++++	
	// synthesis translate off
	wire curr_call_retn = call_as_cb | retn_as_cb;
	reg  last_call_retn = 1'b0;
	always @ (posedge clk) begin
		last_call_retn <= curr_call_retn;
		if  (last_call_retn  & curr_call_retn)
		// call/retn instructions in two continuous cycles
		//	this should never happen bcuz of delay slot - 32'b0
		//	counter_storage won't work in this case
		begin
			$display ("Warning: call/retn instructions in two continuous cycles @ %d", $time);
			//$stop();
		end
	end
	// synthesis translate on

	// synthesis translate_off
	integer log;
	initial begin
		log = $fopen ("counter_storage.log", "w");
		$fwrite (log, " FuncNum | Init_Cnt  |PrevDes_Cnt| Curr_Cnt  | Child_Cnt | Final_Cnt |\n");
		$fclose (log);
	end

	always @ (posedge clk) begin
		if (storage_wren) begin
			log = $fopen ("counter_storage.log", "a");
			$fwrite (log, "   %02x    |%d |%d |%d |%d |%d |\n"
				, storage_addr, storage_rd_data, prev_descendant_count_pop_r, count_r, descendant_count_r, storage_rd_data + descendant_count_r + count_r);
			$fclose(log);
		end
	end
	// synthesis translate_on

endmodule

