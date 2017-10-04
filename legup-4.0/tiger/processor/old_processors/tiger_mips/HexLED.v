module HexLED (
	input iCLOCK,
	input iRESET_N, 
	input iWR,
	input [31:0]iDATA,
	output [7:0]HEX0,
	output [7:0]HEX1,
	output [7:0]HEX2,
	output [7:0]HEX3,
	output [7:0]HEX4,
	output [7:0]HEX5,
	output [7:0]HEX6,
	output [7:0]HEX7
);

	reg [31:0]Number;
	
	always @(posedge iCLOCK) begin
		if(iWR) Number <= iDATA;
	end
	
	hex2leds h0(Number[3:0], HEX0);
	hex2leds h1(Number[7:4], HEX1);
	hex2leds h2(Number[11:8], HEX2);
	hex2leds h3(Number[15:12], HEX3);
	hex2leds h4(Number[19:16], HEX4);
	hex2leds h5(Number[23:20], HEX5);
	hex2leds h6(Number[27:24], HEX6);
	hex2leds h7(Number[31:28], HEX7);
endmodule


module hex2leds(input [3:0] hexval, output [6:0] ledcode);
	reg [6:0] lc;
	assign ledcode = ~lc;

	always @(*)
		case(hexval)
			4'h0 : lc <= 7'b0111111;
			4'h1 : lc <= 7'b0000110;
			4'h2 : lc <= 7'b1011011;
			4'h3 : lc <= 7'b1001111;
			4'h4 : lc <= 7'b1100110;
			4'h5 : lc <= 7'b1101101;
			4'h6 : lc <= 7'b1111101;
			4'h7 : lc <= 7'b0000111;
			4'h8 : lc <= 7'b1111111;
			4'h9 : lc <= 7'b1100111;
			4'hA : lc <= 7'b1110111;
			4'hB : lc <= 7'b1111100;
			4'hC : lc <= 7'b0111001;
			4'hD : lc <= 7'b1011110;
			4'hE : lc <= 7'b1111001;
			4'hF : lc <= 7'b1110001;
		endcase
endmodule