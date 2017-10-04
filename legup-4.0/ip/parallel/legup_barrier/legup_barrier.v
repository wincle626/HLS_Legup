`timescale 1ns / 1ps

module legup_barrier (
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

reg         [31: 0] barrier_count;
reg         [31: 0] numthreads;

always @(posedge clk or negedge reset_n)
begin
    if (!reset_n)
    begin
        numthreads <= 0;
    end
    //barrier initialization to store the number of threads
    else if (avs_s1_write && !avs_s1_address)
    begin
        numthreads <= avs_s1_writedata;
    end
end

always @(posedge clk or negedge reset_n)
begin
    if (!reset_n)
    begin
        barrier_count <= 0;
    end
    //whenever each thread reaches the barrier, it writes to the barrier, which increments the counter
    else if (avs_s1_write && avs_s1_address)
    begin
        barrier_count <= barrier_count + 1;
    end
    //if the counter reaches the total number of threads, it resets to zero
    //this is needed for when the same barrier is used multiplie times without initializing the barrier again
    else if (barrier_count == numthreads)
    begin
        barrier_count <= 0;
    end
end

//if the barrier count is zero, that means all threads have reached the barrier
assign avs_s1_readdata      = barrier_count;
assign avs_s1_waitrequest   = 1'b0;

endmodule

