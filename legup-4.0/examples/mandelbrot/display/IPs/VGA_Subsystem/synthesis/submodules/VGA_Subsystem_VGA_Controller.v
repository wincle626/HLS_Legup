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
 * This module controls VGA output for Altera's DE1 and DE2 Boards.           *
 *                                                                            *
 ******************************************************************************/

module VGA_Subsystem_VGA_Controller (
	// Inputs
	clk,
	reset,

	data,
	startofpacket,
	endofpacket,
	empty,
	valid,

	// Bidirectionals

	// Outputs
	ready,


	VGA_CLK,
	VGA_BLANK,
	VGA_SYNC,
	VGA_HS,
	VGA_VS,
	VGA_R,
	VGA_G,
	VGA_B
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter CW								= 7;
parameter DW								= 29;

parameter R_UI								= 29;
parameter R_LI								= 22;
parameter G_UI								= 19;
parameter G_LI								= 12;
parameter B_UI								= 9;
parameter B_LI								= 2;

/* Number of pixels */
parameter H_ACTIVE 						= 1024;
parameter H_FRONT_PORCH					=   24;
parameter H_SYNC							=  136;
parameter H_BACK_PORCH 					=  160;
parameter H_TOTAL 						= 1344;

/* Number of lines */
parameter V_ACTIVE 						= 768;
parameter V_FRONT_PORCH					=   3;
parameter V_SYNC							=   6;
parameter V_BACK_PORCH 					=  29;
parameter V_TOTAL							= 806;

parameter LW								= 10;
parameter LINE_COUNTER_INCREMENT		= 10'h001;

parameter PW								= 11;
parameter PIXEL_COUNTER_INCREMENT	= 11'h001;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input			[DW: 0]	data;
input						startofpacket;
input						endofpacket;
input			[ 1: 0]	empty;
input						valid;

// Bidirectionals

// Outputs
output					ready;


output					VGA_CLK;
output reg				VGA_BLANK;
output reg				VGA_SYNC;
output reg				VGA_HS;
output reg				VGA_VS;
output reg	[CW: 0]	VGA_R;
output reg	[CW: 0]	VGA_G;
output reg	[CW: 0]	VGA_B;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

// States
localparam	STATE_0_SYNC_FRAME	= 1'b0,
				STATE_1_DISPLAY		= 1'b1;

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire						read_enable;
wire						end_of_active_frame;

wire						vga_blank_sync;
wire						vga_c_sync;
wire						vga_h_sync;
wire						vga_v_sync;
wire						vga_data_enable;
wire			[CW: 0]	vga_red;
wire			[CW: 0]	vga_green;
wire			[CW: 0]	vga_blue;
wire			[CW: 0]	vga_color_data;


// Internal Registers
reg			[ 3: 0]	color_select; // Use for the TRDB_LCM

// State Machine Registers
reg						ns_mode;
reg						s_mode;

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/

always @(posedge clk)	// sync reset
begin
	if (reset == 1'b1)
		s_mode <= STATE_0_SYNC_FRAME;
	else
		s_mode <= ns_mode;
end

always @(*)
begin
	// Defaults
	ns_mode = STATE_0_SYNC_FRAME;

   case (s_mode)
	STATE_0_SYNC_FRAME:
	begin
		if (valid & startofpacket)
			ns_mode = STATE_1_DISPLAY;
		else
			ns_mode = STATE_0_SYNC_FRAME;
	end
	STATE_1_DISPLAY:
	begin
		if (end_of_active_frame)
			ns_mode = STATE_0_SYNC_FRAME;
		else
			ns_mode = STATE_1_DISPLAY;
	end
	default:
	begin
		ns_mode = STATE_0_SYNC_FRAME;
	end
	endcase
end

/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Output Registers

always @(posedge clk)
begin
	VGA_BLANK	<= vga_blank_sync;
	VGA_SYNC		<= 1'b0;
	VGA_HS		<= vga_h_sync;
	VGA_VS		<= vga_v_sync;
	VGA_R			<= vga_red;
	VGA_G			<= vga_green;
	VGA_B			<= vga_blue;
end


// Internal Registers
always @(posedge clk)
begin
	if (reset)
		color_select <= 4'h1;
	else if (s_mode == STATE_0_SYNC_FRAME)
		color_select <= 4'h1;
	else if (~read_enable)
		color_select <= {color_select[2:0], color_select[3]};
end


/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/
// Output Assignments
assign ready = 
	(s_mode == STATE_0_SYNC_FRAME) ? 
		valid & ~startofpacket : 
		read_enable;

assign VGA_CLK = ~clk;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altera_up_avalon_video_vga_timing VGA_Timing (
	// Inputs
	.clk							(clk),
	.reset						(reset),

	.red_to_vga_display		(data[R_UI:R_LI]),
	.green_to_vga_display	(data[G_UI:G_LI]),
	.blue_to_vga_display		(data[B_UI:B_LI]),
	.color_select				(color_select),

//	.data_valid					(1'b1),

	// Bidirectionals

	// Outputs
	.read_enable				(read_enable),

	.end_of_active_frame		(end_of_active_frame),
	.end_of_frame				(), // (end_of_frame),

	// dac pins
	.vga_blank					(vga_blank_sync),
	.vga_c_sync					(vga_c_sync),
	.vga_h_sync					(vga_h_sync),
	.vga_v_sync					(vga_v_sync),
	.vga_data_enable			(vga_data_enable),
	.vga_red						(vga_red),
	.vga_green					(vga_green),
	.vga_blue					(vga_blue),
	.vga_color_data			(vga_color_data)
);
defparam
	VGA_Timing.CW 									= CW,

	VGA_Timing.H_ACTIVE 							= H_ACTIVE,
	VGA_Timing.H_FRONT_PORCH					= H_FRONT_PORCH,
	VGA_Timing.H_SYNC								= H_SYNC,
	VGA_Timing.H_BACK_PORCH 					= H_BACK_PORCH,
	VGA_Timing.H_TOTAL 							= H_TOTAL,

	VGA_Timing.V_ACTIVE 							= V_ACTIVE,
	VGA_Timing.V_FRONT_PORCH					= V_FRONT_PORCH,
	VGA_Timing.V_SYNC								= V_SYNC,
	VGA_Timing.V_BACK_PORCH		 				= V_BACK_PORCH,
	VGA_Timing.V_TOTAL							= V_TOTAL,

	VGA_Timing.LW									= LW,
	VGA_Timing.LINE_COUNTER_INCREMENT		= LINE_COUNTER_INCREMENT,

	VGA_Timing.PW									= PW,
	VGA_Timing.PIXEL_COUNTER_INCREMENT		= PIXEL_COUNTER_INCREMENT;

endmodule

