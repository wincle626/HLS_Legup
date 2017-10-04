//This module contains the state machine to control processor for accessing components over Avalon (non-memory related)
module tiger_avalon (
	input clk,
	input reset,
	
	input [31:0] memaddress,
	input memread,
	input memwrite,
	input [31:0] memwritedata,
	input mem8,
	input mem16,
	output avalon_stall,

	output reg [31:0]avm_procMaster_address,
	output reg avm_procMaster_read,
	output reg avm_procMaster_write,
	output reg [31:0] avm_procMaster_writedata,	
	output reg [3:0] avm_procMaster_byteenable,
	input [31:0]avm_procMaster_readdata,
	input avm_procMaster_waitrequest,
	input avm_procMaster_readdatavalid
);

	localparam stateIDLE = 0;
	localparam stateAVALON_READ = 1;
	localparam stateAVALON_WRITE = 2;

	wire [1:0] bytes;

	reg [1:0] state;
	reg [31:0] procMaster_writedata_32;
	reg [3:0] procMaster_byteenable_32;

	assign bytes = memaddress[1 : 0];
	assign avalon_stall = (state == stateAVALON_READ && !avm_procMaster_readdatavalid) || (memwrite && state != stateIDLE) || (state != stateIDLE && memread);

	//state machine for cache controller
	always @(posedge clk, posedge reset) begin
		if(reset) begin
			state <= stateIDLE;
			//avm_procMaster_address <= 0;
			avm_procMaster_read <= 0;
			avm_procMaster_write <= 0;
			//avm_procMaster_writedata <= 0;
		end else begin
			case(state)
				stateIDLE: begin
					avm_procMaster_address <= {memaddress[31:2], 2'b0};
					//if((memread && bypassCache) || memWrite) begin
					avm_procMaster_byteenable <= procMaster_byteenable_32;
					//end

					if(memread) begin //If we want a read
						avm_procMaster_read <= 1; //start a read
						state <= stateAVALON_READ;

					end else if(memwrite) begin //If we want a write 
						avm_procMaster_write <= 1; //start a read
						state <= stateAVALON_WRITE;
						avm_procMaster_writedata <= procMaster_writedata_32;				
					end
				end

				stateAVALON_READ: begin
					//No more wait request so address has been captured
					if(!avm_procMaster_waitrequest) begin
						avm_procMaster_read <= 0; //So stop asserting read
					end
			
					if(avm_procMaster_readdatavalid) begin //We have our data
						state <= stateIDLE; //so go back to the idle state
					end
				end
				stateAVALON_WRITE: begin
					if(!avm_procMaster_waitrequest) begin //if the write has finished
						state <= stateIDLE; //then go back to the idle state
						avm_procMaster_write <= 0;
					end
				end

			endcase
		end
	end 		

	always @(*)
	begin
		if (mem8)
		begin
			case (bytes)
				2'b00: begin
					   procMaster_writedata_32 <= {24'd0, memwritedata[7 : 0]};
		  			   procMaster_byteenable_32 <= 4'b0001;
					   end
				2'b01: begin
					   procMaster_writedata_32 <= {16'd0, memwritedata[7 : 0], 8'd0};
		  			   procMaster_byteenable_32 <= 4'b0010;	
					   end
				2'b10: begin
					   procMaster_writedata_32 <= {8'd0, memwritedata[7 : 0], 16'd0};
		  			   procMaster_byteenable_32 <= 4'b0100;	
					   end
				2'b11: begin
					   procMaster_writedata_32 <= {memwritedata[7 : 0], 24'b0};
		  			   procMaster_byteenable_32 <= 4'b1000;		
					   end
				default : begin
					   procMaster_writedata_32 <= 32'dx;
		  			   procMaster_byteenable_32 <= 4'bxxxx;		
					   end
			endcase
		end
		else if (mem16)
		begin
			case (bytes[1])
				1'b0: begin
					  procMaster_writedata_32 <= {16'd0, memwritedata[15 : 0]};
					  procMaster_byteenable_32 <= 4'b0011;
					  end
				1'b1: begin
					  procMaster_writedata_32 <= {memwritedata[15 : 0], 16'd0};
					  procMaster_byteenable_32 <= 4'b1100;
					  end
			default : begin
					  procMaster_writedata_32 <= 32'dx;
 	    			  procMaster_byteenable_32 <= 4'bxxxx;	
					  end
			endcase
		end
		else 
		begin
			procMaster_writedata_32 <= memwritedata;
 		    procMaster_byteenable_32 <= 4'b1111;	
		end
	end

endmodule
