module ztiger_mem_controller(
	input         clock, stall_execute, 
	input  [4:0]  control_i, // {mem_opt, readn_write, unsign, mem32, mem8n_mem16}
	input  [31:0] dataa_i, datab_i,
	output [31:0] result,
	output        stall_o,	
	
	input         d_stall, memCanRead, memCanWrite,
	input  [31:0] memreaddata,	
	output        memread, memwrite, mem8, mem16,
	output [31:0] memaddress, memwritedata
);

	assign memread      = ~stall_execute & (control_i[4:3] == 2'b10) ;
	assign memwrite     = ~stall_execute & (control_i[4:3] == 2'b11) ;
	assign memaddress   = dataa_i;
	assign memwritedata = datab_i;
	assign mem8         = control_i[1:0] == 2'b00;
	assign mem16        = control_i[1:0] == 2'b01;
	
	reg unsign, mem8_reg, mem16_reg, reading;
	always @ (posedge clock)
	begin
		if(stall_execute)
		begin
			// stall
		end
		else begin
			reading    <= memread;
			mem8_reg   <= mem8;
			mem16_reg  <= mem16;		
			unsign     <= control_i[2];
		end
	end
	
	reg first_stall;
	always @ (posedge clock)
	begin
		if(stall_execute)
			first_stall <= 1'b0;	
		else
			first_stall <= control_i[4];
	end

	reg d_stall_reg, memCanRead_reg, memCanWrite_reg;
	reg [31:0] memreaddata_reg;
	always @ (posedge clock)
	begin
		d_stall_reg <= d_stall;
		memCanRead_reg <= memCanRead;
		memCanWrite_reg <= memCanWrite;
		memreaddata_reg <= memreaddata;
	end
	
	assign stall_o = (first_stall & (reading | control_i[4])) | d_stall_reg | ((control_i[4:3] == 2'b10) & !memCanRead_reg) | ((control_i[4:3] == 2'b11) & !memCanWrite_reg);
	assign result = ({unsign, mem16_reg} == 2'b01) ? {{16{memreaddata_reg[15]}}, memreaddata_reg[15:0]} : ({unsign, mem8_reg} == 2'b01) ? {{24{memreaddata_reg[7]}}, memreaddata_reg[7:0]} : memreaddata_reg;
	
endmodule 
