module demux_param (word, lineIn, SEL, lineOut);

parameter N = 8; //number of bits wide
parameter M = 4; //number of inputs
parameter S = 2; //number of select lines

parameter W = M * N;
`define DTOTAL W-1:0
`define DWIDTH N-1:0
`define SELW   S-1:0

input [`DTOTAL] lineIn;
input [`DWIDTH] word;
input [`SELW] SEL;
output reg [`DTOTAL] lineOut;

integer i;

always @(*) begin
	//copy the cache line first
	lineOut = lineIn;	
	//then write the data in the correct position in the cache line
	for (i=0; i<N; i=i+1) //for bits in the width
	begin
		lineOut[N*SEL+i] = word[i];
	end
end

endmodule 

