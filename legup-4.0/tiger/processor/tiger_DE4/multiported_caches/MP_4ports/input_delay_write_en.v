`include "cache_parameters.v"

module input_delay_write_en(
    input  wire         clock,
    input  wire         enable,
    input  wire 	    input_port,
    output reg 			output_port
);

always @(posedge clock) begin
    if(enable == `HIGH) begin
        output_port <= input_port;
    end
end

endmodule
