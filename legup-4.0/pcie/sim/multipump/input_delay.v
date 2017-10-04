module input_delay(
    clock,
    enable,

    input_bus,
    output_bus
);

parameter BUS_WIDTH = 2;

input clock;
input enable;

input wire [BUS_WIDTH-1:0] input_bus;
output reg [BUS_WIDTH-1:0] output_bus;

always @(posedge clock) begin
    if(enable == `HIGH) begin
        output_bus <= input_bus;
    end
end

endmodule
