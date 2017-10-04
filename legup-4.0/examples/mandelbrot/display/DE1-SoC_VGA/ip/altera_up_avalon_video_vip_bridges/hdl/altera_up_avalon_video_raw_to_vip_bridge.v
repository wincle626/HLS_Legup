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
 * This module convert a RAW video packet stream into a VIP video packet      *
 *  stream.                                                                   *
 *                                                                            *
 ******************************************************************************/

//`define USE_1_SYMBOL_PER_BEAT
//`define USE_2_SYMBOLS_PER_BEAT
//`define USE_3_SYMBOLS_PER_BEAT

module altera_up_avalon_video_raw_to_vip_bridge (
	// Inputs
	clk,
	reset,

	raw_data,
	raw_startofpacket,
	raw_endofpacket,
	raw_empty,
	raw_valid,

	vip_ready,

	// Bidirectional

	// Outputs
	raw_ready,

	vip_data,
	vip_startofpacket,
	vip_endofpacket,
	vip_empty,
	vip_valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW				= 23;
parameter EW				= 1;

parameter CTRL_PACKET_0	=  'hF;
parameter CTRL_PACKET_1	= 8'h0;
parameter CTRL_PACKET_2	= 8'h2;
parameter CTRL_PACKET_3	= 8'h8;
parameter CTRL_PACKET_4	= 8'h0;
parameter CTRL_PACKET_5	= 8'h0;
parameter CTRL_PACKET_6	= 8'h1;
parameter CTRL_PACKET_7	= 8'hE;
parameter CTRL_PACKET_8	= 8'h0;
parameter CTRL_PACKET_9	= 8'h0;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input			[DW: 0]	raw_data;
input						raw_startofpacket;
input						raw_endofpacket;
input			[EW: 0]	raw_empty;
input						raw_valid;

input						vip_ready;

// Bidirectional

// Outputs
output					raw_ready;

output reg	[DW: 0]	vip_data;
output reg				vip_startofpacket;
output reg				vip_endofpacket;
output reg	[EW: 0]	vip_empty;
output reg				vip_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

localparam	STATE_0_CREATE_CTRL_PACKET		= 2'h0,
				STATE_1_START_CONVERT_TO_VIP	= 2'h1,
				STATE_2_CONVERT_TO_VIP			= 2'h2;

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire			[DW: 0]	ctrl_packet_data;

// Internal Registers
reg			[ 3: 0]	ctrl_packet_counter;

// State Machine Registers
reg			[ 1: 0]	s_raw_to_vip;
reg			[ 1: 0]	ns_raw_to_vip;

// Integers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		s_raw_to_vip <= STATE_0_CREATE_CTRL_PACKET;
	else
		s_raw_to_vip <= ns_raw_to_vip;
end

always @(*)
begin
   case (s_raw_to_vip)
	STATE_0_CREATE_CTRL_PACKET:
		begin
`ifdef USE_1_SYMBOL_PER_BEAT
			if ((~vip_valid | vip_ready) & (ctrl_packet_counter == 4'h9))
`elsif USE_2_SYMBOLS_PER_BEAT
			if ((~vip_valid | vip_ready) & (ctrl_packet_counter == 4'h5))
`else
			if ((~vip_valid | vip_ready) & (ctrl_packet_counter == 4'h3))
`endif
				ns_raw_to_vip = STATE_1_START_CONVERT_TO_VIP;
			else
				ns_raw_to_vip = STATE_0_CREATE_CTRL_PACKET;
		end
	STATE_1_START_CONVERT_TO_VIP:
		begin
			if (~vip_valid | vip_ready)
				ns_raw_to_vip = STATE_2_CONVERT_TO_VIP;
			else
				ns_raw_to_vip = STATE_1_START_CONVERT_TO_VIP;
		end
	STATE_2_CONVERT_TO_VIP:
		begin
			if ((~vip_valid | vip_ready) & raw_endofpacket & raw_valid)
				ns_raw_to_vip = STATE_0_CREATE_CTRL_PACKET;
			else
				ns_raw_to_vip = STATE_2_CONVERT_TO_VIP;
		end
	default:
		begin
			ns_raw_to_vip = STATE_0_CREATE_CTRL_PACKET;
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
		vip_data					<= 'h0;
		vip_startofpacket		<= 1'b0;
		vip_endofpacket		<= 1'b0;
		vip_empty				<= 'h0;
		vip_valid				<= 1'b0;
	end
	else if ((s_raw_to_vip == STATE_0_CREATE_CTRL_PACKET) &
			(~vip_valid | vip_ready))
	begin
		vip_data					<= ctrl_packet_data;
		vip_startofpacket		<= (ctrl_packet_counter == 4'h0);
`ifdef USE_1_SYMBOL_PER_BEAT
		vip_endofpacket		<= (ctrl_packet_counter == 4'h9);
`elsif USE_2_SYMBOLS_PER_BEAT
		vip_endofpacket		<= (ctrl_packet_counter == 4'h5);
`else
		vip_endofpacket		<= (ctrl_packet_counter == 4'h3);
`endif
		vip_empty				<= 'h0;
		vip_valid				<= 1'b1;
	end
	else if ((s_raw_to_vip == STATE_1_START_CONVERT_TO_VIP) &
			(~vip_valid | vip_ready))
	begin
		vip_data					<= 'h0;
		vip_startofpacket		<= 1'b1;
		vip_endofpacket		<= 1'b0;
		vip_empty				<= 'h0;
		vip_valid				<= 1'b1;
	end
	else if ((s_raw_to_vip == STATE_2_CONVERT_TO_VIP) &
			(~vip_valid | vip_ready))
	begin
		vip_data					<= raw_data;
		vip_startofpacket		<= 1'b0;
		vip_endofpacket		<= raw_endofpacket;
		vip_empty				<= raw_empty;
		vip_valid				<= raw_valid;
	end
	else if (vip_ready)
	begin
		vip_valid				<= 1'b0;
	end
end

// Internal Registers
always @(posedge clk)
begin
	if (reset)
		ctrl_packet_counter 	<= 4'h0;
	else if ((s_raw_to_vip == STATE_0_CREATE_CTRL_PACKET) &
			(~vip_valid | vip_ready))
		ctrl_packet_counter 	<= ctrl_packet_counter + 4'h1;
	else if (s_raw_to_vip != STATE_0_CREATE_CTRL_PACKET)
		ctrl_packet_counter 	<= 4'h0;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments
assign raw_ready		= (s_raw_to_vip == STATE_2_CONVERT_TO_VIP) ?
   							(~vip_valid | vip_ready) :	
								1'b0;

// Internal Assignments
`ifdef USE_1_SYMBOL_PER_BEAT
assign ctrl_packet_data	=
		(ctrl_packet_counter == 4'h0) ?	CTRL_PACKET_0 : 
		(ctrl_packet_counter == 4'h1) ?	CTRL_PACKET_1 : 
		(ctrl_packet_counter == 4'h2) ?	CTRL_PACKET_2 : 
		(ctrl_packet_counter == 4'h3) ?	CTRL_PACKET_3 : 
		(ctrl_packet_counter == 4'h4) ?	CTRL_PACKET_4 : 
		(ctrl_packet_counter == 4'h5) ?	CTRL_PACKET_5 : 
		(ctrl_packet_counter == 4'h6) ?	CTRL_PACKET_6 : 
		(ctrl_packet_counter == 4'h7) ?	CTRL_PACKET_7 : 
		(ctrl_packet_counter == 4'h8) ?	CTRL_PACKET_8 : 
													CTRL_PACKET_9;
`elsif USE_2_SYMBOLS_PER_BEAT
assign ctrl_packet_data	=
		(ctrl_packet_counter == 4'h0) ?	CTRL_PACKET_0 : 
		(ctrl_packet_counter == 4'h1) ?	{CTRL_PACKET_2, CTRL_PACKET_1} : 
		(ctrl_packet_counter == 4'h2) ?	{CTRL_PACKET_4, CTRL_PACKET_3} : 
		(ctrl_packet_counter == 4'h3) ?	{CTRL_PACKET_6, CTRL_PACKET_5} : 
		(ctrl_packet_counter == 4'h4) ?	{CTRL_PACKET_8, CTRL_PACKET_7} : 
													CTRL_PACKET_9; 
`elsif USE_3_SYMBOLS_PER_BEAT
assign ctrl_packet_data	=
		(ctrl_packet_counter == 4'h0) ?	CTRL_PACKET_0 : 
		(ctrl_packet_counter == 4'h1) ?	{CTRL_PACKET_3, CTRL_PACKET_2, CTRL_PACKET_1} : 
		(ctrl_packet_counter == 4'h2) ?	{CTRL_PACKET_6, CTRL_PACKET_5, CTRL_PACKET_4} : 
													{CTRL_PACKET_9, CTRL_PACKET_8, CTRL_PACKET_7}; 
`else
assign ctrl_packet_data	=
		(ctrl_packet_counter == 4'h0) ?	CTRL_PACKET_0 : 
		(ctrl_packet_counter == 4'h1) ?	{CTRL_PACKET_4, CTRL_PACKET_3, CTRL_PACKET_2, CTRL_PACKET_1} : 
		(ctrl_packet_counter == 4'h2) ?	{CTRL_PACKET_8, CTRL_PACKET_7, CTRL_PACKET_6, CTRL_PACKET_5} : 
													CTRL_PACKET_9; 
`endif

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


endmodule

