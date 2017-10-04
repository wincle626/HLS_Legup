module ztiger_mult_divider (
	input             clock, stall_execute, reset_execute,
	input      [3:0]  control_i, // {md_opt, hl, multn_div, unsign_an_direction}
	input      [31:0] dataa_i, datab_i,

	output reg [31:0] result,
	output            stall_o
);

	reg [31:0] dataa, datab;
	always @ (posedge clock)
	begin
		if(stall_execute)
		begin
			// stall
		end
		else begin
			dataa   <= dataa_i;
			datab   <= datab_i;
		end
	end

	reg [3:0]  control;
	always @ (posedge clock)
	begin
		if (stall_execute)
			; // stall
		else if(reset_execute)
			control <= 4'd0;
		else
			control <= control_i;
	end
   
	wire [5:0] cycles = ~control[3] ? 6'd0 : control[2] ? 6'd1 : ~control[1] ? 6'd4: 6'd36;   
	reg  [5:0] counter;
	always @ (posedge clock)
	begin
		if(stall_execute != stall_o)
			; // stall
		else if(reset_execute | ~stall_o)
			counter <= 6'd0;
		else
			counter <= counter + 6'd1;
	end

    assign stall_o = counter != cycles;

	wire [65:0] mult_result_tmp;
	wire [63:0] mult_result = mult_result_tmp[63:0];
	lpm_mult multiplier(
		.clock    (clock),
		.dataa    ({~control[0] & dataa[31], dataa}),
		.datab    ({~control[0] & datab[31], datab}),
		.result   (mult_result_tmp)
	);
	defparam
		multiplier.lpm_widtha = 33,
		multiplier.lpm_widthb = 33,
		multiplier.lpm_widthp = 66,
		multiplier.lpm_representation = "SIGNED",
		multiplier.lpm_pipeline = 3;

	wire [31:0] quotient_tmp, remain_tmp;     
	reg  run;
	always @ (posedge clock)
	begin
		run <= stall_o;
	end
	
	ztiger_divider div_inst( 
		.clock      (clock), 
		.run        (run),
		.numerator  ({~control[0] & dataa[31], dataa}), 
		.denominator({~control[0] & datab[31], datab}),
	
		.quotient_o (quotient_tmp), 
		.remainder_o(remain_tmp)
	);
		   
	reg [31:0] HI, LO;
	always @ (posedge clock)
	begin
		if(stall_o & (control[2:0] == 3'b101))
			{HI, LO} <= {dataa, LO};
		else if(stall_o & (control[2:0] == 3'b111))
			{HI, LO} <= {HI, dataa};
		else if(stall_o & (control[2:1] == 2'b00))
			{HI, LO} <= mult_result;
		else if(stall_o & (control[2:1] == 2'b01))
			{HI, LO} <= {remain_tmp, quotient_tmp};
		else
			{HI, LO} <= {HI, LO};
	end    

	always @ (posedge clock)
	begin
		if(stall_o & (control[2:0] == 3'b100))
			result <= HI;
		else if(stall_o & (control[2:0] == 3'b110))
			result <= LO;
		else
			result <= result;
	end
   
endmodule


module ztiger_divider ( // Nonrestoring_Signed_Divider
	input             clock, run,
	input [32:0]      numerator, denominator,
	
	output [31:0] quotient_o, remainder_o
);

	wire [32:0] temp;
	reg [32:0] quotient, remainder;
	always @ (posedge clock)
	begin
		if(~run)
			quotient <= numerator[32:0];
		else
			quotient <= {quotient[31:0], (temp[32] ^~ numerator[32]) ^ (numerator[32] ^ denominator[32])};
	end

	reg  [32:0] sub;
	always @ (posedge clock)
	begin
		if(~run)
			sub <= {33{numerator[32]}};
		else
			sub <= {temp[31:0], quotient[32]} - denominator;
	end
	
	reg  [32:0] add;
	always @ (posedge clock)
	begin
		if(~run)
			add <= {33{numerator[32]}};
		else 
			add <= {temp[31:0], quotient[32]} + denominator;
	end
	
	reg  sel;
	always @ (posedge clock)
	begin
		if(~run)
			sel <= numerator[32];
		else
			sel <= temp[32];
	end
	
	assign temp = sel ^~ denominator[32] ? sub : add;
	
	wire [32:0] remain = numerator[32] ^~ denominator[32] ? temp[32:0] + denominator : temp[32:0] - denominator;
	
	always @ (posedge clock)
	begin
		remainder <= temp[32] ^~ numerator[32] ? temp[32:0] : remain[32:0];
	end
	
	reg zero;
	always @ (posedge clock)
	begin
		zero <= temp[31:0] == 32'd0;
	end
	
	assign quotient_o  = {quotient[31:1], quotient[0] ^ (zero & numerator[32])} + {30'd0, numerator[32] ^ denominator[32]};
	assign remainder_o = zero ? 32'd0 : remainder[31:0];	
	
endmodule

