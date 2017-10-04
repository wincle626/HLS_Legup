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
 * This module decodes video streams from the Terasic CCD cameras and         * 
 *  outputs the video in RGB format.                                          *
 *                                                                            *
 ******************************************************************************/

module altera_up_video_camera_decoder_RGB (
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

parameter IDW			= 9;
parameter ODW			= 29;

parameter WIDTH		= 1280;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input			[IDW:0]	PIXEL_DATA;
input						LINE_VALID;
input						FRAME_VALID;

input						ready;

// Bidirectional

// Outputs
output reg	[ODW:0]	data;
output reg				startofpacket;
output reg			 	endofpacket;
output reg			 	valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire						read_temps;

wire			[IDW:0]	shift_reg_data;
wire			[IDW:0]	new_green;

// Internal Registers
reg			[IDW:0]	io_pixel_data;
reg						io_line_valid;
reg						io_frame_valid;

reg						last_io_line_valid;
reg			[IDW:0]	last_pixel_data;
reg			[IDW:0]	last_shift_reg_data;

reg						even_line;
reg						even_pixel;
reg						frame_sync;

reg			[ODW:0]	temp_data;
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
	io_pixel_data				<= PIXEL_DATA;
	io_line_valid				<= LINE_VALID;
	io_frame_valid				<= FRAME_VALID;
end

// Output Registers
always @ (posedge clk)
begin
	if (reset)
	begin
		data						<= 'h0;
		startofpacket			<= 1'b0;
		endofpacket				<= 1'b0;
		valid						<= 1'b0;
	end
	else if (read_temps)
	begin
		data						<= temp_data;
		startofpacket			<= temp_start;
		endofpacket				<= temp_end;
		valid						<= temp_valid;
	end
	else if (ready)
		valid						<= 1'b0;
end

// Internal Registers
always @ (posedge clk)
begin
	if (reset)
	begin
		last_io_line_valid	<= 1'b0;
		last_pixel_data		<= 'h0;
		last_shift_reg_data	<= 'h0;
	end
	else if (~io_frame_valid | ~io_line_valid)
	begin
		last_io_line_valid	<= io_line_valid;
		last_pixel_data		<= 'h0;
		last_shift_reg_data	<= 'h0;
	end
	else
	begin
		last_io_line_valid	<= io_line_valid;
		last_pixel_data		<= io_pixel_data;
		last_shift_reg_data	<= shift_reg_data;
	end
end

always @ (posedge clk)
begin
	if (reset)
	begin
		even_line 				<= 1'b0;
		even_pixel 				<= 1'b0;
	end
	else if (~io_frame_valid)
	begin
		even_line 				<= 1'b0;
		even_pixel 				<= 1'b0;
	end
	else if (io_line_valid)
	begin
		even_pixel				<= even_pixel ^ 1'b1;
	end
	else
	begin
		even_pixel				<= 1'b0;
		if (last_io_line_valid)
			even_line 			<= even_line ^ 1'b1;
	end
end

always @ (posedge clk)
begin
	if (reset)
		frame_sync 				<= 1'b0;
	else if (~io_frame_valid)
		frame_sync 				<= 1'b1;
	else if (read_temps)
		frame_sync 				<= 1'b0;
end

always @ (posedge clk)
begin
	if (reset)
	begin
		temp_data 				<= 'h0;
		temp_start				<= 1'b0;
		temp_end					<= 1'b0;
		temp_valid				<= 1'b0;
	end
	else if (read_temps)
	begin
//		temp_data[29:20] 		<= shift_reg_data;
//		temp_data[19:10] 		<= last_shift_reg_data[9:1] + io_pixel_data[9:1];
//		temp_data[ 9: 0] 		<= last_pixel_data;
		temp_data 				<= {shift_reg_data, new_green, last_pixel_data};
		temp_start				<= frame_sync;
		temp_end					<= ~io_frame_valid;
		temp_valid				<= even_pixel & even_line;
	end
	else if (~io_frame_valid)
	begin
		temp_end					<= ~io_frame_valid;
	end
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/
// Output Assignments

// Internal Assignments
assign read_temps = (ready | ~valid) & 
	((even_pixel & even_line) | ((temp_start | temp_end) & temp_valid));

assign new_green	=  
	{1'b0, last_shift_reg_data[IDW:1]} + {1'b0, io_pixel_data[IDW:1]};

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altshift_taps bayern_pattern_shift_reg (
	// Inputs
	.clock		(clk),

	.clken		(io_line_valid & io_frame_valid),
	.shiftin		(io_pixel_data),
	
	.aclr 		(),

	// Outputs
	.shiftout	(shift_reg_data),

	// synopsys translate_off
	.taps			()
	
	// synopsys translate_on
);
defparam
	bayern_pattern_shift_reg.lpm_hint			= "RAM_BLOCK_TYPE=M4K",
	bayern_pattern_shift_reg.lpm_type			= "altshift_taps",
	bayern_pattern_shift_reg.number_of_taps	= 1,
	bayern_pattern_shift_reg.tap_distance		= WIDTH,
	bayern_pattern_shift_reg.width				= (IDW + 1);

endmodule

