`include "cache_parameters.v"

module MUX_WORD_3to1_0(
input wire `LVT_ENTRY selector,

input wire `WORD input_0,
input wire `WORD input_1,
input wire `WORD input_2,

output reg `WORD output_0
);

always @(*) begin
    case(selector)

        `ACCEL_0, `ACCEL_1: begin
			output_0 <= input_0;
        end

        `ACCEL_2: begin
			output_0 <= input_1;
        end

        `ACCEL_3: begin
			output_0 <= input_2;
        end

        default: begin
			output_0 <= 'dX;
        end

    endcase
end

endmodule
