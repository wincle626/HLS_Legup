module avm_accel_mem_bridge (
    //qsys inputs
    input clk,
    input reset,

    //accel outputs
    output [127:0] avm_accel_writedata,
    output avm_accel_write,
    output avm_accel_read,
    input [127:0] avm_accel_readdata,
    input avm_accel_waitrequest,

    //mem inputs
    output avs_mem_waitrequest,
    input [31:0] avs_mem_address,
    output [63:0] avs_mem_readdata,
    input avs_mem_read,
    input avs_mem_write,
    input [63:0] avs_mem_writedata,
    input [7:0] avs_mem_byteenable
);

reg        mem8, mem16, mem32, mem64;
reg [2:0]  mta_byteSel_32bit;

assign avm_accel_writedata[96] = mem8;
assign avm_accel_writedata[97] = mem16;
assign avm_accel_writedata[98] = mem64;

// unused
assign avm_accel_writedata[127:99] = 0;

assign avm_accel_write = avs_mem_write;
assign avm_accel_read = avs_mem_read;

always @ (*)
begin
  mem8 = 1'b0;
  mem16 = 1'b0;
  mem64 = 1'b0;
  case (avs_mem_byteenable)
    8'b00000001:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd0;
    end
    8'b00000010:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd1;
    end
    8'b00000100:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd2;
    end
    8'b00001000:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd3;
    end
    8'b00010000:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd4;
    end
    8'b00100000:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd5;
    end
    8'b01000000:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd6;
    end
    8'b10000000:
    begin
      mem8 = 1'b1;
      mta_byteSel_32bit = 3'd7;
    end
    8'b00000011:
    begin
      mem16 = 1'b1;
      mta_byteSel_32bit = 3'd0;
    end
    8'b00001100:
    begin
      mem16 = 1'b1;
      mta_byteSel_32bit = 3'd2;
    end
    8'b00110000:
    begin
      mem16 = 1'b1;
      mta_byteSel_32bit = 3'd4;
    end
    8'b11000000:
    begin
      mem16 = 1'b1;
      mta_byteSel_32bit = 3'd6;
    end
    8'b00001111:
    begin
      // mem32 is implicit
      mta_byteSel_32bit = 3'd0;
    end
    8'b11110000:
    begin
      // mem32 is implicit
      mta_byteSel_32bit = 3'd4;
    end
    8'b11111111:
    begin
      mem64 = 1'b1;
      mta_byteSel_32bit = 3'd0;
    end
    default:
      mta_byteSel_32bit = 3'b0;
  endcase
end

//set address in writedata
assign avm_accel_writedata[31:0] = avs_mem_address;
//concatenate readdata with byteenable shifted version 
assign avs_mem_readdata = avm_accel_readdata[63:0] << (mta_byteSel_32bit * 8);
//set actual writedata to middle 64 bits
assign avm_accel_writedata[95:32] = avs_mem_writedata >> (mta_byteSel_32bit * 8);
//set waitrequest
assign avs_mem_waitrequest = avm_accel_waitrequest & (avs_mem_write | avs_mem_read);

endmodule
