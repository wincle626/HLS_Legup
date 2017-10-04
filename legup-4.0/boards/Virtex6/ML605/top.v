module ML605 (
       USER_CLOCK,
	    KEY,
	    SW,	    
       LED,
       LEDG
	    <LEGUP_PORT_LIST>
	    );

   input USER_CLOCK;
   input [4:0] KEY;
   input [7:0] SW;
   output [7:0] LED;
   output [7:0] LEDG;
   wire CLOCK_50;

   <LEGUP_SIGNAL_DEC>


   wire 	reset = ~KEY[0];
   wire 	start;
   wire [31:0] 	return_val;
   reg  [31:0] 	return_val_reg;
   wire 	finish;
   wire [3:0]	state;
   
   reg [6:0]   hex0, hex1, hex2, hex3, hex4, hex5, hex6, hex7;

   assign CLOCK_50 = USER_CLOCK;
   assign LED = 0;
   
   <LEGUP_LOGIC>
   
   top top_inst (
      .clk (clk),
      .reset (reset),
      .finish (finish),
      .return_val (return_val)
      <LEGUP_TOP_SIGNALS>
    );

   

endmodule

