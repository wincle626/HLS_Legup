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
 *   This module generates the clocks needed for the I/O devices on           *
 * Altera's DE-series boards.                                                 *
 *                                                                            *
 ******************************************************************************/

module altera_up_altpll (
	// Inputs
	refclk,
	reset,

	// Bidirectional

	// Outputs
	outclk0,
	outclk1,
	outclk2,
	locked
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter OUTCLK0_MULT	= 1;
parameter OUTCLK0_DIV	= 1;
parameter OUTCLK1_MULT	= 1;
parameter OUTCLK1_DIV	= 1;
parameter OUTCLK2_MULT	= 1;
parameter OUTCLK2_DIV	= 1;

parameter PHASE_SHIFT	= 0;

parameter DEVICE_FAMILY	= "Cyclone IV";

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						refclk;
input						reset;

// Bidirectionals

// Outputs
output					outclk0;
output					outclk1;
output					outclk2;
output					locked;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires

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


/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altpll PLL_for_DE_Series_Boards (
	// Inputs
	.inclk			({1'b0, refclk}),

	// Outputs
	.clk				({outclk2, outclk1, outclk0}),
	.locked			(locked),

	// Unused
	.activeclock	(),
	.areset			(1'b0),
	.clkbad			(),
	.clkena			({6{1'b1}}),
	.clkloss			(),
	.clkswitch		(1'b0),
	.enable0			(),
	.enable1			(),
	.extclk			(),
	.extclkena		({4{1'b1}}),
	.fbin				(1'b1),
	.pfdena			(1'b1),
	.pllena			(1'b1),
	.scanaclr		(1'b0),
	.scanclk			(1'b0),
	.scandata		(1'b0),
	.scandataout	(),
	.scandone		(),
	.scanread		(1'b0),
	.scanwrite		(1'b0),
	.sclkout0		(),
	.sclkout1		()
);
defparam
	PLL_for_DE_Series_Boards.clk0_divide_by				= OUTCLK0_DIV,
	PLL_for_DE_Series_Boards.clk0_duty_cycle				= 50,
	PLL_for_DE_Series_Boards.clk0_multiply_by				= OUTCLK0_MULT,
	PLL_for_DE_Series_Boards.clk0_phase_shift				= "0",
	PLL_for_DE_Series_Boards.clk1_divide_by				= OUTCLK1_DIV,
	PLL_for_DE_Series_Boards.clk1_duty_cycle				= 50,
	PLL_for_DE_Series_Boards.clk1_multiply_by				= OUTCLK1_MULT,
	PLL_for_DE_Series_Boards.clk1_phase_shift				= PHASE_SHIFT,
	PLL_for_DE_Series_Boards.clk2_divide_by				= OUTCLK2_DIV,
	PLL_for_DE_Series_Boards.clk2_duty_cycle				= 50,
	PLL_for_DE_Series_Boards.clk2_multiply_by				= OUTCLK2_MULT,
	PLL_for_DE_Series_Boards.clk2_phase_shift				= "0",
	PLL_for_DE_Series_Boards.compensate_clock				= "CLK0",
	PLL_for_DE_Series_Boards.gate_lock_signal				= "NO",
	PLL_for_DE_Series_Boards.inclk0_input_frequency		= 20000,
	PLL_for_DE_Series_Boards.intended_device_family		= DEVICE_FAMILY,
	PLL_for_DE_Series_Boards.invalid_lock_multiplier	= 5,
	PLL_for_DE_Series_Boards.lpm_type						= "altpll",
	PLL_for_DE_Series_Boards.operation_mode				= "NORMAL",
	PLL_for_DE_Series_Boards.pll_type						= "FAST",
	PLL_for_DE_Series_Boards.port_activeclock				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_areset					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkbad0					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkbad1					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkloss					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkswitch				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_fbin						= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_inclk0					= "PORT_USED",
	PLL_for_DE_Series_Boards.port_inclk1					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_locked					= "PORT_USED",
	PLL_for_DE_Series_Boards.port_pfdena					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_pllena					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_scanaclr					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_scanclk					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_scandata					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_scandataout				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_scandone					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_scanread					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_scanwrite				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clk0						= "PORT_USED",
	PLL_for_DE_Series_Boards.port_clk1						= "PORT_USED",
	PLL_for_DE_Series_Boards.port_clk2						= "PORT_USED",
	PLL_for_DE_Series_Boards.port_clk3						= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clk4						= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clk5						= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkena0					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkena1					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkena2					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkena3					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkena4					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_clkena5					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_enable0					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_enable1					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclk0					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclk1					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclk2					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclk3					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclkena0				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclkena1				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclkena2				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_extclkena3				= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_sclkout0					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.port_sclkout1					= "PORT_UNUSED",
	PLL_for_DE_Series_Boards.valid_lock_multiplier		= 1;

endmodule

