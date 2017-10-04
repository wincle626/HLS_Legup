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
 * This module decodes video streams from the Terasic CCD cameras.            *
 *                                                                            *
 ******************************************************************************/

module altera_up_video_camera_decoder_bayer (
	// Inputs
	clk,
	reset,

	PIXEL_DATA,
	LINE_VALID,
	FRAME_VALID,

	ready,

	// Bidirectional

	// Outputs
	data,
	startofpacket,
	endofpacket,
	valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW = 9;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input			[DW: 0]	PIXEL_DATA;
input						LINE_VALID;
input						FRAME_VALID;

input						ready;

// Bidirectional

// Outputs
output reg	[DW: 0]	data;
output reg				startofpacket;
output reg			 	endofpacket;
output reg		 		valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire						read_temps;

// Internal Registers
reg			[DW: 0]	io_pixel_data;
reg						io_line_valid;
reg						io_frame_valid;

reg						frame_sync;

reg			[DW: 0]	temp_data;
reg						temp_start;
reg						temp_end;
reg						temp_valid;

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/
// Input Registers
always @ (posedge clk)
begin
	io_pixel_data		<= PIXEL_DATA;
	io_line_valid		<= LINE_VALID;
	io_frame_valid		<= FRAME_VALID;
end

// Output Registers
always @ (posedge clk)
begin
	if (reset)
	begin
		data				<= 'h0;
		startofpacket	<= 1'b0;
		endofpacket		<= 1'b0;
		valid				<= 1'b0;
	end
	else if (read_temps)
	begin
		data				<= temp_data;
		startofpacket	<= temp_start;
		endofpacket		<= temp_end;
		valid				<= temp_valid;
	end
	else if (ready)
		valid				<= 1'b0;
end

// Internal Registers
always @ (posedge clk)
begin
	if (reset)
		frame_sync	 	<= 1'b0;
	else if (~io_frame_valid)
		frame_sync 		<= 1'b1;
	else if (io_line_valid & io_frame_valid)
		frame_sync 		<= 1'b0;
end

always @ (posedge clk)
begin
	if (reset)
	begin
		temp_data 		<= 'h0;
		temp_start		<= 1'b0;
		temp_end			<= 1'b0;
		temp_valid		<= 1'b0;
	end
	else if (read_temps)
	begin
		temp_data 		<= io_pixel_data;
		temp_start		<= frame_sync;
		temp_end			<= ~io_frame_valid;
		temp_valid		<= io_line_valid & io_frame_valid;
	end
	else if (~io_frame_valid)
	begin
		temp_end			<= ~io_frame_valid;
	end
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/
// Output Assignments

// Internal Assignments
assign read_temps = (ready | ~valid) & 
	((io_line_valid & io_frame_valid) | 
	 ((temp_start | temp_end) & temp_valid));


/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


endmodule

