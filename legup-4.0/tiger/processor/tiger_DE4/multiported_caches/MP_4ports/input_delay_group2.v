`include "cache_parameters.v"

module input_delay_group2(
    input  wire         clock,
    input  wire         enable,

	input  wire			write_en_input_0,
	input  wire			write_en_input_1,

	output  wire		write_en_output_0,
	output  wire		write_en_output_1,

	input  wire	`BYTE_EN byte_en_input_0,
	input  wire	`BYTE_EN byte_en_input_1,

	output  wire `BYTE_EN byte_en_output_0,
	output  wire `BYTE_EN byte_en_output_1,

    input  wire `MEM_ADDR addr_input_0,
    input  wire `MEM_ADDR addr_input_1,

    output  wire `MEM_ADDR addr_output_0,
    output  wire `MEM_ADDR addr_output_1,

    input  wire `WORD data_input_0,
    input  wire `WORD data_input_1,

    output  wire `WORD data_output_0,
    output  wire `WORD data_output_1

);

input_delay_MEM_ADDR input_delay_MEM_ADDR_0(
    .clock(clock),
    .enable(enable),
    .input_port(addr_input_0),
    .output_port(addr_output_0)
);

input_delay_MEM_ADDR input_delay_MEM_ADDR_1(
    .clock(clock),
    .enable(enable),
    .input_port(addr_input_1),
    .output_port(addr_output_1)
);

input_delay_WORD input_delay_WORD_0(
    .clock(clock),
    .enable(enable),
    .input_port(data_input_0),
    .output_port(data_output_0)
);

input_delay_WORD input_delay_WORD_1(
    .clock(clock),
    .enable(enable),
    .input_port(data_input_1),
    .output_port(data_output_1)
);

input_delay_write_en input_delay_write_en_0(
    .clock(clock),
    .enable(enable),
    .input_port(write_en_input_0),
    .output_port(write_en_output_0)
);

input_delay_write_en input_delay_write_en_1(
    .clock(clock),
    .enable(enable),
    .input_port(write_en_input_1),
    .output_port(write_en_output_1)
);

input_delay_byte_en input_delay_byte_en_0(
    .clock(clock),
    .enable(enable),
    .input_port(byte_en_input_0),
    .output_port(byte_en_output_0)
);

input_delay_byte_en input_delay_byte_en_1(
    .clock(clock),
    .enable(enable),
    .input_port(byte_en_input_1),
    .output_port(byte_en_output_1)
);


endmodule
