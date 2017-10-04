`include "MP_parameters.v"

module MP_2ports(
    input  wire             base_clock, // system clock
    input  wire             clock,      // multiple of system clock
    input  wire             reset,

    // port 1 input, at base_clock
    input  wire  `MEM_ADDR  addr_0,
    input  wire             write_en_0,
    input  wire             read_en_0,
    input  wire  `BYTE_EN   byte_en_0,
    input  wire  `WORD      write_data_0,
    output wire  `WORD      read_data_0,
    output wire             waitrequest_0,

    // port 2 input, at base_clock
    input  wire  `MEM_ADDR  addr_1,
    input  wire             write_en_1,
    input  wire             read_en_1,
    input  wire  `BYTE_EN   byte_en_1,
    input  wire  `WORD      write_data_1,
    output wire  `WORD      read_data_1,
    output wire             waitrequest_1,

    // MP output at clock
    output wire  `MEM_ADDR  avm_addr,
    output wire             avm_write_en,
    output wire             avm_read_en,
    output wire  `BYTE_EN   avm_byte_en,
    output wire  `WORD      avm_write_data,
    input  wire  `WORD      avm_read_data,
    input  wire             avm_waitrequest
);

wire `PHASE       phase;

// assign wait request signals
assign waitrequest_0 = avm_waitrequest & (write_en_0 | read_en_0);
assign waitrequest_1 = avm_waitrequest & (write_en_1 | read_en_1);

multipumping_controller_2x multipumping_controller_2x(
    .base_clock(base_clock),
    .clock(clock),
    .phase(phase)
);

wire `MEM_ADDR addr_delayed;
wire           write_en_delayed;
wire           read_en_delayed;
wire `BYTE_EN  byte_en_delayed;
wire `WORD     write_data_delayed;

input_delay #(
    .BUS_WIDTH(`MEM_ADDR_WIDTH + 1 + 1 + `BYTE_EN_WIDTH + `WORD_WIDTH)
) input_delay_inst(
    .clock(clock),
    .enable(~phase),

    .input_bus({addr_1, write_en_1, read_en_1, byte_en_1, write_data_1}),
    .output_bus({addr_delayed, write_en_delayed, read_en_delayed, byte_en_delayed, write_data_delayed})
);

assign {avm_addr, avm_write_en, avm_read_en, avm_byte_en, avm_write_data} = (phase == `PHASE_0) ? {addr_0, write_en_0, read_en_0, byte_en_0, write_data_0} : {addr_delayed, write_en_delayed, read_en_delayed, byte_en_delayed, write_data_delayed};

wire `WORD     readdata_delayed;

input_delay #(
    .BUS_WIDTH(`WORD_WIDTH)
) input_delay_read_data_inst(
    .clock(clock),
    .enable(phase),

    .input_bus(avm_read_data),
    .output_bus(readdata_delayed)
);

assign read_data_0 = readdata_delayed;
assign read_data_1 = avm_read_data;

endmodule
