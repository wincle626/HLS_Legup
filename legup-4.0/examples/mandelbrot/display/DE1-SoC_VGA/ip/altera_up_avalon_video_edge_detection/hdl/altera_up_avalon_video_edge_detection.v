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
 * This module perform Canny Edge Detection on video streams on the DE        *
 *  boards.                                                                   *
 *                                                                            *
 ******************************************************************************/

module altera_up_avalon_video_edge_detection (
	// Inputs
	clk,
	reset,

	in_data,
	in_startofpacket,
	in_endofpacket,
	in_empty,
	in_valid,

	out_ready,
	
	// Bidirectional

	// Outputs
	in_ready,


	out_data,
	out_startofpacket,
	out_endofpacket,
	out_empty,
	out_valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter WIDTH	= 640; // Image width in pixels

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input			[ 7: 0]	in_data;
input						in_startofpacket;
input						in_endofpacket;
input						in_empty;
input						in_valid;

input						out_ready;

// Bidirectional

// Outputs
output					in_ready;

output reg	[ 7: 0]	out_data;
output reg				out_startofpacket;
output reg				out_endofpacket;
output reg				out_empty;
output reg				out_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire						transfer_data;

wire			[ 8: 0]	filter_1_data_out;  	// Gaussian_Smoothing
wire			[ 9: 0]	filter_2_data_out;  	// Sobel operator
wire			[ 7: 0]	filter_3_data_out;  	// Nonmaximum Suppression
wire			[ 7: 0]	filter_4_data_out;  	// Hyteresis
wire			[ 7: 0]	final_value;			// Intensity Correction

wire			[ 1: 0]	pixel_info_in;
wire			[ 1: 0]	pixel_info_out;

// Internal Registers
reg			[ 7: 0]	data;
reg						startofpacket;
reg						endofpacket;
reg						empty;
reg						valid;

reg						flush_pipeline;

// State Machine Registers

// Integers

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
		out_data				<= 8'h00;
		out_startofpacket	<= 1'b0;
		out_endofpacket	<= 1'b0;
		out_empty			<= 1'b0;
		out_valid			<= 1'b0;
	end
	else if (transfer_data)
	begin
		out_data				<= final_value;
		out_startofpacket	<= pixel_info_out[1] & ~(&(pixel_info_out));
		out_endofpacket	<= pixel_info_out[0] & ~(&(pixel_info_out));
		out_empty			<= 1'b0;
		out_valid			<= (|(pixel_info_out));
	end
	else if (out_ready)
		out_valid			<= 1'b0;
end

// Internal Registers
always @(posedge clk)
begin
	if (reset)
	begin
		data					<= 8'h00;
		startofpacket		<= 1'b0;
		endofpacket			<= 1'b0;
		empty					<= 1'b0;
		valid					<= 1'b0;
	end
	else if (in_ready)
	begin
		data					<= in_data;
		startofpacket		<= in_startofpacket;
		endofpacket			<= in_endofpacket;
		empty					<= in_empty;
		valid					<= in_valid;
	end
end

always @(posedge clk)
begin
	if (reset)
		flush_pipeline		<= 1'b0;
	else if (in_ready & in_endofpacket)
		flush_pipeline		<= 1'b1;
	else if (in_ready & in_startofpacket)
		flush_pipeline		<= 1'b0;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/
// Output Assignments
assign in_ready 			= in_valid & (out_ready | ~out_valid);

// Internal Assignments
assign transfer_data		= in_ready | 
		(flush_pipeline & (out_ready | ~out_valid));

`ifdef USE_DOUBLE_INTENSITY
assign final_value		= 
			{filter_4_data_out[6:0], 1'b0} | {8{filter_4_data_out[7]}};
`else
assign final_value		= filter_4_data_out;
`endif

assign pixel_info_in[1]	= in_valid & ~in_endofpacket; 
assign pixel_info_in[0]	= in_valid & ~in_startofpacket;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altera_up_edge_detection_gaussian_smoothing_filter Filter_1 (
	// Inputs
	.clk			(clk),
	.reset		(reset),

	.data_in		(data),
	.data_en		(transfer_data),

	// Bidirectionals

	// Outputs
	.data_out	(filter_1_data_out)
);
defparam 
	Filter_1.WIDTH = WIDTH;	

altera_up_edge_detection_sobel_operator Filter_2 (
	// Inputs
	.clk			(clk),
	.reset		(reset),

	.data_in		(filter_1_data_out),
	.data_en		(transfer_data),

	// Bidirectionals

	// Outputs
	.data_out	(filter_2_data_out)
);
defparam 
	Filter_2.WIDTH = WIDTH;	

altera_up_edge_detection_nonmaximum_suppression Filter_3 (
	// Inputs
	.clk			(clk),
	.reset		(reset),

	.data_in		(filter_2_data_out),
	.data_en		(transfer_data),

	// Bidirectionals

	// Outputs
	.data_out	(filter_3_data_out)
);
defparam 
	Filter_3.WIDTH = WIDTH;	

altera_up_edge_detection_hysteresis Filter_4 (
	// Inputs
	.clk			(clk),
	.reset		(reset),

	.data_in		(filter_3_data_out),
	.data_en		(transfer_data),

	// Bidirectionals

	// Outputs
	.data_out	(filter_4_data_out)
);
defparam 
	Filter_4.WIDTH = WIDTH;	

altera_up_edge_detection_pixel_info_shift_register Pixel_Info_Shift_Register (
	// Inputs
	.clock		(clk),
	.clken		(transfer_data),
	
	.shiftin		(pixel_info_in),

	// Bidirectionals

	// Outputs
	.shiftout	(pixel_info_out),
	.taps			()
);
// defparam Pixel_Info_Shift_Register.SIZE = WIDTH;	
defparam 
	Pixel_Info_Shift_Register.SIZE = (WIDTH * 5) + 28;	

endmodule

