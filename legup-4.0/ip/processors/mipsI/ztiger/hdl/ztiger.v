module ztiger (
	input         clock,
	input         reset, i_stall, d_stall,
	input         memCanRead, memCanWrite,

	output        reset_fetch,
	
	input  [31:0] i_readdata_i,
	output [31:0] i_address_o,
	
	input  [31:0] d_readdata_i,
	output [31:0] d_address_o, d_writedata_o,
	output        d_read_o, d_write_o, mem8, mem16
);

	wire stall_fetch, stall_decode, stall_execute, reset_execute, reset_wb;
	wire mc_stall, md_stall, sh_stall, asc_stall;
	wire [7:0]  data_mux_sel;
	wire [31:0] rs, rt, add_result, dataa, datab, control_ex, control_wb;
	wire [31:0] asc_result, logic_result, direct_result, mc_result, shift_result, md_result;
	wire        comp_result;

	// control_unit
	ztiger_control_unit cu_inst(
		.clock          (clock),
		.reset          (reset),
		.i_stall        (i_stall),
		.d_stall_bus    ({1'd0, md_stall, sh_stall, mc_stall, 3'd0, asc_stall}),
	
		.reset_fetch    (reset_fetch),

		.instruction    (i_readdata_i),
		.rs             (rs), 
		.rt             (rt),
		
		.stall_fetch    (stall_fetch), 
		.stall_decode   (stall_decode), 
		.stall_execute  (stall_execute), 
		.reset_execute  (reset_execute), 
		.reset_wb       (reset_wb),
		.control_ex     (control_ex), 
		.control_wb     (control_wb),
		.data_mux_sel   (data_mux_sel),
	
		.nextnextPC     (i_address_o), 
		.add_result     (add_result)
	);
	
	// datapath
	ztiger_datapath dp_inst(
		.clock          (clock), 
		.stall_fetch    (stall_fetch), 
		.stall_decode   (stall_decode),
		.reset_wb       (reset_wb),		
		
		.r0             (asc_result), 
		.r1             ({31'd0, comp_result}), 
		.r2             (logic_result), 
		.r3             (direct_result), 
		.r4             (mc_result), 
		.r5             (shift_result), 
		.r6             (md_result), 
		.r7             (32'd0),
		.control_ex     (control_ex),
		.control_wb     (control_wb),

		.instruction    (i_readdata_i),
		.rs             (rs), 
		.rt             (rt),

		.data_mux_sel   (data_mux_sel),
		.add_result     (add_result),
		.dataa          (dataa), 
		.datab          (datab)	
	);

	// Add_Sub_Comp
	ztiger_add_sub_comp asc_inst(
		.clock          (clock),
		.stall_execute  (stall_execute),
		.control_i      ({control_ex[9] | control_ex[8], control_ex[17:16]}),
		.dataa_i        (dataa), 
		.datab_i        (datab), 
		.result         (asc_result),
		.a_lt_b         (comp_result),
		.stall_o        (asc_stall)
	);
	
	// Logic
	ztiger_logic_cal lc_inst(
		.clock          (clock),
		.stall_execute  (stall_execute),
		.control_i      (control_ex[17:16]),
		.dataa_i        (dataa), 
		.datab_i        (datab),
		.result         (logic_result)
	);	
	
	// Direct
	reg [31:0] direct_a, direct_b;
	reg is_lui;
	always @ (posedge clock)
	begin
		if(stall_execute)
		begin
			// stall
		end
		else begin
			is_lui   <= control_ex[21];
			direct_a <= dataa;
			direct_b <= datab;
		end
	end

	assign direct_result = is_lui ? {direct_b[15:0], 16'd0} : direct_a;

	// Memory Access
	ztiger_mem_controller mc_inst(
		.clock          (clock), 
		.d_stall        (d_stall), 
		.memCanRead     (memCanRead), 
		.memCanWrite    (memCanWrite),
		.stall_execute  (stall_execute), 

		.control_i      ({control_ex[12], control_ex[18], control_ex[20], control_ex[17:16]}),
		.dataa_i        (dataa), 
		.datab_i        (datab),
	
		.memreaddata    (d_readdata_i),	
		.memread        (d_read_o), 
		.memwrite       (d_write_o), 
		.mem8           (mem8), 
		.mem16          (mem16),
		.memaddress     (d_address_o),
		.memwritedata   (d_writedata_o),
	
		.result         (mc_result),
		.stall_o        (mc_stall)
	);
	
	// Shifter
	ztiger_shifter sh_inst(	
		.clock          (clock),
		.stall_execute  (stall_execute),
		.control_i      ({control_ex[13], control_ex[17:16]}),
		.dataa_i        (dataa[4:0]),
		.datab_i        (datab),
		.result         (shift_result),
		.stall_o        (sh_stall)
	);	
	
	// Mult_Divider
	ztiger_mult_divider md_inst(
		.clock          (clock), 
		.stall_execute  (stall_execute), 
		.reset_execute  (reset_execute),
		.control_i      ({control_ex[14], control_ex[19], control_ex[17:16]}),
		.dataa_i        (dataa), 
		.datab_i        (datab),
	
		.result         (md_result),
		.stall_o        (md_stall)
	);	
		
endmodule 
