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
 * This module splits video in streams for the DE-series boards.              *
 *                                                                            *
 ******************************************************************************/

module altera_up_avalon_video_stream_splitter (
	// Inputs
	clk,
	reset,

`ifdef USE_SYNC
	sync_ready,

`endif
	stream_in_data,
	stream_in_startofpacket,
	stream_in_endofpacket,
	stream_in_empty,
	stream_in_valid,

	stream_out_ready_0,

	stream_out_ready_1,

	stream_select,
	
	// Bidirectional

	// Outputs
`ifdef USE_SYNC
	sync_data,
	sync_valid,

`endif
	stream_in_ready,

	stream_out_data_0,
	stream_out_startofpacket_0,
	stream_out_endofpacket_0,
	stream_out_empty_0,
	stream_out_valid_0,
	
	stream_out_data_1,
	stream_out_startofpacket_1,
	stream_out_endofpacket_1,
	stream_out_empty_1,
	stream_out_valid_1
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW = 15; // Frame's data width
parameter EW =  0; // Frame's empty width

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

`ifdef USE_SYNC
input						sync_ready;

`endif
input			[DW: 0]	stream_in_data;
input						stream_in_startofpacket;
input						stream_in_endofpacket;
input			[EW: 0]	stream_in_empty;
input						stream_in_valid;

input						stream_out_ready_0;

input						stream_out_ready_1;

input						stream_select;

// Bidirectional

// Outputs
`ifdef USE_SYNC
output reg				sync_data;
output reg				sync_valid;

`endif
output					stream_in_ready;

output reg	[DW: 0]	stream_out_data_0;
output reg				stream_out_startofpacket_0;
output reg				stream_out_endofpacket_0;
output reg	[EW: 0]	stream_out_empty_0;
output reg				stream_out_valid_0;

output reg	[DW: 0]	stream_out_data_1;
output reg				stream_out_startofpacket_1;
output reg				stream_out_endofpacket_1;
output reg	[EW: 0]	stream_out_empty_1;
output reg				stream_out_valid_1;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire						enable_setting_stream_select;

// Internal Registers
reg						between_frames;
reg						stream_select_reg;

// State Machine Registers

// Integers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Output Registers
`ifdef USE_SYNC
always @(posedge clk)
begin
	if (reset)
	begin
		sync_data	<= 1'b0;
		sync_valid	<= 1'b0;
	end
	else if (enable_setting_stream_select)
	begin
		sync_data	<= stream_select;
		sync_valid	<= 1'b1;
	end
	else if (sync_ready)
		sync_valid	<= 1'b0;
end
`endif

always @(posedge clk)
begin
	if (reset)
	begin
		stream_out_data_0				<=  'h0;
		stream_out_startofpacket_0	<= 1'b0;
		stream_out_endofpacket_0	<= 1'b0;
		stream_out_empty_0			<=  'h0;
		stream_out_valid_0			<= 1'b0;
	end
	else if (stream_in_ready & ~stream_select_reg)
	begin
		stream_out_data_0				<= stream_in_data;
		stream_out_startofpacket_0	<= stream_in_startofpacket;
		stream_out_endofpacket_0	<= stream_in_endofpacket;
		stream_out_empty_0			<= stream_in_empty;
		stream_out_valid_0			<= stream_in_valid;
	end
	else if (stream_out_ready_0)
		stream_out_valid_0			<= 1'b0;
end

always @(posedge clk)
begin
	if (reset)
	begin
		stream_out_data_1				<=  'h0;
		stream_out_startofpacket_1	<= 1'b0;
		stream_out_endofpacket_1	<= 1'b0;
		stream_out_empty_1			<=  'h0;
		stream_out_valid_1			<= 1'b0;
	end
	else if (stream_in_ready & stream_select_reg)
	begin
		stream_out_data_1				<= stream_in_data;
		stream_out_startofpacket_1	<= stream_in_startofpacket;
		stream_out_endofpacket_1	<= stream_in_endofpacket;
		stream_out_empty_1			<= stream_in_empty;
		stream_out_valid_1			<= stream_in_valid;
	end
	else if (stream_out_ready_1)
		stream_out_valid_1			<= 1'b0;
end

// Internal Registers
always @(posedge clk)
begin
	if (reset)
		between_frames <= 1'b1;
	else if (stream_in_ready & stream_in_endofpacket)
		between_frames <= 1'b1;
	else if (stream_in_ready & stream_in_startofpacket)
		between_frames <= 1'b0;
end

always @(posedge clk)
begin
	if (reset)
		stream_select_reg <= 1'b0;
	else if (enable_setting_stream_select)
		stream_select_reg <= stream_select;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments
assign stream_in_ready = (stream_select_reg) ? 
			stream_in_valid & (~stream_out_valid_1 | stream_out_ready_1) :
			stream_in_valid & (~stream_out_valid_0 | stream_out_ready_0);

// Internal Assignments
assign enable_setting_stream_select = 
		  (stream_in_ready & stream_in_endofpacket) |
		(~(stream_in_ready & stream_in_startofpacket) & between_frames);

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


endmodule

