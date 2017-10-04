`include "cache_parameters.v"

module data_cache (
	input csi_clockreset_clk,
	input csi_clockreset_reset_n,

	`ifdef MP
	//2X clock for multi-pumped memory
	input csi_clockreset2X_clk,
	input csi_clockreset2X_reset_n,
	`endif
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
	output [`SDRAM_WIDTH-1:0]avm_dataMaster1_writedata,
	output [`BYTEEN_WIDTH-1:0]avm_dataMaster1_byteenable,
	input [`SDRAM_WIDTH-1:0]avm_dataMaster1_readdata,
	output avm_dataMaster1_beginbursttransfer,
	output [`DBURSTCOUNTWIDTH-1:0]avm_dataMaster1_burstcount,	
	input avm_dataMaster1_waitrequest,
	input avm_dataMaster1_readdatavalid,
	
	`ifdef FOUR_PORTS
	input avs_CACHE2_begintransfer,
	input avs_CACHE2_read,
	input avs_CACHE2_write,
	input [127:0]avs_CACHE2_writedata,
	output wire [127:0]avs_CACHE2_readdata,
	output wire avs_CACHE2_waitrequest,
	
	//Master inferface to talk to SDRAM for accelerators
	output avm_dataMaster2_read,
	output avm_dataMaster2_write,
	output [31:0]avm_dataMaster2_address,
	output [`SDRAM_WIDTH-1:0]avm_dataMaster2_writedata,
	output [`BYTEEN_WIDTH-1:0]avm_dataMaster2_byteenable,
	input [`SDRAM_WIDTH-1:0]avm_dataMaster2_readdata,
	output avm_dataMaster2_beginbursttransfer,
	output [`DBURSTCOUNTWIDTH-1:0]avm_dataMaster2_burstcount,	
	input avm_dataMaster2_waitrequest,
	input avm_dataMaster2_readdatavalid,

	input avs_CACHE3_begintransfer,
	input avs_CACHE3_read,
	input avs_CACHE3_write,
	input [127:0]avs_CACHE3_writedata,
	output wire [127:0]avs_CACHE3_readdata,
	output wire avs_CACHE3_waitrequest,
	
	//Master inferface to talk to SDRAM for accelerators
	output avm_dataMaster3_read,
	output avm_dataMaster3_write,
	output [31:0]avm_dataMaster3_address,
	output [`SDRAM_WIDTH-1:0]avm_dataMaster3_writedata,
	output [`BYTEEN_WIDTH-1:0]avm_dataMaster3_byteenable,
	input [`SDRAM_WIDTH-1:0]avm_dataMaster3_readdata,
	output avm_dataMaster3_beginbursttransfer,
	output [`DBURSTCOUNTWIDTH-1:0]avm_dataMaster3_burstcount,	
	input avm_dataMaster3_waitrequest,
	input avm_dataMaster3_readdatavalid,
	`endif
	`endif

	//Master interface to talk to SDRAM for processor
	output avm_dataMaster0_read,
	output avm_dataMaster0_write,
	output [31:0]avm_dataMaster0_address,
	output [`SDRAM_WIDTH-1:0]avm_dataMaster0_writedata,
	output [`BYTEEN_WIDTH-1:0]avm_dataMaster0_byteenable,
	input [`SDRAM_WIDTH-1:0]avm_dataMaster0_readdata,
	output avm_dataMaster0_beginbursttransfer,
	output [`DBURSTCOUNTWIDTH-1:0]avm_dataMaster0_burstcount,	
	input avm_dataMaster0_waitrequest,
	input avm_dataMaster0_readdatavalid

);

	//define cache parameters
	localparam NumPorts = `NUM_DCACHE_PORTS;	
	localparam cacheSize = `DCACHE_SIZE; //number of lines in cache 2^9 = 512 lines
	localparam ways = `DWAYS;
    localparam blockSize = `DBLOCKSIZE;

	localparam blockSizeBits = 8*(2**blockSize); //total bits per line
/*	`ifdef LVT
	localparam tagSizeBits = 32 - cacheSize - blockSize; //if not using byteenable, 
	`else*/
	localparam tagSizeBits = 31; //FIXING THIS TO 31 IN ORDER FOR THE RAM TO BE BYTE ADDRESSABLE
//	`endif
	//if not using byteenable, localparam tagSizeBits = 32 - cacheSize - blockSize;
	localparam burstCount = (2**blockSize)/`BURSTCOUNT_DIV; //number of burst to main memory
	localparam wordWidth = 32; //bits in a word
	localparam cachelineBytes = (2**blockSize)+4;
	localparam sdramWidth = `SDRAM_WIDTH;
	localparam byteEnWidth = `BYTEEN_WIDTH;

	//define states
	localparam stateIDLE = 3'b000;
	localparam stateREAD = 3'b001;
	localparam stateFETCH = 3'b010;
	localparam stateWRITE = 3'b011;
	//localparam stateFLUSH = 3'b100;
	//localparam stateHOLD = 3'b101;

	wire cacheHit [NumPorts - 1 : 0];
	reg [ways - 1 : 0] cacheHit_ways [NumPorts - 1 : 0];
	wire [cacheSize - 1 : 0]cacheAddress [NumPorts - 1 : 0];
	wire [cacheSize - 1 : 0] cacheAddress_forSet [NumPorts - 1 : 0];
	wire fetch_one_left [NumPorts - 1 : 0];
	wire [tagSizeBits - 1 : 0]tag [NumPorts - 1 : 0];
	wire [1 : 0]bytes [NumPorts - 1 : 0]; //lower 2 bits of address
	wire [2 : 0]words [NumPorts - 1 : 0]; //upper 3 bits of address, selects which 32 bits to use out of 256 bits	

	wire [wordWidth-1:0]memReadDataWord [NumPorts - 1 : 0];
	
	wire [ways - 1 : 0] cacheWrite [NumPorts - 1 : 0];
	wire cacheWrite_ports [NumPorts - 1 : 0]; //indicates if write signal is asserted (for all sets) for this port
	wire cacheClkEn;
	wire [blockSizeBits + tagSizeBits : 0]cacheData [NumPorts - 1 : 0];
	wire [blockSizeBits + tagSizeBits : 0]cacheQ [NumPorts - 1 : 0][ways - 1 : 0];
	
	wire [tagSizeBits - 1 : 0]cacheTag [NumPorts - 1 : 0][ways - 1 : 0]; //this is the current tag that will compared against to see if cache hit
	wire validBit [NumPorts - 1 : 0][ways - 1 : 0];
	wire [blockSizeBits - 1 : 0]cacheQData [NumPorts - 1 : 0];
	
	wire [blockSizeBits - 1 : 0]cacheWriteData [NumPorts - 1 : 0];
	reg [blockSizeBits - 1 : 0]cacheWriteData_128 [NumPorts - 1 : 0];

	reg [wordWidth-1 : 0]writeData [NumPorts - 1 : 0];
	
	wire [tagSizeBits - 1 : 0]savedTag [NumPorts - 1 : 0];
	wire [blockSize - 3 : 0]savedBlockWord [NumPorts - 1 : 0];
	wire [1 : 0]savedByte [NumPorts - 1 : 0];
	
	wire fetchDone [NumPorts - 1 : 0];
	
	//wire bypassCache [NumPorts - 1 : 0]; //should we bypass the cache for the current read/write operation?
		
	reg [31:0]address [NumPorts - 1 : 0];
	reg [wordWidth-1:0]writeDataWord [NumPorts - 1 : 0];
	
//	reg [blockSizeBits - 33 : 0]fetchData [NumPorts - 1 : 0];
	reg [blockSizeBits - (sdramWidth+1) : 0]fetchData [NumPorts - 1 : 0]; // 257 = size of avm_dataMaster_readdata + 1
	reg [`DBURSTCOUNTWIDTH - 1 : 0]fetchWord [NumPorts - 1 : 0];
	
	reg [2:0]state [NumPorts - 1 : 0];
	reg [1:0]state_64 [NumPorts - 1 : 0];
	//reg [2:0]state_accel;
	
	reg savedMem16 [NumPorts - 1 : 0];
	reg savedMem8 [NumPorts - 1 : 0];
	
	//reg [cacheSize - 1 : 0]flushAddr [NumPorts - 1 : 0];
		
//////////////new cache signals///////////////
	wire clk;

	`ifdef MP
	wire clk_double;
	`endif

	wire reset_n;

	wire memRead [NumPorts - 1 : 0];
	wire memWrite [NumPorts - 1 : 0];
	reg memWrite_reg [NumPorts - 1 : 0];
	wire [31:0]memAddress [NumPorts - 1 : 0];
	wire [wordWidth-1:0]memWriteData [NumPorts - 1 : 0];
	reg [wordWidth-1:0]memReadData [NumPorts - 1 : 0];
	reg [wordWidth-1:0] memReadData_lo [NumPorts - 1 : 0];
	reg [7:0] memReadData_8bits [NumPorts - 1 : 0];
	reg [15:0] memReadData_16bits [NumPorts - 1 : 0];
	//wire flush [NumPorts - 1 : 0];

	wire mem8 [NumPorts - 1 : 0];
	wire mem16 [NumPorts - 1 : 0];
	wire mem64 [NumPorts - 1 : 0];

/*
	wire canRead [NumPorts - 1 : 0];
	wire canWrite [NumPorts - 1 : 0];
	wire canFlush [NumPorts - 1 : 0];
	wire stall [NumPorts - 1 : 0];*/
	wire canRead;
	wire canWrite;
	//wire canFlush;
	wire stall;
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
	reg [sdramWidth-1:0] dataMaster_writedata [NumPorts - 1 : 0];
	reg [byteEnWidth-1:0] dataMaster_byteenable [NumPorts - 1 : 0];
	wire [sdramWidth-1:0] dataMaster_readdata [NumPorts - 1 : 0];
	reg dataMaster_beginbursttransfer [NumPorts - 1 : 0];
	reg [`DBURSTCOUNTWIDTH-1:0] dataMaster_burstcount [NumPorts - 1 : 0];	
	wire dataMaster_waitrequest [NumPorts - 1 : 0];
	wire dataMaster_readdatavalid [NumPorts - 1 : 0];

	//Byte enable used for when writing into RAM
	//first 4 bits are reserved for Tagbits
	//there's 1 bit for each byte	
	wire [cachelineBytes-1:0] cache_Byteenable [NumPorts - 1 : 0];
	reg [3:0] cache_word_Byteenable [NumPorts - 1 : 0];
	wire [30:0] cache_tagData [NumPorts - 1 : 0]; //this is the tag data that will be written into cache


	reg [wordWidth-1:0] dataMaster_writedata_32 [NumPorts - 1 : 0];
	reg	[3:0] dataMaster_byteenable_32 [NumPorts - 1 : 0];
	wire isProc;

//	wire [log2(sum(NumPorts-1)) : 0] cacheAddress_match;
//	wire [5 : 0] cacheAddress_match;
	wire [NumPorts-1 : 0] cacheAddress_match [NumPorts-1 : 0];
	wire stallWrite_for_fetch [NumPorts - 1 : 0];
	wire stallFetch_for_write [NumPorts - 1 : 0];
	wire stallRead [NumPorts - 1 : 0];

	// Registers to hold the First In line of each set (to facilitate FIFO policy)
	reg [log2(ways)-1 : 0] writetoSet [2**cacheSize - 1 : 0];
	reg [log2(ways)-1 : 0] writetoSet_reg [NumPorts - 1 : 0];
	reg [log2(ways)-1 : 0] cacheHitWasIn [NumPorts - 1 : 0];

	function integer log2;
		input [wordWidth-1:0] value;
		integer i;
		begin
		log2 = 0;
		for(i = 0; 2**i < value; i = i + 1)
			log2 = i + 1;
		end
	endfunction

	function integer sum;
		input [wordWidth-1:0] value;
		integer i;
		begin
		sum = 0;
		for(i = value; i >= 1; i = i - 1)
			sum = sum + i;
		end
	endfunction

    genvar i, j, p;
	integer k, l;
//////////////////////////////////////////////////	
	assign clk = csi_clockreset_clk;
	assign reset_n = csi_clockreset_reset_n;

	assign CACHE_begintransfer[0] =  avs_CACHE0_begintransfer;
	assign CACHE_read[0] = avs_CACHE0_read;
	assign CACHE_write[0] = avs_CACHE0_write;
	assign CACHE_writedata[0] = avs_CACHE0_writedata;
	assign avs_CACHE0_readdata = CACHE_readdata[0];

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

	`ifdef ACCELERATOR_PORT //can't use generate if's, since it requires all signals inside to be defined (avm_dataMaster1_xxx)
		assign avs_CACHE0_waitrequest = CACHE_waitrequest[0] && !isProc;

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
		
		assign isProc = avs_CACHE0_writedata[103];
		assign aso_PROC_data[0] = stall_cpu[0] || stall_cpu[1];
		assign aso_PROC_data[1] = canRead && isProc;
		assign aso_PROC_data[2] = canWrite && isProc;
		//assign aso_PROC_data[3] = canFlush && isProc;
		assign aso_PROC_data[3] = 1'b0;
		assign aso_PROC_data[4] = stall && isProc;
		assign aso_PROC_data[5] = cacheHit[0] && isProc;

		//first dimension is down to 1 since bitwidth should be one less than number of ports (shows a relationship of one port to another port)
		wire [NumPorts - 1 : 1] stallReadCondition [NumPorts - 1 : 0];
		wire [NumPorts - 1 : 1]	stallWrite_for_fetchCondition [NumPorts - 1 : 0];
		wire [NumPorts - 1 : 1]	stallFetch_for_writeCondition [NumPorts - 1 : 0];
			
		generate
			for (i=0; i<NumPorts; i=i+1) begin : iteratePortA
				for (j=0; j<NumPorts; j=j+1) begin : iteratePortB
					if (i == j) begin
						assign cacheAddress_match[i][j] = 1'b1;				
					end else begin					
						assign cacheAddress_match[i][j] = (cacheAddress[i] == cacheAddress[j]);				
					end
				end

				for (j=1; j<NumPorts; j=j+1) begin : stallConditionGen
					//This will stall when two ports want to read and write to/from the same cache line, in the exact same cycle. The reading port will be delayed by one cycle, since it will take 1 cycle for the write to complete. If you read/write at the exact same cycle, the reading port will return undefined data. We use this signal as an if statement condition to go back to stateIDLE. It needs to go back to stateIDLE to update other signals such as "address"
					`ifndef LVT
					assign stallReadCondition[i][j] = fetchDone[(i+j)%NumPorts] && cacheAddress_match[i][(i+j)%NumPorts];
					`endif

					//If another port is ALREADY fetching from the same cache address, then we need to wait until that fetch is done, before writing. If you write, while the other port fetching, the cache will first hold new data (because it's a write-through cache, the write happens both to cache and off-chip memory at the same time). However, when the fetch finishes, it returns returns old data (since fetch was initiated before the write), and the cache line gets overwritten with the old data. Condition is: it they map to the same cache address, and the other port is already in stateHOLD/FETCH (started fetching)
					assign stallWrite_for_fetchCondition[i][j] = cacheAddress_match[i][(i+j)%NumPorts] && state[(i+j)%NumPorts] == stateFETCH;

					//This is when fetch and write occur to the same address at the same time. If one port wants to write, the fetching port needs to wait until the writing port is done, or else it will fetch old data. 
					//check the condition to go into stateWRITE, if this occurs, and both ports map to the same cache address, don't go into stateHOLD/FETCH
					//Condition is: first part, if they map to the same cacheaddress (Since that is how much of data it will be returned from DDR2); second part, the conditions for the other port to go into stateWRITE have been met, which means it will go into stateWRITE (write to main memory over avalon will occur next cycle).
					//this signal will be used as a condition not to go into FETCH state for the first port 
					assign stallFetch_for_writeCondition[i][j] = cacheAddress_match[i][(i+j)%NumPorts] && (memWrite[(i+j)%NumPorts] && !stallWrite_for_fetch[(i+j)%NumPorts]);
				end

				`ifndef LVT
				assign stallRead[i] = memRead[i]&&|stallReadCondition[i];
				`endif
				assign stallWrite_for_fetch[i] = |stallWrite_for_fetchCondition[i];
				assign stallFetch_for_write[i] = |stallFetch_for_writeCondition[i];

			end
		endgenerate

		`ifdef TWO_PORTS //if there are 2 ports in cache
			//use both ports of RAM
			generate 
				for (i = 0; i < ways; i = i+1) begin : loop_instantiateRAM
					dcacheMem dcacheMemIns(
						.address_a(cacheAddress[0]),
						.address_b(cacheAddress[1]),
						.byteena_a(cache_Byteenable[0]),
						.byteena_b(cache_Byteenable[1]),
						.clock(clk),
						.data_a(cacheData[0]),
						.data_b(cacheData[1]),
						//.enable(cacheClkEn),
						.wren_a(cacheWrite[0][i]),
						.wren_b(cacheWrite[1][i]),
						.q_a(cacheQ[0][i]),
						.q_b(cacheQ[1][i])
					);	
				end
			endgenerate
		`elsif FOUR_PORTS //if there are 4 ports in cache
			assign CACHE_begintransfer[2] =  avs_CACHE2_begintransfer;
			assign CACHE_read[2] = avs_CACHE2_read;
			assign CACHE_write[2] = avs_CACHE2_write;
			assign CACHE_writedata[2] = avs_CACHE2_writedata;
			assign avs_CACHE2_readdata = CACHE_readdata[2];
			assign avs_CACHE2_waitrequest = CACHE_waitrequest[2];

			assign avm_dataMaster2_read = dataMaster_read[2];
			assign avm_dataMaster2_write = dataMaster_write[2];
			assign avm_dataMaster2_address = dataMaster_address[2];
			assign avm_dataMaster2_writedata = dataMaster_writedata[2];
			assign avm_dataMaster2_byteenable = dataMaster_byteenable[2];
			assign avm_dataMaster2_beginbursttransfer = dataMaster_beginbursttransfer[2];
			assign avm_dataMaster2_burstcount = dataMaster_burstcount[2];
			assign dataMaster_readdata[2] = avm_dataMaster2_readdata;
			assign dataMaster_waitrequest[2] = avm_dataMaster2_waitrequest;
			assign dataMaster_readdatavalid[2] = avm_dataMaster2_readdatavalid;

			assign CACHE_begintransfer[3] = avs_CACHE3_begintransfer;
			assign CACHE_read[3] = avs_CACHE3_read;
			assign CACHE_write[3] = avs_CACHE3_write;
			assign CACHE_writedata[3] = avs_CACHE3_writedata;
			assign avs_CACHE3_readdata = CACHE_readdata[3];
			assign avs_CACHE3_waitrequest = CACHE_waitrequest[3];

			assign avm_dataMaster3_read = dataMaster_read[3];
			assign avm_dataMaster3_write = dataMaster_write[3];
			assign avm_dataMaster3_address = dataMaster_address[3];
			assign avm_dataMaster3_writedata = dataMaster_writedata[3];
			assign avm_dataMaster3_byteenable = dataMaster_byteenable[3];
			assign avm_dataMaster3_beginbursttransfer = dataMaster_beginbursttransfer[3];
			assign avm_dataMaster3_burstcount = dataMaster_burstcount[3];
			assign dataMaster_readdata[3] = avm_dataMaster3_readdata;
			assign dataMaster_waitrequest[3] = avm_dataMaster3_waitrequest;
			assign dataMaster_readdatavalid[3] = avm_dataMaster3_readdatavalid;

			//if using 4-port MP cache
			`ifdef MP
				assign clk_double = csi_clockreset2X_clk;
				generate 
					for (i = 0; i < ways; i = i+1) begin : loop_instantiateRAM
						MP_4ports dcacheMemIns(
							.base_clock(clk), // system clock
							.clock(clk_double),      // multiple of system clock

							.addr_0(cacheAddress[0]),   //proc
							.write_en_0(cacheWrite[0][i]),
							.byte_en_0(cache_Byteenable[0]),
							.write_data_0(cacheData[0]),
							.read_data_0(cacheQ[0][i]),

							.addr_1(cacheAddress[1]),   //accel1   
							.write_en_1(cacheWrite[1][i]),
							.byte_en_1(cache_Byteenable[1]),
							.write_data_1(cacheData[1]),
							.read_data_1(cacheQ[1][i]),

							.addr_2(cacheAddress[2]),   //accel2
							.write_en_2(cacheWrite[2][i]),
							.byte_en_2(cache_Byteenable[2]),
							.write_data_2(cacheData[2]),
							.read_data_2(cacheQ[2][i]),

							.addr_3(cacheAddress[3]),   //accel3
							.write_en_3(cacheWrite[3][i]),
							.byte_en_3(cache_Byteenable[3]),
							.write_data_3(cacheData[3]),
							.read_data_3(cacheQ[3][i])
						);
					end
				endgenerate
			//if using 4-port LVT cache
			`elsif LVT

				wire [NumPorts-1 : 0] cacheAddress_match_reg [NumPorts-1 : 0];
				reg [cacheSize-1 : 0] cacheAddress_reg [NumPorts-1 : 0];
				wire stallWrite [NumPorts-1 : 0];
				wire stallWrite_final [NumPorts-1 : 0];
				reg stallWrite_asserted [NumPorts-1 : 0];
				wire [ways-1 : 0] cacheWrite_arbiter_gnt [NumPorts-1 : 0];
				reg [ways-1 : 0] cacheWrite_arbiter_gnt_reg [NumPorts-1 : 0];
				wire cacheWrite_arbiter_gnt_ports [NumPorts-1 : 0];
				wire [ways-1 : 0] stallWrite_arbiter [NumPorts-1 : 0];
				wire stallWrite_arbiter_ports [NumPorts-1 : 0];		
				wire [ways-1 : 0] twoWrites_sameAddress_sameCycle [NumPorts-1 : 0];
				wire twoWrites_sameAddress_sameCycle_ports [NumPorts-1 : 0];
				reg stallRead_reg [NumPorts-1 : 0];

				//first dimension is down to 1 since bitwidth should be one less than number of ports (shows a relationship of one port to another port)
				wire [NumPorts-1 : 1] stallWriteCondition [NumPorts-1 : 0];
				wire [NumPorts-1 : 1] stallWriteArbiterCondition [NumPorts-1 : 0][ways-1 : 0];
				wire [NumPorts-1 : 1] sameAddressSameCycleCondition [NumPorts-1 : 0][ways-1 : 0];
	
				generate
					for (i=0; i<NumPorts; i=i+1) begin : iteratePortALVT
						for (j=0; j<NumPorts; j=j+1) begin : iteratePortBLVT
							assign cacheAddress_match_reg[i][j] = (cacheAddress[i] == cacheAddress_reg[j]);
						end

						for (j=1; j<NumPorts; j=j+1) begin : stallConditionGenLVT
						    assign stallReadCondition[i][j] = (cacheWrite_arbiter_gnt_ports[(i+j)%NumPorts] && cacheAddress_match[i][(i+j)%NumPorts]);
							//This is used as a condition to stay in stateWRITE, since write had to be stalled, and one of the conditions for the waitrequest to stall accelerator execution. 
							//Finally, take the registered stallWrite_asserted but deassert it right away as soon as cacheWrite_accel_arbiter_gnt is asserted. 
							//This signal makes sure that if an accelerator wants to write but is stalled because another port is writing, then this accelerator will stay stalled until the write can go through
							//However, if it was stalled due to another port that was writing a fetch, then the whole line will be overwritten anyways so the write does not need to happen. 
							assign stallWriteCondition[i][j] = !(fetchDone[(i+j)%NumPorts] && cacheAddress_match[i][(i+j)%NumPorts]);
						end

						assign stallRead[i] = memRead[i] && |stallReadCondition[i];
						assign stallWrite_final[i] = stallWrite_asserted[i] && !cacheWrite_arbiter_gnt_ports[i] && &stallWriteCondition[i];

/*						assign stallWrite_final[i] = stallWrite_asserted[i] && !cacheWrite_arbiter_gnt_ports[i] && 
													!(fetchDone[(i+1)%NumPorts] && cacheAddress_match[i][(i+1)%NumPorts]) && 
													!(fetchDone[(i+2)%NumPorts] && cacheAddress_match[i][(i+2)%NumPorts]) && 
													!(fetchDone[(i+3)%NumPorts] && cacheAddress_match[i][(i+3)%NumPorts]);

						assign stallRead[i] = memRead[i] && ((cacheWrite_arbiter_gnt_ports[(i+1)%NumPorts] && cacheAddress_match[i][(i+1)%NumPorts]) || 
															 (cacheWrite_arbiter_gnt_ports[(i+2)%NumPorts] && cacheAddress_match[i][(i+2)%NumPorts]) || 
															 (cacheWrite_arbiter_gnt_ports[(i+3)%NumPorts] && cacheAddress_match[i][(i+3)%NumPorts]));

*/

						for (p=0; p < ways; p=p+1) begin : iterateCacheWayLVT
							for (j=1; j<NumPorts; j=j+1) begin : stallConditionGenLVT2
								//This is used as a condition to arbitrate in the arbiter: We need to arbitrate for a given set, for 2 different ports, if they WANT to write to the same cache address and the same set, or if one wrote to the same cache address and same set in the previous cycle and the other wants to write to the same cache address in the current cycle
								//This signal will be for individual sets
								assign stallWriteArbiterCondition[i][p][j] = (cacheWrite_arbiter_gnt_reg[(i+j)%NumPorts][p] && cacheAddress_match_reg[i][(i+j)%NumPorts]) || (cacheWrite[(i+j)%NumPorts][p] && cacheAddress_match[i][(i+j)%NumPorts]);

								//twoWrites_sameAddress_sameCycle will be asserted if another port is writing to the same address and the same set
								assign sameAddressSameCycleCondition[i][p][j] = (cacheWrite_arbiter_gnt[(i+j)%NumPorts][p] && cacheAddress_match[i][(i+j)%NumPorts]);
							end
		
							assign stallWrite_arbiter[i][p]	= cacheWrite[i][p] && |stallWriteArbiterCondition[i][p];
							assign twoWrites_sameAddress_sameCycle[i][p] = cacheWrite[i][p] && |sameAddressSameCycleCondition[i][p];
/*
							//This is used as a condition to arbitrate in the arbiter: We need to arbitrate for a given set, for 2 different ports, if they WANT to write to the same cache address and the same set, or if one wrote to the same cache address and same set in the previous cycle and the other wants to write to the same cache address in the current cycle
							//This signal will be for individual sets
							assign stallWrite_arbiter[i][p] = cacheWrite[i][p] && ((cacheWrite_arbiter_gnt_reg[(i+1)%NumPorts][p] && cacheAddress_match_reg[i][(i+1)%NumPorts]) 
																				|| (cacheWrite_arbiter_gnt_reg[(i+2)%NumPorts][p] && cacheAddress_match_reg[i][(i+2)%NumPorts])
																				|| (cacheWrite_arbiter_gnt_reg[(i+3)%NumPorts][p] && cacheAddress_match_reg[i][(i+3)%NumPorts])  
																				|| (cacheWrite[(i+1)%NumPorts][p] && cacheAddress_match[i][(i+1)%NumPorts]) 
																				|| (cacheWrite[(i+2)%NumPorts][p] && cacheAddress_match[i][(i+2)%NumPorts])
																				|| (cacheWrite[(i+3)%NumPorts][p] && cacheAddress_match[i][(i+3)%NumPorts])); 

							//twoWrites_sameAddress_sameCycle will be asserted if another port is writing to the same address and the same set
							assign twoWrites_sameAddress_sameCycle[i][p] = (cacheWrite[i][p] && ((cacheWrite_arbiter_gnt[(i+1)%NumPorts][p] && cacheAddress_match[i][(i+1)%NumPorts]) 
																							  || (cacheWrite_arbiter_gnt[(i+2)%NumPorts][p] && cacheAddress_match[i][(i+2)%NumPorts])
																							  || (cacheWrite_arbiter_gnt[(i+3)%NumPorts][p] && cacheAddress_match[i][(i+3)%NumPorts])));
*/
						end

						assign cacheWrite_arbiter_gnt_ports[i] = |cacheWrite_arbiter_gnt[i];
						assign stallWrite_arbiter_ports[i] = |stallWrite_arbiter[i];
						assign twoWrites_sameAddress_sameCycle_ports[i] = |twoWrites_sameAddress_sameCycle[i];

						always @(posedge clk)
						begin
							cacheAddress_reg[i] <= cacheAddress[i];
							stallRead_reg [i] <= stallRead [i];
							for (k = 0; k < ways; k=k+1) begin
								cacheWrite_arbiter_gnt_reg[i][k] <= cacheWrite_arbiter_gnt[i][k];
							end
						end

						//assign cacheWrite_arbiter_gnt_ports[i] = |cacheWrite_arbiter_gnt[i][ways-1];
						//assign stallWrite_arbiter_ports[i] = |stallWrite_arbiter[i][ways-1];
						//assign twoWrites_sameAddress_sameCycle_ports[i] = |twoWrites_sameAddress_sameCycle[i][ways-1];

						//This signal is used as a condition to stay in stateWRITE and also as a waitrequest condition to stall accel: We need to stall if any of the stallWrite_arbiter_accel signal is asserted for this port, or in this exact cycle if another port is writing to the same cache address and same cache set as the one that I want to write to. 
						//This signal will be for all sets
						assign stallWrite[i] = stallWrite_arbiter_ports[i] || twoWrites_sameAddress_sameCycle_ports[i];

						//This signal is used as one of the conditions for the waitrequest to stall the accelerator
						//It will be asserted when stallWrite_accel for that port gets asserted and keep high until the write has happened for that port. Hence it will stall the accelerator if the write had been stalled until the write goes through. 
						always @(posedge clk)
						begin
							if (!reset_n)
							begin
								stallWrite_asserted[i] <=1'b0;	
							end
							//if this write is stalled
							if (stallWrite[i] && !cacheWrite_arbiter_gnt_ports[i])
							begin //assert 1
								stallWrite_asserted[i] <=1'b1;
							end //if the stalled write has been written
							else if (cacheWrite_arbiter_gnt_ports[i])
							begin //assert 0
								stallWrite_asserted[i] <=1'b0;
							end
						end
					end
				endgenerate

				generate
					for (i = 0; i < ways; i = i+1) begin : loop_instantiateRAMArbiter
						LVT_4ports dcacheMemIns(
							.clock(clk), // system clock

							.addr_0(cacheAddress[0]),   //proc
				 			.write_0(cacheWrite_arbiter_gnt[0][i]),
							.byte_en_0(cache_Byteenable[0]),
						  	.write_data_0(cacheData[0]),
							.read_data_0(cacheQ[0][i]),

							.addr_1(cacheAddress[1]),   //accel1
				 			.write_1(cacheWrite_arbiter_gnt[1][i]),
							.byte_en_1(cache_Byteenable[1]),
					 	   	.write_data_1(cacheData[1]),
							.read_data_1(cacheQ[1][i]),

							.addr_2(cacheAddress[2]),   //accel2
				 			.write_2(cacheWrite_arbiter_gnt[2][i]),
							.byte_en_2(cache_Byteenable[2]),
						   	.write_data_2(cacheData[2]),
							.read_data_2(cacheQ[2][i]),

							.addr_3(cacheAddress[3]),   //accel3
				  			.write_3(cacheWrite_arbiter_gnt[3][i]),
							.byte_en_3(cache_Byteenable[3]),
							.write_data_3(cacheData[3]),
							.read_data_3(cacheQ[3][i])
						);

						arbiter cacheWrite_arbiter (
							.clk(clk),    
							.rst(!reset_n),    

							.req0(cacheWrite[0][i]),
							.address0(cacheAddress[0]),
							.stallWrite0(stallWrite_arbiter[0][i]),
							.fetchWrite0(fetchDone[0]),   
							.gnt0(cacheWrite_arbiter_gnt[0][i]),

							.req1(cacheWrite[1][i]),
							.address1(cacheAddress[1]),
							.stallWrite1(stallWrite_arbiter[1][i]),
							.fetchWrite1(fetchDone[1]),   
							.gnt1(cacheWrite_arbiter_gnt[1][i]),   

							.req2(cacheWrite[2][i]),   
							.address2(cacheAddress[2]),
							.stallWrite2(stallWrite_arbiter[2][i]),
							.fetchWrite2(fetchDone[2]),
							.gnt2(cacheWrite_arbiter_gnt[2][i]),   

							.req3(cacheWrite[3][i]),      
							.address3(cacheAddress[3]),
							.stallWrite3(stallWrite_arbiter[3][i]),
							.fetchWrite3(fetchDone[3]),
							.gnt3(cacheWrite_arbiter_gnt[3][i])
						);
					end
				endgenerate
			`endif
		`endif
	`else
		//if only processor port exists
		//else begin
		assign avs_CACHE0_waitrequest = 1'b0;
	
		assign aso_PROC_data[0] = stall_cpu[0];
		assign aso_PROC_data[1] = canRead;
		assign aso_PROC_data[2] = canWrite;
		//assign aso_PROC_data[3] = canFlush;
		assign aso_PROC_data[3] = 1'b0;
		assign aso_PROC_data[4] = stall;
		assign aso_PROC_data[5] = cacheHit[0];

		assign stallWrite_for_fetch[0] = 1'b0;
		assign stallFetch_for_write[0] = 1'b0;
		assign stallRead[0] = 1'b0; 

		//only use one port of RAM
		generate 
			for (i = 0; i < ways; i = i+1) begin : loop_instantiateRAM
				dcacheMem dcacheMemIns(
									.address_a(cacheAddress[0]),
									.address_b(),
									.byteena_a(cache_Byteenable[0]),
									.byteena_b(),
									.clock(clk),
									.data_a(cacheData[0]),
									.data_b(),
									//.enable(cacheClkEn),
									.wren_a(cacheWrite[0][i]),
									.wren_b(),
									.q_a(cacheQ[0][i]),
									.q_b()
									);		
			end
		endgenerate
	`endif

	assign cacheClkEn = 1'b1;
	assign aso_PROC_data[7:6] = 0;

	//writetoSet keeps track of which set cache should write to on next write
	generate 
		if (ways != 1) begin
			//initialize to 0
			initial begin
			for (k=0; k<2**cacheSize; k=k+1)
				writetoSet[k] = 0;
			end

			//increment by 1 every time
			always @ (posedge clk) begin
				for (k=0; k<NumPorts; k=k+1) begin
					if (fetchDone[k]) begin
						writetoSet[cacheAddress[k]] <= writetoSet[cacheAddress[k]] + 1;
					end
				end
			end
		end
	endgenerate

	////////PROCESSOR ONLY SIGNALS////////
	assign stall = state[0] == stateFETCH 
//		|| (memRead && avs_CACHE0_begintransfer)
		//|| (state[0] == stateFLUSH)
		//|| (flush[0] && !canFlush)
		|| (state[0] == stateREAD && !cacheHit[0])
	//	|| (state[0] == stateAVALON_READ && !dataMaster_readdatavalid[0])
		|| (memWrite[0] && state[0] != stateIDLE) || (state[0] != stateIDLE && state[0] != stateREAD && memRead[0])
		|| (stallWrite_for_fetch[0] && memWrite[0]);
		//|| (state[0] == stateHOLD);

	//We can start a read in the idle state or the read state if we've had a cache hit
	assign canRead = state[0] == stateIDLE || (state[0] == stateREAD && cacheHit[0]);
	//We can start a write in the idle state
	assign canWrite = state[0] == stateIDLE;
	//We can start a flush in the idle state
	//assign canFlush = state[0] == stateIDLE || (state[0] == stateREAD && cacheHit[0]);
	////////PROCESSOR ONLY SIGNALS////////


	//GENVAR I IS USED FOR EACH CACHE PORT
	//INTEGER J IS USED FOR EACH SET (ASSOCIATIVITY)
	/////////////////////////////////////////////////////////////
    generate
    for (i = 0 ; i < NumPorts ; i = i + 1)
    begin : loop_cacheSignals


		assign memRead[i] = CACHE_read[i];
		assign memWrite[i] =  (!stall_cpu_from_accel[i] && !unstall_cpu_from_accel[i])? CACHE_write[i] : 1'b0;
		assign memAddress[i] = (state_64[i] == 2)? CACHE_writedata[i][31:0] + 4 : CACHE_writedata[i][31:0];
		//assign memAddress[i] = CACHE_write[i]? (state_64[i] == 2)? CACHE_writedata[i][31:0] + 4 : CACHE_writedata[i][31:0] : 32'd0;
		assign memWriteData[i] = (state_64[i] == 2)? CACHE_writedata[i][95:64] : CACHE_writedata[i][63:32];

		assign mem8[i] = CACHE_writedata[i][96];
		assign mem16[i] = CACHE_writedata[i][97];
		assign mem64[i] = CACHE_writedata[i][98];
		//assign flush[i] = CACHE_writedata[i][99];
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
							//|| state[i] == stateHOLD //before going into fetch state
							|| (memRead[i] && CACHE_begintransfer[i]) //when accel first accesses cache
							|| (memRead[i] && !cacheHit[i]) || (state[i] != stateREAD && memRead[i]) //if on a read, it's not a cache hit, and it's not in stateREAD
							|| (memWrite[i] && state[i] != stateWRITE) //if on a write, it's not in stateWRITE
							|| dataMaster_waitrequest[i] //if waitrequest is asserted from off-chip memory
							//|| stallWrite_for_fetch[i]
							`ifdef LVT
							|| (stallRead[i] || stallRead_reg[i]) || stallWrite[i] || stallWrite_final[i] || dataMaster_waitrequest[i] || CACHE_begintransfer[i]
							`endif
							|| state_64[i] == 1 || state_64[i] == 2; //for 64-bit accesses

		assign tag[i] = memAddress[i][31 : cacheSize + blockSize];
		assign bytes[i] = memAddress[i][1 : 0];
		assign words[i] = memAddress[i][4 : 2];
		assign savedTag[i] = address[i][31 : cacheSize + blockSize];
		assign savedBlockWord[i] = address[i][blockSize  - 1 : 2];
		assign savedByte[i] = address[i][1 : 0];

		for (j=0; j<ways; j=j+1) begin : loop_cacheTagValidbit
			assign cacheTag[i][j] = cacheQ[i][j][tagSizeBits : 1];
			//assign validBit[i][j] = (^cacheQ[i][j][0] === 1'bX || cacheQ[i][j][0] == 0) ? 1'b0 : 1'b1; 	//to take care of undefined case
			assign validBit[i][j] = cacheQ[i][j][0];
		end

		//if not direct-mapped
		if (ways != 1) begin
			assign cacheQData[i] = cacheQ[i][cacheHitWasIn[i]][blockSizeBits + tagSizeBits : tagSizeBits + 1];
			assign cacheAddress_forSet[i] = address[i][cacheSize + blockSize - 1 : blockSize];

			always @(posedge clk)
			begin
				if (fetch_one_left[i])	
				begin
					writetoSet_reg[i] <= writetoSet[cacheAddress_forSet[i]];
				end
			end

			for (j = 0; j < ways; j=j+1) begin : loop_cacheWrite
				assign cacheWrite[i][j] = (fetchDone[i] && writetoSet_reg[i] == j) || (state[i] == stateWRITE && cacheHit[i] && cacheHitWasIn[i] == j);
			end
		end
		//if direct-mapped
		else begin
			assign cacheQData[i] = cacheQ[i][0][blockSizeBits + tagSizeBits : tagSizeBits + 1];
			assign cacheWrite[i][0] = fetchDone[i] || (state[i] == stateWRITE && cacheHit[i]);
		end
		
		always @(posedge clk)
		begin
			memWrite_reg[i] <= memWrite[i];
		end

		//If we're in the fetch state, the data is valid and we've fetched
		//all but the last word we've done the fetch
		`ifdef DE4
		if (blockSize == 5) begin
		`elsif DE2
		if (blockSize == 2) begin
		`endif		
			assign fetchDone[i] = (state[i] == stateFETCH && dataMaster_readdatavalid[i]);
			assign fetch_one_left[i] = (state[i] == stateFETCH);
		end 
		else begin
			assign fetchDone[i] = (state[i] == stateFETCH && dataMaster_readdatavalid[i] && fetchWord[i] == burstCount - 1);
			assign fetch_one_left[i] = (state[i] == stateFETCH && fetchWord[i] == burstCount - 2);
		end

		//If the fetched data from the cache has the valid bit set
		//and the tag is the one we want we have a hit
		//assign cacheHit[i] = validBit[i] && savedTag[i] == cacheTag[i];
		if (ways != 1) begin
			always @(*)
			begin
				for (k=0; k<ways; k=k+1) 
				begin
					cacheHit_ways[i][k] = (validBit[i][k] && savedTag[i] == cacheTag[i][k]);
				end
			end
		    assign cacheHit[i] = |cacheHit_ways[i];

			always @(*)
			begin
				cacheHitWasIn[i] = "X";
				for (k=0; k<ways; k=k+1) 
				begin
					if (validBit[i][k] && savedTag[i] == cacheTag[i][k])
						cacheHitWasIn[i] = k;
				end
			end
		end
		else begin
		    assign cacheHit[i] = validBit[i][0] && savedTag[i] == cacheTag[i][0];
		end

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
						//	else if (!dataMaster_waitrequest[i])
						//	begin
								//if done first write
						//		state_64[i] <= 2;
						//	end
						end
						else if (memWrite[i])	//if write
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
						//	else if (!dataMaster_waitrequest[i])
						//	begin
								//if done first write
						//		state_64[i] <= 0;
						//	end
						end
						else if (memWrite[i])	//if write
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

		`ifdef DE4
		if (blockSize == 5) begin
		`elsif DE2
		if (blockSize == 2) begin
		`endif
			assign cacheData[i] =
				(state[i] == stateWRITE) ?
					{cacheWriteData[i], 31'd0, 1'b1} :
					{dataMaster_readdata[i], cache_tagData[i], 1'b1};
		end
		else begin
			assign cacheData[i] = 
				(state[i] == stateWRITE) ? 
					{cacheWriteData[i], 31'd0, 1'b1} :
					{dataMaster_readdata[i], fetchData[i], cache_tagData[i], 1'b1};
		end


		//If we're reading or writing 8 or 16 bits rather than a full 32-bit word, we need to only write
		//the bits we want to write in the word and keep the rest as they were
		always @(*)
		begin
			if (savedMem8[i])
			begin
				case (savedByte[i])
					2'b00: begin
						   writeData[i] = {24'd0, writeDataWord[i][7 : 0]};
			  			   cache_word_Byteenable[i] = 4'b0001;
						   end
					2'b01: begin
						   writeData[i] = {16'd0, writeDataWord[i][7 : 0], 8'd0};
			  			   cache_word_Byteenable[i] = 4'b0010;	
						   end
					2'b10: begin
						   writeData[i] = {8'd0, writeDataWord[i][7 : 0], 16'd0};
			  			   cache_word_Byteenable[i] = 4'b0100;	
						   end
					2'b11: begin
						   writeData[i] = {writeDataWord[i][7 : 0], 24'b0};
			  			   cache_word_Byteenable[i] = 4'b1000;		
						   end
					default : begin
						   writeData[i] = 32'bx;
			  			   cache_word_Byteenable[i] = 4'dx;		
						   end
				endcase
			end
			else if (savedMem16[i])
			begin
				case (savedByte[i][1])
					1'b0: begin
						  writeData[i] = {16'd0, writeDataWord[i][15 : 0]};
						  cache_word_Byteenable[i] = 4'b0011;
						  end
					1'b1: begin
						  writeData[i] = {writeDataWord[i][15 : 0], 16'd0};
						  cache_word_Byteenable[i] = 4'b1100;
						  end
				default : begin
						  writeData[i] = 32'bx;
	 	    			  cache_word_Byteenable[i] = 4'dx;	
						  end
				endcase
			end
			else 
			begin
				writeData[i] = writeDataWord[i];
	 		    cache_word_Byteenable[i] = 4'b1111;	
			end
		end

		//could also do with shifts, or multiplexers as before
		assign cacheWriteData[i] = writeData[i] << (savedBlockWord[i] * wordWidth);

		//if cachewrite_ports[i] is asserted, it means port i of cache needs to be written.
		assign cacheWrite_ports[i] = |cacheWrite[i];

		assign cache_Byteenable[i] = fetchDone[i]? {cachelineBytes{1'b1}} : 
								  cacheWrite_ports[i]? (cache_word_Byteenable[i] << (savedBlockWord[i]*4)) << 4 :    
								  {cachelineBytes{1'b1}};

		//Multiplexer to select which word of the cache block we want
		//Mux is parameterized to change the size of cache line easily
		mux_param # ( 
			.N(wordWidth), //number of bits of output
			.M(blockSizeBits/wordWidth), //number of N inputs
			.S(log2(blockSizeBits/wordWidth))  //width of select lines
		) mux_param_inst (
			.D(cacheQData[i]),
			.SEL(savedBlockWord[i]),
			.Z(memReadDataWord[i])
		);

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
				memReadData[i] = {24'd0, memReadData_8bits[i]};
			else if (savedMem16[i])
				memReadData[i] = {16'd0, memReadData_16bits[i]};
			else
				memReadData[i] = memReadDataWord[i];
		end

		always @(*)
		begin
			if (mem8[i])
			begin
				case (bytes[i])
					2'b00: begin
						   dataMaster_writedata_32[i] = {24'd0, memWriteData[i][7 : 0]};
			  			   dataMaster_byteenable_32[i] = 4'b0001;
						   end
					2'b01: begin
						   dataMaster_writedata_32[i] = {16'd0, memWriteData[i][7 : 0], 8'd0};
			  			   dataMaster_byteenable_32[i] = 4'b0010;	
						   end
					2'b10: begin
						   dataMaster_writedata_32[i] = {8'd0, memWriteData[i][7 : 0], 16'd0};
			  			   dataMaster_byteenable_32[i] = 4'b0100;	
						   end
					2'b11: begin
						   dataMaster_writedata_32[i] = {memWriteData[i][7 : 0], 24'b0};
			  			   dataMaster_byteenable_32[i] = 4'b1000;		
						   end
					default : begin
						   dataMaster_writedata_32[i] = 32'dx;
			  			   dataMaster_byteenable_32[i] = 4'bxxxx;		
						   end
				endcase
			end
			else if (mem16[i])
			begin
				case (bytes[i][1])
					1'b0: begin
						  dataMaster_writedata_32[i] = {16'd0, memWriteData[i][15 : 0]};
						  dataMaster_byteenable_32[i] = 4'b0011;
						  end
					1'b1: begin
						  dataMaster_writedata_32[i] = {memWriteData[i][15 : 0], 16'd0};
						  dataMaster_byteenable_32[i] = 4'b1100;
						  end
				default : begin
						  dataMaster_writedata_32[i] = 32'dx;
	 	    			  dataMaster_byteenable_32[i] = 4'bxxxx;	
						  end
				endcase
			end
			else 
			begin
				dataMaster_writedata_32[i] = memWriteData[i];
	 		    dataMaster_byteenable_32[i] = 4'b1111;	
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
							dataMaster_byteenable[i] <= {byteEnWidth{1'b1}};
					
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
							`ifdef DE2
							dataMaster_writedata[i] <= dataMaster_writedata_32[i];
							dataMaster_address[i] <= {memAddress[i][31:2], 2'b0};
							`elsif DE4
							dataMaster_writedata[i] <= dataMaster_writedata_32[i] << (words[i]*32);												
							dataMaster_address[i] <= {memAddress[i][31:5], 5'b0};
							`endif
						end /*else if(flush[i]) begin
							state[i] <= stateFLUSH;
							flushAddr[i] <= 0;
						end*/
				
						//If we're reading and bypassing the cache
						//or performing a write we may need to set
						//bytes enable to something other than
						//reading/writing the entire word
						//if((memRead[i] && bypassCache[i]) || memWrite[i]) begin
						if(memWrite[i]) begin
							`ifdef DE2
							dataMaster_byteenable[i] <= dataMaster_byteenable_32[i];
							`elsif DE4
							dataMaster_byteenable[i] <= dataMaster_byteenable_32[i] << (words[i]*4) ;
							`endif
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
								/*if(flush[i]) begin
									state[i] <= stateFLUSH;
									flushAddr[i] <= 0;
								//if write is asserted (this is for when write happens right after read)
								end else */
								if (memWrite[i] && !stallWrite_for_fetch[i]) begin
									state[i] <= stateWRITE;
									address[i] <= memAddress[i];
									writeDataWord[i] <= memWriteData[i][31:0];						
									savedMem8[i] <= mem8[i];
									savedMem16[i] <= mem16[i];
									dataMaster_write[i] <= 1;
									`ifdef DE2
									dataMaster_address[i] <= {memAddress[i][31:2], 2'b0};
									dataMaster_writedata[i] <= dataMaster_writedata_32[i];
									dataMaster_byteenable[i] <= dataMaster_byteenable_32[i];
									`elsif DE4
									dataMaster_address[i] <= {memAddress[i][31:5], 5'b0};
									//need to shift writedata and byteenable to the correct position
									dataMaster_writedata[i] <= dataMaster_writedata_32[i] << (words[i]*32);									
									dataMaster_byteenable[i] <= dataMaster_byteenable_32[i] << (words[i]*4);
									`endif
								//if read is deasserted
								end else if(!memRead[i]) begin
									state[i] <= stateIDLE; //go back to IDLE state
								//if read is still asserted
								end else begin
									state[i] <= stateREAD; 
									dataMaster_address[i] <= {tag[i], cacheAddress[i], {blockSize{1'b0}}};
									dataMaster_byteenable[i] <= {byteEnWidth{1'b1}};

									savedMem8[i] <= mem8[i];
									savedMem16[i] <= mem16[i];
						
									address[i] <= memAddress[i];
								end
							//if cache miss
							end else begin //fetch data from off-chip memory
								if (!stallFetch_for_write[i]) begin
									//state[i] <= stateHOLD;
									state[i] <= stateFETCH;
									dataMaster_read[i] <= 1;
									dataMaster_burstcount[i] <= burstCount;
									dataMaster_beginbursttransfer[i] <= 1;
								end
							end
						end						
					end
					stateFETCH: begin	
						dataMaster_beginbursttransfer[i] <= 0;
						if(!dataMaster_waitrequest[i]) begin
							dataMaster_read[i] <= 0;
						end

						//If we have valid data
						if(dataMaster_readdatavalid[i]) begin
							//store it in the fetchData register if it's not the last word
							//(the last word is fed straight into the data register of the memory
							// block)
	
							//storing fetched data in the correct positions (shift in every time)
							//go through each fetchword
							for (l=0; l<(blockSizeBits/sdramWidth)-1; l=l+1) begin
								//if this is the fetchword we want
								if (fetchWord[i] == l) begin
									//copy all bits of DDR
									for (k=0; k<sdramWidth; k=k+1) begin
										fetchData[i][sdramWidth*l+k] <= dataMaster_readdata[i][k];
									end
								end
							end
					
							fetchWord[i] <= fetchWord[i] + 1;

							if(fetchWord[i] == burstCount - 1) begin
									state[i] <= stateREAD;
									fetchWord[i] <= 0;
							end
						end
					end
					/*stateHOLD: begin //extra state to begin fetch before it goes into stateFETCH
						dataMaster_beginbursttransfer[i] <= 0;
						if(!dataMaster_waitrequest[i]) begin
							dataMaster_read[i] <= 0;
							state[i] <= stateFETCH;
						end
					end*/
					stateWRITE: begin	
						//Once the memory write has completed either go back to idle
						//and stop writing or continue with another write
						if(!dataMaster_waitrequest[i]) begin
							dataMaster_write[i] <= 0;
							`ifdef LVT
							if (!stallWrite[i] && !stallWrite_final[i]) begin
								state[i] <= stateIDLE;
							end
							`else
								state[i] <= stateIDLE;
							`endif
						end
					end
					/*stateFLUSH: begin
						flushAddr[i] <= flushAddr[i] + 1'b1;
						if(flushAddr[i] == {cacheSize{1'b1}})
							state[i] <= stateIDLE;
					end*/
				endcase
			end
		end 		      
	end
    endgenerate

endmodule


