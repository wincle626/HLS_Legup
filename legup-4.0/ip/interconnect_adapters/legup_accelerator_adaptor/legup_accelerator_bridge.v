//  This module connects a 'Tiger' interconnect style accelerator
// to the avalon interconnect

module legup_accelerator_bridge
#(
///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Parameter definitions ////////////////////////////
///////////////////////////////////////////////////////////////////////////////
parameter ADDR_WIDTH = 5 // bits

) (
clk,
reset, 

// CPU to Accelerator Bridge
avs_from_cpu_address,
avs_from_cpu_byteenable,
avs_from_cpu_read,
avs_from_cpu_write,
avs_from_cpu_writedata,
avs_from_cpu_readdata,
avs_from_cpu_waitrequest,
    
avm_to_accel_readdata,
avm_to_accel_waitrequest,
avm_to_accel_address,
avm_to_accel_byteenable,
avm_to_accel_read,
avm_to_accel_write,
avm_to_accel_writedata,

// Accelerator to Memory Bridge
avs_from_accel_address,
avs_from_accel_read,
avs_from_accel_write,
avs_from_accel_writedata,
avs_from_accel_readdata,
avs_from_accel_waitrequest,

avm_to_memory_readdata,
avm_to_memory_waitrequest,
avm_to_memory_address,
avm_to_memory_byteenable,
avm_to_memory_read,
avm_to_memory_write,
avm_to_memory_writedata
);


///////////////////////////////////////////////////////////////////////////////
///////////////////////// Local parameter definitions /////////////////////////
///////////////////////////////////////////////////////////////////////////////
localparam	AW = ADDR_WIDTH - 1;  // Address width


///////////////////////////////////////////////////////////////////////////////
////////////////////////////// Port declarations //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
input               clk;
input               reset; 

// CPU to Accelerator Bridge
input       [AW: 0] avs_from_cpu_address;
input       [ 3: 0] avs_from_cpu_byteenable;
input               avs_from_cpu_read;
input               avs_from_cpu_write;
input       [31: 0] avs_from_cpu_writedata;
output      [31: 0] avs_from_cpu_readdata;
output              avs_from_cpu_waitrequest;
    
input       [31: 0] avm_to_accel_readdata;
input               avm_to_accel_waitrequest;
output      [31: 0] avm_to_accel_address;
output      [ 3: 0] avm_to_accel_byteenable;
output              avm_to_accel_read;
output              avm_to_accel_write;
output      [31: 0] avm_to_accel_writedata;

// Accelerator to Memory Bridge
input       [ 3: 0] avs_from_accel_address;
input               avs_from_accel_read;
input               avs_from_accel_write;
input       [127:0] avs_from_accel_writedata;
output      [127:0] avs_from_accel_readdata;
output              avs_from_accel_waitrequest;

input       [31: 0] avm_to_memory_readdata;
input               avm_to_memory_waitrequest;
output      [31: 0] avm_to_memory_address;
output      [ 3: 0] avm_to_memory_byteenable;
output              avm_to_memory_read;
output              avm_to_memory_write;
output      [31: 0] avm_to_memory_writedata;


///////////////////////////////////////////////////////////////////////////////
////////////////////////// Local signals definitions //////////////////////////
///////////////////////////////////////////////////////////////////////////////
wire                stall_cpu;
wire                unstall_cpu;

reg                 cpu_stalled;

reg         [31: 0] last_to_memory_readdata;
reg                 transfer_64bits;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////// Output assignments //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Avalon Master to Tiger Slave
assign avs_from_cpu_readdata            = avm_to_accel_readdata[31:0];
assign avs_from_cpu_waitrequest         = avm_to_accel_waitrequest | cpu_stalled;

assign avm_to_accel_address             = avs_from_cpu_address;
assign avm_to_accel_byteenable          = avs_from_cpu_byteenable;
assign avm_to_accel_read                = avs_from_cpu_read & ~cpu_stalled;
assign avm_to_accel_write               = avs_from_cpu_write & ~cpu_stalled;
assign avm_to_accel_writedata           = avs_from_cpu_writedata;


// Tiger Master to Avalon Slave    
assign avs_from_accel_readdata[127:64]  = 64'h0;
assign avs_from_accel_readdata[63:0]    = //avm_to_memory_readdata;
    avs_from_accel_writedata[96] ? 
        avs_from_accel_writedata[1:0] == 2'h0 ? avm_to_memory_readdata[ 7: 0] : 
        avs_from_accel_writedata[1:0] == 2'h1 ? avm_to_memory_readdata[15: 8] : 
        avs_from_accel_writedata[1:0] == 2'h2 ? avm_to_memory_readdata[23:16] :
                                                avm_to_memory_readdata[31:24] : 
    avs_from_accel_writedata[97] ? 
        avs_from_accel_writedata[1:0] == 2'h0 ? avm_to_memory_readdata[15: 0] : 
                                                avm_to_memory_readdata[31:16] : 
    transfer_64bits ?
        {avm_to_memory_readdata, last_to_memory_readdata} :
        avm_to_memory_readdata;
assign avs_from_accel_waitrequest       = (avm_to_memory_waitrequest | 
    (avs_from_accel_writedata[98] & ~transfer_64bits)) &
    (avm_to_memory_read | avm_to_memory_write);

assign avm_to_memory_address            = 
    avs_from_accel_writedata[98] ?    
        {avs_from_accel_writedata[31:3], transfer_64bits, 2'h0} :
        {avs_from_accel_writedata[31:2], 2'h0};
assign avm_to_memory_byteenable         = 
    avs_from_accel_writedata[96] ? 
        avs_from_accel_writedata[1:0] == 2'h0 ? 4'h1 : 
        avs_from_accel_writedata[1:0] == 2'h1 ? 4'h2 : 
        avs_from_accel_writedata[1:0] == 2'h2 ? 4'h4 :
                                                4'h8 : 
    avs_from_accel_writedata[97] ? 
        avs_from_accel_writedata[1:0] == 2'h0 ? 4'h3 : 
                                                4'hC : 
        4'hF;
assign avm_to_memory_read               = avs_from_accel_read;
assign avm_to_memory_write              = avs_from_accel_write & ~stall_cpu & ~unstall_cpu;
assign avm_to_memory_writedata          = //avs_from_accel_writedata[63:32];
        avs_from_accel_writedata[96] ?  {4{avs_from_accel_writedata[39:32]}} :
        avs_from_accel_writedata[97] ?  {2{avs_from_accel_writedata[47:32]}} :
        transfer_64bits              ?  avs_from_accel_writedata[95:64] :
                                        avs_from_accel_writedata[63:32];


///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Internal assignments /////////////////////////////
///////////////////////////////////////////////////////////////////////////////
assign stall_cpu    = avs_from_accel_write & avs_from_accel_writedata[100];
assign unstall_cpu  = avs_from_accel_write & avs_from_accel_writedata[101];

always @(posedge clk)
begin
    if (reset)
        cpu_stalled <= 1'b0;
    else if (stall_cpu)
        cpu_stalled <= 1'b1;
    else if (unstall_cpu)
        cpu_stalled <= 1'b0;
end

always @(posedge clk)
    if (avm_to_memory_read & ~avm_to_memory_waitrequest)
        last_to_memory_readdata <= avm_to_memory_readdata;

always @(posedge clk)
    if (reset)
        transfer_64bits <= 1'b0;
    else if (~avm_to_memory_waitrequest & 
            (avm_to_memory_read | avm_to_memory_write))
        transfer_64bits <= avs_from_accel_writedata[98] & ~transfer_64bits;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Finite state machines ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Submodules //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

endmodule

