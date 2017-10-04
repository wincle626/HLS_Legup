`include "cache_parameters.v"

module read_port_registers_2r2x(
    input  wire         clock,
    input  wire         write_en,

    input  wire  `WORD  input_0,
    input  wire  `WORD  input_1,

    output  reg  `WORD  output_0,
    output  reg  `WORD  output_1
);

always @(posedge clock) begin
	if(write_en == 1'b1) 
	begin
		output_0 <= input_0;
		output_1 <= input_1;
	end
end

endmodule
