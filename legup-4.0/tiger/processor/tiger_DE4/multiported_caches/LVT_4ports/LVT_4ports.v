//a write occurs to a single memory bank, but to all 4 RAMs within a memory bank
//a read occurs to all memory banks, but to a single RAM within a memory bank
//4to1 MUX selects the output bank according to the value from LVT during a read

//there are as many banks as the number of write ports
//there are as many RAMs within a bank as the number of read ports 
`include "cache_parameters.v"

module LVT_4ports(
    input  wire             clock,
	input  wire  			write_0,
	input  wire  `BYTE_EN	byte_en_0,
	input  wire  			write_1,
	input  wire  `BYTE_EN	byte_en_1,
	input  wire  			write_2,
	input  wire  `BYTE_EN	byte_en_2,
	input  wire  			write_3,
	input  wire  `BYTE_EN	byte_en_3,

    input  wire  `MEM_ADDR  addr_0,
    output wire  `WORD      read_data_0,
    input  wire  `WORD      write_data_0,

    input  wire  `MEM_ADDR  addr_1,
    output wire  `WORD      read_data_1,
    input  wire  `WORD      write_data_1,

    input  wire  `MEM_ADDR  addr_2,
    output wire  `WORD      read_data_2,
    input  wire  `WORD      write_data_2,

    input  wire  `MEM_ADDR  addr_3,
    output wire  `WORD      read_data_3,
    input  wire  `WORD      write_data_3

);

//indicates which accelerator has written to that memory address most recently
wire `LVT_ENTRY RAM_select_0;
wire `LVT_ENTRY RAM_select_1;
wire `LVT_ENTRY RAM_select_2;
wire `LVT_ENTRY RAM_select_3;

LVT_4w4r LVT_4w4r(
    .clock(clock),
	.write_0(write_0),
	.write_1(write_1),
	.write_2(write_2),
	.write_3(write_3),

    .write_addr_0(addr_0),
    .write_addr_1(addr_1),
    .write_addr_2(addr_2),
    .write_addr_3(addr_3),

    .read_addr_0(addr_0),
    .selector_0(RAM_select_0),
    .read_addr_1(addr_1),
    .selector_1(RAM_select_1),
    .read_addr_2(addr_2),
    .selector_2(RAM_select_2),
    .read_addr_3(addr_3),
    .selector_3(RAM_select_3)
);

//naming convention read_data_accel#_RAM#
wire `WORD read_data_0_0;
wire `WORD read_data_0_1;
wire `WORD read_data_0_2;

wire `WORD read_data_1_0;
wire `WORD read_data_1_3;
wire `WORD read_data_1_4;

wire `WORD read_data_2_1;
wire `WORD read_data_2_3;
wire `WORD read_data_2_5;

wire `WORD read_data_3_2;
wire `WORD read_data_3_4;
wire `WORD read_data_3_5;

//RAM_0 between port0 port1
dcacheMem RAM_0(
	.clock(clock),

	.address_a(addr_0),
	.address_b(addr_1),

	.byteena_a(byte_en_0),
	.byteena_b(byte_en_1),

	.data_a(write_data_0),
	.data_b(write_data_1),

	.wren_a(write_0),
	.wren_b(write_1),

	.q_a(read_data_0_0),
	.q_b(read_data_1_0)
);

//RAM_1 between port0 port2
dcacheMem RAM_1(
	.clock(clock),

	.address_a(addr_0),
	.address_b(addr_2),

	.byteena_a(byte_en_0),
	.byteena_b(byte_en_2),

	.data_a(write_data_0),
	.data_b(write_data_2),

	.wren_a(write_0),
	.wren_b(write_2),

	.q_a(read_data_0_1),
	.q_b(read_data_2_1)
);

//RAM_2 between port0 port3
dcacheMem RAM_2(
	.clock(clock),

	.address_a(addr_0),
	.address_b(addr_3),

	.byteena_a(byte_en_0),
	.byteena_b(byte_en_3),

	.data_a(write_data_0),
	.data_b(write_data_3),

	.wren_a(write_0),
	.wren_b(write_3),

	.q_a(read_data_0_2),
	.q_b(read_data_3_2)
);

//RAM_3 between port1 port2
dcacheMem RAM_3(
	.clock(clock),

	.address_a(addr_1),
	.address_b(addr_2),

	.byteena_a(byte_en_1),
	.byteena_b(byte_en_2),

	.data_a(write_data_1),
	.data_b(write_data_2),

	.wren_a(write_1),
	.wren_b(write_2),

	.q_a(read_data_1_3),
	.q_b(read_data_2_3)
);

//RAM_4 between port1 port3
dcacheMem RAM_4(
	.clock(clock),

	.address_a(addr_1),
	.address_b(addr_3),

	.byteena_a(byte_en_1),
	.byteena_b(byte_en_3),

	.data_a(write_data_1),
	.data_b(write_data_3),

	.wren_a(write_1),
	.wren_b(write_3),

	.q_a(read_data_1_4),
	.q_b(read_data_3_4)
);

//RAM_5 between port2 port3
dcacheMem RAM_5(
	.clock(clock),

	.address_a(addr_2),
	.address_b(addr_3),

	.byteena_a(byte_en_2),
	.byteena_b(byte_en_3),

	.data_a(write_data_2),
	.data_b(write_data_3),

	.wren_a(write_2),
	.wren_b(write_3),

	.q_a(read_data_2_5),
	.q_b(read_data_3_5)
);

MUX_WORD_3to1_0 read_data_MUX_0(
    .selector(RAM_select_0),

    .input_0(read_data_0_0),
    .input_1(read_data_0_1),
    .input_2(read_data_0_2),

    .output_0(read_data_0)
);


MUX_WORD_3to1_1 read_data_MUX_1(
    .selector(RAM_select_1),

    .input_0(read_data_1_0),
    .input_1(read_data_1_3),
    .input_2(read_data_1_4),

    .output_0(read_data_1)
);

MUX_WORD_3to1_2 read_data_MUX_2(
    .selector(RAM_select_2),

    .input_0(read_data_2_1),
    .input_1(read_data_2_3),
    .input_2(read_data_2_5),

    .output_0(read_data_2)
);

MUX_WORD_3to1_3 read_data_MUX_3(
    .selector(RAM_select_3),

    .input_0(read_data_3_2),
    .input_1(read_data_3_4),
    .input_2(read_data_3_5),

    .output_0(read_data_3)
);


endmodule
