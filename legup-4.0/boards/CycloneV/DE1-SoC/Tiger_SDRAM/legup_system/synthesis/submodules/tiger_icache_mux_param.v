module tiger_icache_mux_param (Z, SEL, D);

parameter N = 8; //number of bits wide
parameter M = 4; //number of inputs
parameter S = 2; //number of select lines

parameter W = M * N;
`define DTOTAL W-1:0
`define DWIDTH N-1:0
`define SELW   S-1:0
`define WORDS  M-1:0

input [`DTOTAL] D;
input [`SELW]   SEL;
output [`DWIDTH] Z;

integer i;
reg[`DWIDTH] tmp, Z; //tmp will be used to minimize events

always @(SEL or D) begin
	for (i=0; i<N; i=i+1) //for bits in the width
		tmp[i] = D[N*SEL+i];
	Z = tmp;
end

endmodule 

