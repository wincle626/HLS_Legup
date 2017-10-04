
module legup_simple_cache
#(
///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Parameter definitions ////////////////////////////
///////////////////////////////////////////////////////////////////////////////
parameter DATA_WIDTH    = 32,
parameter CACHE_LINES   = 512
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
localparam	IW  = $clog2(CACHE_LINES);		// Cache address' index width
localparam	TW  = AW - OW - IW + 1;			// Cache address' tag width

localparam	OLB	= 0;                        // Offest lower bit
localparam	OUB	= OW - 1;                   // Offest upper bit
localparam	ILB	= OW;                       // Index lower bit
localparam	IUB	= IW + OW - 1;              // Index upper bit
localparam	TLB	= IW + OW;                  // Tag lower bit
localparam	TUB = AW;                       // Tag upper bit

localparam	STATE_FLUSH         = 3'h0,
            STATE_IDLE          = 3'h1,
            STATE_ISSUE_WRITE   = 3'h2,
            STATE_CHECK_HIT     = 3'h3,
            STATE_ISSUE_READ    = 3'h4,
            STATE_WAIT_FOR_DATA = 3'h5; // Cache states

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

reg         [ 2: 0] cache_state;
reg         [ 2: 0] cache_next_state;

wire        [DW: 0] port_a_data_out;
wire        [TW: 0] port_a_tag_out;
wire                cache_hit;
wire                cache_miss;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////// Output assignments //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
assign avs_cache_readdata       = avm_cache_readdatavalid ? 
                                    avm_cache_readdata : 
                                    port_a_data_out;
assign avs_cache_readdatavalid  = avm_cache_readdatavalid | cache_hit;
assign avs_cache_waitrequest    = (cache_state != STATE_IDLE);

assign avm_cache_address        = last_avs_address;
assign avm_cache_byteenable     = last_avs_byteenable;
assign avm_cache_read           = (cache_state == STATE_ISSUE_READ);
assign avm_cache_write          = (cache_state == STATE_ISSUE_WRITE);
assign avm_cache_writedata      = last_avs_writedata;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Internal assignments /////////////////////////////
///////////////////////////////////////////////////////////////////////////////
assign flush_complete   = flush_address == 4'h4;
//assign flush_complete = flush_address == {IW{1'b1}};

assign tag_memory_addr_a    = cache_state == STATE_FLUSH ?
                                flush_address :
                                cache_state == STATE_IDLE ?
                                    avs_cache_address[IUB:ILB] :
                                    last_avs_address[IUB:ILB];
assign tag_memory_we_a      = cache_state == STATE_FLUSH ?
                                1'b1 :
                                cache_state == STATE_ISSUE_WRITE;

assign cache_hit	= (cache_state == STATE_CHECK_HIT) & port_a_tag_out[TW] & 
                        (port_a_tag_out[TW-1:0] == last_avs_address[TUB:TLB]);
assign cache_miss	= (cache_state == STATE_CHECK_HIT) & ~cache_hit;

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
        last_avs_byteenable <= avs_cache_read ? {BE{1'b1}} : avs_cache_byteenable;
        last_avs_read       <= avs_cache_read;
        last_avs_write      <= avs_cache_write;
        last_avs_writedata  <= avs_cache_writedata;
    end
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
        cache_next_state = STATE_ISSUE_WRITE;
	
    else if ((cache_state == STATE_ISSUE_WRITE) & ~avm_cache_waitrequest)
        cache_next_state = STATE_IDLE;
	
    // If a read has been requested    
    else if ((cache_state == STATE_IDLE) & avs_cache_read)
        cache_next_state = STATE_CHECK_HIT;
	
    else if ((cache_state == STATE_CHECK_HIT) & cache_hit)
        cache_next_state = STATE_IDLE;
	
    else if ((cache_state == STATE_CHECK_HIT) & cache_miss)
        cache_next_state = STATE_ISSUE_READ;
	
    else if ((cache_state == STATE_ISSUE_READ) & ~avm_cache_waitrequest)
        cache_next_state = STATE_WAIT_FOR_DATA;
	
    else if ((cache_state == STATE_WAIT_FOR_DATA) & avm_cache_readdatavalid)
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

true_dual_port_ram_single_clock #(.ADDR_WIDTH(IW)) data_memory (
	.clk        (clk),

	.addr_a     (avs_cache_address[IUB:ILB]),
	.data_a     (0),
	.we_a       (0),

	.addr_b     (last_avs_address[IUB:ILB]),
	.data_b     (avm_cache_readdata),
	.we_b       (avm_cache_readdatavalid),

	.q_a        (port_a_data_out),
	.q_b        ()
);

endmodule

