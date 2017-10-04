`timescale 1ns / 1ps

module legup_barrier (

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


	input            avs_s1_address;
	input            csi_clockreset_clk;
	input            csi_clockreset_reset_n; 
	input            avs_s1_read;
	input            avs_s1_write;
	input  [127: 0]  avs_s1_writedata;

	output [127: 0]  avs_s1_readdata; 
	output  		 avs_s1_waitrequest;

	reg    [ 31: 0]  barrier_count;
	reg    [ 31: 0]  numthreads;
	wire 			 address;
	wire 			 reset_n;
	wire 			 clk;
	wire 			 read;
	wire 			 write;
	wire   [ 31: 0]  writedata;

	assign address = avs_s1_address;
	assign reset_n = csi_clockreset_reset_n;
	assign clk = csi_clockreset_clk;
	assign read = avs_s1_read;
	assign write = avs_s1_write;
	assign writedata = avs_s1_writedata;
	assign avs_s1_waitrequest = 1'b0;

    always @(posedge clk or negedge reset_n)
    begin
		if (!reset_n)
		begin
			numthreads <= 0;
		end
		//barrier initialization to store the number of threads
		else if (write && !address)
		begin
			numthreads <= writedata[31:0];
		end
    end

    always @(posedge clk or negedge reset_n)
    begin
		if (!reset_n)
		begin
			barrier_count <= 0;
		end
		//whenever each thread reaches the barrier, it writes to the barrier, which increments the counter
		else if (write && address)
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
	assign avs_s1_readdata = barrier_count;

endmodule

