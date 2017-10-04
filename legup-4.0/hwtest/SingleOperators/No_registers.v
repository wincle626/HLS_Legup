// SINGLE OPERATOR SHARING TEMPLATE MODULE
// No sharing
// No registers on inputs / outputs

module top
#(parameter WIDTH=32)
(
  input [WIDTH-1:0] dataa,
  input [WIDTH-1:0] datab,
  input clk,
  output reg [WIDTH-1:0] dataout // WIDTH*2 for multiply
);
  always @ (*)
  begin
    dataout <= dataa + datab; // select operation here
  end
endmodule

