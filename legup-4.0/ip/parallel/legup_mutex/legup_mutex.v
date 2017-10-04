// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

module legup_mutex (
clk,
reset_n, 

// inputs:
avs_s1_address,
avs_s1_read,
avs_s1_write,
avs_s1_writedata,

// outputs:
avs_s1_readdata,
avs_s1_waitrequest
);


input               clk;
input               reset_n; 

input               avs_s1_address;
input               avs_s1_read;
input               avs_s1_write;
input       [31: 0] avs_s1_writedata;

output      [31: 0] avs_s1_readdata; 
output  		    avs_s1_waitrequest;

wire        [31: 0] accel_id;
wire                mutex_free;
reg         [31: 0] mutex_owner;
reg                 mutex_state;
wire                owner_valid;

assign avs_s1_readdata      = mutex_owner;
assign avs_s1_waitrequest   = 1'b0;

always @(posedge clk or negedge reset_n)
begin
    if (!reset_n)
    begin
        mutex_state <= 0;
        mutex_owner <= 0;
    end
    //if mutex is free and someone tries to get the mutex
    else if (mutex_free && avs_s1_write && !avs_s1_address)
    begin
        //mutex_state <= avs_s1_writedata[0];
        mutex_state <= 1'b1;
        mutex_owner <= accel_id;
    end
    //if mutex is taken and the owner wants to release the mutex
    else if (owner_valid && avs_s1_write && avs_s1_address)
    begin
        mutex_state <= 0;
        mutex_owner <= 0;
    end
end

assign accel_id         = avs_s1_writedata;
assign mutex_free       = mutex_state == 0;
assign owner_valid      = mutex_owner == accel_id;

endmodule

