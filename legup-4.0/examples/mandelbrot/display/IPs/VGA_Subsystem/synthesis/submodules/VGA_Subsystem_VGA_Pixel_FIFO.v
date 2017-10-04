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

/******************************************************************************
 *                                                                            *
 * This module is a buffer that can be used the transfer streaming data from  *
 *  one clock domain to another.                                              *
 *                                                                            *
 ******************************************************************************/

module VGA_Subsystem_VGA_Pixel_FIFO (
	// Inputs
	clk_stream_in,
	reset_stream_in,
	clk_stream_out,
	reset_stream_out,

	stream_in_data,
	stream_in_startofpacket,
	stream_in_endofpacket,
	stream_in_empty,
	stream_in_valid,

	stream_out_ready,

	// Bi-Directional

	// Outputs
	stream_in_ready,

	stream_out_data,
	stream_out_startofpacket,
	stream_out_endofpacket,
	stream_out_empty,
	stream_out_valid
);


/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW	= 15; // Frame's data width
parameter EW	= 0; // Frame's empty width

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk_stream_in;
input						reset_stream_in;
input						clk_stream_out;
input						reset_stream_out;

input			[DW: 0]	stream_in_data;
input						stream_in_startofpacket;
input						stream_in_endofpacket;
input			[EW: 0]	stream_in_empty;
input						stream_in_valid;

input						stream_out_ready;

// Bi-Directional

// Outputs
output					stream_in_ready;

output		[DW: 0]	stream_out_data;
output					stream_out_startofpacket;
output					stream_out_endofpacket;
output		[EW: 0]	stream_out_empty;
output					stream_out_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 6: 0]	fifo_wr_used;
wire						fifo_empty;

// Internal Registers

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/


/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output assignments
assign stream_in_ready	= ~(&(fifo_wr_used[6:4]));

assign stream_out_empty	= 'h0;
assign stream_out_valid	= ~fifo_empty;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

dcfifo	Data_FIFO (
	// Inputs
	.wrclk	(clk_stream_in),
	.wrreq	(stream_in_ready & stream_in_valid),
	.data		({stream_in_data, stream_in_endofpacket, stream_in_startofpacket}),

	.rdclk	(clk_stream_out),
	.rdreq	(stream_out_ready & ~fifo_empty),

	// Outputs
	.wrusedw	(fifo_wr_used),
		
	.rdempty	(fifo_empty),
	.q			({stream_out_data, stream_out_endofpacket, stream_out_startofpacket})
	// synopsys translate_off
	,
		
	.aclr		(),
	.wrfull	(),
	.wrempty	(),
	.rdfull	(),
	.rdusedw	()
	// synopsys translate_on
);
defparam
	Data_FIFO.intended_device_family	= "Cyclone II",
	Data_FIFO.lpm_hint					= "MAXIMIZE_SPEED=7",
	Data_FIFO.lpm_numwords				= 128,
	Data_FIFO.lpm_showahead				= "ON",
	Data_FIFO.lpm_type					= "dcfifo",
	Data_FIFO.lpm_width					= DW + 3,
	Data_FIFO.lpm_widthu					= 7,
	Data_FIFO.overflow_checking		= "OFF",
	Data_FIFO.rdsync_delaypipe			= 5,
	Data_FIFO.underflow_checking		= "OFF",
	Data_FIFO.use_eab						= "ON",
	Data_FIFO.wrsync_delaypipe			= 5;

endmodule

