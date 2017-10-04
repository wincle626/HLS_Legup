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

module Cache (
	input clk,
	input reset_n,

	//CPU side memory control signals
	input memRead,
	input memWrite,
	input [31:0]memAddress,
	input [31:0]memWriteData,
	output [31:0]memReadData,

	input flush,

	//Used to read or write 8 or 16 bits instead
	//of a full 32 bit word
	input mem8,
	input mem16,

	//Can we read or write?
	//If these signals say we can't then attempting
	//to will cause stall to go high
	output canRead,
	output canWrite,

	output canFlush,

	//True if the data on memReadData is valid (i.e. data we've just read from the cache)
	output readDataValid,

	//CPU pipeline stall
	output stall,

	//Avalon Bus side signals
	output reg avm_dataMaster_read,
	output reg avm_dataMaster_write,
	output reg [31:0]avm_dataMaster_address,
	output reg [31:0]avm_dataMaster_writedata,
	output reg [3:0]avm_dataMaster_byteenable,
	input [31:0]avm_dataMaster_readdata,
	input avm_dataMaster_waitrequest,
	input avm_dataMaster_readdatavalid
);

	parameter stateIDLE = 0;
	parameter stateREAD = 1;
	parameter stateFETCH = 2;
	parameter stateWRITE = 3;
	parameter stateAVALON_READ = 4;
	parameter stateAVALON_WRITE = 5;
	parameter stateFLUSH = 6;
	
	parameter blockSize = 4;
	parameter blockSizeBits = 128;
	parameter cacheSize = 9;
	parameter tagSizeBits = 32 - cacheSize - blockSize;
	
	wire cacheHit;
	wire [cacheSize - 1 : 0]cacheAddress;
	wire [blockSize - 3 : 0]blockWord;
	wire [tagSizeBits - 1 : 0]tag;
	wire [1 : 0]byte; //lower 2 bits of address
	
	wire [31:0]memReadDataWord;
	
	wire cacheWrite;
	wire cacheClkEn;
	wire [blockSizeBits + tagSizeBits : 0]cacheData;
	wire [blockSizeBits + tagSizeBits : 0]cacheQ;
	
	wire [tagSizeBits - 1 : 0]cacheTag;
	wire validBit;
	wire [blockSizeBits - 1 : 0]cacheQData;
	
	wire [blockSizeBits - 1 : 0]cacheWriteData;
	wire [31 : 0]writeData;
	
	wire [tagSizeBits - 1 : 0]savedTag;
	wire [blockSize - 3 : 0]savedBlockWord;
	wire [1 : 0]savedByte;
	
	wire fetchDone;
	
	wire bypassCache; //should we bypass the cache for the current read/write operation?
		
	reg [31:0]address;
	reg [31:0]writeDataWord;
	
	reg [blockSizeBits - 33 : 0]fetchData;
	reg [blockSize - 3 : 0]fetchWord;
	
	reg [2:0]state;
	
	reg savedMem16;
	reg savedMem8;
	
	reg [cacheSize - 1 : 0]flushAddr;
		
	assign blockWord = memAddress[blockSize  - 1: 2];
	assign tag = memAddress[31 : cacheSize + blockSize];
	assign byte = memAddress[1 : 0];
	
	assign cacheTag = cacheQ[tagSizeBits : 1];
	assign validBit = cacheQ[0];
	assign cacheQData = cacheQ[blockSizeBits + tagSizeBits : tagSizeBits + 1];
	
	assign savedTag = address[31 : cacheSize + blockSize];
	assign savedBlockWord = address[blockSize  - 1 : 2];
	assign savedByte = address[1 : 0];
	
	//If we're in the fetch state, the data is valid and we've fetched
	//all but the last word we've done the fetch
	assign fetchDone = (state == stateFETCH 
		&& avm_dataMaster_readdatavalid && fetchWord == blockSize - 1);
	
	//If the fetched data from the cache has the valid bit set
	//and the tag is the one we want we have a hit
	assign cacheHit = validBit && savedTag == cacheTag;
	
	//Stall the pipeline if we're fetching data from memory, or if we've
	//just had a cache miss or if we're trying to write and not in the idle
	//state or if we're bypassing the cache and reading from the avalon bus
	//and the read hasn't completed yet or if we're flushing the cache
	assign stall = state == stateFETCH || (state == stateREAD && !cacheHit) 
		|| (memWrite && state != stateIDLE) || (state != stateIDLE && state != stateREAD && memRead)
		|| (state == stateAVALON_READ && !avm_dataMaster_readdatavalid)
		|| (state == stateFLUSH)
		|| (flush && !canFlush);
	
	//We can start a read in the idle state or the read state if we've had a cache hit
	assign canRead = state == stateIDLE || (state == stateREAD && cacheHit);
	//We can start a write in the idle state
	assign canWrite = state == stateIDLE;
	//We can start a flush in the idle state
	assign canFlush = state == stateIDLE || (state == stateREAD && cacheHit);
	
	assign readDataValid = state == stateREAD && cacheHit;
		
	//If we've just done a fetch we want to write to the correct cache address
	//or if we're writing data we want to write to the correct cache address,
	//if if we're flushing the cache we want to write to the current address we're flushing
	//otherwise we want the address given by memAddress.
	assign cacheAddress = fetchDone ? address[cacheSize + blockSize - 1 : blockSize] :
		state == stateWRITE && cacheHit ? address[cacheSize + blockSize - 1 : blockSize] :
		state == stateFLUSH  ? flushAddr :
			memAddress[cacheSize + blockSize - 1 : blockSize];
	
	//If we've just finished fetching, enable write so we can write the fetched
	//data to the cache next cycle or if we need to write data to the cache
	//enable writing
	assign cacheWrite = fetchDone || (state == stateWRITE && cacheHit) || state == stateFLUSH;
	
	//If we want to read and we're either idle or reading and just had a hit or if we've just finised
	//a fetch then enable the cache memory block clock.  If we want to write and we're idle or we've
	//had a hit (so we can write the data into the cache) enable the cache memory block clock.
	
	// MLA -- this is a bug in simulation where the cache line written (fetched from sdram) doesn't update read line
	assign cacheClkEn = 1'b1; /*(memRead && (state == stateIDLE || (state == stateREAD && cacheHit)) && !bypassCache)
	|| fetchDone || (memWrite && state == stateIDLE && !bypassCache) || (state == stateWRITE && cacheHit)
	|| state == stateFLUSH;*/
	
	//Data to write to the cache, in the format
	//[            data                 | tag | v ]
	//Where v is the valid bit (set if valid)
	//If we're writing and we've had a cache hit we can overwrite what's there
	//with new data, if we're flushing then write 0s, otherwise we just give 
	//data we may have just fetched (The write enable will only be asserted 
	//if we have just fetched this data and want it written)
	assign cacheData = 
		(state == stateWRITE && cacheHit) ? 
			{cacheWriteData, avm_dataMaster_address[31 : cacheSize + blockSize], 1'b1} :
		state == stateFLUSH ? {(blockSizeBits + tagSizeBits + 1){1'b0}} :
			{avm_dataMaster_readdata, fetchData, avm_dataMaster_address[31 : cacheSize + blockSize], 1'b1};
	
	//If we're reading or writing 8 or 16 bits rather than a full 32-bit word, we need to only write
	//the bits we want to write in the word and keep the rest as they were
	assign writeData = savedMem8 ? (savedByte == 2'b00 ? {memReadDataWord[31 : 8], writeDataWord[7 : 0]}
				: savedByte == 2'b01 ? {memReadDataWord[31 : 16], writeDataWord[7 : 0], memReadDataWord[7 : 0]}
				: savedByte == 2'b10 ? {memReadDataWord[31 : 24], writeDataWord[7 : 0], memReadDataWord[15 : 0]}
				: savedByte == 2'b11 ? {writeDataWord[7 : 0], memReadDataWord[23 : 0]}
				: 32'hx)
			: savedMem16 ? (savedByte[1] == 1'b0 ? {memReadDataWord[31 : 16], writeDataWord[15 : 0]}
				: savedByte[1] == 1'b1 ? {writeDataWord[15 : 0], memReadDataWord[15 : 0]}
				: 32'hx)
			: writeDataWord; 
	
	//When writing data to the cache we need to overwrite only the word we are writing and
	//preserve the rest
	assign cacheWriteData = savedBlockWord == 2'b00 ? {cacheQData[127 : 32], writeData} :
		savedBlockWord == 2'b01 ? {cacheQData[127 : 64], writeData, cacheQData[31 : 0]} :
		savedBlockWord == 2'b10 ? {cacheQData[127 : 96], writeData, cacheQData[63 : 0]} :
		savedBlockWord == 2'b11 ? {writeData, cacheQData[95 : 0]} : 32'bx;
	
	//Multiplexer to select which word of the cache block we want
	//if we're reading from the avalon bus, bypass cache and give
	//data read directly from avalon
	//assign memReadData = cacheQData[(savedBlockWord + 1) * 32 - 1 : savedBlockWord * 32];
	assign memReadDataWord = state == stateAVALON_READ ? avm_dataMaster_readdata
		: savedBlockWord == 2'b00 ? cacheQData[31 : 0]
		: savedBlockWord == 2'b01 ? cacheQData[63 : 32]
		: savedBlockWord == 2'b10 ? cacheQData[95 : 64]
		: savedBlockWord == 2'b11 ? cacheQData[127 : 96]
		: 32'bx;
		
	//If mem8 or mem16 are asserted we only want part of the read word,
	//if neither are asserted we want the entire word
	assign memReadData = savedMem8 ? (savedByte == 2'b11 ? memReadDataWord[31 : 24]
							: savedByte == 2'b10 ? memReadDataWord[23 : 16] 
							: savedByte == 2'b01 ? memReadDataWord[15 : 8]
							: savedByte == 2'b00 ? memReadDataWord[7 : 0]
							: 32'hx)
						: savedMem16 ? (savedByte[1] == 1'b1 ? memReadDataWord[31 : 16]
							: savedByte[1] == 1'b0 ? memReadDataWord[15 : 0]
							: 32'hx)
						: memReadDataWord; 
		
	assign bypassCache = memAddress[31]; //If high bit of address is set we bypass the cache
	
	cacheMem cacheMemIns(
						.address(cacheAddress),
						.clken(cacheClkEn),
						.clock(clk),
						.data(cacheData),
						.wren(cacheWrite),
						.q(cacheQ));
	
	always @(posedge clk, negedge reset_n) begin
		if(!reset_n) begin
			state <= stateIDLE;
			avm_dataMaster_read <= 0;
			avm_dataMaster_write <= 0;
			address <= 0;
		end else begin
			case(state)
				stateIDLE: begin
					if(memRead) begin //If we want a read start a read
						if(bypassCache) begin
							avm_dataMaster_read <= 1;
							avm_dataMaster_address <= {memAddress[31:2], 2'b0};
							state <= stateAVALON_READ;
						end else begin
							state <= stateREAD;
							avm_dataMaster_address <= {tag, cacheAddress, {blockSize{1'b0}}};
							avm_dataMaster_byteenable <= 4'b1111;
						end
						
						address <= memAddress;
						savedMem8 <= mem8;
						savedMem16 <= mem16;
					end else if(memWrite) begin //If we want a write start a write
						if(bypassCache) begin
							state <= stateAVALON_WRITE;
						end else begin
							state <= stateWRITE;
							address <= memAddress;
							writeDataWord <= memWriteData;
						end
						
						savedMem8 <= mem8;
						savedMem16 <= mem16;
						
						avm_dataMaster_write <= 1;
						avm_dataMaster_writedata <= 
							mem8 ? (byte == 2'b00 ? {24'b0, memWriteData[7 : 0]}
								: byte == 2'b01 ? {16'b0, memWriteData[7 : 0], 8'b0}
								: byte == 2'b10 ? {8'b0, memWriteData[7 : 0], 16'b0}
								: byte == 2'b11 ? {memWriteData[7 : 0], 24'b0}
								: 32'hx)
							: mem16 ? (byte[1] == 1'b0 ? {16'b0, memWriteData[15 : 0]}
								: byte[1] == 1'b1 ? {memWriteData[15 : 0], 16'b0}
								: 32'hx)
							: memWriteData;
							
						avm_dataMaster_address <= {memAddress[31:2], 2'b0};
					end else if(flush) begin
						state <= stateFLUSH;
						flushAddr <= 0;
					end
					
					//If we're reading and bypassing the cache
					//or performing a write we may need to set
					//byte enable to something other than
					//reading/writing the entire word
					if((memRead && bypassCache) || memWrite) begin
						if(mem8) begin
							avm_dataMaster_byteenable <= byte == 2'b11 ? 4'b1000 
								: byte == 2'b10 ? 4'b0100
								: byte == 2'b01 ? 4'b0010
								: byte == 2'b00 ? 4'b0001
								: 4'bxxxx;
						end else if(mem16) begin
							avm_dataMaster_byteenable <= byte[1] == 1'b1 ? 4'b1100
								: byte[1] == 1'b0 ? 4'b0011
								: 4'bxxxx;
						end else begin
							avm_dataMaster_byteenable <= 4'b1111;
						end
					end
				end
				stateREAD: begin
					//If we've had a cache hit either go back to idle
					//or if we want another read continue in the read state
					//or if we want to flush go to the flush state
					if(cacheHit) begin 
						if(flush) begin
							state <= stateFLUSH;
							flushAddr <= 0;
						end else if(!memRead) begin
							state <= stateIDLE;
						end else begin
							if(bypassCache) begin
								avm_dataMaster_read <= 1;
								avm_dataMaster_address <= memAddress;
								state <= stateAVALON_READ;
								
								if(mem8) begin
									avm_dataMaster_byteenable <= byte == 2'b11 ? 4'b1000 
										: byte == 2'b10 ? 4'b0100
										: byte == 2'b01 ? 4'b0010
										: byte == 2'b00 ? 4'b0001
										: 4'bxxxx;
								end else if(mem16) begin
									avm_dataMaster_byteenable <= byte[1] == 1'b1 ? 4'b1100
										: byte[1] == 1'b0 ? 4'b0011
										: 4'bxxxx;
								end else begin
									avm_dataMaster_byteenable <= 4'b1111;
								end
							end else begin
								state <= stateREAD;
								avm_dataMaster_address <= {tag, cacheAddress, {blockSize{1'b0}}};
								avm_dataMaster_byteenable <= 4'b1111;
							end
							
							savedMem8 <= mem8;
							savedMem16 <= mem16;
							
							address <= memAddress;
						end
					end else begin //otherwise fetch data from memory
						state <= stateFETCH;
						avm_dataMaster_read <= 1;
						fetchWord <= 0;
					end
				end
				stateFETCH: begin
					//If wait request is low we can give another address to read from
					if(!avm_dataMaster_waitrequest) begin
						//If we've given address for all the blocks we want, stop reading
						if(avm_dataMaster_address[blockSize - 1 : 0] == {{(blockSize - 2){1'b1}}, 2'b0})
							avm_dataMaster_read <= 0;
						else //Otherwise give address of the next block
							avm_dataMaster_address <= avm_dataMaster_address + 4;
					end
					
					//If we have valid data
					if(avm_dataMaster_readdatavalid) begin
						//store it in the fetchData register if it's not the last word
						//(the last word is fed straight into the data register of the memory
						// block)
						case(fetchWord)
							2'b00:
								fetchData[31:0] <= avm_dataMaster_readdata;
							2'b01:
								fetchData[63:32] <= avm_dataMaster_readdata;
							2'b10:
								fetchData[95:64] <= avm_dataMaster_readdata;
						endcase
						
						fetchWord <= fetchWord + 1;
						//If this is the last word go back to the read state
						if(fetchWord == blockSize - 1)
							state <= stateREAD;
					end
				end
				stateWRITE: begin					
					//Once the memory write has completed either go back to idle
					//and stop writing or continue with another write
					if(!avm_dataMaster_waitrequest) begin
						avm_dataMaster_write <= 0;
						state <= stateIDLE;
					end
				end
				stateAVALON_READ: begin
					//No more wait request so address has been captured
					if(!avm_dataMaster_waitrequest) begin
						avm_dataMaster_read <= 0; //So stop asserting read
					end
					
					if(avm_dataMaster_readdatavalid) begin //We have our data
						state <= stateIDLE; //so go back to the idle state
					end
				end
				stateAVALON_WRITE: begin
					if(!avm_dataMaster_waitrequest) begin //if the write has finished
						state <= stateIDLE; //then go back to the idle state
						avm_dataMaster_write <= 0;
					end
				end
				stateFLUSH: begin
					flushAddr <= flushAddr + 1;
					if(flushAddr == {cacheSize{1'b1}})
						state <= stateIDLE;
				end
			endcase
		end
	end 			 
endmodule
