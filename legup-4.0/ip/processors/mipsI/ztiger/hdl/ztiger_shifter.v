module ztiger_shifter(	
	input         clock, stall_execute,
	input  [2:0]  control_i,
	input  [4:0]  dataa_i,
	input  [31:0] datab_i,
	output [31:0] result,
	output        stall_o
);

	parameter SPEED_UP = 0;

	reg [31:0] src;		// source data
	reg [4:0]  amt;		// number of bits to shift by
	reg [2:0]  control;
	always @ (posedge clock)
	begin
		if(stall_execute)
		begin
			// stall
		end
		else begin
			src <= datab_i;
			amt <= dataa_i;
			control <= control_i;
		end
	end

	wire dir       = control[1];	// direction to shift (0 = left; 1 = right)
	wire alusigned = control[0];	// signed shift? 0 = unsigned; 1 = signed

	// fill bit for right shifts
	wire fillbit = alusigned & src[31]; 

	// do a right shift by shifting 0-5 times
	wire [31:0] right16;
	wire [31:0] right8;
	wire [31:0] right4;
	wire [31:0] right2;
	wire [31:0] right1;
	wire [31:0] right;
	reg  [31:0] right_reg;
	
	assign right16 = amt[4] ? {{16{fillbit}} , src[31:16]}    : src;
	assign right8  = amt[3] ? {{8{fillbit}}  , right16[31:8]} : right16;
	assign right4  = amt[2] ? {{4{fillbit}}  , right8[31:4]}  : right8;
	assign right2  = amt[1] ? {{2{fillbit}}  , right4[31:2]}  : right4;
	assign right1  = amt[0] ? {{1{fillbit}}  , right2[31:1]}  : right2;

	assign right = right1;
	always @ (posedge clock)
	begin
		right_reg <= right;
	end

	// do a left shift by shifting 0-5 times
	wire [31:0] left16;
	wire [31:0] left8;
	wire [31:0] left4;
	wire [31:0] left2;
	wire [31:0] left1;
	wire [31:0] left;
	reg  [31:0] left_reg;
	
	assign left16 = amt[4] ? {src[15:0]    , 16'b0} : src;
	assign left8  = amt[3] ? {left16[23:0] , 8'b0}  : left16;
	assign left4  = amt[2] ? {left8[27:0]  , 4'b0}  : left8;
	assign left2  = amt[1] ? {left4[29:0]  , 2'b0}  : left4;
	assign left1  = amt[0] ? {left2[30:0]  , 1'b0}  : left2;

	assign left = left1;
	always @ (posedge clock)
	begin
		left_reg <= left;
	end
	
	// select the correct shift output
generate
	if(SPEED_UP == 1)
	begin: pipeline_shifter
		
		assign result = dir ? right_reg : left_reg;

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
	else begin: comb_shifter
		assign result  = dir ? right : left;
		assign stall_o = 1'b0;
	end
endgenerate
	
endmodule 
