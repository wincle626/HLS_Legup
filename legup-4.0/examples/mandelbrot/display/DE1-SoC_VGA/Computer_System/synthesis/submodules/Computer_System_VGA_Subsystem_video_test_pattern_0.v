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
 * This module generators a sample video input stream for DE boards.          *
 *                                                                            *
 ******************************************************************************/

module Computer_System_VGA_Subsystem_video_test_pattern_0 (
	// Inputs
	clk,
	reset,


	ready,
	
	// Bidirectional

	// Outputs
	data,
	startofpacket,
	endofpacket,
	empty,
	valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW							= 23;
parameter WW							= 10;
parameter HW							= 10;

parameter WIDTH						= 1024;
parameter HEIGHT						= 768;

parameter VALUE						= 8'd160;
parameter P_RATE 						= 24'd13653;
parameter TQ_START_RATE 			= 25'd61440;
parameter TQ_RATE_DECELERATION	= 25'd80;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input				clk;
input				reset;


input						ready;

// Bidirectional

// Outputs
output		[DW: 0]	data;
output					startofpacket;
output					endofpacket;
output					empty;
output					valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires

wire			[ 7: 0]	red;
wire			[ 7: 0]	green;
wire			[ 7: 0]	blue;


// Internal Registers
reg			[WW: 0]	x;
reg			[HW: 0]	y;

reg			[10: 0]	hue;
reg			[ 2: 0]	hue_i;

reg			[23: 0]	p;
reg			[24: 0]	q;
reg			[24: 0]	t;

reg			[24: 0]	rate;

// State Machine Registers

// Integers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Color Space Conversion from HSV to RGB
//
// HSV - Hue, Saturation, and Value
//
// Hue			- 0 to 360
// Saturation	- 0 to 1
// Value		- 0 to 1
//
// h_i	= floor (h / 60) mod 6
// f	= (h / 60) - floor (h / 60)
// p	= v * (1 - s) 
// q	= v * (1 - f * s) 
// t	= v * (1 - (1 - f) * s) 
//
//       { (v, t, p) if h_i = 0
//       { (q, v, p) if h_i = 1
// RGB = { (p, v, t) if h_i = 2
//       { (p, q, v) if h_i = 3
//       { (t, p, v) if h_i = 4
//       { (v, p, q) if h_i = 5
//
// Source: http://en.wikipedia.org/wiki/HSL_color_space#Conversion_from_HSV_to_RGB
//

// Output Registers

// Internal Registers
always @(posedge clk)
begin
	if (reset)
		x <= 'h0;
	else if (ready)
	begin
		if (x == (WIDTH - 1))
			x <= 'h0;
		else
			x <= x + 1'b1;
	end
end

always @(posedge clk)
begin
	if (reset)
		y <= 'h0;
	else if (ready && (x == (WIDTH - 1)))
	begin
		if (y == (HEIGHT - 1))
			y <= 'h0;
		else
			y <= y + 1'b1;
	end
end

always @(posedge clk)
begin
	if (reset)
	begin
		hue	<= 'h0;
		hue_i	<= 'h0;
	end
	else if (ready)
	begin
		if (x == (WIDTH - 1))
		begin
			hue	<= 'h0;
			hue_i	<= 'h0;
		end
		else if (hue == ((WIDTH / 6) - 1))
		begin
			hue	<= 'h0;
			hue_i	<= hue_i + 1'b1;
		end
		else
			hue	<= hue + 1'b1;
	end
end

always @(posedge clk)
begin
	if (reset)
	begin
		p		<= 'h0;
		q		<= {1'b0, VALUE, 16'h0000};
		t		<= 'h0;
		rate	<= TQ_START_RATE;
	end
	else if (ready)
	begin
		if ((x == (WIDTH - 1)) && (y == (HEIGHT - 1)))
		begin
			p		<= 'h0;
			rate	<= TQ_START_RATE;
		end
		else if (x == (WIDTH - 1))
		begin
			p		<= p + P_RATE;
			rate	<= rate - TQ_RATE_DECELERATION;
		end

		if ((x == (WIDTH - 1)) && (y == (HEIGHT - 1)))
		begin
			q		<= {1'b0, VALUE, 16'h0000};
			t		<= 'h0;
		end
		else if (x == (WIDTH - 1))
		begin
			q		<= {1'b0, VALUE, 16'h0000};
			t		<= p + P_RATE;
		end
		else if ((hue == ((WIDTH / 6) - 1)) && (hue_i != 5))
		begin
			q		<= {1'b0, VALUE, 16'h0000};
			t		<= p;
		end
		else
		begin
			q		<= q - rate;
			t		<= t + rate;
		end


	end
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments

assign data				= {red, green, blue};
assign startofpacket	= (x == 0) & (y == 0);
assign endofpacket	= (x == (WIDTH - 1)) & (y == (HEIGHT - 1));
assign empty			= 1'b0;
assign valid			= 1'b1;

// Internal Assignments
assign red = 
		(hue_i == 0) ?	VALUE :
		(hue_i == 1) ?	q[23:16] & {8{~q[24]}}: // 8'h00 :
		(hue_i == 2) ?	p[23:16] :
		(hue_i == 3) ?	p[23:16] :
		(hue_i == 4) ?	t[23:16] | {8{t[24]}} : // 8'h00 :
						VALUE;
	
assign green = 
		(hue_i == 0) ?	t[23:16] | {8{t[24]}} : // 8'h00 :
		(hue_i == 1) ?	VALUE :
		(hue_i == 2) ?	VALUE :
		(hue_i == 3) ?	q[23:16] & {8{~q[24]}} : // 8'h00 :
		(hue_i == 4) ?	p[23:16] :
						p[23:16];
assign blue = 
		(hue_i == 0) ?	p[23:16] :
		(hue_i == 1) ?	p[23:16] :
		(hue_i == 2) ?	t[23:16] | {8{t[24]}} : // 8'h00 :
		(hue_i == 3) ?	VALUE :
		(hue_i == 4) ?	VALUE :
						q[23:16] & {8{~q[24]}}; // 8'h00;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


endmodule

