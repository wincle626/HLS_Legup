module clockbuf (
	ena,
	inclk,
	outclk);

	input	  ena;
	input	  inclk;
	output	  outclk;

	BUFGCE BUFGCE_inst(	
				.O(outclk),
				.I(inclk),				
				.CE(ena)
				);

endmodule
