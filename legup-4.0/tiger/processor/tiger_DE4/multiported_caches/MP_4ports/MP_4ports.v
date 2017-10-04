`include "cache_parameters.v"

module MP_4ports(
    input  wire             base_clock, // system clock
    input  wire             clock,      // multiple of system clock

    input  wire  `MEM_ADDR 	addr_0,   //proc
	input  wire 			write_en_0,
	input  wire  `BYTE_EN	byte_en_0,
    input  wire  `WORD      write_data_0,
    output wire  `WORD      read_data_0,

    input  wire  `MEM_ADDR  addr_1,   //accel1   
	input  wire 			write_en_1,
	input  wire  `BYTE_EN	byte_en_1,
    input  wire  `WORD      write_data_1,
    output wire  `WORD      read_data_1,

    input  wire  `MEM_ADDR  addr_2,   //accel2
	input  wire 			write_en_2,
	input  wire  `BYTE_EN	byte_en_2,
    input  wire  `WORD      write_data_2,
	output reg  `WORD      read_data_2,

    input  wire  `MEM_ADDR  addr_3,   //accel3
	input  wire 			write_en_3,
	input  wire  `BYTE_EN	byte_en_3,
    input  wire  `WORD      write_data_3,
    output reg  `WORD      read_data_3
);

//proc and accel1 are in group1, which access the memory in the first internal cycle, accel2 and accel3 are in group2, which access the memory in the second internal cycle
wire `PHASE       phase;

multipumping_controller_2x multipumping_controller_2x(
    .base_clock(base_clock),
    .clock(clock),
    .phase(phase)
);

wire `MEM_ADDR addr_delayed_0;
wire `MEM_ADDR addr_delayed_1;

wire `WORD write_data_delayed_0;
wire `WORD write_data_delayed_1;

wire  write_en_delayed_0;
wire  write_en_delayed_1;

wire `BYTE_EN byte_en_delayed_0;
wire `BYTE_EN byte_en_delayed_1;

reg  delay_group2_en;

input_delay_group2 input_delay_group2_inst(
    .clock(clock),
    .enable(~phase),

	.write_en_input_0(write_en_2),
	.write_en_input_1(write_en_3),

	.write_en_output_0(write_en_delayed_0),
	.write_en_output_1(write_en_delayed_1),

	.byte_en_input_0(byte_en_2),
	.byte_en_input_1(byte_en_3),

	.byte_en_output_0(byte_en_delayed_0),
	.byte_en_output_1(byte_en_delayed_1),

    .addr_input_0(addr_2),
    .addr_input_1(addr_3),

    .addr_output_0(addr_delayed_0),
    .addr_output_1(addr_delayed_1),

    .data_input_0(write_data_2),
    .data_input_1(write_data_3),

    .data_output_0(write_data_delayed_0),
    .data_output_1(write_data_delayed_1)
);

reg `MEM_ADDR port1_addr;
reg `MEM_ADDR port2_addr;
reg  		  port1_write_en;
reg  		  port2_write_en;
reg `BYTE_EN  port1_byte_en;
reg `BYTE_EN  port2_byte_en;
reg `WORD     port1_write_data;
reg `WORD     port2_write_data;

//in the first interal cycle, take the address for proc and accel1 (group1), and second internal cycle take address for accel2 and accel3 (group2)
always @(*) begin
    if(phase == `PHASE_0) begin
        port1_addr <= addr_0;
        port2_addr <= addr_1;
		port1_write_en <= write_en_0;
		port2_write_en <= write_en_1;
		port1_byte_en <= byte_en_0;
		port2_byte_en <= byte_en_1;
		port1_write_data <= write_data_0;
		port2_write_data <= write_data_1;
    end
    else begin
        port1_addr <= addr_delayed_0;
        port2_addr <= addr_delayed_1;
		port1_write_en <= write_en_delayed_0;
		port2_write_en <= write_en_delayed_1;
		port1_byte_en <= byte_en_delayed_0;
		port2_byte_en <= byte_en_delayed_1;
		port1_write_data <= write_data_delayed_0;
		port2_write_data <= write_data_delayed_1;
    end
end

wire `WORD port1_read_data_internal;
wire `WORD port2_read_data_internal;


memory_2w2r memory_2w2r(
    .clock(clock),      
    .write_en_0(port1_write_en),
    .write_en_1(port2_write_en),
    .byte_en_0(port1_byte_en),
    .byte_en_1(port2_byte_en),
    .addr_0(port1_addr),
    .addr_1(port2_addr),
    .write_data_0(port1_write_data),
    .write_data_1(port2_write_data),
    .read_data_0(port1_read_data_internal),
    .read_data_1(port2_read_data_internal)
); 

reg `WORD read_data_2_reg;
reg `WORD read_data_3_reg;

always @(*) begin
    if(phase == `PHASE_0) begin
		read_data_2 <= port1_read_data_internal;
		read_data_3 <= port2_read_data_internal;
	end else begin
		read_data_2 <= read_data_2_reg;
		read_data_3 <= read_data_3_reg;
	end
end

always @(posedge clock) begin
    if(phase == `PHASE_0) begin
		read_data_2_reg <= port1_read_data_internal;
		read_data_3_reg <= port2_read_data_internal;
	end
end

//to hold the read values for the group 1 in registers after first internal cycle
read_port_registers_2r2x read_port_registers_2r2x_inst(
    .clock(clock),
    .write_en(phase),

    .input_0(port1_read_data_internal),
    .input_1(port2_read_data_internal),

    .output_0(read_data_0),
    .output_1(read_data_1)

);

endmodule
