// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

module omp_core (

                 // inputs:
                  avs_s1_address,
                 // chipselect,
                  csi_clockreset_clk,
				  csi_clockreset_reset_n,
                  avs_s1_writedata,
                  avs_s1_read,
                  avs_s1_write,

                 // outputs:
				  avs_s1_waitrequest,
                  avs_s1_readdata
               );

	input  [ 31: 0] avs_s1_writedata;
//	input  [ 15: 0] avs_s1_writedata;
	input            avs_s1_address;
	input            csi_clockreset_clk;
	output   [ 31: 0] avs_s1_readdata;
//	output   [ 15: 0] avs_s1_readdata;
	input            avs_s1_read;
	input            csi_clockreset_reset_n;
	input            avs_s1_write;
	output  		   avs_s1_waitrequest;

	//we hard-code this for now to indicate the total number of threads in system
//	parameter num_threads = 128'd2;
//	parameter num_threads = 16'd2;

	reg [31:0] thread_ID;
//	reg [15:0] thread_ID;
//	reg [15:0] num_threads;
	reg [31:0] num_threads;

	wire reset_n;
	wire clk;
	wire read;
	wire write;


	assign reset_n = csi_clockreset_reset_n;
	assign clk = csi_clockreset_clk;
	assign read = avs_s1_read;
	assign write = avs_s1_write;
	//assign accel_id = avs_s1_writedata;
	assign avs_s1_waitrequest = 1'b0;
	always @(posedge clk or negedge reset_n)
	begin
		if (reset_n == 0)
		begin
		  thread_ID <= 0;
		  num_threads <= 0;
		end
		//when getting the thread number for each thread
		else if (read & !avs_s1_address)
		begin
		  thread_ID <= thread_ID+1;
		end	
		//when setting the number of omp threads
		else if (write & avs_s1_address)
		begin
		  num_threads <= avs_s1_writedata;
		end
	end



  assign avs_s1_readdata = (avs_s1_address)? num_threads : thread_ID;

endmodule

