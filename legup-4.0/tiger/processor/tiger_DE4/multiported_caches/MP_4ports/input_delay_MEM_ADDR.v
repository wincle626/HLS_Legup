`include "cache_parameters.v"

module input_delay_MEM_ADDR(
    input  wire         clock,
    input  wire         enable,
    input  wire `MEM_ADDR input_port,
    output reg `MEM_ADDR output_port
);

always @(posedge clock) begin
    if(enable == `HIGH) begin
        output_port <= input_port;
    end
end

endmodule
