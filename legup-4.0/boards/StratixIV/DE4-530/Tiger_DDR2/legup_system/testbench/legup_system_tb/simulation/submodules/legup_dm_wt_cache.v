
module legup_dm_wt_cache
#(
///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Parameter definitions ////////////////////////////
///////////////////////////////////////////////////////////////////////////////
parameter DATA_WIDTH    = 32,   // bits
parameter BLOCK_SIZE    = 4,    // words
parameter CACHE_LINES   = 512   // lines
)
(
	clk,
	reset,
	
	avs_cache_address,
	avs_cache_byteenable,
	avs_cache_read,
	avs_cache_write,
	avs_cache_writedata,
	avs_cache_readdata,
	avs_cache_readdatavalid,
	avs_cache_waitrequest,

	avm_cache_readdata,
	avm_cache_readdatavalid,
	avm_cache_waitrequest,
	avm_cache_address,
	avm_cache_beginbursttransfer,
	avm_cache_burstcount,
	avm_cache_byteenable,
	avm_cache_read,
	avm_cache_write,
	avm_cache_writedata
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////// Local parameter definitions /////////////////////////
///////////////////////////////////////////////////////////////////////////////
localparam	AW  = 30;						// Slave address width
localparam	DW 	= DATA_WIDTH - 1;			// Data width
localparam	BE 	= (DATA_WIDTH / 8) - 1;		// Byte enable width
localparam	OW 	= $clog2(DATA_WIDTH / 8);	// Cache address' offset width
localparam	BW  = $clog2(BLOCK_SIZE);		// Cache address' block size
localparam	IW  = $clog2(CACHE_LINES);		// Cache address' index width
localparam	TW  = AW - OW - BW - IW + 1;	// Cache address' tag width

localparam	OLB	= 0;                        // Offest lower bit
localparam	OUB	= OW - 1;                   // Offest upper bit
localparam	BLB	= OW;                       // Block lower bit
localparam	BUB	= BW + OW - 1;              // Block upper bit
localparam	ILB	= BW + OW;                  // Index lower bit
localparam	IUB	= IW + BW + OW - 1;         // Index upper bit
localparam	TLB	= IW + BW + OW;             // Tag lower bit
localparam	TUB = AW;                       // Tag upper bit

// Cache states
localparam	STATE_FLUSH                 = 3'h0,
            // Idle state: waiting for a new transaction on the slave port
            STATE_IDLE                  = 3'h1,
            // Write and check hit state: issue the write on the master and
            // update the value in the cache is appropriate
            STATE_ISSUE_WRITE_CHECK_HIT = 3'h2,
            // Write and wait state: continue issuing the write on the master 
            // until waitrequest has been lowered
            STATE_ISSUE_WRITE_WAIT      = 3'h3,
            // Read check hit state: check if the pending read is in the cache
            STATE_CHECK_HIT             = 3'h4,
            // Begin burst read state: if it a cache miss, start a burst read
            STATE_BEGIN_BURST           = 3'h5,
            // Issue read state: continue issuing the read until the
            // waitrequest has been lowered
            STATE_ISSUE_READ            = 3'h6,
            // Read wait state: for the data to be returned
            STATE_WAIT_FOR_DATA         = 3'h7;

localparam  READ_MASK = 32'hFFFFFFFF << (BW + OW);

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// Port declarations //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
input               clk;
input               reset;
	
input       [AW: 0] avs_cache_address;
input       [BE: 0] avs_cache_byteenable;
input               avs_cache_read;
input               avs_cache_write;
input       [DW: 0] avs_cache_writedata;
output      [DW: 0] avs_cache_readdata;
output              avs_cache_readdatavalid;
output              avs_cache_waitrequest;

input       [DW: 0] avm_cache_readdata;
input               avm_cache_readdatavalid;
input               avm_cache_waitrequest;
output      [31: 0] avm_cache_address;
output              avm_cache_beginbursttransfer;
output      [BW: 0] avm_cache_burstcount;
output      [BE: 0] avm_cache_byteenable;
output              avm_cache_read;
output              avm_cache_write;
output      [DW: 0] avm_cache_writedata;

///////////////////////////////////////////////////////////////////////////////
////////////////////////// Local signals definitions //////////////////////////
///////////////////////////////////////////////////////////////////////////////
reg         [IW: 0] flush_address;
wire                flush_complete;

wire        [IW: 0] tag_memory_addr_a;
wire                tag_memory_we_a;

reg         [AW: 0] last_avs_address;
reg         [BE: 0] last_avs_byteenable;
reg                 last_avs_read;
reg                 last_avs_write;
reg         [DW: 0] last_avs_writedata;

reg         [BW: 1] read_word_count;

reg         [ 2: 0] cache_state;
reg         [ 2: 0] cache_next_state;

wire        [DW: 0] port_a_data_out;
wire        [TW: 0] port_a_tag_out;
wire                cache_hit;
wire                cache_miss;
wire                cache_hit_write;
wire        [DW: 0] cache_hit_writedata;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////// Output assignments //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
assign avs_cache_readdata           = avm_cache_readdatavalid ? 
                                        avm_cache_readdata : 
                                        port_a_data_out;
assign avs_cache_readdatavalid      = cache_hit | (avm_cache_readdatavalid &
                                        (read_word_count == 
                                            last_avs_address[BUB:BLB]));
assign avs_cache_waitrequest        = (cache_state != STATE_IDLE);

assign avm_cache_address            = (cache_state == STATE_ISSUE_WRITE_CHECK_HIT) |
                                      (cache_state == STATE_ISSUE_WRITE_WAIT) ?
                                        last_avs_address :
                                        last_avs_address & READ_MASK;
assign avm_cache_beginbursttransfer = (cache_state == STATE_BEGIN_BURST);
assign avm_cache_burstcount         = (cache_state == STATE_ISSUE_WRITE_CHECK_HIT) |
                                      (cache_state == STATE_ISSUE_WRITE_WAIT) ?
                                        1 : BLOCK_SIZE;
assign avm_cache_byteenable         = last_avs_byteenable;
assign avm_cache_read               = (cache_state == STATE_BEGIN_BURST) | 
                                      (cache_state == STATE_ISSUE_READ);
assign avm_cache_write              = (cache_state == STATE_ISSUE_WRITE_CHECK_HIT) |
                                      (cache_state == STATE_ISSUE_WRITE_WAIT);
assign avm_cache_writedata          = last_avs_writedata;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Internal assignments /////////////////////////////
///////////////////////////////////////////////////////////////////////////////
assign flush_complete   = flush_address == 4'h4;
//assign flush_complete = flush_address == {IW{1'b1}};

// Addres for Tag Memory Port A
// Uses:
//   address to flush (STATE_FLUSH)
//   address to check cache hit (STATE_IDLE)
assign tag_memory_addr_a    = cache_state == STATE_FLUSH ?
                                flush_address :
//                                cache_state == STATE_IDLE ?
//                                    avs_cache_address[IUB:ILB] :
//                                    last_avs_address[IUB:ILB];
                                    avs_cache_address[IUB:ILB];
assign tag_memory_we_a      = cache_state == STATE_FLUSH ?
                                1'b1 : 1'b0;
//                                cache_state == STATE_ISSUE_WRITE_CHECK_HIT;

assign cache_hit_write  = (cache_state == STATE_ISSUE_WRITE_CHECK_HIT ) & port_a_tag_out[TW] & 
                          (port_a_tag_out[TW-1:0] == last_avs_address[TUB:TLB]);
assign cache_hit	= (cache_state == STATE_CHECK_HIT) & port_a_tag_out[TW] & 
                        (port_a_tag_out[TW-1:0] == last_avs_address[TUB:TLB]);
assign cache_miss	= (cache_state == STATE_CHECK_HIT) & ~cache_hit;
assign cache_hit_writedata  = {
            last_avs_byteenable[3]  ? 
                last_avs_writedata[31:24] :
                port_a_data_out[31:24] ,
            last_avs_byteenable[2]  ? 
                last_avs_writedata[23:16] :
                port_a_data_out[23:16] ,
            last_avs_byteenable[1]  ? 
                last_avs_writedata[15: 8] :
                port_a_data_out[15: 8] ,
            last_avs_byteenable[0]  ? 
                last_avs_writedata[ 7: 0] :
                port_a_data_out[ 7: 0]
        };
                
always @(posedge clk)
begin
    if (reset)
        flush_address <= 'h0;
    else if (~flush_complete)
        flush_address <= flush_address + 'h1;
end

always @(posedge clk)
begin
    if (reset)
    begin
        last_avs_address    <= 0;
        last_avs_byteenable <= 0;
        last_avs_read       <= 0;
        last_avs_write      <= 0;
        last_avs_writedata  <= 0;
    end
    else if (cache_state == STATE_IDLE)
    begin
        last_avs_address    <= avs_cache_address;
        last_avs_byteenable <= avs_cache_read ? 
                                {(BE + 1){1'b1}} : 
                                avs_cache_byteenable;
        last_avs_read       <= avs_cache_read;
        last_avs_write      <= avs_cache_write;
        last_avs_writedata  <= avs_cache_writedata;
    end
end

always @(posedge clk)
begin
    if (reset)
        read_word_count <= 0;
    else if ((cache_state == STATE_WAIT_FOR_DATA) & avm_cache_readdatavalid)
        read_word_count <= read_word_count + 1;
    else if (cache_state != STATE_WAIT_FOR_DATA)
        read_word_count <= 0;
end


///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Finite state machines ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

always @(posedge clk)
begin
    if (reset)
        cache_state <= STATE_IDLE;
    else
        cache_state <= cache_next_state;
end

always @(*)
begin
    cache_next_state = cache_state;
	
    if ((cache_state == STATE_FLUSH) & flush_complete)
        cache_next_state = STATE_IDLE;

    // If a write has been requested    
    else if ((cache_state == STATE_IDLE) & avs_cache_write)
        cache_next_state = STATE_ISSUE_WRITE_CHECK_HIT;
	
    else if ((cache_state == STATE_ISSUE_WRITE_CHECK_HIT) & ~avm_cache_waitrequest)
        cache_next_state = STATE_IDLE;
	
    else if (cache_state == STATE_ISSUE_WRITE_CHECK_HIT)
        cache_next_state = STATE_ISSUE_WRITE_WAIT;
	
    else if ((cache_state == STATE_ISSUE_WRITE_WAIT) & ~avm_cache_waitrequest)
        cache_next_state = STATE_IDLE;
	
    // If a read has been requested    
    else if ((cache_state == STATE_IDLE) & avs_cache_read)
        cache_next_state = STATE_CHECK_HIT;
	
    else if ((cache_state == STATE_CHECK_HIT) & cache_hit)
        cache_next_state = STATE_IDLE;
	
    else if ((cache_state == STATE_CHECK_HIT) & cache_miss)
        cache_next_state = STATE_BEGIN_BURST;
	
    else if ((cache_state == STATE_BEGIN_BURST) & ~avm_cache_waitrequest)
        cache_next_state = STATE_WAIT_FOR_DATA;
	
    else if (cache_state == STATE_BEGIN_BURST)
        cache_next_state = STATE_ISSUE_READ;
	
    else if ((cache_state == STATE_ISSUE_READ) & ~avm_cache_waitrequest)
        cache_next_state = STATE_WAIT_FOR_DATA;
	
    else if ((cache_state == STATE_WAIT_FOR_DATA) & 
            avm_cache_readdatavalid & (&(read_word_count)))
        cache_next_state = STATE_IDLE;
end


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Submodules //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
true_dual_port_ram_single_clock #(.ADDR_WIDTH(IW)) tag_memory (
    .clk        (clk),

    .addr_a     (tag_memory_addr_a),
    .data_a     (0),
    .we_a		(tag_memory_we_a),

    .addr_b     (last_avs_address[IUB:ILB]),
    .data_b     ({1'b1, last_avs_address[TUB:TLB]}),
    .we_b       (avm_cache_readdatavalid),

    .q_a        (port_a_tag_out),
    .q_b        ()
);

true_dual_port_ram_single_clock #(.ADDR_WIDTH(IW + BW)) data_memory (
	.clk        (clk),

	.addr_a     ((cache_state == STATE_IDLE) ? avs_cache_address[IUB:BLB] : last_avs_address[IUB:BLB]),
	.data_a     (cache_hit_writedata),
	.we_a       (cache_hit_write),

	.addr_b     ({last_avs_address[IUB:ILB], read_word_count}),
	.data_b     (avm_cache_readdata),
	.we_b       (avm_cache_readdatavalid),

	.q_a        (port_a_data_out),
	.q_b        ()
);

endmodule

