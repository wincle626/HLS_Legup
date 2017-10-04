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
 * This module removes VIP packet from a video stream and passes only RAW     *
 *  video frame packet downstream.                                            *
 *                                                                            *
 ******************************************************************************/

module altera_up_avalon_video_vip_to_raw_bridge (
	// Inputs
	clk,
	reset,

	vip_data,
	vip_startofpacket,
	vip_endofpacket,
	vip_empty,
	vip_valid,

	raw_ready,

	// Bidirectional

	// Outputs
	vip_ready,

	raw_data,
	raw_startofpacket,
	raw_endofpacket,
	raw_empty,
	raw_valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW = 23;
parameter EW = 1;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input			[DW: 0]	vip_data;
input						vip_startofpacket;
input						vip_endofpacket;
input			[EW: 0]	vip_empty;
input						vip_valid;

input						raw_ready;

// Bidirectional

// Outputs
output					vip_ready;

output reg	[DW: 0]	raw_data;
output reg				raw_startofpacket;
output reg				raw_endofpacket;
output reg	[EW: 0]	raw_empty;
output reg				raw_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

localparam	STATE_0_DISCARD_PACKET			= 2'h0,
				STATE_1_START_CONVERT_TO_RAW	= 2'h1,
				STATE_2_CONVERT_TO_RAW			= 2'h2;

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire						incoming_vip_packet_is_frame_data;

// Internal Registers

// State Machine Registers
reg			[ 1: 0]	s_vip_to_raw;
reg			[ 1: 0]	ns_vip_to_raw;

// Integers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		s_vip_to_raw <= STATE_0_DISCARD_PACKET;
	else
		s_vip_to_raw <= ns_vip_to_raw;
end

always @(*)
begin
   case (s_vip_to_raw)
	STATE_0_DISCARD_PACKET:
		begin
			if (incoming_vip_packet_is_frame_data)
				ns_vip_to_raw = STATE_1_START_CONVERT_TO_RAW;
			else
				ns_vip_to_raw = STATE_0_DISCARD_PACKET;
		end
	STATE_1_START_CONVERT_TO_RAW:
		begin
			if ((~raw_valid | raw_ready) & vip_valid)
				ns_vip_to_raw = STATE_2_CONVERT_TO_RAW;
			else
				ns_vip_to_raw = STATE_1_START_CONVERT_TO_RAW;
		end
	STATE_2_CONVERT_TO_RAW:
		begin
			if ((~raw_valid | raw_ready) & 
					vip_endofpacket & vip_valid)
				ns_vip_to_raw = STATE_0_DISCARD_PACKET;
			else
				ns_vip_to_raw = STATE_2_CONVERT_TO_RAW;
		end
	default:
		begin
			ns_vip_to_raw = STATE_0_DISCARD_PACKET;
		end
	endcase
end

/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Output Registers

always @(posedge clk)
begin
	if (reset)
	begin
		raw_data				<= 'h0;
		raw_startofpacket	<= 1'b0;
		raw_endofpacket	<= 1'b0;
		raw_empty			<= 1'b0;
		raw_valid			<= 1'b0;
	end
	else if ((s_vip_to_raw == STATE_1_START_CONVERT_TO_RAW) &
			(~raw_valid | raw_ready))
	begin
		raw_data				<= vip_data;
		raw_startofpacket	<= 1'b1;
		raw_endofpacket	<= vip_endofpacket;
		raw_empty			<= vip_empty;
		raw_valid			<= vip_valid;
	end
	else if ((s_vip_to_raw == STATE_2_CONVERT_TO_RAW) &
			(~raw_valid | raw_ready))
	begin
		raw_data				<= vip_data;
		raw_startofpacket	<= 1'b0;
		raw_endofpacket	<= vip_endofpacket;
		raw_empty			<= vip_empty;
		raw_valid			<= vip_valid;
	end
	else if (raw_ready)
	begin
		raw_valid			<= 1'b0;
	end
end

// Internal Registers


/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments
assign vip_ready	= (s_vip_to_raw == STATE_0_DISCARD_PACKET) ?
   						1'b1 :	
						(~raw_valid | raw_ready);

// Internal Assignments
assign incoming_vip_packet_is_frame_data = 
		vip_valid & vip_startofpacket & (vip_data == 'h0);

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


endmodule

