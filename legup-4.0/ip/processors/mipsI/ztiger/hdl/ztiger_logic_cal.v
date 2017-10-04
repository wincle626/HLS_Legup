module ztiger_logic_cal (
	input         clock, stall_execute,
	input  [1:0]  control_i,
	input  [31:0] dataa_i, datab_i,
	output [31:0] result
);

	reg [31:0] dataa,  datab;
	reg [1:0]  control;
	always @ (posedge clock)
	begin
		if(stall_execute)
		begin
			// stall
		end
		else begin
			dataa   <= dataa_i;
			datab   <= datab_i;
			control <= control_i;
		end
	end

	assign result = control[1:0] == 2'b00 ? dataa & datab :
	                control[1:0] == 2'b01 ? dataa | datab :
	                control[1:0] == 2'b10 ? dataa ^ datab : ~(dataa | datab);						
endmodule 
