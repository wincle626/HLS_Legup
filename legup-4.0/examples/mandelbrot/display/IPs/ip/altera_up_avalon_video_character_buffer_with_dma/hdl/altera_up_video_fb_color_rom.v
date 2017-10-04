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
 * This module contains a color map for foreground and background of          *
 *  characters.                                                               *
 *                                                                            *
 ******************************************************************************/

module altera_up_video_fb_color_rom (
	// Inputs
	clk,
	clk_en,

	color_index,
	
	// Bidirectionals

	// Outputs
	red,
	green,
	blue
);


/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/


/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						clk_en;

input			[ 3: 0]	color_index;

// Bidirectionals

// Outputs
output		[ 9: 0]	red;
output		[ 9: 0]	green;
output		[ 9: 0]	blue;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[29: 0]	color_data;

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

assign red		= color_data[29:20];
assign green	= color_data[19:10];
assign blue		= color_data[ 9: 0];

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altsyncram	color_data_rom (
	// Inputs
	.clock0				(clk),
	.clocken0			(clk_en),

	.address_a			(color_index),

	// Bidirectionals

	// Outputs
	.q_a					(color_data),
	
	// Unused 
	.aclr0				(1'b0),
	.aclr1				(1'b0),
	.q_b					(),
	.clocken1			(1'b1),
	.data_b				(1'b1),
	.wren_a				(1'b0),
	.data_a				(1'b1),
	.rden_b				(1'b1),
	.address_b			(1'b1),
	.wren_b				(1'b0),
	.byteena_b			(1'b1),
	.addressstall_a	(1'b0),
	.byteena_a			(1'b1),
	.addressstall_b	(1'b0),
	.clock1				(1'b1)
);
defparam
	color_data_rom.clock_enable_input_a						= "NORMAL",
	color_data_rom.clock_enable_output_a					= "NORMAL",
	color_data_rom.init_file					= "altera_up_video_fb_color_rom.mif",
	color_data_rom.intended_device_family					= "Cyclone II",
	color_data_rom.lpm_hint										= "ENABLE_RUNTIME_MOD=NO",
	color_data_rom.lpm_type										= "altsyncram",
	color_data_rom.numwords_a									= 16,
	color_data_rom.operation_mode								= "ROM",
	color_data_rom.outdata_aclr_a								= "NONE",
	color_data_rom.outdata_reg_a								= "CLOCK0",
	color_data_rom.power_up_uninitialized					= "FALSE",
	color_data_rom.read_during_write_mode_mixed_ports	= "DONT_CARE",
	color_data_rom.widthad_a									= 4,
	color_data_rom.width_a										= 30,
	color_data_rom.width_byteena_a							= 1;
	
endmodule

