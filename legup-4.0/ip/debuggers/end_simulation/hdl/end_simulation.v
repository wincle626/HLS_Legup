
module end_simulation (
	input						clk,
	input						reset, 

	input						avs_control_write,
	input			[31: 0]	avs_control_writedata
);

always @(*)
begin
	if (avs_control_write)
	begin
		$display("Ending simulation\n");
		$stop;
	end
end

endmodule

