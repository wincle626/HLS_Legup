module divider_steve (Clock, Resetn, s, LA, EB, DataA, DataB, R, Q, Done);
	parameter n = 8, logn = 3;
	input Clock, Resetn, s, LA, EB;
	input [n-1:0] DataA, DataB;
	output [n-1:0] R, Q;
	output reg Done;

	wire Cout, z, R0;
	wire [n-1:0] DataR;
	wire [n:0] Sum;
	reg [1:0] y, Y;
	wire [n-1:0] A, B;
	wire [logn-1:0] Count;
	reg EA, Rsel, LR, ER, ER0, LC, EC;
	integer k;

// control circuit

	parameter S1 = 2'b00, S2 = 2'b01, S3 = 2'b10;

	always @(s, y, z)
	begin: State_table
		case (y)
			S1:	if (s == 0) Y = S1;
				else Y = S2;
			S2:	if (z == 0) Y = S2;
				else Y = S3;
			S3:	if (s == 1) Y = S3;
				else Y = S1;
			default: Y = 2'bxx;
		endcase
	end

	always @(posedge Clock, negedge Resetn)
	begin: State_flipflops
		if (Resetn == 0)
			y <= S1;
		else
			y <= Y;
	end

	always @(y, s, Cout, z)
	begin: FSM_outputs
		// defaults
		LR = 0; ER = 0; ER0 = 0; LC = 0; EC = 0; EA = 0;
		Rsel = 0; Done = 0;
		case (y)
			S1:	begin
					LC = 1; ER = 1;
					if (s == 0)
					begin
						LR = 1; ER0 = 0;
					end
					else
					begin
						LR = 0; EA = 1; ER0 = 1;
					end
				end
			S2: begin
					Rsel = 1; ER = 1; ER0 = 1; EA = 1;
					if (Cout) LR = 1;
					else LR = 0;
					if (z == 0) EC = 1;
					else EC = 0;
				end
			S3:	Done = 1;
		endcase
	end

//datapath circuit

	regne RegB (DataB, Clock, Resetn, EB, B);
	defparam RegB.n = n;

	shiftlne ShiftR (DataR, LR, ER, R0, Clock, R);
	defparam ShiftR.n = n;

	muxdff FF_R0 (1'b0, A[n-1], ER0, Clock, R0);

	shiftlne ShiftA (DataA, LA, EA, Cout, Clock, A);
	defparam ShiftA.n = n;
	assign Q = A;

	downcount Counter (Clock, EC, LC, Count);
	defparam Counter.n = logn;

	assign z = (Count == 0);

	assign Sum = {1'b0, R[n-2:0], R0} + {1'b0, ~B} + 1;
	assign Cout = Sum[n];

	// define the n 2-to-1 multiplexers
	assign DataR = Rsel ? Sum : 0;
endmodule


module regne (R, Clock, Resetn, E, Q);
	parameter n = 8;
	input [n-1:0] R;
	input Clock, Resetn, E;
	output reg [n-1:0] Q;
	
	always @(posedge Clock, negedge Resetn)
		if (Resetn == 0)
			Q <= 0;
		else if (E)
			Q <= R;
endmodule


module shiftlne (R, L, E, w, Clock, Q);
	parameter n = 8;
	input [n-1:0] R;
	input L, E, w, Clock;
	output reg [n-1:0] Q;
	integer k;

	always @(posedge Clock)
	begin
	 	if (L)
			Q <= R;
		else if (E)
	 	begin
	 		Q[0] <= w;
			for (k=1; k < n; k = k+1)
				Q[k] <= Q[k-1];
		end
	end
endmodule


module muxdff(D0, D1, Sel, Clock, Q);
	input D0, D1, Sel, Clock;
	output reg Q;
	
	always @(posedge Clock)
	 	if (~Sel)
			Q <= D0;
		else 
			Q <= D1;
endmodule


module downcount(Clock, E, L, Q);
	parameter n=4;
	input Clock, L, E;
	output reg [n-1:0] Q;
		
	always @(posedge Clock)
		if (L)
			Q <= {n{1'b1}};
		else if (E)
			Q <= Q - 1;
endmodule
