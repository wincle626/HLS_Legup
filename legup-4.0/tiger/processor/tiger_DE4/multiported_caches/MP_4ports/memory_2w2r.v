`include "cache_parameters.v"

module memory_2w2r(
    input  wire           clock,      
    input  wire           write_en_0,
    input  wire           write_en_1,
	input  wire `BYTE_EN  byte_en_0,
	input  wire `BYTE_EN  byte_en_1,
    input  wire `MEM_ADDR addr_0,
    input  wire `MEM_ADDR addr_1,
    input  wire `WORD     write_data_0,
    input  wire `WORD     write_data_1,
    output wire `WORD     read_data_0,
    output wire `WORD     read_data_1
); 

dcacheMem dcacheMem_inst(
	.address_a(addr_0),
	.address_b(addr_1),
	.byteena_a(byte_en_0),
	.byteena_b(byte_en_1),
	.clock(clock),
	.data_a(write_data_0),
	.data_b(write_data_1),
	.wren_a(write_en_0),
	.wren_b(write_en_1),
	.q_a(read_data_0),
	.q_b(read_data_1));

endmodule

