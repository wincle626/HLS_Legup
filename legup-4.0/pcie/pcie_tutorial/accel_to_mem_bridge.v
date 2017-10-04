module avm_mem_accel_bridge (
    //qsys inputs
    input clk,
    input reset,

    //accel inputs
    input [127:0] avs_accel_writedata,
    input avs_accel_address,
    input avs_accel_write,
    input avs_accel_read,
    output [127:0] avs_accel_readdata,
    output avs_accel_waitrequest,

    //mem outputs
    //ignore the upper bit of the address so that the s2 interface
    //of on chip memory can sit at 0x0 
    input avm_mem_waitrequest,
    output [31:0] avm_mem_address,
    input [63:0] avm_mem_readdata,
    output avm_mem_read,
    output avm_mem_write,
    output [63:0] avm_mem_writedata,
    output [7:0] avm_mem_byteenable
);

wire        mem8, mem16, mem32, mem64;
wire [2:0]  atm_byteSel_32bit;

assign mem8 = avs_accel_writedata[96];
assign mem16 = avs_accel_writedata[97];
assign mem64 = avs_accel_writedata[98];
assign mem32 = ~(mem8 | mem16 | mem64);

assign atm_byteSel_32bit = avs_accel_writedata[2:0];
assign avm_mem_write = avs_accel_write;
assign avm_mem_read = avs_accel_read;

//set byteenable according to appropiate byte enable signals
assign avm_mem_byteenable = ({{4{mem64}}, {2{mem32|mem64}}, {mem16|mem32|mem64}, {1'b1}}) << atm_byteSel_32bit;
//set address to least significant 31 bits of writedata
assign avm_mem_address = avs_accel_writedata[30:0];
//concatenate readdata with byteenable shifted version 
assign avs_accel_readdata = {64'd0, avm_mem_readdata} >> (atm_byteSel_32bit * 8);
//set actual writedata to middle 64 bits
assign avm_mem_writedata = avs_accel_writedata[95:32] << (atm_byteSel_32bit * 8);

assign avs_accel_waitrequest = avm_mem_waitrequest & (avs_accel_write | avs_accel_read);

endmodule
