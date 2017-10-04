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

`define DBURSTCOUNTWIDTH 10
`define IBURSTCOUNTWIDTH 10

//other definitions
`define BYTE 8
`define BYTEEN_WIDTH `SDRAM_WIDTH/`BYTE

`endif
