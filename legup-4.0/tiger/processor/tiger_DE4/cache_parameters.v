`ifndef CACHE_PARAMETERS_H
`define CACHE_PARAMETERS_H

//defines the number of ports to the cache
//1 when only the processor, 2 when accelerator present
`define NUM_DCACHE_PORTS 1
`define NUM_ICACHE_PORTS 1

//this is defined if an accelerator is present
//`define ACCELERATOR_PORT

//defines the associativity of the cache
`define DWAYS 1
`define IWAYS 1

//define FPGA Board
`define DE4

//external memory width (256 = DDR2 on DE4, 32 = SDRAM on DE2)
`ifdef DE2
	`define SDRAM_WIDTH 32
	`define BURSTCOUNT_DIV 4 //(32 = DDR2 on DE4, 4 = SDRAM on DE2)
	//define the cache size
	`define DCACHE_SIZE 9
	`define DBLOCKSIZE 4
	`define ICACHE_SIZE 9
	`define IBLOCKSIZE 4
`elsif DE4
	`define SDRAM_WIDTH 256
	`define BURSTCOUNT_DIV 32 //(32 = DDR2 on DE4, 4 = SDRAM on DE2)
	//define the cache size
	`define DCACHE_SIZE 9
	`define DBLOCKSIZE 7
	`define ICACHE_SIZE 9
	`define IBLOCKSIZE 7
`endif

`define DBURSTCOUNTWIDTH 6
`define IBURSTCOUNTWIDTH 6

//other definitions
`define BYTE 8
`define BYTEEN_WIDTH `SDRAM_WIDTH/`BYTE

`define MP_cacheSize		`DCACHE_SIZE //log2(number of lines in cache)
`define MP_cacheDepth		(2 ** `DCACHE_SIZE) //number of lines in cache  
`define MP_blockSize  `DBLOCKSIZE
`define MP_blockSizeBits	(8*(2**`MP_blockSize)) //total data bits per cache line
`define MP_cacheWidth		(`MP_blockSizeBits + `MP_tagSizeBits + 1) //total bits per cache line

`define WORD_WIDTH       `MP_cacheWidth
`define WORD             [`WORD_WIDTH-1:0]

`define MEM_ADDR_WIDTH   `MP_cacheSize
`define MEM_ADDR         [`MEM_ADDR_WIDTH-1:0]

`define MEM_DEPTH        `MP_cacheDepth
`define MEM              [`MEM_DEPTH-1:0]

`define MP_tagSizeBits		31
`define HIGH 1'b1
`define LOW  1'b0

// Used for write enables
`define READ  `LOW
`define WRITE `HIGH

// Multipumping phases
`define PHASE_WIDTH     1
`define PHASE           [`PHASE_WIDTH-1:0]

`define PHASE_0 `PHASE_WIDTH'd0
`define PHASE_1 `PHASE_WIDTH'd1

`define BYTE_EN [((2**`MP_blockSize)+4)-1:0]

// Table pointing to which bank holds live register value
`define LVT_ENTRY_WIDTH   2
`define LVT_ENTRY         [`LVT_ENTRY_WIDTH-1:0]

// One entry for each register
`define LVT_DEPTH         (1 << `MEM_ADDR_WIDTH)

// This changes with number of ports in memory
`define ACCEL_0 `LVT_ENTRY_WIDTH'd0
`define ACCEL_1 `LVT_ENTRY_WIDTH'd1
`define ACCEL_2 `LVT_ENTRY_WIDTH'd2
`define ACCEL_3 `LVT_ENTRY_WIDTH'd3

`define HIGH 1'b1
`define LOW  1'b0

// Used for various write enables
`define READ  `LOW
`define WRITE `HIGH

`endif
