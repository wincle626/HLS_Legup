// SINGLE OPERATOR SHARING TEMPLATE MODULE
// No sharing
// Registers on inputs / outputs

module top
#(parameter WIDTH=32)
(
  input [WIDTH-1:0] dataa,
  input [WIDTH-1:0] datab,
  input clk,
  output reg [WIDTH-1:0] dataout // WIDTH*2 for multiply
);
  reg [WIDTH-1:0] dataa_reg;
  reg [WIDTH-1:0] datab_reg;

  always @ (posedge clk)
  begin
    dataa_reg <= dataa;
    datab_reg <= datab;
    dataout <= dataa_reg + datab_reg; // select operation here
  end
endmodule

