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
 * This module contains a character map for 128 different characters.         *
 *                                                                            *
 ******************************************************************************/

module altera_up_video_128_character_rom (
	// Inputs
	clk,
	clk_en,

	character,
	x_coordinate,
	y_coordinate,
	
	// Bidirectionals

	// Outputs
	character_data
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

input			[ 6: 0]	character;
input			[ 2: 0]	x_coordinate;
input			[ 2: 0]	y_coordinate;

// Bidirectionals

// Outputs
output					character_data;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[12: 0]	character_address;

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

assign character_address = {character, y_coordinate, x_coordinate};

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altsyncram	character_data_rom (
	// Inputs
	.clock0				(clk),
	.clocken0			(clk_en),

	.address_a			(character_address),

	// Bidirectionals

	// Outputs
	.q_a					(character_data),
	
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
	character_data_rom.clock_enable_input_a 					= "NORMAL",
	character_data_rom.clock_enable_output_a	 				= "NORMAL",
	character_data_rom.init_file 									= "altera_up_video_char_mode_rom_128.mif",
	character_data_rom.intended_device_family 				= "Cyclone II",
	character_data_rom.lpm_hint 									= "ENABLE_RUNTIME_MOD=NO",
	character_data_rom.lpm_type			 						= "altsyncram",
	character_data_rom.numwords_a						 			= 8192,
	character_data_rom.operation_mode	 						= "ROM",
	character_data_rom.outdata_aclr_a 							= "NONE",
	character_data_rom.outdata_reg_a 							= "CLOCK0",
	character_data_rom.power_up_uninitialized 				= "FALSE",
	character_data_rom.read_during_write_mode_mixed_ports = "DONT_CARE",
	character_data_rom.widthad_a 									= 13,
	character_data_rom.width_a 									= 1,
	character_data_rom.width_byteena_a							= 1;
	
endmodule
