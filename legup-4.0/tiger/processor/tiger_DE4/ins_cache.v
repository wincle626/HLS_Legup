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

`include "cache_parameters.v"

module ins_cache (
	input clk,
	input reset_n,

	//CPU side memory control signals
	input PROC0_memRead,
	input [31:0]PROC0_memAddress,
	output [31:0]PROC0_memReadData,

	//input PROC0_flush,

	//output PROC0_canFlush,

	//True if the data on memReadData is valid (i.e. data we've just read from the cache)
	output PROC0_readDataValid,

	//CPU pipeline stall
	output PROC0_stall,

	//Avalon Bus side signals
	output avm_dataMaster0_read,
	output [31:0]avm_dataMaster0_address,
	output avm_dataMaster0_beginbursttransfer,
	output [`IBURSTCOUNTWIDTH-1:0]avm_dataMaster0_burstcount,
	input [`SDRAM_WIDTH-1:0]avm_dataMaster0_readdata,
	input avm_dataMaster0_waitrequest,
	input avm_dataMaster0_readdatavalid
);

	//define cache parameters
	localparam NumPorts = `NUM_ICACHE_PORTS;
	localparam cacheSize = `ICACHE_SIZE; //number of lines in cache 2^9 = 512 lines
	localparam ways = `IWAYS;
    localparam blockSize = `IBLOCKSIZE;

	localparam blockSizeBits = 8*(2**blockSize); //total bits per line
	//localparam tagSizeBits = 31; //FIXING THIS TO 31 IN ORDER FOR THE RAM TO BE BYTE ADDRESSABLE
	localparam tagSizeBits = 32 - cacheSize - blockSize;
	localparam burstCount = (2**blockSize)/`BURSTCOUNT_DIV; //number of burst to main memory
	localparam wordWidth = 32; //bits in a word
	localparam cachelineBytes = (2**blockSize)+4;
	localparam sdramWidth = `SDRAM_WIDTH;
	localparam byteEnWidth = `BYTEEN_WIDTH;

	//define states
	localparam stateIDLE = 3'b000;
	localparam stateREAD = 3'b001;
	localparam stateFETCH = 3'b010;
	//localparam stateFLUSH = 3'b100;
	//localparam stateHOLD = 3'b101;

	wire cacheHit [NumPorts - 1 : 0];
	reg [ways - 1 : 0] cacheHit_ways [NumPorts - 1 : 0];
	wire [cacheSize - 1 : 0]cacheAddress [NumPorts - 1 : 0];
	wire [cacheSize - 1 : 0] cacheAddress_forSet [NumPorts - 1 : 0];
	wire fetch_one_left [NumPorts - 1 : 0];
	wire [tagSizeBits - 1 : 0]tag [NumPorts - 1 : 0];

	wire [wordWidth-1:0]memReadDataWord [NumPorts - 1 : 0];
	
	wire [ways - 1 : 0] cacheWrite [NumPorts - 1 : 0];
	wire cacheWrite_ports [NumPorts - 1 : 0]; //indicates if write signal is asserted (for all sets) for this port
	wire cacheClkEn [NumPorts - 1 : 0];
	wire [blockSizeBits + tagSizeBits : 0]cacheData [NumPorts - 1 : 0];
	wire [blockSizeBits + tagSizeBits : 0]cacheQ [NumPorts - 1 : 0][ways - 1 : 0];
	
	wire [tagSizeBits - 1 : 0]cacheTag [NumPorts - 1 : 0][ways - 1 : 0]; //this is the current tag that will compared against to see if cache hit
	wire validBit [NumPorts - 1 : 0][ways - 1 : 0];
	wire [blockSizeBits - 1 : 0]cacheQData [NumPorts - 1 : 0];
	
	wire [tagSizeBits - 1 : 0]savedTag [NumPorts - 1 : 0];
	wire [blockSize - 3 : 0]savedBlockWord [NumPorts - 1 : 0];
	
	wire fetchDone [NumPorts - 1 : 0];
	reg fetchDone_reg [NumPorts - 1 : 0];
	
	reg [31:0]address [NumPorts - 1 : 0];
	
//	reg [blockSizeBits - 33 : 0]fetchData [NumPorts - 1 : 0];
	reg [blockSizeBits - (sdramWidth+1) : 0]fetchData [NumPorts - 1 : 0]; // 257 = size of avm_dataMaster_readdata + 1
	reg [`IBURSTCOUNTWIDTH - 1 : 0]fetchWord [NumPorts - 1 : 0];
	
	reg [2:0]state [NumPorts - 1 : 0];
	
	//reg [cacheSize - 1 : 0]flushAddr [NumPorts - 1 : 0];
		
//////////////new cache signals///////////////
	wire memRead [NumPorts - 1 : 0];
	wire [31:0]memAddress [NumPorts - 1 : 0];
	wire [wordWidth-1:0]memReadData [NumPorts - 1 : 0];
	//wire flush [NumPorts - 1 : 0];

	//wire canFlush [NumPorts - 1 : 0];
	wire stall [NumPorts - 1 : 0];
	wire readDataValid [NumPorts - 1 : 0];
//////////////////////////////////////////////////
////////for generate signals//////////////////////
	reg dataMaster_read [NumPorts - 1 : 0];
	reg dataMaster_write [NumPorts - 1 : 0];
	reg [31:0] dataMaster_address [NumPorts - 1 : 0];
	reg [sdramWidth-1:0] dataMaster_writedata [NumPorts - 1 : 0];
	reg [byteEnWidth-1:0] dataMaster_byteenable [NumPorts - 1 : 0];
	wire [sdramWidth-1:0] dataMaster_readdata [NumPorts - 1 : 0];
	reg dataMaster_beginbursttransfer [NumPorts - 1 : 0];
	reg [`IBURSTCOUNTWIDTH-1:0] dataMaster_burstcount [NumPorts - 1 : 0];	
	wire dataMaster_waitrequest [NumPorts - 1 : 0];
	wire dataMaster_readdatavalid [NumPorts - 1 : 0];

	wire cacheAddress_match;
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

    genvar i, j;
	integer k, l;
//////////////////////////////////////////////////
	assign memRead[0] = PROC0_memRead;
	assign memAddress[0] = PROC0_memAddress;
	//assign flush[0] = PROC0_flush;

	assign PROC0_memReadData = memReadData[0];
	//assign PROC0_canFlush = canFlush[0];
	assign PROC0_readDataValid = readDataValid[0];
	assign PROC0_stall = stall[0];

	assign avm_dataMaster0_read = dataMaster_read[0];
	assign avm_dataMaster0_address = dataMaster_address[0];
	assign avm_dataMaster0_beginbursttransfer = dataMaster_beginbursttransfer[0];
	assign avm_dataMaster0_burstcount = dataMaster_burstcount[0];
	assign dataMaster_readdata[0] = avm_dataMaster0_readdata;
	assign dataMaster_waitrequest[0] = avm_dataMaster0_waitrequest;
	assign dataMaster_readdatavalid[0] = avm_dataMaster0_readdatavalid;

	assign stallRead[0] = 1'b0; 

	generate 
		for (i = 0; i < ways; i = i+1) begin : loop_instantiateRAM
			icacheMem icacheMemIns (
    								.address(cacheAddress[0]),
    								.clken(cacheClkEn[0]),
    								.clock(clk),
    								.data(cacheData[0]),
    								.wren(cacheWrite[0][i]),
   									.q(cacheQ[0][i])
								);
		end
	endgenerate

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

	//GENVAR I IS USED FOR EACH CACHE PORT
	//INTEGER J IS USED FOR EACH SET (ASSOCIATIVITY)
	/////////////////////////////////////////////////////////////
    generate
    for (i = 0 ; i < NumPorts ; i = i + 1)
    begin : loop_cacheSignals

		//Stall the pipeline if we're fetching data from memory, or if we've
		//just had a cache miss or if we're trying to write and not in the idle
		//state or if we're bypassing the cache and reading from the avalon bus
		//and the read hasn't completed yet or if we're flushing the cache
		assign stall[i] = state[i] == stateFETCH || (state[i] == stateREAD && !cacheHit[i]) 
			|| (state[i] != stateIDLE && state[i] != stateREAD && memRead[i]);
			//|| (state[i] == stateHOLD);
			//|| (state[i] == stateFLUSH)
			//|| (flush[i] && !canFlush[i]);
	
		//We can start a flush in the idle state
		//assign canFlush[i] = state[i] == stateIDLE || (state[i] == stateREAD && cacheHit[i]);
		assign readDataValid[i] = state[i] == stateREAD && cacheHit[i];

		assign tag[i] = memAddress[i][31 : cacheSize + blockSize];
		assign savedTag[i] = address[i][31 : cacheSize + blockSize];
		assign savedBlockWord[i] = address[i][blockSize  - 1 : 2];

		for (j=0; j<ways; j=j+1) begin : loop_cacheTagValidbit
			assign cacheTag[i][j] = cacheQ[i][j][tagSizeBits : 1];
			//assign validBit[i][j] = (^cacheQ[i][j][0] === 1'bX || cacheQ[i][j][0] == 0) ? 1'b0 : 1'b1; 	//to take care of undefined case
			assign validBit[i][j] = cacheQ[i][j][0];
		end

		//assign cacheClkEn[i] = (memRead[i] && (state[i] == stateIDLE || (state[i] == stateREAD && cacheHit[i]))) || fetchDone[i] || state[i] == stateFLUSH || fetchDone_reg[i];
		assign cacheClkEn[i] = (memRead[i] && (state[i] == stateIDLE || (state[i] == stateREAD && cacheHit[i]))) || fetchDone[i] || fetchDone_reg[i];

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
				//assign cacheWrite[i][j] = (fetchDone[i] && writetoSet_reg[i] == j) || (state[i] == stateFLUSH);
				assign cacheWrite[i][j] = (fetchDone[i] && writetoSet_reg[i] == j);
			end
		end
		//if direct-mapped
		else begin
			assign cacheQData[i] = cacheQ[i][0][blockSizeBits + tagSizeBits : tagSizeBits + 1];
			//assign cacheWrite[i][0] = fetchDone[i] || (state[i] == stateFLUSH);
			assign cacheWrite[i][0] = fetchDone[i];
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

		//had a hit (so we can write the data into the cache) enable the cache memory block clock.
		always @(posedge clk)
		begin
			fetchDone_reg[i] <= fetchDone[i];
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

		//assign cacheAddress[i] = (fetchDone[i])? address[i][cacheSize + blockSize - 1 : blockSize] : state[i] == stateFLUSH ? flushAddr[i] : memAddress[i][cacheSize + blockSize - 1 : blockSize];
		assign cacheAddress[i] = (fetchDone[i])? address[i][cacheSize + blockSize - 1 : blockSize] : memAddress[i][cacheSize + blockSize - 1 : blockSize];

		`ifdef DE4
		if (blockSize == 5) begin
		`elsif DE2
		if (blockSize == 2) begin
		`endif
			assign cacheData[i] = 
				//state[i] == stateFLUSH ? {(blockSizeBits + tagSizeBits + 1){1'b0}} :
				{dataMaster_readdata[i], dataMaster_address[i][31 : cacheSize + blockSize], 1'b1};
		end
		else begin
			assign cacheData[i] = 
				//state[i] == stateFLUSH ? {(blockSizeBits + tagSizeBits + 1){1'b0}} :
				{dataMaster_readdata[i], fetchData[i], dataMaster_address[i][31 : cacheSize + blockSize], 1'b1};
		end

		//if cachewrite_ports[i] is asserted, it means port i of cache needs to be written.
		assign cacheWrite_ports[i] = |cacheWrite[i];

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

		//for instruction cache, always 32-bit data is read out
		assign memReadData[i] = memReadDataWord[i];

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
							address[i] <= memAddress[i];
						end /*else if(flush[i]) begin
							state[i] <= stateFLUSH;
							flushAddr[i] <= 0;
						end*/
				
						//If we're reading and bypassing the cache
						//or performing a write we may need to set
						//bytes enable to something other than
						//reading/writing the entire word
						//if((memRead[i] && bypassCache[i]) || memWrite[i]) begin
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
								//if read is deasserted
								end else */
								if(!memRead[i]) begin
									state[i] <= stateIDLE; //go back to IDLE state
								//if read is still asserted
								end else begin
									state[i] <= stateREAD; 
									dataMaster_address[i] <= {tag[i], cacheAddress[i], {blockSize{1'b0}}};
									address[i] <= memAddress[i];
								end
							//if cache miss
							end else begin //fetch data from off-chip memory
								//state[i] <= stateHOLD;
								state[i] <= stateFETCH;
								dataMaster_read[i] <= 1;
								dataMaster_burstcount[i] <= burstCount;
								dataMaster_beginbursttransfer[i] <= 1;
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


