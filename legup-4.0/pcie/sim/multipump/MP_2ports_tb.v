`include "MP_parameters.v"

module MP_2ports_tb(
  output `WORD     read_data_0,
  output `WORD     read_data_1,
  output `MEM_ADDR avm_addr,
  output           avm_write_en,
  output           avm_read_en,
  output `BYTE_EN  avm_byte_en,
  output `WORD     avm_write_data
);

reg clk;
reg clk2X;

initial
    clk <= 0;
always @(clk)
    clk <= #20 ~clk;

initial
    clk2X <= 0;
always @(clk2X)
    clk2X <= #10 ~clk2X;

reg `MEM_ADDR addr_0;
reg           write_en_0;
reg           read_en_0;
reg `BYTE_EN  byte_en_0;
reg `WORD     write_data_0; 
reg `MEM_ADDR addr_1;
reg           write_en_1;
reg           read_en_1;
reg `BYTE_EN  byte_en_1;
reg `WORD     write_data_1; 

reg `WORD     avm_read_data;

MP_2ports MP2_ports_inst (
  .base_clock(clk),
  .clock(clk2X),

  .addr_0(addr_0),
  .write_en_0(write_en_0),
  .read_en_0(read_en_0),
  .byte_en_0(byte_en_0),
  .write_data_0(write_data_0),
  .read_data_0(read_data_0),

  .addr_1(addr_1),
  .write_en_1(write_en_1),
  .read_en_1(read_en_1),
  .byte_en_1(byte_en_1),
  .write_data_1(write_data_1),
  .read_data_1(read_data_1),

  .avm_addr(avm_addr),
  .avm_write_en(avm_write_en),
  .avm_read_en(avm_read_en),
  .avm_byte_en(avm_byte_en),
  .avm_write_data(avm_write_data),
  .avm_read_data(avm_read_data)
);

initial begin
@ (negedge clk) ;
addr_0 = 64'h7f;
addr_1 = 64'hff;
@ (negedge clk) ;
addr_0 = 64'h17f;
addr_1 = 64'h1ff;
avm_read_data = 64'hdeadbeef;
@ (negedge clk2X) ;
avm_read_data = 64'hbeefdead;
@ (negedge clk) ;
avm_read_data = 64'h0;
end

endmodule
