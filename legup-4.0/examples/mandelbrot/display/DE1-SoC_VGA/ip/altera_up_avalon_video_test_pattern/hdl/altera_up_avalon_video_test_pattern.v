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

module altera_up_avalon_video_test_pattern (
	// Inputs
	clk,
	reset,

`ifdef USE_HSV_VALUE
	value,
`endif

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
parameter WW							= 8;
parameter HW							= 7;

parameter WIDTH						= 320;
parameter HEIGHT						= 240;

`ifndef USE_HSV_VALUE
parameter VALUE						=  8'd192;
parameter P_RATE 						= 24'd52155;
parameter TQ_START_RATE 			= 25'd234700;
parameter TQ_RATE_DECELERATION	= 25'd977;
`endif

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input				clk;
input				reset;

`ifdef USE_HSV_VALUE
input			[ 7: 0]	value;
`endif

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
`ifdef USE_HSV_VALUE
wire			[ 7: 0]	v;
`endif

wire			[ 7: 0]	red;
wire			[ 7: 0]	green;
wire			[ 7: 0]	blue;

`ifdef USE_HSV_VALUE
wire			[15: 0]	p_rate;
wire			[16: 0]	tq_start_rate;
wire			[ 8: 0]	tq_rate_deceleration;
`endif

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

`ifdef USE_HSV_VALUE
always @(posedge clk)
begin
	if (reset)
	begin
		p		<= 'h0;
		q		<= {1'b0, v, 16'h0000};
		t		<= 'h0;
		rate	<= {8'h00, tq_start_rate};
	end
	else if (ready)
	begin
		if ((x == (WIDTH - 1)) && (y == (HEIGHT - 1)))
		begin
			p		<= 'h0;
			rate	<= {8'h00, tq_start_rate};
		end
		else if (x == (WIDTH - 1))
		begin
			p		<= p + {8'h00, p_rate};
			rate	<= rate - {16'h0000, tq_rate_deceleration};
		end

		if ((x == (WIDTH - 1)) && (y == (HEIGHT - 1)))
		begin
			q		<= {1'b0, v, 16'h0000};
			t		<= 'h0;
		end
		else if (x == (WIDTH - 1))
		begin
			q		<= {1'b0, v, 16'h0000};
			t		<= p + {8'h00, p_rate};
		end
		else if ((hue == ((WIDTH / 6) - 1)) && (hue_i != 5))
		begin
			q		<= {1'b0, v, 16'h0000};
			t		<= p;
		end
		else
		begin
			q		<= q - rate;
			t		<= t + rate;
		end


	end
end
`else
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
`endif

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments
`ifdef USE_HSV_VALUE
assign v					= value;
`endif

assign data				= {red, green, blue};
assign startofpacket	= (x == 0) & (y == 0);
assign endofpacket	= (x == (WIDTH - 1)) & (y == (HEIGHT - 1));
assign empty			= 1'b0;
assign valid			= 1'b1;

// Internal Assignments
`ifdef USE_HSV_VALUE
assign red = 
		(hue_i == 0) ?	v :
		(hue_i == 1) ?	q[23:16] & {8{~q[24]}}: // 8'h00 :
		(hue_i == 2) ?	p[23:16] :
		(hue_i == 3) ?	p[23:16] :
		(hue_i == 4) ?	t[23:16] | {8{t[24]}} : // 8'h00 :
						v;
	
assign green = 
		(hue_i == 0) ?	t[23:16] | {8{t[24]}} : // 8'h00 :
		(hue_i == 1) ?	v :
		(hue_i == 2) ?	v :
		(hue_i == 3) ?	q[23:16] & {8{~q[24]}} : // 8'h00 :
		(hue_i == 4) ?	p[23:16] :
						p[23:16];
assign blue = 
		(hue_i == 0) ?	p[23:16] :
		(hue_i == 1) ?	p[23:16] :
		(hue_i == 2) ?	t[23:16] | {8{t[24]}} : // 8'h00 :
		(hue_i == 3) ?	v :
		(hue_i == 4) ?	v :
						q[23:16] & {8{~q[24]}}; // 8'h00;
`else
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
`endif

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

`ifdef USE_HSV_VALUE
altsyncram	P_Rate_ROM (
	// Inputs
	.clock0				(clk),
	.address_a			(v),

	// Outputs
	.q_a					(p_rate),

	// Unused
	.aclr0				(1'b0),
	.aclr1				(1'b0),
	.address_b			(1'b1),
	.addressstall_a	(1'b0),
	.addressstall_b	(1'b0),
	.byteena_a			(1'b1),
	.byteena_b			(1'b1),
	.clock1				(1'b1),
	.clocken0			(1'b1),
	.clocken1			(1'b1),
	.clocken2			(1'b1),
	.clocken3			(1'b1),
	.data_a				({16{1'b1}}),
	.data_b				(1'b1),
	.eccstatus			(),
	.q_b					(),
	.rden_a				(1'b1),
	.rden_b				(1'b1),
	.wren_a				(1'b0),
	.wren_b				(1'b0)
);
defparam
	P_Rate_ROM.clock_enable_input_a		= "BYPASS",
	P_Rate_ROM.clock_enable_output_a		= "BYPASS",
	P_Rate_ROM.init_file						= "p_rate.mif",
	P_Rate_ROM.intended_device_family	= "Cyclone II",
	P_Rate_ROM.lpm_hint						= "ENABLE_RUNTIME_MOD=NO",
	P_Rate_ROM.lpm_type						= "altsyncram",
	P_Rate_ROM.numwords_a					= 256,
	P_Rate_ROM.operation_mode				= "ROM",
	P_Rate_ROM.outdata_aclr_a				= "NONE",
	P_Rate_ROM.outdata_reg_a				= "CLOCK0",
	P_Rate_ROM.widthad_a						= 8,
	P_Rate_ROM.width_a						= 16,
	P_Rate_ROM.width_byteena_a				= 1;

altsyncram	TQ_Start_Rate_ROM (
	// Inputs
	.clock0				(clk),
	.address_a			(v),

	// Outputs
	.q_a					(tq_start_rate),

	// Unused
	.aclr0				(1'b0),
	.aclr1				(1'b0),
	.address_b			(1'b1),
	.addressstall_a	(1'b0),
	.addressstall_b	(1'b0),
	.byteena_a			(1'b1),
	.byteena_b			(1'b1),
	.clock1				(1'b1),
	.clocken0			(1'b1),
	.clocken1			(1'b1),
	.clocken2			(1'b1),
	.clocken3			(1'b1),
	.data_a				({17{1'b1}}),
	.data_b				(1'b1),
	.eccstatus			(),
	.q_b					(),
	.rden_a				(1'b1),
	.rden_b				(1'b1),
	.wren_a				(1'b0),
	.wren_b				(1'b0)
);
defparam
	TQ_Start_Rate_ROM.clock_enable_input_a		= "BYPASS",
	TQ_Start_Rate_ROM.clock_enable_output_a	= "BYPASS",
	TQ_Start_Rate_ROM.init_file					= "tq_rate.mif",
	TQ_Start_Rate_ROM.intended_device_family	= "Cyclone II",
	TQ_Start_Rate_ROM.lpm_hint						= "ENABLE_RUNTIME_MOD=NO",
	TQ_Start_Rate_ROM.lpm_type						= "altsyncram",
	TQ_Start_Rate_ROM.numwords_a					= 256,
	TQ_Start_Rate_ROM.operation_mode				= "ROM",
	TQ_Start_Rate_ROM.outdata_aclr_a				= "NONE",
	TQ_Start_Rate_ROM.outdata_reg_a				= "CLOCK0",
	TQ_Start_Rate_ROM.widthad_a					= 8,
	TQ_Start_Rate_ROM.width_a						= 17,
	TQ_Start_Rate_ROM.width_byteena_a			= 1;

altsyncram	TQ_Rate_Deceleration_ROM (
	// Inputs
	.clock0				(clk),
	.address_a			(v),

	// Outputs
	.q_a					(tq_rate_deceleration),

	// Unused
	.aclr0				(1'b0),
	.aclr1				(1'b0),
	.address_b			(1'b1),
	.addressstall_a	(1'b0),
	.addressstall_b	(1'b0),
	.byteena_a			(1'b1),
	.byteena_b			(1'b1),
	.clock1				(1'b1),
	.clocken0			(1'b1),
	.clocken1			(1'b1),
	.clocken2			(1'b1),
	.clocken3			(1'b1),
	.data_a				({9{1'b1}}),
	.data_b				(1'b1),
	.eccstatus			(),
	.q_b					(),
	.rden_a				(1'b1),
	.rden_b				(1'b1),
	.wren_a				(1'b0),
	.wren_b				(1'b0)
);
defparam
	TQ_Rate_Deceleration_ROM.clock_enable_input_a	= "BYPASS",
	TQ_Rate_Deceleration_ROM.clock_enable_output_a	= "BYPASS",
	TQ_Rate_Deceleration_ROM.init_file					= "tq_accelerate.mif",
	TQ_Rate_Deceleration_ROM.intended_device_family	= "Cyclone II",
	TQ_Rate_Deceleration_ROM.lpm_hint					= "ENABLE_RUNTIME_MOD=NO",
	TQ_Rate_Deceleration_ROM.lpm_type					= "altsyncram",
	TQ_Rate_Deceleration_ROM.numwords_a					= 256,
	TQ_Rate_Deceleration_ROM.operation_mode			= "ROM",
	TQ_Rate_Deceleration_ROM.outdata_aclr_a			= "NONE",
	TQ_Rate_Deceleration_ROM.outdata_reg_a				= "CLOCK0",
	TQ_Rate_Deceleration_ROM.widthad_a					= 8,
	TQ_Rate_Deceleration_ROM.width_a						= 9,
	TQ_Rate_Deceleration_ROM.width_byteena_a			= 1;
`endif

endmodule

