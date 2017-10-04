
module legup_av_width_adapter 
#(
///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Parameter definitions ////////////////////////////
///////////////////////////////////////////////////////////////////////////////
parameter ADDR_WIDTH            = 32,   // bits
parameter SLAVE_DATA_WIDTH      = 32,   // bits
parameter MASTER_DATA_WIDTH     = 16    // bits
)
(
	clk,
	reset,
	
	avs_width_adapter_address,
	avs_width_adapter_byteenable,
	avs_width_adapter_read,
	avs_width_adapter_write,
	avs_width_adapter_writedata,
	avs_width_adapter_readdata,
	avs_width_adapter_readdatavalid,
	avs_width_adapter_waitrequest,

	avm_width_adapter_readdata,
	avm_width_adapter_readdatavalid,
	avm_width_adapter_waitrequest,
	avm_width_adapter_address,
	avm_width_adapter_byteenable,
	avm_width_adapter_read,
	avm_width_adapter_write,
	avm_width_adapter_writedata
);


///////////////////////////////////////////////////////////////////////////////
///////////////////////// Local parameter definitions /////////////////////////
///////////////////////////////////////////////////////////////////////////////
localparam	AW  = ADDR_WIDTH - 1;               // Address width
localparam	SDW = SLAVE_DATA_WIDTH - 1;         // Slave data width
localparam	SBE = (SLAVE_DATA_WIDTH / 8) - 1;   // Slave byte enable width
localparam	MDW = MASTER_DATA_WIDTH - 1;        // Master data width
localparam	MBE = (MASTER_DATA_WIDTH / 8) - 1;  // Master byte enable width

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// Port declarations //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
input               clk;
input               reset;
	
input       [AW: 0] avs_width_adapter_address;
input       [SBE:0] avs_width_adapter_byteenable;
input               avs_width_adapter_read;
input               avs_width_adapter_write;
input       [SDW:0] avs_width_adapter_writedata;
output      [SDW:0] avs_width_adapter_readdata;
output              avs_width_adapter_readdatavalid;
output              avs_width_adapter_waitrequest;

input       [MDW:0] avm_width_adapter_readdata;
input               avm_width_adapter_readdatavalid;
input               avm_width_adapter_waitrequest;
output      [31: 0] avm_width_adapter_address;
output      [MBE:0] avm_width_adapter_byteenable;
output              avm_width_adapter_read;
output              avm_width_adapter_write;
output      [MDW:0] avm_width_adapter_writedata;

///////////////////////////////////////////////////////////////////////////////
////////////////////////// Local signals definitions //////////////////////////
///////////////////////////////////////////////////////////////////////////////
reg                 half_readdata_valid;
reg         [MDW:0] half_result;

wire                has_pending_transfer;
reg         [AW: 0] pending_address;
reg         [MBE:0] pending_byteenable;
reg                 pending_read;
reg                 pending_write;
reg         [MDW:0] pending_writedata;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////// Output assignments //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
assign avs_width_adapter_readdata       = 
        {avm_width_adapter_readdata, half_result};
assign avs_width_adapter_readdatavalid  = 
        avm_width_adapter_readdatavalid & half_readdata_valid;
assign avs_width_adapter_waitrequest    = 
        has_pending_transfer | avm_width_adapter_waitrequest;

assign has_pending_transfer         = pending_read | pending_write;
assign avm_width_adapter_address    = has_pending_transfer ?
                                        pending_address :
                                        avs_width_adapter_address;
assign avm_width_adapter_byteenable = has_pending_transfer ?
                                        pending_byteenable :
                                        avs_width_adapter_byteenable[1:0];
assign avm_width_adapter_read       = has_pending_transfer ?
                                        pending_read :
                                        avs_width_adapter_read;
assign avm_width_adapter_write      = has_pending_transfer ?
                                        pending_write :
                                        avs_width_adapter_write;
assign avm_width_adapter_writedata  = has_pending_transfer ?
                                        pending_writedata :
                                        avs_width_adapter_writedata[15:0];


///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Internal assignments /////////////////////////////
///////////////////////////////////////////////////////////////////////////////
always @(posedge clk)
begin
    if (reset)
    begin
        half_readdata_valid <= 0;
        half_result         <= 0;
    end
    else if (avm_width_adapter_readdatavalid)
    begin
        half_readdata_valid <= half_readdata_valid ^ 1'b1;
        half_result         <= avm_width_adapter_readdata;
    end
end

always @(posedge clk)
begin
    if (reset)
    begin
        pending_address     <= 0;
        pending_byteenable  <= 0;
        pending_read        <= 0;
        pending_write       <= 0;
        pending_writedata   <= 0;
    end
    else if (has_pending_transfer)
    begin
        if (~avm_width_adapter_waitrequest)
        begin
            pending_read    <= 0;
            pending_write   <= 0;
        end
    end
    else if (~avm_width_adapter_waitrequest)
    begin
        pending_address     <= avs_width_adapter_address | 32'h00000002;
        pending_byteenable  <= avs_width_adapter_byteenable[3:2];
        pending_read        <= avs_width_adapter_read;
        pending_write       <= avs_width_adapter_write;
        pending_writedata   <= avs_width_adapter_writedata[31:16];
    end
end

///////////////////////////////////////////////////////////////////////////////
//////////////////////////// Finite state machines ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Submodules //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


endmodule

