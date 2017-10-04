// SINGLE OPERATOR SHARING TEMPLATE MODULE
// Sharing with 2-to-1 MUXing
// Registers on inputs / outputs

module top
#(parameter WIDTH=32)
(
  input [WIDTH-1:0] data1,
  input [WIDTH-1:0] data2,
  input [WIDTH-1:0] data3,
  input [WIDTH-1:0] data4,
  input clk,
  input s,
  output reg [WIDTH-1:0] dataout // WIDTH*2 for multiply
);
  reg [WIDTH-1:0] dataa_reg;
  reg [WIDTH-1:0] datab_reg;
  reg [WIDTH-1:0] datac_reg;
  reg [WIDTH-1:0] datad_reg;
  reg [WIDTH-1:0] w1;
  reg [WIDTH-1:0] w2;

  always @ (posedge clk)
  begin
    dataa_reg <= data1;
    datab_reg <= data2;
    datac_reg <= data3;
    datad_reg <= data4;

    dataout <= w1 + w2; // select operation here
  end

  always @ (*)
  begin
    if (s==0)
    begin
      w1 <= dataa_reg;
      w2 <= datac_reg;
    end
    else 
    begin
      w1 <= datab_reg;
      w2 <= datad_reg;
    end
  end
endmodule

