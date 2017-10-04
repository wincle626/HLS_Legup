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
 * This module convert Bayer Pattern video in regular RGB by joining four     *
 *  adjacent pixels in one. This effectively halfs both the width and height  *
 *  of the incoming video frame.                                              *
 *                                                                            *
 ******************************************************************************/

module altera_up_avalon_video_bayer_resampler (
	// Inputs
	clk,
	reset,

	stream_in_data,
	stream_in_startofpacket,
	stream_in_endofpacket,
	stream_in_empty,
	stream_in_valid,

	stream_out_ready,
	
	// Bidirectional

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

parameter WW		= 11;

parameter IMAGE_WIDTH	= 2592;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input			[ 7: 0]	stream_in_data;
input						stream_in_startofpacket;
input						stream_in_endofpacket;
input						stream_in_empty;
input						stream_in_valid;

input						stream_out_ready;

// Bidirectional

// Outputs
output					stream_in_ready;

output reg	[23: 0]	stream_out_data;
output reg				stream_out_startofpacket;
output reg				stream_out_endofpacket;
output reg	[ 1: 0]	stream_out_empty;
output reg				stream_out_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 7: 0]	shift_reg_data;
wire						transfer_data;

wire			[ 7: 0]	red;
wire			[ 8: 0]	green;
wire			[ 7: 0]	blue;


// Internal Registers
reg						saved_stream_in_startofpacket;

reg			[ 7: 0]	last_stream_in_data;
reg			[ 7: 0]	last_shift_reg_data;

reg			[WW: 0]	width;
reg						pixel_toggle;
reg						line_toggle;

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Output Registers
always @(posedge clk)
begin
	if (reset)
	begin
		stream_out_data				<=  'b0;
		stream_out_startofpacket	<= 1'b0;
		stream_out_endofpacket		<= 1'b0;
		stream_out_empty				<=  'b0;
		stream_out_valid				<= 1'b0;
	end
	else if (stream_out_ready | ~stream_out_valid)
	begin
		stream_out_data				<= {red, green[8:1], blue};
		stream_out_startofpacket	<= saved_stream_in_startofpacket;
		stream_out_endofpacket		<= stream_in_endofpacket;
		stream_out_empty				<= {1'b0, stream_in_empty};
		stream_out_valid				<= stream_in_valid & 
												pixel_toggle & line_toggle;
	end
end

// Internal Registers
always @ (posedge clk)
begin
	if (reset)
		saved_stream_in_startofpacket <= 1'b0;
	else if (stream_in_startofpacket & stream_in_valid)
		saved_stream_in_startofpacket <= 1'b1;
	else if (stream_out_valid)
		saved_stream_in_startofpacket <= 1'b0;
end

always @ (posedge clk)
begin
	if (reset)
	begin
		last_stream_in_data	<= 'h0;
		last_shift_reg_data	<= 'h0;
	end
	else if (transfer_data)
	begin
		last_stream_in_data	<= stream_in_data;
		last_shift_reg_data	<= shift_reg_data;
	end
end

always @ (posedge clk)
begin
	if (reset)
		width <= 'h0;
	else if (transfer_data & stream_in_startofpacket)
		width <= 'h1;
	else if (transfer_data & (width == (IMAGE_WIDTH - 1)))
		width <= 'h0;
	else if (transfer_data)
		width <= width + 1;
end

always @ (posedge clk)
begin
	if (reset)
		pixel_toggle <= 1'b0;
	else if (transfer_data & stream_in_startofpacket)
		pixel_toggle <= 1'b1;
	else if (transfer_data & (width == (IMAGE_WIDTH - 1)))
		pixel_toggle <= 1'b0;
	else if (transfer_data)
		pixel_toggle <= pixel_toggle ^ 1'b1;
end

always @ (posedge clk)
begin
	if (reset)
		line_toggle <= 1'b0;
	else if (transfer_data & (stream_in_startofpacket | stream_in_endofpacket))
		line_toggle <= 1'b0;
	else if (transfer_data & (width == (IMAGE_WIDTH - 1)))
		line_toggle <= line_toggle ^ 1'b1;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments
assign stream_in_ready	= stream_out_ready | ~stream_out_valid;

// Internal Assignments
assign transfer_data	= stream_in_valid & stream_in_ready;

`ifdef USE_RED_GREEN_LINE_FIRST
assign red				= last_stream_in_data;
assign green			= {1'b0, last_shift_reg_data} + {1'b0, stream_in_data};
assign blue				= shift_reg_data;
`else
assign red				= shift_reg_data;
assign green			= {1'b0, last_shift_reg_data} + {1'b0, stream_in_data};
assign blue				= last_stream_in_data;
`endif

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altshift_taps bayern_pattern_shift_reg (
	// Inputs
	.clock		(clk),

	.clken		(transfer_data),
	.shiftin		(stream_in_data),
	
	.aclr 		(),
	
	// Bidirectionals
	
	// Outputs
	.shiftout	(shift_reg_data)

	// synopsys translate_off
	,
	.taps			()
	
	// synopsys translate_on
);
defparam
	bayern_pattern_shift_reg.lpm_hint			= "RAM_BLOCK_TYPE=M4K",
	bayern_pattern_shift_reg.lpm_type			= "altshift_taps",
	bayern_pattern_shift_reg.number_of_taps	= 1,
	bayern_pattern_shift_reg.tap_distance		= IMAGE_WIDTH,
	bayern_pattern_shift_reg.width				= 8;

endmodule

