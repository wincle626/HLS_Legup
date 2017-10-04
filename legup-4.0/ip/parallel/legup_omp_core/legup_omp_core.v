// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

module legup_omp_core (
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

//we hard-code this for now to indicate the total number of threads in system
//	parameter num_threads = 128'd2;
//	parameter num_threads = 16'd2;

reg [31:0] thread_ID;
reg [31:0] num_threads;

always @(posedge clk or negedge reset_n)
begin
    if (reset_n == 0)
    begin
        thread_ID   <= 0;
        num_threads <= 0;
    end
    //when getting the thread number for each thread
    else if (avs_s1_read & !avs_s1_address)
    begin
        thread_ID   <= thread_ID+1;
    end	
    //when setting the number of omp threads
    else if (avs_s1_write & avs_s1_address)
    begin
        num_threads <= avs_s1_writedata;
    end
end



assign avs_s1_readdata      = (avs_s1_address) ? num_threads : thread_ID;
assign avs_s1_waitrequest   = 1'b0;

endmodule

