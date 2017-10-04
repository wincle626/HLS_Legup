/******************************************************************************
 * License Agreement                                                          *
 *                                                                            *
 * Copyright (c) 1991-2013 Altera Corporation, San Jose, California, USA.     *
 * All rights reserved.                                                       *
 *                                                                            *
 * Any megafunction design, and related net list (encrypted or decrypted),    *
 *  support information, device programming or simulation file, and any other *
 *  associated documentation or information provided by Altera or a partner   *
 *  under Altera's Megafunction Partnership Program may be used only to       *
 *  program PLD devices (but not masked PLD devices) from Altera.  Any other  *
 *  use of such megafunction design, net list, support information, device    *
 *  programming or simulation file, or any other related documentation or     *
 *  information is prohibited for any other purpose, including, but not       *
 *  limited to modification, reverse engineering, de-compiling, or use with   *
 *  any other silicon devices, unless such use is explicitly licensed under   *
 *  a separate agreement with Altera or a megafunction partner.  Title to     *
 *  the intellectual property, including patents, copyrights, trademarks,     *
 *  trade secrets, or maskworks, embodied in any such megafunction design,    *
 *  net list, support information, device programming or simulation file, or  *
 *  any other related documentation or information provided by Altera or a    *
 *  megafunction partner, remains with Altera, the megafunction partner, or   *
 *  their respective licensors.  No other licenses, including any licenses    *
 *  needed under any third party's intellectual property, are provided herein.*
 *  Copying or modifying any file, or portion thereof, to which this notice   *
 *  is attached violates this copyright.                                      *
 *                                                                            *
 * THIS FILE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
 * FROM, OUT OF OR IN CONNECTION WITH THIS FILE OR THE USE OR OTHER DEALINGS  *
 * IN THIS FILE.                                                              *
 *                                                                            *
 * This agreement shall be governed in all respects by the laws of the State  *
 *  of California and by the laws of the United States of America.            *
 *                                                                            *
 ******************************************************************************/

module altera_up_video_dma_to_stream (
	// Inputs
	clk,
	reset,

	stream_ready,

	master_readdata,
	master_readdatavalid,
	master_waitrequest,
	
	reading_first_pixel_in_frame,
	reading_last_pixel_in_frame,

	// Bidirectional

	// Outputs
	stream_data,
	stream_startofpacket,
	stream_endofpacket,
	stream_empty,
	stream_valid,

	master_arbiterlock,
	master_read,

	inc_address,
	reset_address
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW	=  15; // Frame's datawidth
parameter EW	=   0; // Frame's empty width

parameter MDW	=  15; // Avalon master's datawidth

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input						stream_ready;

input			[MDW:0]	master_readdata;
input						master_readdatavalid;
input						master_waitrequest;
	
input						reading_first_pixel_in_frame;
input						reading_last_pixel_in_frame;

// Bidirectional

// Outputs
output		[DW: 0]	stream_data;
output					stream_startofpacket;
output					stream_endofpacket;
output		[EW: 0]	stream_empty;
output					stream_valid;

output					master_arbiterlock;
output					master_read;

output					inc_address;
output					reset_address;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

// states
localparam	STATE_0_IDLE							= 2'h0,
				STATE_1_WAIT_FOR_LAST_PIXEL		= 2'h1,
				STATE_2_READ_BUFFER					= 2'h2,
				STATE_3_MAX_PENDING_READS_STALL	= 2'h3;

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire		[(DW+2):0]	fifo_data_in;
wire						fifo_read;
wire						fifo_write;

wire		[(DW+2):0]	fifo_data_out;
wire						fifo_empty;
wire						fifo_full;
wire						fifo_almost_empty;
wire						fifo_almost_full;

// Internal Registers
reg			[ 3: 0]	pending_reads;
reg						startofpacket;

// State Machine Registers
reg			[ 1: 0]	s_dma_to_stream;
reg			[ 1: 0]	ns_dma_to_stream;

// Integers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		s_dma_to_stream <= STATE_0_IDLE;
	else
		s_dma_to_stream <= ns_dma_to_stream;
end

always @(*)
begin
   case (s_dma_to_stream)
	STATE_0_IDLE:
		begin
			if (fifo_almost_empty)
				ns_dma_to_stream = STATE_2_READ_BUFFER;
			else
				ns_dma_to_stream = STATE_0_IDLE;
		end
	STATE_1_WAIT_FOR_LAST_PIXEL:
		begin
			if (pending_reads == 4'h0) 
				ns_dma_to_stream = STATE_0_IDLE;
			else
				ns_dma_to_stream = STATE_1_WAIT_FOR_LAST_PIXEL;
		end
	STATE_2_READ_BUFFER:
		begin
			if (~master_waitrequest)
			begin
				if (reading_last_pixel_in_frame)
					ns_dma_to_stream = STATE_1_WAIT_FOR_LAST_PIXEL;
				else if (fifo_almost_full) 
					ns_dma_to_stream = STATE_0_IDLE;
				else if (pending_reads >= 4'hC)
					ns_dma_to_stream = STATE_3_MAX_PENDING_READS_STALL;
				else
					ns_dma_to_stream = STATE_2_READ_BUFFER;
			end
			else
				ns_dma_to_stream = STATE_2_READ_BUFFER;
		end
	STATE_3_MAX_PENDING_READS_STALL:
		begin
			if (pending_reads <= 4'h7) 
				ns_dma_to_stream = STATE_2_READ_BUFFER;
			else if (fifo_almost_full) 
				ns_dma_to_stream = STATE_0_IDLE;
			else
				ns_dma_to_stream = STATE_3_MAX_PENDING_READS_STALL;
		end
	default:
		begin
			ns_dma_to_stream = STATE_0_IDLE;
		end
	endcase
end

/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Output Registers

// Internal Registers
always @(posedge clk)
begin
	if (reset)
		pending_reads <= 4'h0;
	else if (master_read & ~master_waitrequest)
	begin
		if (~master_readdatavalid)
			pending_reads <= pending_reads + 1'h1;
	end
	else if (master_readdatavalid & (pending_reads != 4'h0))
		pending_reads <= pending_reads - 1'h1;
end

always @(posedge clk)
begin
	if (reset)
		startofpacket <= 1'b0;
	else if ((s_dma_to_stream == STATE_0_IDLE) & (reading_first_pixel_in_frame))
		startofpacket <= 1'b1;
	else if (master_readdatavalid)
		startofpacket <= 1'b0;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/
// Output Assignments
assign stream_data				= fifo_data_out[DW:0];
assign stream_startofpacket	= fifo_data_out[DW+1];
assign stream_endofpacket		= fifo_data_out[DW+2];
assign stream_empty				= 'h0;
assign stream_valid				= ~fifo_empty;

assign master_arbiterlock		= !((s_dma_to_stream == STATE_2_READ_BUFFER) |
		(s_dma_to_stream == STATE_3_MAX_PENDING_READS_STALL));
assign master_read				= (s_dma_to_stream == STATE_2_READ_BUFFER);

assign inc_address				= master_read & ~master_waitrequest;
assign reset_address				= inc_address & reading_last_pixel_in_frame;

// Internal Assignments
assign fifo_data_in[DW:0]		= master_readdata[DW:0];
assign fifo_data_in[DW+1]		= startofpacket;
assign fifo_data_in[DW+2]		= (s_dma_to_stream == STATE_1_WAIT_FOR_LAST_PIXEL) & 
											(pending_reads == 4'h1);
assign fifo_write					= master_readdatavalid & ~fifo_full;

assign fifo_read					= stream_ready & stream_valid;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

scfifo Image_Buffer (
	// Inputs
	.clock			(clk),
	.sclr				(reset),
   
	.data				(fifo_data_in),
	.wrreq			(fifo_write),

	.rdreq			(fifo_read),

	// Outputs
	.q					(fifo_data_out),

	.empty			(fifo_empty),
	.full				(fifo_full),
	   
	.almost_empty	(fifo_almost_empty),
	.almost_full	(fifo_almost_full),
	// synopsys translate_off
	
	.aclr				(),
	.usedw			()
	// synopsys translate_on
);
defparam
	Image_Buffer.add_ram_output_register	= "OFF",
	Image_Buffer.almost_empty_value			= 32,
	Image_Buffer.almost_full_value			= 96,
	Image_Buffer.intended_device_family		= "Cyclone II",
	Image_Buffer.lpm_numwords					= 128,
	Image_Buffer.lpm_showahead					= "ON",
	Image_Buffer.lpm_type						= "scfifo",
	Image_Buffer.lpm_width						= DW + 3,
	Image_Buffer.lpm_widthu						= 7,
	Image_Buffer.overflow_checking			= "OFF",
	Image_Buffer.underflow_checking			= "OFF",
	Image_Buffer.use_eab							= "ON";

endmodule

