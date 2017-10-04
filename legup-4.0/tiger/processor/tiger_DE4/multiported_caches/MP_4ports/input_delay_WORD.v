`include "cache_parameters.v"

module input_delay_WORD(
    input  wire         clock,
    input  wire         enable,
    input  wire `WORD input_port,
    output reg `WORD output_port
);

always @(posedge clock) begin
    if(enable == `HIGH) begin
        output_port <= input_port;
    end
end

endmodule
