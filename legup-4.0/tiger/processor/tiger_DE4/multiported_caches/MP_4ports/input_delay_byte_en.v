`include "cache_parameters.v"

module input_delay_byte_en(
    input  wire     	    clock,
    input  wire  	        enable,
    input  wire `BYTE_EN 	input_port,
    output reg 	`BYTE_EN	output_port
);

always @(posedge clock) begin
    if(enable == `HIGH) begin
        output_port <= input_port;
    end
end

endmodule
