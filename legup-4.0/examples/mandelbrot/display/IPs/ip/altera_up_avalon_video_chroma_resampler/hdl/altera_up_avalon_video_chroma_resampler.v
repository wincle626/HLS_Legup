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
 * This module converts resamples the chroma components of a video in         *
 *  stream, whos colour space is YCrCb.                                       *
 *                                                                            *
 ******************************************************************************/

module altera_up_avalon_video_chroma_resampler (
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

parameter IDW = 23; // Incoming frame's data width
parameter ODW = 23; // Outcoming frame's data width

parameter IEW =  0; // Incoming frame's empty width
parameter OEW =  0; // Outcoming frame's empty width

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input			[IDW:0]	stream_in_data;
input						stream_in_startofpacket;
input						stream_in_endofpacket;
input			[IEW:0]	stream_in_empty;
input						stream_in_valid;

input						stream_out_ready;

// Bidirectional

// Outputs
output					stream_in_ready;

output reg	[ODW:0]	stream_out_data;
output reg				stream_out_startofpacket;
output reg				stream_out_endofpacket;
output reg	[OEW:0]	stream_out_empty;
output reg				stream_out_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire						transfer_data;

wire			[ODW:0]	converted_data;

wire						converted_startofpacket;
wire						converted_endofpacket;
wire			[OEW:0]	converted_empty;
wire						converted_valid;

// Internal Registers
reg			[IDW:0]	data;
reg						startofpacket;
reg						endofpacket;
reg			[IEW:0]	empty;
reg						valid;

`ifdef USE_422_TO_444
reg			[ 7: 0]	saved_CrCb;
reg						cur_is_Cr_or_Cb;
`elsif USE_444_TO_422
reg						cur_is_Cr_or_Cb;
`endif

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
		stream_out_data				<=  'h0;
		stream_out_startofpacket	<= 1'b0;
		stream_out_endofpacket		<= 1'b0;
		stream_out_empty				<=  'h0;
		stream_out_valid				<= 1'b0;
	end
	else if (transfer_data)
	begin
		stream_out_data				<= converted_data;
		stream_out_startofpacket	<= converted_startofpacket;
		stream_out_endofpacket		<= converted_endofpacket;
		stream_out_empty				<= converted_empty;
		stream_out_valid				<= converted_valid;
	end
end

// Internal Registers
always @(posedge clk)
begin
	if (reset)
	begin
		data								<=  'h0;
		startofpacket					<= 1'b0;
		endofpacket						<= 1'b0;
		empty								<=  'h0;
		valid								<= 1'b0;
	end
	else if (stream_in_ready)
	begin
		data								<= stream_in_data;
		startofpacket					<= stream_in_startofpacket;
		endofpacket						<= stream_in_endofpacket;
		empty								<= stream_in_empty;
		valid								<= stream_in_valid;
	end
	else if (transfer_data)
	begin
		data								<=  'h0;
		startofpacket					<= 1'b0;
		endofpacket						<= 1'b0;
		empty								<=  'h0;
		valid								<= 1'b0;
	end
end

`ifdef USE_422_TO_444
always @(posedge clk)
begin
	if (reset)
		saved_CrCb						<= 8'h00;
	else if (stream_in_ready & stream_in_startofpacket)
		saved_CrCb						<= 8'h00;
	else if (transfer_data & valid)
		saved_CrCb						<= data[15: 8];
end

always @(posedge clk)
begin
	if (reset)
		cur_is_Cr_or_Cb				<= 1'b0;
	else if (stream_in_ready & stream_in_startofpacket)
		cur_is_Cr_or_Cb				<= 1'b0;
	else if (stream_in_ready)
		cur_is_Cr_or_Cb				<= cur_is_Cr_or_Cb ^ 1'b1;
end
`elsif USE_444_TO_422
always @(posedge clk)
begin
	if (reset)
		cur_is_Cr_or_Cb				<= 1'b0;
	else if (stream_in_ready & stream_in_startofpacket)
		cur_is_Cr_or_Cb				<= 1'b0;
	else if (stream_in_ready)
		cur_is_Cr_or_Cb				<= cur_is_Cr_or_Cb ^ 1'b1;
end
`endif

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments
assign stream_in_ready 			= stream_in_valid & (~valid | transfer_data);

// Internal Assignments
assign transfer_data				= 
		~stream_out_valid | (stream_out_ready & stream_out_valid);

`ifdef USE_444_TO_422
assign converted_data[15: 8]	= (cur_is_Cr_or_Cb) ? data[23:16] : data[15: 8];
assign converted_data[ 7: 0]	= data[ 7: 0];
`elsif USE_444_TO_400
assign converted_data[ 7: 0]	= data[ 7: 0];
`elsif USE_422_TO_444
assign converted_data[23:16]	= (cur_is_Cr_or_Cb) ? data[15: 8] : saved_CrCb;
assign converted_data[15: 8]	= (cur_is_Cr_or_Cb) ? saved_CrCb : data[15: 8];
assign converted_data[ 7: 0]	= data[ 7: 0];
`elsif USE_422_TO_400
assign converted_data[ 7: 0]	= data[ 7: 0];
`elsif USE_400_TO_444
assign converted_data[23:16]	= 8'h80;
assign converted_data[15: 8]	= 8'h80;
assign converted_data[ 7: 0]	= data[7:0];
`elsif USE_400_TO_422
assign converted_data[15: 8]	= 8'h80;
assign converted_data[ 7: 0]	= data[7:0];
`else
assign converted_data			= data;
`endif
assign converted_startofpacket	= startofpacket;
assign converted_endofpacket	= endofpacket;
assign converted_empty			= empty;
assign converted_valid			= valid;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


endmodule

