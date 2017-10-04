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

module altera_up_video_alpha_blender_normal (
	// Inputs
	background_data,
	foreground_data,

	// Bidirectionals

	// Outputs
	new_red,
	new_green,
	new_blue
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/


/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input			[29: 0]	background_data;
input			[39: 0]	foreground_data;

// Bidirectionals

// Outputs
output		[ 9: 0]	new_red;
output		[ 9: 0]	new_green;
output		[ 9: 0]	new_blue;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 9: 0] one_minus_a;

wire			[17: 0] r_x_alpha;
wire			[17: 0] g_x_alpha;
wire			[17: 0] b_x_alpha;
wire			[17: 0] r_x_one_minus_alpha;
wire			[17: 0] g_x_one_minus_alpha;
wire			[17: 0] b_x_one_minus_alpha;

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
assign new_red		= {1'b0, r_x_alpha[17:9]} + 
					  {1'b0, r_x_one_minus_alpha[17:9]};
assign new_green	= {1'b0, g_x_alpha[17:9]} + 
					  {1'b0, g_x_one_minus_alpha[17:9]};
assign new_blue	= {1'b0, b_x_alpha[17:9]} + 
					  {1'b0, b_x_one_minus_alpha[17:9]};

// Internal Assignments
assign one_minus_a	= 10'h3FF - foreground_data[39:30];

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

lpm_mult r_times_alpha (
	// Inputs
	.clock	(1'b0),
	.clken	(1'b1),
	.aclr		(1'b0),
	.sum		(1'b0),
	.dataa	(foreground_data[29:21]),
	.datab	(foreground_data[39:31]),

	// Outputs
	.result	(r_x_alpha)
);
defparam
	r_times_alpha.lpm_hint								= "MAXIMIZE_SPEED=5",
	r_times_alpha.lpm_representation					= "UNSIGNED",
	r_times_alpha.lpm_type								= "LPM_MULT",
	r_times_alpha.lpm_widtha							= 9,
	r_times_alpha.lpm_widthb							= 9,
	r_times_alpha.lpm_widthp							= 18;

lpm_mult g_times_alpha (
	// Inputs
	.clock	(1'b0),
	.clken	(1'b1),
	.aclr		(1'b0),
	.sum		(1'b0),
	.dataa	(foreground_data[19:11]),
	.datab	(foreground_data[39:31]),

	// Outputs
	.result	(g_x_alpha)
);
defparam
	g_times_alpha.lpm_hint								= "MAXIMIZE_SPEED=5",
	g_times_alpha.lpm_representation					= "UNSIGNED",
	g_times_alpha.lpm_type								= "LPM_MULT",
	g_times_alpha.lpm_widtha							= 9,
	g_times_alpha.lpm_widthb							= 9,
	g_times_alpha.lpm_widthp							= 18;

lpm_mult b_times_alpha (
	// Inputs
	.clock	(1'b0),
	.clken	(1'b1),
	.aclr		(1'b0),
	.sum		(1'b0),
	.dataa	(foreground_data[ 9: 1]),
	.datab	(foreground_data[39:31]),

	// Outputs
	.result	(b_x_alpha)
);
defparam
	b_times_alpha.lpm_hint								= "MAXIMIZE_SPEED=5",
	b_times_alpha.lpm_representation					= "UNSIGNED",
	b_times_alpha.lpm_type								= "LPM_MULT",
	b_times_alpha.lpm_widtha							= 9,
	b_times_alpha.lpm_widthb							= 9,
	b_times_alpha.lpm_widthp							= 18;

lpm_mult r_times_one_minus_alpha (
	// Inputs
	.clock	(1'b0),
	.clken	(1'b1),
	.aclr		(1'b0),
	.sum		(1'b0),
	.dataa	(background_data[29:21]),
	.datab	(one_minus_a[ 9: 1]),

	// Outputs
	.result	(r_x_one_minus_alpha)
);
defparam
	r_times_one_minus_alpha.lpm_hint					= "MAXIMIZE_SPEED=5",
	r_times_one_minus_alpha.lpm_representation	= "UNSIGNED",
	r_times_one_minus_alpha.lpm_type					= "LPM_MULT",
	r_times_one_minus_alpha.lpm_widtha				= 9,
	r_times_one_minus_alpha.lpm_widthb				= 9,
	r_times_one_minus_alpha.lpm_widthp				= 18;

lpm_mult g_times_one_minus_alpha (
	// Inputs
	.clock	(1'b0),
	.clken	(1'b1),
	.aclr		(1'b0),
	.sum		(1'b0),
	.dataa	(background_data[19:11]),
	.datab	(one_minus_a[ 9: 1]),

	// Outputs
	.result	(g_x_one_minus_alpha)
);
defparam
	g_times_one_minus_alpha.lpm_hint					= "MAXIMIZE_SPEED=5",
	g_times_one_minus_alpha.lpm_representation	= "UNSIGNED",
	g_times_one_minus_alpha.lpm_type					= "LPM_MULT",
	g_times_one_minus_alpha.lpm_widtha				= 9,
	g_times_one_minus_alpha.lpm_widthb				= 9,
	g_times_one_minus_alpha.lpm_widthp				= 18;

lpm_mult b_times_one_minus_alpha (
	// Inputs
	.clock	(1'b0),
	.clken	(1'b1),
	.aclr		(1'b0),
	.sum		(1'b0),
	.dataa	(background_data[ 9: 1]),
	.datab	(one_minus_a[ 9: 1]),

	// Outputs
	.result	(b_x_one_minus_alpha)
);
defparam
	b_times_one_minus_alpha.lpm_hint					= "MAXIMIZE_SPEED=5",
	b_times_one_minus_alpha.lpm_representation	= "UNSIGNED",
	b_times_one_minus_alpha.lpm_type					= "LPM_MULT",
	b_times_one_minus_alpha.lpm_widtha				= 9,
	b_times_one_minus_alpha.lpm_widthb				= 9,
	b_times_one_minus_alpha.lpm_widthp				= 18;

endmodule

