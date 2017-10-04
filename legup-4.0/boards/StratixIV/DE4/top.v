module de4 ( 
    	     OSC_50_BANK2, 
             BUTTON, 
             LED, 
    	     SEG0_D, 
    	     SEG1_D 
	     ); 
   input OSC_50_BANK2; 
   input [1:0] BUTTON; 
   output [6:0] SEG0_D; 
   output [6:0] SEG1_D; 
   output [7:0] LED; 
   
   de2 de2_inst ( 
		  .CLOCK_50 (OSC_50_BANK2), 
		  .LEDG (LED), 
		  .KEY (BUTTON), 
		  .SW (), 
		  .HEX0 (SEG0_D), 
		  .HEX1 (SEG1_D), 
		  .HEX2 (), 
		  .HEX3 (), 
		  .HEX4 (), 
		  .HEX5 (), 
		  .HEX6 (), 
		  .HEX7 () 
		  ); 
   
endmodule
