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
 * This module replicates lines in video data to enlarge the image.           *
 *                                                                            *
 ******************************************************************************/

module altera_up_video_scaler_multiply_height (
	// Inputs
	clk,
	reset,

	stream_in_data,
	stream_in_startofpacket,
	stream_in_endofpacket,
	stream_in_valid,

	stream_out_ready,

	// Bi-Directional

	// Outputs
	stream_in_ready,

	stream_out_data,
	stream_out_startofpacket,
	stream_out_endofpacket,
	stream_out_valid
);


/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW		=  15; // Image's data width
parameter WW		=   8; // Image width's address width
parameter WIDTH	= 320; // Image's width in pixels

parameter CW		=   0; // Multiply height's counter width

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input			[DW: 0]	stream_in_data;
input						stream_in_startofpacket;
input						stream_in_endofpacket;
input						stream_in_valid;

input						stream_out_ready;

// Bi-Directional

// Outputs
output					stream_in_ready;

output reg	[DW: 0]	stream_out_data;
output reg				stream_out_startofpacket;
output reg				stream_out_endofpacket;
output reg				stream_out_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

localparam	STATE_0_GET_CURRENT_LINE	= 2'h0,
				STATE_1_LOOP_FIFO				= 2'h1,
				STATE_2_OUTPUT_LAST_LINE	= 2'h2;

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire		[(DW + 2): 0]	fifo_data_in;
wire		[(DW + 2): 0]	fifo_data_out;


wire						fifo_empty;
wire						fifo_full;
wire						fifo_read;
wire						fifo_write;

// Internal Registers
reg			[WW: 0]	width_in;
reg			[WW: 0]	width_out;
reg			[CW: 0]	enlarge_height_counter;

// State Machine Registers
reg			[ 1: 0]	s_multiply_height;
reg			[ 1: 0]	ns_multiply_height;

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		s_multiply_height <= STATE_0_GET_CURRENT_LINE;
	else
		s_multiply_height <= ns_multiply_height;
end

always @(*)
begin
   case (s_multiply_height)
	STATE_0_GET_CURRENT_LINE:
		begin
			if (width_in == WIDTH)
				ns_multiply_height = STATE_1_LOOP_FIFO;
			else
				ns_multiply_height = STATE_0_GET_CURRENT_LINE;
		end
	STATE_1_LOOP_FIFO:
		begin
			if (fifo_read & (width_out == (WIDTH - 1)) & 
					(&(enlarge_height_counter | 1'b1)))
				ns_multiply_height = STATE_2_OUTPUT_LAST_LINE;
			else
				ns_multiply_height = STATE_1_LOOP_FIFO;
		end
	STATE_2_OUTPUT_LAST_LINE:
		begin
			if (fifo_read & (width_out == (WIDTH - 1)))
				ns_multiply_height = STATE_0_GET_CURRENT_LINE;
			else
				ns_multiply_height = STATE_2_OUTPUT_LAST_LINE;
		end
	default:
		begin
			ns_multiply_height = STATE_0_GET_CURRENT_LINE;
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
		stream_out_data					<=  'h0;
		stream_out_startofpacket		<= 1'b0;
		stream_out_endofpacket			<= 1'b0;
		stream_out_valid					<= 1'b0;
	end
	else if (stream_out_ready | ~stream_out_valid)
	begin
		stream_out_data					<= fifo_data_out[DW : 0];

		if (|(enlarge_height_counter))
			stream_out_startofpacket	<= 1'b0;
		else
			stream_out_startofpacket	<= fifo_data_out[DW + 1];

		if (&(enlarge_height_counter))
			stream_out_endofpacket		<= fifo_data_out[DW + 2];
		else
			stream_out_endofpacket		<= 1'b0;

		if (s_multiply_height == STATE_0_GET_CURRENT_LINE)
			stream_out_valid				<= 1'b0;
		else
			stream_out_valid				<= ~fifo_empty;
	end
end

// Internal Registers
always @(posedge clk)
begin
	if (reset)
		width_in <= 'h0;
	else if (s_multiply_height == STATE_1_LOOP_FIFO)
		width_in <= 'h0;
	else if (stream_in_ready & stream_in_valid)
		width_in <= width_in + 1;
end

always @(posedge clk)
begin
	if (reset)
		width_out <= 'h0;
	else if (fifo_read)
	begin
		if (width_out == (WIDTH - 1))
			width_out <= 'h0;
		else
			width_out <= width_out + 1;
	end
end

always @(posedge clk)
begin
	if (reset)
		enlarge_height_counter <= 'h0;
	else if (s_multiply_height == STATE_0_GET_CURRENT_LINE)
		enlarge_height_counter <= 'h0;
	else if (fifo_read & (width_out == (WIDTH - 1)))
		enlarge_height_counter <= enlarge_height_counter + 1;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output assignments
assign stream_in_ready			= 
		(s_multiply_height == STATE_1_LOOP_FIFO) ? 
			1'b0 : 
			~fifo_full & ~(width_in == WIDTH);

// Internal assignments
assign fifo_data_in[DW : 0]	= 
		(s_multiply_height == STATE_1_LOOP_FIFO) ?
			fifo_data_out[DW : 0] :
			stream_in_data;
assign fifo_data_in[DW + 1]	=
		(s_multiply_height == STATE_1_LOOP_FIFO) ?
			fifo_data_out[DW + 1] :
			stream_in_startofpacket;
assign fifo_data_in[DW + 2]	=
		(s_multiply_height == STATE_1_LOOP_FIFO) ?
			fifo_data_out[DW + 2] :
			stream_in_endofpacket;

assign fifo_write					= 
		(s_multiply_height == STATE_1_LOOP_FIFO) ?
			fifo_read :
			stream_in_ready & stream_in_valid & ~fifo_full;
assign fifo_read					=
		(s_multiply_height == STATE_0_GET_CURRENT_LINE) ?
			1'b0 :
			(stream_out_ready | ~stream_out_valid) & ~fifo_empty;
		
/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

scfifo Multiply_Height_FIFO (
	// Inputs
	.clock			(clk),
	.sclr				(reset),
   
	.data				(fifo_data_in),
	.wrreq			(fifo_write),

	.rdreq			(fifo_read),

	// Outputs
	.q					(fifo_data_out),
	.empty			(fifo_empty),
	.full				(fifo_full),
	   
	// synopsys translate_off
	
	.aclr				(),
	.almost_empty	(),
	.almost_full	(),
	.usedw			()
	// synopsys translate_on
);
defparam
	Multiply_Height_FIFO.add_ram_output_register	= "OFF",
	Multiply_Height_FIFO.intended_device_family	= "Cyclone II",
	Multiply_Height_FIFO.lpm_numwords				= WIDTH + 1,
	Multiply_Height_FIFO.lpm_showahead				= "ON",
	Multiply_Height_FIFO.lpm_type						= "scfifo",
	Multiply_Height_FIFO.lpm_width					= DW + 3,
	Multiply_Height_FIFO.lpm_widthu					= WW + 1,
	Multiply_Height_FIFO.overflow_checking			= "OFF",
	Multiply_Height_FIFO.underflow_checking		= "OFF",
	Multiply_Height_FIFO.use_eab						= "ON";

endmodule

