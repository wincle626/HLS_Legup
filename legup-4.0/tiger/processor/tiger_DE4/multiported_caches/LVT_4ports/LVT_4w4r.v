`include "cache_parameters.v"

module LVT_4w4r(
    input  wire            clock,
	input  wire			   write_0,
	input  wire			   write_1,	
	input  wire			   write_2,	
	input  wire			   write_3,		

    input  wire `MEM_ADDR  write_addr_0,
    input  wire `MEM_ADDR  write_addr_1,
    input  wire `MEM_ADDR  write_addr_2,
    input  wire `MEM_ADDR  write_addr_3,

    input  wire `MEM_ADDR  read_addr_0,
    output reg  `LVT_ENTRY selector_0,
    input  wire `MEM_ADDR  read_addr_1,
    output reg  `LVT_ENTRY selector_1,
    input  wire `MEM_ADDR  read_addr_2,
    output reg  `LVT_ENTRY selector_2,
    input  wire `MEM_ADDR  read_addr_3,
    output reg  `LVT_ENTRY selector_3

);

reg `LVT_ENTRY live_value_table [`LVT_DEPTH-1:0];

// synthesis translate_off
//initialize LVT to zero to avaid X's
integer i;
initial begin
	for (i=0; i<`LVT_DEPTH; i=i+1) begin
		live_value_table[i] <= 'd0;
	end	
end
// synthesis translate_on

//storing which accelerator most recently wrote to the given address in the live value table
always @(posedge clock) begin
	if (write_0)
	begin
	    live_value_table[write_addr_0] <= `ACCEL_0;
	end

	if (write_1)
	begin
	    live_value_table[write_addr_1] <= `ACCEL_1;
	end

	if (write_2)
	begin
    	live_value_table[write_addr_2] <= `ACCEL_2;
	end

	if (write_3)
	begin
	    live_value_table[write_addr_3] <= `ACCEL_3;
	end

end

//selector for the output mux, controls which RAM to take the memory from 
//selector_n used as the selector signal for nth 3to1 MUX. 
always @(posedge clock) begin

	//when write is happening, take the data from ACCEL, since this data needs to be used right away
	if (write_0)
		selector_0 <= `ACCEL_0;
	else
	    selector_0 <= live_value_table[read_addr_0];

	if (write_1)
		selector_1 <= `ACCEL_1;
	else
	    selector_1 <= live_value_table[read_addr_1];

	if (write_2)
		selector_2 <= `ACCEL_2;
	else
	    selector_2 <= live_value_table[read_addr_2];

	if (write_3)
		selector_3 <= `ACCEL_3;
	else
	    selector_3 <= live_value_table[read_addr_3];


end

endmodule
