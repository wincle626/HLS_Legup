/*
  This work by Simon Moore and Gregory Chadwick is licenced under the
  Creative Commons Attribution-Non-Commercial-Share Alike 2.0
  UK: England & Wales License.

  To view a copy of this licence, visit:
     http://creativecommons.org/licenses/by-nc-sa/2.0/uk/
  or send a letter to:
     Creative Commons,
     171 Second Street,
     Suite 300,
     San Francisco,
     California 94105,
     USA.
*/
/*
   Cache.v - Toplevel for direct mapped cache for use with the Tiger MIPS
   processor.
   
   The cache is organised into blocks, each block contains several words,
   A memory address is partitioned into a tag which is stored in the cache,
   a cache address which is used to locate a particular block in the cache,
   and a block word which is used to select a word from a block in the cache.
   
   [         tag            | cache address | block word | 0 0 ]
   
   The number of blocks in the cache and number of words per block
   is parameterised.  2^blockSize is the block size in bytes,
   2^cacheSize is the number of blocks in the cache, blockSizeBits
   must give the block size in bits (so blockSizeBits = 2^blockSize * 8).
   
   When modifying blockSize there are two bits of code that must be altered.
   The assignment of memReadDataWord, and the case statement where fetchWord
   is written in the fetch state.
   
   When we lookup an address in the cache we use the cache address portion
   of it to address the internal memory block to retrieve the cache entry,
   we then compare the tag in the cache to the tag of the address, if
   they match and the valid bit is set we have a cache hit, otherwise we
   fetch the entire block from memory (read block number bytes from memory
   starting from the address formed by the tag and cache address with 0s for
   all lower bits).
   
   On a write we immediately start writing the data to memory (write through
   cache behaviour) we also lookup the address in the cache, if we have a hit
   we write the data to the cache as well, otherwise we don't bother.
   
   If the high bit of the address is set the cache is bypassed and data is
   directly read from and written to the avalon bus.
   
   Written by Greg Chadwick, Summer 2007 
*/

//`include "tiger_dcache_parameters.v"
`define NUM_CACHE_PORTS 1
`define NUM_CACHE_WAYS 1

module tiger_dcache (
	input csi_clockreset_clk,
	input csi_clockreset_reset_n,

	//Avalon Bus side signals

	//Avalon-ST interface to send and receive data from tiger_top
	output wire [7:0] aso_PROC_data,

	input avs_CACHE0_begintransfer,
	input avs_CACHE0_read,
	input avs_CACHE0_write,
	input [127:0]avs_CACHE0_writedata,
	output wire [127:0]avs_CACHE0_readdata,
	output wire avs_CACHE0_waitrequest,

	`ifdef ACCELERATOR_PORT
	input avs_CACHE1_begintransfer,
	input avs_CACHE1_read,
	input avs_CACHE1_write,
	input [127:0]avs_CACHE1_writedata,
	output wire [127:0]avs_CACHE1_readdata,
	output wire avs_CACHE1_waitrequest,
	
	//Master inferface to talk to SDRAM for accelerators
	output avm_dataMaster1_read,
	output avm_dataMaster1_write,
	output [31:0]avm_dataMaster1_address,
	output [31:0]avm_dataMaster1_writedata,
	output [3:0]avm_dataMaster1_byteenable,
	input [31:0]avm_dataMaster1_readdata,
	output avm_dataMaster1_beginbursttransfer,
	output [2:0]avm_dataMaster1_burstcount,	
	input avm_dataMaster1_waitrequest,
	input avm_dataMaster1_readdatavalid,
	`endif

	//Master interface to talk to SDRAM for processor
	output avm_dataMaster0_read,
	output avm_dataMaster0_write,
	output [31:0]avm_dataMaster0_address,
	output [31:0]avm_dataMaster0_writedata,
	output [3:0]avm_dataMaster0_byteenable,
	input [31:0]avm_dataMaster0_readdata,
	output avm_dataMaster0_beginbursttransfer,
	output [2:0]avm_dataMaster0_burstcount,	
	input avm_dataMaster0_waitrequest,
	input avm_dataMaster0_readdatavalid

);

	localparam NumPorts = `NUM_CACHE_PORTS;
    localparam ways = `NUM_CACHE_WAYS;


	localparam stateIDLE = 3'b000;
	localparam stateREAD = 3'b001;
	localparam stateFETCH = 3'b010;
	localparam stateWRITE = 3'b011;
	localparam stateFLUSH = 3'b100;
	localparam stateHOLD = 3'b101;
	//localparam stateAVALON_READ = 4;
	//localparam stateAVALON_WRITE = 5;

	
	localparam blockSize = 4;
	localparam blockSizeBits = 128;
	localparam cacheSize = 9;
//	localparam tagSizeBits = 32 - cacheSize - blockSize;
	localparam tagSizeBits = 31;
//	localparam burstCount = (2**blockSize)/32; //number of burst to main memory	
	localparam burstCount = 3'd4; //number of burst to main memory	

	wire cacheHit [NumPorts - 1 : 0];
	wire [cacheSize - 1 : 0]cacheAddress [NumPorts - 1 : 0];
	wire [tagSizeBits - 1 : 0]tag [NumPorts - 1 : 0];
	wire [1 : 0]bytes [NumPorts - 1 : 0]; //lower 2 bits of address
	
	reg [31:0]memReadDataWord [NumPorts - 1 : 0];
	
	wire cacheWrite [NumPorts - 1 : 0];
	wire cacheClkEn;
	wire [blockSizeBits + tagSizeBits : 0]cacheData [NumPorts - 1 : 0];
	wire [blockSizeBits + tagSizeBits : 0]cacheQ [NumPorts - 1 : 0];
	
	wire [tagSizeBits - 1 : 0]cacheTag [NumPorts - 1 : 0]; //this is the current tag that will compared against to see if cache hit
	wire [30:0] cache_tagData [NumPorts - 1 : 0]; //this is the tag data that will be written into cache
	wire validBit [NumPorts - 1 : 0];
	wire [blockSizeBits - 1 : 0]cacheQData [NumPorts - 1 : 0];
	
	wire [blockSizeBits - 1 : 0]cacheWriteData [NumPorts - 1 : 0];
	reg [blockSizeBits - 1 : 0]cacheWriteData_128 [NumPorts - 1 : 0];

	reg [31 : 0]writeData [NumPorts - 1 : 0];
	
	wire [tagSizeBits - 1 : 0]savedTag [NumPorts - 1 : 0];
	wire [blockSize - 3 : 0]savedBlockWord [NumPorts - 1 : 0];
	wire [1 : 0]savedByte [NumPorts - 1 : 0];
	
	wire fetchDone [NumPorts - 1 : 0];
	
	//wire bypassCache [NumPorts - 1 : 0]; //should we bypass the cache for the current read/write operation?
		
	reg [31:0]address [NumPorts - 1 : 0];
	reg [31:0]writeDataWord [NumPorts - 1 : 0];
	
	reg [blockSizeBits - 33 : 0]fetchData [NumPorts - 1 : 0];
	reg [blockSize - 3 : 0]fetchWord [NumPorts - 1 : 0];
	
	reg [2:0]state [NumPorts - 1 : 0];
	reg [1:0]state_64 [NumPorts - 1 : 0];
	//reg [2:0]state_accel;
	
	reg savedMem16 [NumPorts - 1 : 0];
	reg savedMem8 [NumPorts - 1 : 0];
	
	reg [cacheSize - 1 : 0]flushAddr [NumPorts - 1 : 0];
		
//////////////new cache signals///////////////
	wire clk;
	wire reset_n;

	wire memRead [NumPorts - 1 : 0];
	wire memWrite [NumPorts - 1 : 0];
	reg memWrite_reg [NumPorts - 1 : 0];
	wire [31:0]memAddress [NumPorts - 1 : 0];
	wire [31:0]memWriteData [NumPorts - 1 : 0];
	reg [31:0]memReadData [NumPorts - 1 : 0];
	reg [31:0] memReadData_lo [NumPorts - 1 : 0];
	wire flush [NumPorts - 1 : 0];

	wire mem8 [NumPorts - 1 : 0];
	wire mem16 [NumPorts - 1 : 0];
	wire mem64 [NumPorts - 1 : 0];

	wire canRead [NumPorts - 1 : 0];
	wire canWrite [NumPorts - 1 : 0];
	wire canFlush [NumPorts - 1 : 0];
	wire stall [NumPorts - 1 : 0];
/////////for accelerator ////////////////
	reg stall_cpu_for_accel_reg [NumPorts - 1 : 0];
	wire stall_cpu_nodly [NumPorts - 1 : 0];
	wire stall_cpu [NumPorts - 1 : 0];
	wire stall_cpu_from_accel [NumPorts - 1 : 0];
	wire unstall_cpu_from_accel [NumPorts - 1 : 0];
//////////////////////////////////////////////////
////////for generate signals//////////////////////
	wire CACHE_begintransfer [NumPorts - 1 : 0];
	wire CACHE_read [NumPorts - 1 : 0];
	wire CACHE_write [NumPorts - 1 : 0];
	wire [127:0]CACHE_writedata [NumPorts - 1 : 0];
	reg [127:0]CACHE_readdata [NumPorts - 1 : 0];
	wire CACHE_waitrequest [NumPorts - 1 : 0];

	reg dataMaster_read [NumPorts - 1 : 0];
	reg dataMaster_write [NumPorts - 1 : 0];
	reg [31:0] dataMaster_address [NumPorts - 1 : 0];
	reg [31:0] dataMaster_writedata [NumPorts - 1 : 0];
	reg [3:0] dataMaster_byteenable [NumPorts - 1 : 0];
	wire [31:0] dataMaster_readdata [NumPorts - 1 : 0];
	reg dataMaster_beginbursttransfer [NumPorts - 1 : 0];
	reg [2:0] dataMaster_burstcount [NumPorts - 1 : 0];	
	wire dataMaster_waitrequest [NumPorts - 1 : 0];
	wire dataMaster_readdatavalid [NumPorts - 1 : 0];

	reg [3:0] cache_word_Byteenable [NumPorts - 1 : 0];
	wire [19:0] cache_Byteenable [NumPorts - 1 : 0];
	reg [15:0] cache_Byteenable_32 [NumPorts - 1 : 0];
	reg [7:0] memReadData_8bits [NumPorts - 1 : 0];
	reg [15:0] memReadData_16bits [NumPorts - 1 : 0];

	reg [31:0] dataMaster_writedata_32 [NumPorts - 1 : 0];
	reg	[3:0] dataMaster_byteenable_32 [NumPorts - 1 : 0];
	wire isProc;
//////////////////////////////////////////////////	
	assign clk = csi_clockreset_clk;
	assign reset_n = csi_clockreset_reset_n;

	assign CACHE_begintransfer[0] =  avs_CACHE0_begintransfer;
	assign CACHE_read[0] = avs_CACHE0_read;
	assign CACHE_write[0] = avs_CACHE0_write;
	assign CACHE_writedata[0] = avs_CACHE0_writedata;
	assign avs_CACHE0_readdata = CACHE_readdata[0];
	`ifdef ACCELERATOR_PORT
		assign avs_CACHE0_waitrequest = CACHE_waitrequest[0] && !isProc;
	`else
		assign avs_CACHE0_waitrequest = 1'b0;
	`endif

	assign avm_dataMaster0_read = dataMaster_read[0];
	assign avm_dataMaster0_write = dataMaster_write[0];
	assign avm_dataMaster0_address = dataMaster_address[0];
	assign avm_dataMaster0_writedata = dataMaster_writedata[0];
	assign avm_dataMaster0_byteenable = dataMaster_byteenable[0];
	assign avm_dataMaster0_beginbursttransfer = dataMaster_beginbursttransfer[0];
	assign avm_dataMaster0_burstcount = dataMaster_burstcount[0];
	assign dataMaster_readdata[0] = avm_dataMaster0_readdata;
	assign dataMaster_waitrequest[0] = avm_dataMaster0_waitrequest;
	assign dataMaster_readdatavalid[0] = avm_dataMaster0_readdatavalid;

	`ifdef ACCELERATOR_PORT
		assign CACHE_begintransfer[1] =  avs_CACHE1_begintransfer;
		assign CACHE_read[1] = avs_CACHE1_read;
		assign CACHE_write[1] = avs_CACHE1_write;
		assign CACHE_writedata[1] = avs_CACHE1_writedata;
		assign avs_CACHE1_readdata = CACHE_readdata[1];
		assign avs_CACHE1_waitrequest = CACHE_waitrequest[1];

		assign avm_dataMaster1_read = dataMaster_read[1];
		assign avm_dataMaster1_write = dataMaster_write[1];
		assign avm_dataMaster1_address = dataMaster_address[1];
		assign avm_dataMaster1_writedata = dataMaster_writedata[1];
		assign avm_dataMaster1_byteenable = dataMaster_byteenable[1];
		assign avm_dataMaster1_beginbursttransfer = dataMaster_beginbursttransfer[1];
		assign avm_dataMaster1_burstcount = dataMaster_burstcount[1];
		assign dataMaster_readdata[1] = avm_dataMaster1_readdata;
		assign dataMaster_waitrequest[1] = avm_dataMaster1_waitrequest;
		assign dataMaster_readdatavalid[1] = avm_dataMaster1_readdatavalid;
	`endif

	`ifdef ACCELERATOR_PORT
		assign isProc = avs_CACHE0_writedata[103];
		assign aso_PROC_data[0] = stall_cpu[0] || stall_cpu[1];
		assign aso_PROC_data[1] = canRead[0] && isProc;
		assign aso_PROC_data[2] = canWrite[0] && isProc;
		assign aso_PROC_data[3] = canFlush[0] && isProc;
		assign aso_PROC_data[4] = stall[0] && isProc;
		assign aso_PROC_data[5] = cacheHit[0] && isProc;
	`else
		assign aso_PROC_data[0] = stall_cpu[0];
		assign aso_PROC_data[1] = canRead[0];
		assign aso_PROC_data[2] = canWrite[0];
		assign aso_PROC_data[3] = canFlush[0];
		assign aso_PROC_data[4] = stall[0];
		assign aso_PROC_data[5] = cacheHit[0];
	`endif

	assign cacheClkEn = 1'b1;
	assign aso_PROC_data[7:6] = 0;

	wire cacheAddress_match;
	wire stallWrite_for_fetch [NumPorts - 1 : 0];
	wire stallRead [NumPorts - 1 : 0];
	
	
	`ifdef ACCELERATOR_PORT
		assign cacheAddress_match = (^cacheAddress[0] === 1'bX || ^cacheAddress[1] === 1'bX || (cacheAddress[0] != cacheAddress[1]))? 1'b0: 1'b1;
		assign stallWrite_for_fetch[0] = cacheAddress_match && (state[1] == stateFETCH || state[1] == stateHOLD) && memWrite[0]; 
		assign stallWrite_for_fetch[1] = cacheAddress_match && (state[0] == stateFETCH || state[0] == stateHOLD) && memWrite[1]; 
		assign stallRead[0] = memRead[0] && (fetchDone[1] && cacheAddress_match);
		assign stallRead[1] = memRead[1] && (fetchDone[0] && cacheAddress_match); 
	`else
		assign stallWrite_for_fetch[0] = 1'b0;
		assign stallRead[0] = 1'b0; 
	`endif

	`ifdef ACCELERATOR_PORT
		tiger_dcache_memory dcacheMemIns(
							.address_a(cacheAddress[0]),
							.address_b(cacheAddress[1]),
							.byteena_a(cache_Byteenable[0]),
							.byteena_b(cache_Byteenable[1]),
							.clock(clk),
							.data_a(cacheData[0]),
							.data_b(cacheData[1]),
							.enable(cacheClkEn),
							.wren_a(cacheWrite[0]),
							.wren_b(cacheWrite[1]),
							.q_a(cacheQ[0]),
							.q_b(cacheQ[1])
							);		
	`else
		tiger_dcache_memory dcacheMemIns(
							.address_a(cacheAddress[0]),
							.address_b(),
							.byteena_a(cache_Byteenable[0]),
							.byteena_b(),
							.clock(clk),
							.data_a(cacheData[0]),
							.data_b(),
							.enable(cacheClkEn),
							.wren_a(cacheWrite[0]),
							.wren_b(),
							.q_a(cacheQ[0]),
							.q_b()
							);		
	`endif
	/////////////////////////////////////////////////////////////
    genvar i;
    generate
    for (i = 0 ; i < NumPorts ; i = i + 1)
    begin : cache_signals


		assign memRead[i] = CACHE_read[i];
		assign memWrite[i] =  (!stall_cpu_from_accel[i] && !unstall_cpu_from_accel[i])? CACHE_write[i] : 1'b0;
		assign memAddress[i] = (state_64[i] == 2)? CACHE_writedata[i][31:0] + 4 : CACHE_writedata[i][31:0];
		//assign memAddress[i] = CACHE_write[i]? (state_64[i] == 2)? CACHE_writedata[i][31:0] + 4 : CACHE_writedata[i][31:0] : 32'd0;
		assign memWriteData[i] = (state_64[i] == 2)? CACHE_writedata[i][95:64] : CACHE_writedata[i][63:32];

		assign mem8[i] = CACHE_writedata[i][96];
		assign mem16[i] = CACHE_writedata[i][97];
		assign mem64[i] = CACHE_writedata[i][98];
		assign flush[i] = CACHE_writedata[i][99];
		assign stall_cpu_from_accel[i] = CACHE_writedata[i][100];
		assign unstall_cpu_from_accel[i] = CACHE_writedata[i][101];

		always @(*)
		begin
			case(mem64[i])
				0: CACHE_readdata[i] = {96'd0, memReadData[i]};
				1: CACHE_readdata[i] = {64'd0, memReadData[i],memReadData_lo[i]};
				default: CACHE_readdata[i] = 128'dx;
			endcase
		end

		assign CACHE_waitrequest[i] = state[i] == stateFETCH //while fetching from off-chip
							|| state[i] == stateHOLD //before going into fetch state
							|| (memRead[i] && CACHE_begintransfer[i]) //when accel first accesses cache
							|| (memRead[i] && !cacheHit[i]) || (state[i] != stateREAD && memRead[i]) //if on a read, it's not a cache hit, and it's not in stateREAD
							|| (memWrite[i] && state[i] != stateWRITE) //if on a write, it's not in stateWRITE
							|| dataMaster_waitrequest[i] //if waitrequest is asserted from off-chip memory
							//|| stallWrite_for_fetch[i]
							|| state_64[i] == 1 || state_64[i] == 2; //for 64-bit accesses

		////////PROCESSOR ONLY SIGNALS////////
		//assign bypassCache[i] = memAddress[i][31]; //If high bit of address is set we bypass the cache

		assign stall[i] = state[i] == stateFETCH 
	//		|| (memRead && avs_CACHE0_begintransfer)
			|| (state[i] == stateFLUSH)
			|| (flush[i] && !canFlush[i])
			|| (state[i] == stateREAD && !cacheHit[i])
		//	|| (state[i] == stateAVALON_READ && !dataMaster_readdatavalid[i])
			|| (memWrite[i] && state[i] != stateIDLE) || (state[i] != stateIDLE && state[i] != stateREAD && memRead[i])
			|| stallWrite_for_fetch[i]
			|| (state[i] == stateHOLD);



		//We can start a read in the idle state or the read state if we've had a cache hit
		assign canRead[i] = state[i] == stateIDLE || (state[i] == stateREAD && cacheHit[i]);
		//We can start a write in the idle state
		assign canWrite[i] = state[i] == stateIDLE;
		//We can start a flush in the idle state
		assign canFlush[i] = state[i] == stateIDLE || (state[i] == stateREAD && cacheHit[i]);
		////////PROCESSOR ONLY SIGNALS////////


		assign tag[i] = memAddress[i][31 : cacheSize + blockSize];
		assign bytes[i] = memAddress[i][1 : 0];
		//assign word[i] = memAddress[i][4 : 2];
		assign savedTag[i] = address[i][31 : cacheSize + blockSize];
		assign savedBlockWord[i] = address[i][blockSize  - 1 : 2];
		assign savedByte[i] = address[i][1 : 0];


	    assign cacheTag[i] = cacheQ[i][tagSizeBits : 1];
		assign validBit[i] = (^cacheQ[i][0] === 1'bX || cacheQ[i][0] == 0) ? 1'b0 : 1'b1; 	//to take care of undefined case

		assign cacheQData[i] = cacheQ[i][blockSizeBits + tagSizeBits : tagSizeBits + 1];

		assign cacheWrite[i] = fetchDone[i] || (state[i] == stateWRITE && cacheHit[i]);

//		assign cacheWrite[i] = fetchDone[i] || (state[i] == stateWRITE && cacheHit[i] && memWrite_reg[i]) || state[i] == stateFLUSH;

		
		always @(posedge clk)
		begin
			memWrite_reg[i] <= memWrite[i];
		end

		//If we're in the fetch state, the data is valid and we've fetched
		//all but the last word we've done the fetch
		assign fetchDone[i] = (state[i] == stateFETCH && dataMaster_readdatavalid[i] && fetchWord[i] == burstCount - 1);

		assign cacheHit[i] = validBit[i] && savedTag[i] == cacheTag[i];

		//cpu stalling logic for when stalling is enabled instead of polling
		assign stall_cpu[i] = stall_cpu_for_accel_reg[i] || stall_cpu_nodly[i];


		//cpu stalling logic for when stalling is enabled instead of polling
		always @(posedge clk)
		begin
			if (!reset_n)
				stall_cpu_for_accel_reg[i] <= 0;
			else if (CACHE_write[i] && stall_cpu_from_accel[i])
				stall_cpu_for_accel_reg[i] <= 1'b1;
			else if (CACHE_write[i] && unstall_cpu_from_accel[i])
				stall_cpu_for_accel_reg[i] <= 1'b0;
		end

		//for when processor is stalled during the execution of accelerator,
		//when the accelerator begins execution it will send a stall signal on stall_cpu_from_accel
		assign stall_cpu_nodly[i] = (CACHE_write[i] == 1'b1)? stall_cpu_from_accel[i]: 1'b0;
	
		//state machine for controlling 64 bit operations
		//need to do two consecutive 32 bit reads or writes
		always @(posedge clk, negedge reset_n)
		begin
			if(!reset_n)
			begin
				memReadData_lo[i] <= 0;
			end
			else if (state_64[i] == 1)
			begin
				memReadData_lo[i] <= memReadDataWord[i];
			end
		end

		always @(posedge clk, negedge reset_n)
		begin
			if(!reset_n)
			begin
				state_64[i] <= 0;
			end
			else if (mem64[i])
			begin
				case(state_64[i])
				0 : begin
						//if first write or read
						if (memWrite[i] || memRead[i])
						begin
							state_64[i] <= 1;
						end
					end
				1 : begin
						if (memRead[i])//if read
						begin
							if (cacheHit[i])
							begin
								state_64[i] <= 2;
							end
							else if (!dataMaster_waitrequest[i])
							begin
								//if done first write
								state_64[i] <= 2;
							end
						end
						else	//if write
						begin
							if (!dataMaster_waitrequest[i])
							begin
								//if done first write
								state_64[i] <= 2;
							end
						end
					end
				2 : begin
						//assert second address and data
						//for extra state in between two consecutive operations
						state_64[i] <= 3;
					end
				3 : begin
						//if second write or read
						if (memRead[i])//if read
						begin
							if (cacheHit[i])
							begin
								state_64[i] <= 0;
							end
							else if (!dataMaster_waitrequest[i])
							begin
								//if done first write
								state_64[i] <= 0;
							end
						end
						else	//if write
						begin
							if (!dataMaster_waitrequest[i])
							begin
								//if done first write
								state_64[i] <= 0;
							end
						end
					end
				endcase
			end
		end

		assign cacheAddress[i] = (fetchDone[i] || state[i] == stateWRITE)? address[i][cacheSize + blockSize - 1 : blockSize] : memAddress[i][cacheSize + blockSize - 1 : blockSize];

		//this is needed to extend data to 31 bits
		assign cache_tagData[i] = dataMaster_address[i][31 : cacheSize + blockSize];


		assign cacheData[i] = 
			(state[i] == stateWRITE) ? 
				{cacheWriteData[i], 31'd0, 1'b1} :
				{dataMaster_readdata[i], fetchData[i], cache_tagData[i], 1'b1};


		//If we're reading or writing 8 or 16 bits rather than a full 32-bit word, we need to only write
		//the bits we want to write in the word and keep the rest as they were
		always @(*)
		begin
			if (savedMem8[i])
			begin
				case (savedByte[i])
					2'b00: begin
						   writeData[i] <= {24'd0, writeDataWord[i][7 : 0]};
			  			   cache_word_Byteenable[i] <= 4'b0001;
						   end
					2'b01: begin
						   writeData[i] <= {16'd0, writeDataWord[i][7 : 0], 8'd0};
			  			   cache_word_Byteenable[i] <= 4'b0010;	
						   end
					2'b10: begin
						   writeData[i] <= {8'd0, writeDataWord[i][7 : 0], 16'd0};
			  			   cache_word_Byteenable[i] <= 4'b0100;	
						   end
					2'b11: begin
						   writeData[i] <= {writeDataWord[i][7 : 0], 24'b0};
			  			   cache_word_Byteenable[i] <= 4'b1000;		
						   end
					default : begin
						   writeData[i] <= 32'bx;
			  			   cache_word_Byteenable[i] <= 4'dx;		
						   end
				endcase
			end
			else if (savedMem16[i])
			begin
				case (savedByte[i][1])
					1'b0: begin
						  writeData[i] <= {16'd0, writeDataWord[i][15 : 0]};
						  cache_word_Byteenable[i] <= 4'b0011;
						  end
					1'b1: begin
						  writeData[i] <= {writeDataWord[i][15 : 0], 16'd0};
						  cache_word_Byteenable[i] <= 4'b1100;
						  end
				default : begin
						  writeData[i] <= 32'bx;
	 	    			  cache_word_Byteenable[i] <= 4'dx;	
						  end
				endcase
			end
			else 
			begin
				writeData[i] <= writeDataWord[i];
	 		    cache_word_Byteenable[i] <= 4'b1111;	
			end
		end

		//When writing data to the cache we need to overwrite only the word we are writing and
		//preserve the rest
		always @(*)
		begin
			case (savedBlockWord[i][1:0])
				3'd0: begin
						cache_Byteenable_32[i] <= {12'd0, cache_word_Byteenable[i]};
					  end
				3'd1: begin
						cache_Byteenable_32[i] <= {8'd0, cache_word_Byteenable[i], 4'd0};
					  end
				3'd2: begin
						cache_Byteenable_32[i] <= {4'd0, cache_word_Byteenable[i], 8'd0};
					  end
				3'd3: begin
						cache_Byteenable_32[i] <= {cache_word_Byteenable[i], 12'd0};
					  end
				default: begin
						 cache_Byteenable_32[i] <= 16'dx;
					  end
			endcase
		end

		//For cache_Byteenable, first 4 bits are always reserved for tagbits, hence when writing into cache, first 4 bits are always 0
		//Tagbits only need to be written, on fetch from main memory, when entire cache line is written
		assign cache_Byteenable[i] = fetchDone[i]? 20'b1111_1111_1111_1111_1111 : {cache_Byteenable_32[i], 4'd0};

		//replicate writeDataWord to fill up the cache line, however use byte enable signals to only write to bits under interest
		assign cacheWriteData[i] = {writeData[i], writeData[i], writeData[i], writeData[i]};

		//Multiplexer to select which word of the cache block we want
		//if we're reading from the avalon bus, bypass cache and give
		//data read directly from avalon
		always @(*)
		begin
			case(savedBlockWord[i])
				3'd0 : memReadDataWord[i] = cacheQData[i][31 : 0];
				3'd1 : memReadDataWord[i] = cacheQData[i][63 : 32];
				3'd2 : memReadDataWord[i] = cacheQData[i][95 : 64];
				3'd3 : memReadDataWord[i] = cacheQData[i][127 : 96];
				default : memReadDataWord[i] = 32'bxxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx;
			endcase
		end

		//If mem8 or mem16 are asserted we only want part of the read word,
		//if neither are asserted we want the entire word
		always @(*)
		begin
			case (savedByte[i])
				2'b00: memReadData_8bits[i] = memReadDataWord[i][7 : 0];
				2'b01: memReadData_8bits[i] = memReadDataWord[i][15 : 8];
				2'b10: memReadData_8bits[i] = memReadDataWord[i][23 : 16];
				2'b11: memReadData_8bits[i] = memReadDataWord[i][31 : 24];
				default: memReadData_8bits[i] = 8'dX;
			endcase
		end

		always @(*)
		begin
			case (savedByte[i][1])
				1'b0: memReadData_16bits[i] = memReadDataWord[i][15 : 0];
				1'b1: memReadData_16bits[i] = memReadDataWord[i][31 : 16];
				default: memReadData_16bits[i] = 16'dX;
			endcase
		end

		//MAY HAVE TO CHANGE THIS??? TO SEQUENTIAL
		//always @(posedge clk)
		always @(*)
		begin
			if (savedMem8[i])
				memReadData[i] <= {24'd0, memReadData_8bits[i]};
			else if (savedMem16[i])
				memReadData[i] <= {16'd0, memReadData_16bits[i]};
			else
				memReadData[i] <= memReadDataWord[i];
		end

		always @(*)
		begin
				dataMaster_writedata_32[i] <= memWriteData[i];
			if (mem8[i])
			begin
				case (bytes[i])
					2'b00: begin
//						   dataMaster_writedata_32[i] <= {24'd0, memWriteData[i][7 : 0]};
			  			   dataMaster_byteenable_32[i] <= 4'b0001;
						   end
					2'b01: begin
//						   dataMaster_writedata_32[i] <= {16'd0, memWriteData[i][7 : 0], 8'd0};
			  			   dataMaster_byteenable_32[i] <= 4'b0010;	
						   end
					2'b10: begin
//						   dataMaster_writedata_32[i] <= {8'd0, memWriteData[i][7 : 0], 16'd0};
			  			   dataMaster_byteenable_32[i] <= 4'b0100;	
						   end
					2'b11: begin
//						   dataMaster_writedata_32[i] <= {memWriteData[i][7 : 0], 24'b0};
			  			   dataMaster_byteenable_32[i] <= 4'b1000;		
						   end
					default : begin
//						   dataMaster_writedata_32[i] <= 32'dx;
			  			   dataMaster_byteenable_32[i] <= 4'bxxxx;		
						   end
				endcase
			end
			else if (mem16[i])
			begin
				case (bytes[i][1])
					1'b0: begin
//						  dataMaster_writedata_32[i] <= {16'd0, memWriteData[i][15 : 0]};
						  dataMaster_byteenable_32[i] <= 4'b0011;
						  end
					1'b1: begin
//						  dataMaster_writedata_32[i] <= {memWriteData[i][15 : 0], 16'd0};
						  dataMaster_byteenable_32[i] <= 4'b1100;
						  end
				default : begin
//						  dataMaster_writedata_32[i] <= 32'dx;
	 	    			  dataMaster_byteenable_32[i] <= 4'bxxxx;	
						  end
				endcase
			end
			else 
			begin
//				dataMaster_writedata_32[i] <= memWriteData[i];
	 		    dataMaster_byteenable_32[i] <= 4'b1111;	
			end
		end


		//state machine for cache controller
		always @(posedge clk, negedge reset_n) begin
			if(!reset_n) begin
				state[i] <= stateIDLE;
				dataMaster_read[i] <= 0;
				dataMaster_write[i] <= 0;
				address[i] <= 0;
				dataMaster_burstcount[i] <= 0;
				dataMaster_beginbursttransfer[i] <= 0;
			end else begin
				case(state[i])
					stateIDLE: begin
						dataMaster_burstcount[i] <= 1;
						dataMaster_beginbursttransfer[i] <= 0;
						fetchWord[i] <= 0;
						if(memRead[i] && !stallRead[i]) begin //If we want a read start a read
							state[i] <= stateREAD;
							dataMaster_address[i] <= {tag[i], cacheAddress[i], {blockSize{1'b0}}};
							dataMaster_byteenable[i] <= 4'b1111;
					
							address[i] <= memAddress[i];
							savedMem8[i] <= mem8[i];
							savedMem16[i] <= mem16[i];
						end else if(memWrite[i] && !stallWrite_for_fetch[i]) begin //If we want a write start a write
						//end else if(memWrite[i]) begin //If we want a write start a write
							state[i] <= stateWRITE;
							address[i] <= memAddress[i];
							writeDataWord[i] <= memWriteData[i];
				
							savedMem8[i] <= mem8[i];
							savedMem16[i] <= mem16[i];
					
							dataMaster_write[i] <= 1;
							dataMaster_writedata[i] <= dataMaster_writedata_32[i];						
							dataMaster_address[i] <= {memAddress[i][31:2], 2'b0};
						end else if(flush[i]) begin
							state[i] <= stateFLUSH;
							flushAddr[i] <= 0;
						end
				
						//If we're reading and bypassing the cache
						//or performing a write we may need to set
						//bytes enable to something other than
						//reading/writing the entire word
						//if((memRead[i] && bypassCache[i]) || memWrite[i]) begin
						if(memWrite[i]) begin
							dataMaster_byteenable[i] <= dataMaster_byteenable_32[i];
						end
					end
					stateREAD: begin
						if (stallRead[i]) begin
							state[i] <= stateIDLE;
						end else begin
							dataMaster_burstcount[i] <= 1'b1;
							//If we've had a cache hit either go back to idle
							//or if we want another read continue in the read state
							//or if we want to flush go to the flush state
							if(cacheHit[i]) begin 
								if(flush[i]) begin
									state[i] <= stateFLUSH;
									flushAddr[i] <= 0;
								//if write is asserted (this is for when write happens right after read)
								end else if (memWrite[i] && !stallWrite_for_fetch[i]) begin
								//end else if(memWrite[i]) begin //If we want a write start a write
									state[i] <= stateWRITE;
									address[i] <= memAddress[i];
									writeDataWord[i] <= memWriteData[i][31:0];						
									savedMem8[i] <= mem8[i];
									savedMem16[i] <= mem16[i];
									dataMaster_write[i] <= 1;
									dataMaster_writedata[i] <= dataMaster_writedata_32[i];					
									dataMaster_address[i] <= {memAddress[i][31:2], 2'b0};
						
									dataMaster_byteenable[i] <= dataMaster_byteenable_32[i];
								//if read is deasserted
								end else if(!memRead[i]) begin
									state[i] <= stateIDLE; //go back to IDLE state
								//if read is still asserted
								end else begin
									state[i] <= stateREAD; 
									dataMaster_address[i] <= {tag[i], cacheAddress[i], {blockSize{1'b0}}};
									dataMaster_byteenable[i] <= 4'b1111;

									savedMem8[i] <= mem8[i];
									savedMem16[i] <= mem16[i];
						
									address[i] <= memAddress[i];
								end
							//if cache miss
							end else begin //fetch data from off-chip memory
								state[i] <= stateHOLD;
								dataMaster_read[i] <= 1;
								dataMaster_burstcount[i] <= burstCount;
								dataMaster_beginbursttransfer[i] <= 1;
							end
						end						
					end
					stateFETCH: begin
						//If wait request is low we can give another address to read from
	/*					if(!dataMaster_waitrequest) begin
							//If we've given address for all the blocks we want, stop reading
							if(dataMaster_address[blockSize - 1 : 0] == {{(blockSize - 2){1'b1}}, 2'b0})
								dataMaster_read <= 0;
							else //Otherwise give address of the next block
								dataMaster_address <= dataMaster_address + 4;
						end
		*/				
						//If we have valid data
						if(dataMaster_readdatavalid[i]) begin
							//store it in the fetchData register if it's not the last word
							//(the last word is fed straight into the data register of the memory
							// block)
							case(fetchWord[i])
								2'b00:
									fetchData[i][31:0] <= dataMaster_readdata[i];
								2'b01:
									fetchData[i][63:32] <= dataMaster_readdata[i];
								2'b10:
									fetchData[i][95:64] <= dataMaster_readdata[i];
							endcase
					
							fetchWord[i] <= fetchWord[i] + 1'b1;
							//If this is the last word go back to the read state
	//						if(fetchWord == blockSize - 1)
	//							state <= stateREAD;
							if(fetchWord[i] == burstCount - 1) begin
									state[i] <= stateREAD;
							end
						end
					end
					stateHOLD: begin //extra state to begin fetch before it goes into stateFETCH
						dataMaster_beginbursttransfer[i] <= 0;
						if(!dataMaster_waitrequest[i]) begin
							dataMaster_read[i] <= 0;
							state[i] <= stateFETCH;
						end
					end
					stateWRITE: begin	
						//Once the memory write has completed either go back to idle
						//and stop writing or continue with another write
						if(!dataMaster_waitrequest[i]) begin
							dataMaster_write[i] <= 0;
							state[i] <= stateIDLE;
						end
					end
					stateFLUSH: begin
						flushAddr[i] <= flushAddr[i] + 1'b1;
						if(flushAddr[i] == {cacheSize{1'b1}})
							state[i] <= stateIDLE;
					end
				endcase
			end
		end 		




      
	end
    endgenerate

endmodule


