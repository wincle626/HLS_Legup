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
 * This module combines two video streams by overlaying one onto the          *
 *  other using alpha blending.  The foreground image must include alpha      *
 *  bits to be used in the blending formula: Cn = (1 - a)Cb + (a)Cf           *
 *  Cn - new color                                                            *
 *  a  - alpha                                                                *
 *  Cb - background colour                                                    *
 *  Cf - foreground colour                                                    *
 *                                                                            *
 ******************************************************************************/

module altera_up_avalon_video_alpha_blender (
	// Inputs
	clk,
	reset,

	background_data,
	background_startofpacket,
	background_endofpacket,
	background_empty,
	background_valid,
	
	foreground_data,
	foreground_startofpacket,
	foreground_endofpacket,
	foreground_empty,
	foreground_valid,

	output_ready,

	// Bidirectionals

	// Outputs
	background_ready,

	foreground_ready,
	
	output_data,
	output_startofpacket,
	output_endofpacket,
	output_empty,
	output_valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/


/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input			[29: 0]	background_data;
input						background_startofpacket;
input						background_endofpacket;
input			[ 1: 0]	background_empty;
input						background_valid;

input			[39: 0]	foreground_data;
input						foreground_startofpacket;
input						foreground_endofpacket;
input			[ 1: 0]	foreground_empty;
input						foreground_valid;

input						output_ready;

// Bidirectionals

// Outputs
output					background_ready;

output					foreground_ready;

output		[29: 0]	output_data;
output					output_startofpacket;
output					output_endofpacket;
output		[ 1: 0]	output_empty;
output					output_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 9: 0]	new_red;
wire			[ 9: 0]	new_green;
wire			[ 9: 0]	new_blue;

wire						sync_foreground;
wire						sync_background;

wire						valid;

// Internal Registers

// State Machine Registers

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
assign background_ready = (output_ready & output_valid) | sync_background;
assign foreground_ready = (output_ready & output_valid) | sync_foreground;

assign output_data				= {new_red, new_green, new_blue};
assign output_startofpacket 	= foreground_startofpacket;
assign output_endofpacket		= foreground_endofpacket;
assign output_empty				= 2'h0;
assign output_valid				= valid;

// Internal Assignments
assign sync_foreground = (foreground_valid & background_valid &
			((background_startofpacket & ~foreground_startofpacket) |
			(background_endofpacket & ~foreground_endofpacket)));
assign sync_background = (foreground_valid & background_valid &
			((foreground_startofpacket & ~background_startofpacket) |
			(foreground_endofpacket & ~background_endofpacket)));

assign valid =	foreground_valid & background_valid & 
				~sync_foreground & ~sync_background;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

`ifdef USE_NORMAL_MODE
altera_up_video_alpha_blender_normal alpha_blender (
	// Inputs
	.background_data	(background_data),
	.foreground_data	(foreground_data),

	// Bidirectionals

	// Outputs
	.new_red				(new_red),
	.new_green			(new_green),
	.new_blue			(new_blue)
);
`else
altera_up_video_alpha_blender_simple alpha_blender (
	// Inputs
	.background_data	(background_data),
	.foreground_data	(foreground_data),

	// Bidirectionals

	// Outputs
	.new_red				(new_red),
	.new_green			(new_green),
	.new_blue			(new_blue)
);
`endif


endmodule

