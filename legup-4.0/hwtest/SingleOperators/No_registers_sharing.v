// SINGLE OPERATOR SHARING TEMPLATE MODULE
// Sharing with 2-to-1 MUXing
// No registers on inputs / outputs

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
  reg [WIDTH-1:0] w1;
  reg [WIDTH-1:0] w2;

  always @ (*)
  begin
    dataout <= w1 + w2; // select operation here
  end

  always @ (*)
  begin
    if (s==0)
    begin
      w1 <= data1;
      w2 <= data2;
    end
    else 
    begin
      w1 <= data3;
      w2 <= data4;
    end
  end
endmodule

