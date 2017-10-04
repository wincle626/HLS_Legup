module ztiger_add_sub_comp(
	input        clock, stall_execute,
	input [2:0]  control_i, // {addn_sub, unsign_comp}
	input [31:0] dataa_i, datab_i, 
	output[31:0] result,
	output       a_lt_b,
	output       stall_o
);
	
	parameter SPEED_UP = 0;
		
	reg [31:0] dataa, datab;
	reg [2:0]  control;
	always @ (posedge clock)
	begin
		if(stall_execute)
		begin
			// stall
		end
		else begin
			dataa   <= dataa_i;
			datab   <= datab_i;
			control <= control_i;
		end
	end
	
	wire cout;
	
	// if compare signed and the sign bit are not the same, then look at the sign bit of dataa_i, otherwise look at cout
	assign a_lt_b = ~control[0] & (dataa[31] ^ datab[31]) ? dataa[31] : ~cout;	
	
generate
	if(SPEED_UP == 1)
	begin: pipeline_adder		
		lpm_add_sub lpm_add_sub_inst(
			.clock   (clock),
			.dataa   (dataa),
			.datab   (datab),
			.add_sub (~control[1]),	
			.result  (result),
			.cout    (cout)
		);
		defparam
			lpm_add_sub_inst.lpm_width = 32,
			lpm_add_sub_inst.lpm_pipeline = 1,
			lpm_add_sub_inst.lpm_representation = "SIGNED";

		reg first_stall;
		always @ (posedge clock)
		begin
			if(stall_execute)
				first_stall <= 1'b0;	
			else
				first_stall <= control_i[2];
		end		
		
		assign stall_o = first_stall;
	end
	else begin: comb_adder
		lpm_add_sub lpm_add_sub_inst(
			.dataa   (dataa),
			.datab   (datab),
			.add_sub (~control[1]),	
			.result  (result),
			.cout    (cout)
		);
		defparam
			lpm_add_sub_inst.lpm_width = 32,
			lpm_add_sub_inst.lpm_representation = "SIGNED";	
	
		assign stall_o = 1'b0;
	end
endgenerate	

endmodule 
