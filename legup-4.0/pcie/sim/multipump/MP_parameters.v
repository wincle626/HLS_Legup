`define MEM_ADDR_WIDTH 20
`define MEM_ADDR [`MEM_ADDR_WIDTH-1:0]
`define WORD_WIDTH 64
`define WORD [`WORD_WIDTH-1:0]
`define BYTE_EN_WIDTH (`WORD_WIDTH/8)
`define BYTE_EN [`BYTE_EN_WIDTH-1:0]

`define PHASE_WIDTH 1
`define PHASE [`PHASE_WIDTH-1:0]

`define PHASE_0 `PHASE_WIDTH'd0
`define PHASE_1 `PHASE_WIDTH'd1

`define HIGH 1'b1
`define LOW 1'b0
