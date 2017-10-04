`timescale 1ns / 1ps

module legup_mutex (

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
	input  [ 31: 0]  avs_s1_writedata;

	output [ 31: 0]  avs_s1_readdata; 
	output  		   avs_s1_waitrequest;

	wire             mutex_free;
	reg    [ 31: 0]  mutex_owner;
	reg              mutex_state;
	wire             owner_valid;
	wire 			 address;
	wire 			 reset_n;
	wire 			 clk;
	wire 			 read;
	wire 			 write;
	wire   [ 31: 0]  accel_id;

	assign address = avs_s1_address;
	assign reset_n = csi_clockreset_reset_n;
	assign clk = csi_clockreset_clk;
	assign read = avs_s1_read;
	assign write = avs_s1_write;
	assign accel_id = avs_s1_writedata;
	assign avs_s1_waitrequest = 1'b0;

    always @(posedge clk or negedge reset_n)
    begin
      if (!reset_n)
	  begin
          mutex_state <= 0;
		  mutex_owner <= 0;
	  end
	  //if mutex is free and someone tries to get the mutex
      else if (mutex_free && write && !address)
	  begin
		  //mutex_state <= avs_s1_writedata[0];
          mutex_state <= 1'b1;
		  mutex_owner <= accel_id;
	  end
	  //if mutex is taken and the owner wants to release the mutex
	  else if (owner_valid && write && address)
	  begin
		  mutex_state <= 0;
		  mutex_owner <= 0;
	  end
    end

	assign avs_s1_readdata = mutex_owner;
	assign mutex_free = mutex_state == 0;
	assign owner_valid = mutex_owner == accel_id;

endmodule

