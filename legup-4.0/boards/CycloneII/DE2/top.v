module de2 (
	    CLOCK_50,
	    KEY,
	    SW,
	    HEX0,
	    HEX1,
	    HEX2,
	    HEX3,
	    HEX4,
	    HEX5,
	    HEX6,
	    HEX7,
	    LEDG
	    <LEGUP_PORT_LIST>
	    );

   input CLOCK_50;
   input [3:0] KEY;
   input [17:0] SW;
   output [6:0] HEX0, HEX1,  HEX2,  HEX3,  HEX4,  HEX5,  HEX6,  HEX7;
   reg [6:0] 	hex0, hex1, hex2, hex3, hex4, hex5, hex6, hex7;
   

   output [7:0] LEDG;
   <LEGUP_SIGNAL_DEC>


   wire 	reset = ~KEY[0];
   wire 	start;
   wire [31:0] 	return_val;
   reg  [31:0] 	return_val_reg;
   wire 	finish;
   wire [3:0]	state;

   hex_digits h7( .x(hex7), .hex_LEDs(HEX7));
   hex_digits h6( .x(hex6), .hex_LEDs(HEX6));
   hex_digits h5( .x(hex5), .hex_LEDs(HEX5));
   hex_digits h4( .x(hex4), .hex_LEDs(HEX4));
   hex_digits h3( .x(hex3), .hex_LEDs(HEX3));
   hex_digits h2( .x(hex2), .hex_LEDs(HEX2));
   hex_digits h1( .x(hex1), .hex_LEDs(HEX1));
   hex_digits h0( .x(hex0), .hex_LEDs(HEX0));
   
   <LEGUP_LOGIC>
   
   top top_inst (
      .clk (clk),
      .reset (reset),
      .finish (finish),
      .return_val (return_val)
      <LEGUP_TOP_SIGNALS>
    );

endmodule

