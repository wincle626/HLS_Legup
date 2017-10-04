// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module AddressHash # (
	parameter N = 256,
	parameter N2 = 8
	) (
	input clk,
	input reset,

	input [31:0] pc_od_ah,
	input call_od_ah,
	input retn_od_ah,
	input cnt_inc_od_ah,

	output reg call_ah_as,
	output reg retn_ah_as,
	output reg cnt_inc_ah_as,
	output [N2-1:0] funcNum_ah_as,
	
	// These 3 ports are controlled by JTAG Master
	//	to write in hash parameters (register or blk ram).
	input [N2-2:0] ram_wr_addr_ah,
	input [31  :0] ram_wr_data_ah,
	input          ram_wr_en_ah
);

//+++++++++++++++++++++++++++++
// Memory Map Hash Parameters
//+++++++++++++++++++++++++++++
	reg [31:0] V1; 
	reg [7 :0] A1, A2, B1, B2;
	always @ (posedge clk) begin
		if (reset) begin
			V1 <= 32'b0;
			B1 <= 8'b0;
			B2 <= 8'b0;
			A1 <= 8'b0;
			A2 <= 8'b0;
		end
		else if (ram_wr_en_ah) begin
			if      (ram_wr_addr_ah[N2-2:0] == { (N2-1){1'b0}}        )  V1           <= ram_wr_data_ah;
			else if (ram_wr_addr_ah[N2-2:0] == {{(N2-2){1'b0}}, 1'b1} ) {A1,A2,B1,B2} <= ram_wr_data_ah;
		end
	end

//+++++++++++++++++++++++++++++
// PIPELINING
//+++++++++++++++++++++++++++++
	reg call_r, call_rr, call_rrr;
	reg retn_r, retn_rr, retn_rrr;
	reg cnt_inc_r, cnt_inc_rr, cnt_inc_rrr;
	
	always @ (posedge clk) begin
		if (reset) begin
			call_r   <= 1'b0;
			call_rr  <= 1'b0;
			call_rrr <= 1'b0;
			call_ah_as <= 1'b0;
			
			retn_r   <= 1'b0;
			retn_rr  <= 1'b0;
			retn_rrr <= 1'b0;
			retn_ah_as <= 1'b0;
			
			cnt_inc_r   <= 1'b0;
			cnt_inc_rr  <= 1'b0;
			cnt_inc_rrr <= 1'b0;
			cnt_inc_ah_as <= 1'b0;
		end
		else begin
			call_r   <= call_od_ah;
			call_rr  <= call_r;
			call_rrr <= call_rr;
			call_ah_as <= call_rrr;
			
			retn_r   <= retn_od_ah;
			retn_rr  <= retn_r;
			retn_rrr <= retn_rr;
			retn_ah_as <= retn_rrr;
			
			cnt_inc_r   <= cnt_inc_od_ah;
			cnt_inc_rr  <= cnt_inc_r;
			cnt_inc_rrr <= cnt_inc_rr;
			cnt_inc_ah_as <= cnt_inc_rrr;
		end
	end
			
//+++++++++++++++++++++++++++++
// HASHING
//+++++++++++++++++++++++++++++
	// Read tab from RAM
	wire [N2-1:0] ram_rd_addr_ah;
	wire [   7:0] ram_rd_data_ah;
	
	reg [31:0] hashing_addr, val, a;
	reg [N2-1:0] rsl;

	wire [31:0]	val_1 = hashing_addr + V1;
	wire [31:0]	val_2 = val_1 + (val_1 << 8);
	wire [31:0]	val_3 = val_2 ^ (val_2 >> 4);
	
	wire [31:0]	b;
	wire [ 7:0]	tab_b;

	assign b = (val >> B1) & B2;
	assign ram_rd_addr_ah = b[N2-1:0];
	assign tab_b = ram_rd_data_ah;
	
	always @ (posedge clk) begin
		if (reset) begin
			hashing_addr <= 32'h0;
			val <= 32'h0;
			a   <= 32'h0;
			rsl <= 32'h0;
		end
		else begin
			if (call_od_ah)	// make sure the output funcNum won't change unless a call is detected
				hashing_addr <= {6'b0, pc_od_ah[25:0]};	// 1st pipeline - mark's hashing algorithm takes 26 bits of address as input...
			val <= val_3;               // 2nd pipeline
			a   <= (val + (val << A1)); // 3rd pipeline
			rsl <= ((a >> A2) ^ tab_b); // 4th pipeline -> *_ah_as
		end
	end
	
	assign funcNum_ah_as = rsl; // output @ 4th pipeline

	// Simple Dual Port RAM - one read port & one write port
	//	Write port is accessed by JTAG Master (N/4 words * 32 bits_per_word)
	//	Read port is locally accessed by Hash Logic (N words * 8 bits_per_word)
	altsyncram	hash_params (
				.address_a (ram_wr_addr_ah [N2-3:0]),
				.clock0    (clk),
				.data_a    (ram_wr_data_ah),
				.wren_a    (ram_wr_en_ah & ram_wr_addr_ah [N2-2]),
				.address_b (ram_rd_addr_ah),
				.q_b       (ram_rd_data_ah),
				.aclr0 (1'b0),
				.aclr1 (1'b0),
				.addressstall_a (1'b0),
				.addressstall_b (1'b0),
				.byteena_a (1'b1),
				.byteena_b (1'b1),
				.clock1 (1'b1),
				.clocken0 (1'b1),
				.clocken1 (1'b1),
				.clocken2 (1'b1),
				.clocken3 (1'b1),
				.data_b ({8{1'b1}}),
				.eccstatus (),
				.q_a (),
				.rden_a (1'b1),
				.rden_b (1'b1),
				.wren_b (1'b0));
	defparam
		hash_params.address_reg_b = "CLOCK0",
		hash_params.clock_enable_input_a = "BYPASS",
		hash_params.clock_enable_input_b = "BYPASS",
		hash_params.clock_enable_output_a = "BYPASS",
		hash_params.clock_enable_output_b = "BYPASS",
		hash_params.intended_device_family = "Cyclone II",
		hash_params.lpm_type = "altsyncram",
		hash_params.numwords_a = (N>>2),
		hash_params.numwords_b = N,
		hash_params.operation_mode = "DUAL_PORT",
		hash_params.outdata_aclr_b = "NONE",
		hash_params.outdata_reg_b = "UNREGISTERED",
		hash_params.power_up_uninitialized = "FALSE",
		hash_params.read_during_write_mode_mixed_ports = "DONT_CARE",
		hash_params.widthad_a = N2-2,
		hash_params.widthad_b = N2,
		hash_params.width_a = 32,
		hash_params.width_b = 8,
		hash_params.width_byteena_a = 1;
		
//+++++++++++++++++++++++++++++
// DEBUG
//+++++++++++++++++++++++++++++
	// synthesis translate_off
	// address_hash.log is helpful for debugging hash logic
	integer log;
	initial begin
		log = $fopen ("address_hash.log", "w");
		$fclose (log);
	end
	always @ (posedge clk) begin
		log = $fopen ("address_hash.log", "a");
		if (call_r) begin       // 1st pipeline is hashing for a call
			$fwrite (log, "V1 = %h, A1 = %h, A2 = %h, B1 = %h, B2 = %h\n", V1, A1, A2, B1, B2);
			$fwrite (log, "addr = %h\n", hashing_addr);
			$fwrite (log, "val = %h\n", val_1);
			$fwrite (log, "val = %h\n", val_2);
			$fwrite (log, "val = %h\n", val_3);
		end
		if (call_rr) begin      // 2nd pipeline is hashing for a call
			$fwrite (log, "b = %02d\n", b);
		end
		if (call_rrr) begin     // 3rd pipeline is hashing for a call
			$fwrite (log, "a = %h\n", a);
			$fwrite (log, "tab_b = %h\n", tab_b);
		end
		if (call_ah_as) begin   // 4th pipeline is hashing for a call
			$fwrite (log, "rsl = %02x\n\n", rsl);
		end
		$fclose(log);
	end
	// synthesis translate_on

endmodule
