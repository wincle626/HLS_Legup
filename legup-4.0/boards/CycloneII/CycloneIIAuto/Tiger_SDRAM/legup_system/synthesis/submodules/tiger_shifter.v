/*
  This work by Simon Moore and Gregory Chadwick is licenced under the
  Creative Commons Attribution-Non-Commercial-Share Alike 2.0
  UK: England & Wales License.

  To view a copy of this licence, visit:
     http://creativecommons.org/licenses/by-nc-sa/2.0/uk/
  or send a letter to:
     Creative Commons,
     171 Second Street,
     Suite 300,
     San Francisco,
     California 94105,
     USA.
*/
module tiger_shifter(	
				input [31:0] src,		// source data
				input [4:0] amt,		// number of bits to shift by
				input dir,				// direction to shift (0 = left; 1 = right)
				input alusigned,		// signed shift? 0 = unsigned; 1 = signed
				output [31:0] shifted	// output
);

	// fill bit for right shifts
	wire fillbit = alusigned & src[31]; 

	// do a right shift by shifting 0-5 times
	wire [31:0] right16;
	wire [31:0] right8;
	wire [31:0] right4;
	wire [31:0] right2;
	wire [31:0] right1;
	wire [31:0] right;
	
	assign right16 = amt[4] ? {{16{fillbit}} , src[31:16]}    : src;
	assign right8  = amt[3] ? {{8{fillbit}}  , right16[31:8]} : right16;
	assign right4  = amt[2] ? {{4{fillbit}}  , right8[31:4]}  : right8;
	assign right2  = amt[1] ? {{2{fillbit}}  , right4[31:2]}  : right4;
	assign right1  = amt[0] ? {{1{fillbit}}  , right2[31:1]}  : right2;

	assign right = right1;

	// do a left shift by shifting 0-5 times
	wire [31:0] left16;
	wire [31:0] left8;
	wire [31:0] left4;
	wire [31:0] left2;
	wire [31:0] left1;
	wire [31:0] left;
	
	assign left16 = amt[4] ? {src[15:0]    , 16'b0} : src;
	assign left8  = amt[3] ? {left16[23:0] , 8'b0}  : left16;
	assign left4  = amt[2] ? {left8[27:0]  , 4'b0}  : left8;
	assign left2  = amt[1] ? {left4[29:0]  , 2'b0}  : left4;
	assign left1  = amt[0] ? {left2[30:0]  , 1'b0}  : left2;

	assign left = left1;
	
	// select the correct shift output
	assign shifted = dir ? right : left;
endmodule 