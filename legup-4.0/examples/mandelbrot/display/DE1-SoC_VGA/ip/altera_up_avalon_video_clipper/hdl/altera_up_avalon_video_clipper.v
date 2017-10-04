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
 * This module clips video streams on the DE boards.                          *
 *                                                                            *
 ******************************************************************************/
//`define USE_CLIPPER_DROP

module altera_up_avalon_video_clipper (
	// Inputs
	clk,
	reset,

`ifdef USE_CLIPPER_DROP
	stream_in_data,
	stream_in_startofpacket,
	stream_in_endofpacket,
	stream_in_empty,
	stream_in_valid,
`endif

	stream_out_ready,
	
	// Bidirectional

	// Outputs
`ifdef USE_CLIPPER_DROP
	stream_in_ready,
`endif


	stream_out_data,
	stream_out_startofpacket,
	stream_out_endofpacket,
	stream_out_empty,
	stream_out_valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW							=  15; // Frame's data width
parameter EW							=   0; // Frame's empty width

`ifdef USE_CLIPPER_DROP
parameter WIDTH_IN					= 640; // Incoming frame's width in pixels
parameter HEIGHT_IN					= 480; // Incoming frame's height in lines
parameter WW_IN						=   9; // Incoming frame's width's address width
parameter HW_IN						=   8; // Incoming frame's height's address width

parameter DROP_PIXELS_AT_START	=   0;
parameter DROP_PIXELS_AT_END		=   0;
parameter DROP_LINES_AT_START		=   0;
parameter DROP_LINES_AT_END		=   0;
`endif

parameter WIDTH_OUT					= 640; // Final frame's width in pixels
parameter HEIGHT_OUT					= 480; // Final frame's height in lines
parameter WW_OUT						=   9; // Final frame's width's address width
parameter HW_OUT						=   8; // Final frame's height's address width

parameter ADD_PIXELS_AT_START		=   0;
parameter ADD_PIXELS_AT_END		=   0;
parameter ADD_LINES_AT_START		=   0;
parameter ADD_LINES_AT_END			=   0;

parameter ADD_DATA					= 16'h0; // Data value for added pixels

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

`ifdef USE_CLIPPER_DROP
input			[DW: 0]	stream_in_data;
input						stream_in_startofpacket;
input						stream_in_endofpacket;
input			[EW: 0]	stream_in_empty;
input						stream_in_valid;
`endif

input						stream_out_ready;

// Bidirectional

// Outputs
`ifdef USE_CLIPPER_DROP
output					stream_in_ready;
`endif

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
`ifdef USE_CLIPPER_DROP
wire			[DW: 0]	internal_data;
wire						internal_startofpacket;
wire						internal_endofpacket;
wire			[EW: 0]	internal_empty;
wire						internal_valid;

wire						internal_ready;
`endif

// Internal Registers

// State Machine Registers

// Integers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Output Registers

// Internal Registers

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/
// Output Assignments

// Internal Assignments

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

`ifdef USE_CLIPPER_DROP
altera_up_video_clipper_drop Clipper_Drop (
	// Inputs
	.clk								(clk),
	.reset							(reset),

	.stream_in_data				(stream_in_data),
	.stream_in_startofpacket	(stream_in_startofpacket),
	.stream_in_endofpacket		(stream_in_endofpacket),
	.stream_in_empty				(stream_in_empty),
	.stream_in_valid				(stream_in_valid),

	.stream_out_ready				(internal_ready),
	
	// Bidirectional

	// Outputs
	.stream_in_ready				(stream_in_ready),


	.stream_out_data				(internal_data),
	.stream_out_startofpacket	(internal_startofpacket),
	.stream_out_endofpacket		(internal_endofpacket),
	.stream_out_empty				(internal_empty),
	.stream_out_valid				(internal_valid)
);
defparam
	Clipper_Drop.DW							= DW,
	Clipper_Drop.EW							= EW,

	Clipper_Drop.IMAGE_WIDTH				= WIDTH_IN,
	Clipper_Drop.IMAGE_HEIGHT				= HEIGHT_IN,
	Clipper_Drop.WW							= WW_IN,
	Clipper_Drop.HW							= HW_IN,

	Clipper_Drop.DROP_PIXELS_AT_START	= DROP_PIXELS_AT_START,
	Clipper_Drop.DROP_PIXELS_AT_END		= DROP_PIXELS_AT_END,
	Clipper_Drop.DROP_LINES_AT_START		= DROP_LINES_AT_START,
	Clipper_Drop.DROP_LINES_AT_END 		= DROP_LINES_AT_END,

	Clipper_Drop.ADD_DATA					= ADD_DATA;
`endif

altera_up_video_clipper_add Clipper_Add (
	// Inputs
	.clk								(clk),
	.reset							(reset),

`ifdef USE_CLIPPER_DROP
	.stream_in_data				(internal_data),
	.stream_in_startofpacket	(internal_startofpacket),
	.stream_in_endofpacket		(internal_endofpacket),
	.stream_in_empty				(internal_empty),
	.stream_in_valid				(internal_valid),
`else
//	.stream_in_data				('h0),
//	.stream_in_startofpacket	('b0),
//	.stream_in_endofpacket		('b0),
//	.stream_in_empty				('h0),
//	.stream_in_valid				('b0),
`endif

	.stream_out_ready				(stream_out_ready),

	// Bidirectional

	// Outputs
`ifdef USE_CLIPPER_DROP
	.stream_in_ready				(internal_ready),
`else
	.stream_in_ready				(),
`endif

	.stream_out_data				(stream_out_data),
	.stream_out_startofpacket	(stream_out_startofpacket),
	.stream_out_endofpacket		(stream_out_endofpacket),
	.stream_out_empty				(stream_out_empty),
	.stream_out_valid				(stream_out_valid)
);
defparam
	Clipper_Add.DW							= DW,
	Clipper_Add.EW							= EW,

	Clipper_Add.IMAGE_WIDTH				= WIDTH_OUT,
	Clipper_Add.IMAGE_HEIGHT			= HEIGHT_OUT,
	Clipper_Add.WW							= WW_OUT,
	Clipper_Add.HW							= HW_OUT,

	Clipper_Add.ADD_PIXELS_AT_START	= ADD_PIXELS_AT_START,
	Clipper_Add.ADD_PIXELS_AT_END		= ADD_PIXELS_AT_END,
	Clipper_Add.ADD_LINES_AT_START	= ADD_LINES_AT_START,
	Clipper_Add.ADD_LINES_AT_END 		= ADD_LINES_AT_END,

	Clipper_Add.ADD_DATA					= ADD_DATA;

endmodule

