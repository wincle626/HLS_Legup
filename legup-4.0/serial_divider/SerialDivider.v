/* SerialDivider 
 *   This module performs unsigned division by computing one bit of the quotient per cycle and
 *   serially shifting those bits in a register. 
 *   The serial divider is not pipelined and can only perform one division at a time.
 *
 * Inputs:
 *
 * clk			the clock 
 * clken		an active-high clock enable
 * resetn		an active-low asynchronous reset
 * go 			active-high enable to start the division operation if it is not currently running
 *					otherwise the signal is ignored. It must be asserted for a least one cycle and  
 *					de-asserted when the done is asserted. 
 * dividend		the unsigned dividend or numerator. It must have valid data for the first cycle of go
 * 				being asserted.
 * divisor		the unsigned divisor or denominator. It must have valid data for the first cycle of go
 * 				being asserted.
 *
 *
 * Outputs:
 *
 * quotient		the unsigned result of division. It will have valid data while done is asserted
 * remainder	the unsigned remainder after division. It will have valid data while done is asserted
 * done			active-high signalling that the operation is complete. It will stay asserted
 *					until the go signal is de-asserted. 
 *
 *
 * Parameters:
 *
 * n				The bit width for the dividend, divisor, quotient and remainder
 * log2n			The log2 of n (ie. if n = 32, then log2n = 5)
 *  
 *
 * Latency:
 *  The done signal will be asserted n + 1 cycles after the go signal is
 *  asserted. Done will only be high for a single cycle, during which the
 *  quotient and remainder will be valid.
 *
 */

module SerialDivider (clk, clken, resetn, go, dividend, divisor, quotient, remainder, done);
	parameter n = 8, log2n = 3;

	input						clk;
	input						clken;
	input						resetn;
	input						go;
	input			[n-1:0]	dividend;
	input			[n-1:0]	divisor;

	output 		[n-1:0]	quotient;
	output 		[n-1:0]	remainder;
	output reg				done;

	reg			[1:0]		y;
	reg			[1:0]		Y;

	reg						load_input_values;
	reg						run_divider;

	wire			[n-1:0]	stored_divisor;

	wire		[log2n-1:0]	count;
	wire						count_is_zero;

	wire			[n:0]		sum;
	wire						c_out;

// control circuit

	parameter S1 = 2'b00, S2 = 2'b01, S3 = 2'b10;

	always @(*)
	begin: State_table
		case (y)
			S1:	if (go == 0) Y = S1;
					else Y = S2;
			S2:	if (count_is_zero == 0) Y = S2;
					else Y = S3;
			S3:	if (go == 1) Y = S3;
					else Y = S1;
			default: Y = 2'bxx;
		endcase
	end

	always @(posedge clk, negedge resetn)
	begin: State_flipflops
		if (resetn == 0)
			y <= S1;
		else if (clken)
			y <= Y;
	end

	always @(*)
	begin: FSM_outputs
		// defaults
		load_input_values = 0;
		run_divider = 0;
		done = 0;

		case (y)
			S1:	load_input_values = 1;
			S2:	run_divider = 1;
			S3:	done = 1;
		endcase
	end

	//datapath circuit

	DFlipFlop RegDivisor (
		.clk		(clk),
		.resetn	(resetn),
		.enable	(clken & load_input_values),
		.D			(divisor),

		.Q			(stored_divisor)
	);		
	defparam RegDivisor.n = n;

	ShiftLeftRegister ShiftRemainder (
		.clk				(clk),
		.resetn			(resetn & ~load_input_values),
		.load				(c_out & run_divider),
		.parallel_in	(sum[n-1:0]),
		.enable			(clken & run_divider),
		.shift_in		(quotient[n-1]),

		.Q					(remainder)
	);
	defparam ShiftRemainder.n = n;


	ShiftLeftRegister ShiftQuotient (
		.clk				(clk),
		.resetn			(resetn),
		.load				(load_input_values),
		.parallel_in	(dividend),
		.enable			(clken & run_divider),
		.shift_in		(c_out),

		.Q					(quotient)
	);
	defparam ShiftQuotient.n = n;

	DownwardsRunningCounter Counter (
		.clk				(clk),
		.resetn			(resetn & run_divider),
		.enable			(clken & run_divider),

		.Q					(count)
	);
	defparam Counter.log2n = log2n;

	assign count_is_zero = (count == 0);

	assign sum = {1'b0, remainder[n-2:0], quotient[n-1]} + {1'b0, ~stored_divisor} + 1;
	assign c_out = sum[n];
endmodule


module DFlipFlop (clk, resetn, enable, D, Q);
	parameter n = 8;

	input						clk;
	input						resetn;
	input						enable;
	input			[n-1:0]	D;

	output reg	[n-1:0]	Q;
	
	always @(posedge clk, negedge resetn)
		if (resetn == 0)
			Q <= 0;
		else if (enable)
			Q <= D;
endmodule


module ShiftLeftRegister (clk, resetn, load, parallel_in, enable, shift_in, Q);
	parameter n = 8;

	input						clk;
	input						resetn;
	input						load;
	input			[n-1:0]	parallel_in;
	input						enable;
	input						shift_in;

	output reg	[n-1:0]	Q;
	
	integer k;

	always @(posedge clk, negedge resetn)
	begin
		if (resetn == 0)
			Q <= 0;
	 	else if (load)
			Q <= parallel_in;
		else if (enable)
	 	begin
	 		Q[0] <= shift_in;
			for (k=1; k < n; k = k+1)
				Q[k] <= Q[k-1];
		end
	end
endmodule


module DownwardsRunningCounter(clk, resetn, enable, Q);
	parameter log2n = 3;

	input							clk;
	input							resetn;
	input							enable;

	output reg	[log2n-1:0]	Q;
	
	always @(posedge clk)
		if (resetn == 0)
			Q <= {log2n{1'b1}};
		else if (enable)
			Q <= Q - 1;
endmodule
