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
 * This module is a buffer that holds characters to be displayed on a         *
 *  VGA or LCD screen.                                                        *
 *                                                                            *
 ******************************************************************************/

module altera_up_avalon_video_character_buffer_with_dma (
	// Inputs
	clk,
	reset,

	ctrl_address,
	ctrl_byteenable,
	ctrl_chipselect,
	ctrl_read,
	ctrl_write,
	ctrl_writedata,

	buf_address,
	buf_byteenable,
	buf_chipselect,
	buf_read,
	buf_write,
	buf_writedata,
	
	stream_ready,

	// Bidirectionals

	// Outputs
	ctrl_readdata,

	buf_readdata,
	buf_waitrequest,
	
	stream_data,
	stream_startofpacket,
	stream_endofpacket,
	stream_empty,
	stream_valid
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter DW				= 8;

parameter ENLARGE_CHAR	= 0;

`ifdef USE_TRDB_LCM
parameter AW				= 11;
parameter BUFFER_SIZE	= 2048;

parameter PIXELS 			= 320;
parameter LINES 			= 240;
`elsif USE_TRDB_LTM
parameter AW				= 11;
parameter BUFFER_SIZE	= 2048;

parameter PIXELS 			= 800;
parameter LINES 			= 480;
`else
parameter AW				= 13;
parameter BUFFER_SIZE	= 8192;

parameter PIXELS 			= 640;
parameter LINES 			= 480;
`endif

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input						ctrl_address;
input			[ 3: 0]	ctrl_byteenable;
input						ctrl_chipselect;
input						ctrl_read;
input						ctrl_write;
input			[31: 0]	ctrl_writedata;

input		[(AW-1): 0]	buf_address;
input						buf_byteenable;
input						buf_chipselect;
input						buf_read;
input						buf_write;
input			[ 7: 0]	buf_writedata;

input						stream_ready;

// Bidirectionals

// Outputs
output reg	[31: 0]	ctrl_readdata;

output reg	[ 7: 0]	buf_readdata;
output					buf_waitrequest;

`ifdef USE_TRANSPARENCY
output		[39: 0]	stream_data;
`else
output		[29: 0]	stream_data;
`endif
output					stream_startofpacket;
output					stream_endofpacket;
output		[ 1: 0]	stream_empty;
output					stream_valid;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

//localparam NUMBER_OF_BITS_FOR_X_COORD	= 10;
//localparam NUMBER_OF_BITS_FOR_Y_COORD	= 9;

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[DW: 1]	char_data_to_buffer;
wire			[DW: 1]	char_data_from_buffer;

wire			[AW: 1]	cur_char_position;
wire			[15: 0]	cur_char_for_display;

wire						cur_char_data;

`ifdef USE_FOREGROUND_COLOR
wire			[ 9: 0]	fg_R;
wire			[ 9: 0]	fg_G;
wire			[ 9: 0]	fg_B;
`endif

`ifdef USE_BACKGROUND_COLOR
wire			[ 9: 0]	bg_R;
wire			[ 9: 0]	bg_G;
wire			[ 9: 0]	bg_B;
`endif

wire			[ 9: 0]	char_red;
wire			[ 9: 0]	char_green;
wire			[ 9: 0]	char_blue;

// Internal Registers
reg			[31: 0]	control_reg;

reg			[ 1: 0] delayed_buf_waitrequest;

reg						clear_screen;

reg			[ 9: 0]	x_position;
reg			[ 8: 0]	y_position;
reg			[ 5: 0]	delayed_x_position;
reg			[ 5: 0]	delayed_y_position;

reg			[ 3: 0]	delayed_startofpacket;
reg			[ 3: 0]	delayed_endofpacket;

`ifdef USE_9BIT_COLOR
reg			[17: 0]	delayed_char_color;
`endif

// State Machine Registers

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
		ctrl_readdata <= 32'h00000000;
	else if (ctrl_chipselect & ctrl_read & ctrl_address)
`ifdef USE_TRDB_LCM
		ctrl_readdata <= {16'd30, 16'd40};
`elsif USE_TRDB_LTM
		ctrl_readdata <= {16'd30, 16'd50};
`else
		ctrl_readdata <= {16'd60, 16'd80};
`endif
	else if (ctrl_chipselect & ctrl_read)
		ctrl_readdata <= control_reg;
end

always @(posedge clk)
begin
	if (reset)
		buf_readdata <= 8'h00;
	else if (buf_chipselect & buf_read)
		buf_readdata <= {1'b0, char_data_from_buffer[7:1]};
end

// Internal Registers
always @(posedge clk)
begin
	if (reset)
		control_reg <= 32'h00010000;
	else if (ctrl_chipselect & ctrl_write & ~ctrl_address)
	begin
		if (ctrl_byteenable[0]) 
			control_reg[ 7: 0] <= ctrl_writedata[ 7: 0];
		if (ctrl_byteenable[1]) 
			control_reg[15: 8] <= ctrl_writedata[15: 8];
		if (ctrl_byteenable[2]) 
			control_reg[23:16] <= ctrl_writedata[23:16];
		if (ctrl_byteenable[3]) 
			control_reg[31:24] <= ctrl_writedata[31:24];
	end
	else if (clear_screen & stream_ready & 
			(x_position == (PIXELS - 1)) && (y_position == (LINES - 1)))
		control_reg[16] <= 1'b0;
end

always @(posedge clk)
begin
	if (reset)
		delayed_buf_waitrequest <= 2'h0;
	else if (buf_chipselect & buf_read)
		delayed_buf_waitrequest <= {delayed_buf_waitrequest[0], 1'b1};
	else
		delayed_buf_waitrequest <= 2'h0;
end

always @(posedge clk)
begin
	if (reset)
		clear_screen <= 1'b1;
	else if (~(control_reg[16]))
		clear_screen <= 1'b0;
	else if ((x_position == 10'h000) && (y_position == 9'h000))
		clear_screen <= 1'b1;
end

always @(posedge clk)
begin
	if (reset)
		x_position <= 10'h000;
	else if (stream_ready)
	begin
		if (x_position == (PIXELS - 1))
			x_position <= 10'h000;
		else
			x_position <= x_position + 10'h001;
	end
end

always @(posedge clk)
begin
	if (reset)
		y_position <= 9'h000;
	else if (stream_ready && (x_position == (PIXELS - 1)))
	begin
		if (y_position == (LINES - 1))
			y_position <= 9'h000;
		else
			y_position <= y_position + 9'h001;
	end
end

always @(posedge clk)
begin
	if (reset)
	begin
		delayed_x_position <= 6'h00;
		delayed_y_position <= 6'h00;
	end
	else if (stream_ready)
	begin
		delayed_x_position <= {delayed_x_position[2:0], 
			x_position[(ENLARGE_CHAR+2):ENLARGE_CHAR]};
		delayed_y_position <= {delayed_y_position[2:0], 
			y_position[(ENLARGE_CHAR+2):ENLARGE_CHAR]};
	end
end

always @(posedge clk)
begin
	if (reset)
		delayed_startofpacket <= 4'h0;
	else if (stream_ready)
	begin
		delayed_startofpacket[3:1] <= delayed_startofpacket[2:0];
		if ((x_position == 10'h000) && (y_position == 9'h000))
			delayed_startofpacket[0] <= 1'b1;
		else
			delayed_startofpacket[0] <= 1'b0;
	end
end

always @(posedge clk)
begin
	if (reset)
		delayed_endofpacket <= 4'h0;
	else if (stream_ready)
	begin
		delayed_endofpacket[3:1] <= delayed_endofpacket[2:0];
		if ((x_position == (PIXELS - 1)) && (y_position == (LINES - 1)))
			delayed_endofpacket[0] <= 1'b1;
		else
			delayed_endofpacket[0] <= 1'b0;
	end
end

`ifdef USE_9BIT_COLOR
always @(posedge clk)
begin
	if (reset)
		delayed_char_color <= 18'h00000;
	else if (stream_ready)
		delayed_char_color <= {delayed_char_color[8:0], 
			cur_char_for_display[15:7]};
end
`endif

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments
assign buf_waitrequest = 
		(buf_chipselect & buf_read) & ~delayed_buf_waitrequest[1];

`ifdef USE_TRANSPARENCY
assign stream_data[39:30]		= {10{cur_char_data}};
`endif
assign stream_data[29: 0]		= {char_red, char_green, char_blue};
assign stream_startofpacket	= delayed_startofpacket[3];
assign stream_endofpacket		= delayed_endofpacket[3];
assign stream_empty				= 2'h0;
assign stream_valid				= 1'b1;

// Internal Assignments
assign char_data_to_buffer = {control_reg[(DW-8):0], buf_writedata[6:0]};

`ifdef USE_TRDB_LCM
assign cur_char_position = 
	{y_position[7:(3 + ENLARGE_CHAR)], x_position[8:(3 + ENLARGE_CHAR)]};
`else
assign cur_char_position = 
	{y_position[8:(3 + ENLARGE_CHAR)], x_position[9:(3 + ENLARGE_CHAR)]};
`endif

`ifdef USE_1BIT_COLOR
assign char_red		= {10{cur_char_data}};
assign char_green		= {10{cur_char_data}};
assign char_blue		= {10{cur_char_data}};
`elsif USE_4BIT_COLOR
assign char_red		= {{9{cur_char_data}} & fg_R[9:1], cur_char_data};
assign char_green		= {{9{cur_char_data}} & fg_G[9:1], cur_char_data};
assign char_blue		= {{9{cur_char_data}} & fg_B[9:1], cur_char_data};
`elsif USE_8BIT_COLOR
assign char_red		= ({10{cur_char_data}} & fg_R) | ({10{~cur_char_data}} & bg_R);
assign char_green		= ({10{cur_char_data}} & fg_G) | ({10{~cur_char_data}} & bg_G);
assign char_blue		= ({10{cur_char_data}} & fg_B) | ({10{~cur_char_data}} & bg_B);
`else
assign char_red		= {{3{delayed_char_color[17:15]}}, cur_char_data};
assign char_green		= {{3{delayed_char_color[14:12]}}, cur_char_data};
assign char_blue		= {{3{delayed_char_color[11: 9]}}, cur_char_data};
`endif

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altsyncram	Char_Buffer_Memory (
	// Inputs
	.clock0				(clk),
	.address_a			(buf_address),
	.wren_a				(buf_byteenable & buf_chipselect & buf_write),
	.data_a				(char_data_to_buffer),

	.clock1				(clk),
	.clocken1			(stream_ready),
	.address_b			(cur_char_position),
	.wren_b				(clear_screen),
	.data_b				({{(DW - 7){1'b0}}, 7'h20}),

	// Bidirectionals

	// Outputs
	.q_a					(char_data_from_buffer),
	.q_b					(cur_char_for_display),
	
	// Unused 
	.rden_b				(1'b1),

	.aclr0				(1'b0),
	.aclr1				(1'b0),
	.clocken0			(1'b1),
	.clocken2			(1'b1),
	.clocken3			(1'b1),
	.byteena_a			(1'b1),
	.byteena_b			(1'b1),
	.rden_a				(1'b1),
	.addressstall_a	(1'b0),
	.addressstall_b	(1'b0)
);
defparam
	Char_Buffer_Memory.init_file									= "UNUSED",
	Char_Buffer_Memory.intended_device_family					= "Cyclone II",
	Char_Buffer_Memory.lpm_type									= "altsyncram",
	Char_Buffer_Memory.operation_mode							= "BIDIR_DUAL_PORT",
	Char_Buffer_Memory.read_during_write_mode_mixed_ports	= "DONT_CARE",
	Char_Buffer_Memory.power_up_uninitialized					= "FALSE",
	Char_Buffer_Memory.address_reg_b								= "CLOCK1",
	Char_Buffer_Memory.indata_reg_b								= "CLOCK1",
	Char_Buffer_Memory.wrcontrol_wraddress_reg_b				= "CLOCK1",
	Char_Buffer_Memory.clock_enable_input_a					= "BYPASS",
	Char_Buffer_Memory.clock_enable_input_b					= "NORMAL",
	Char_Buffer_Memory.clock_enable_output_a					= "BYPASS",
	Char_Buffer_Memory.clock_enable_output_b					= "NORMAL",
	Char_Buffer_Memory.numwords_a									= BUFFER_SIZE,
	Char_Buffer_Memory.numwords_b									= BUFFER_SIZE,
	Char_Buffer_Memory.outdata_aclr_a							= "NONE",
	Char_Buffer_Memory.outdata_aclr_b							= "NONE",
	Char_Buffer_Memory.outdata_reg_a								= "CLOCK0",
	Char_Buffer_Memory.outdata_reg_b								= "CLOCK1",
	Char_Buffer_Memory.widthad_a									= AW,
	Char_Buffer_Memory.widthad_b									= AW,
	Char_Buffer_Memory.width_a										= DW,
	Char_Buffer_Memory.width_b										= DW,
	Char_Buffer_Memory.width_byteena_a							= 1,
	Char_Buffer_Memory.width_byteena_b							= 1;

altera_up_video_128_character_rom Character_Rom (
	// Inputs
	.clk					(clk),
	.clk_en				(stream_ready),

	.character			(cur_char_for_display[ 6: 0]),
	.x_coordinate		(delayed_x_position[ 5: 3]),
	.y_coordinate		(delayed_y_position[ 5: 3]),
	
	// Bidirectionals

	// Outputs
	.character_data	(cur_char_data)
);
	
`ifdef USE_FOREGROUND_COLOR
altera_up_video_fb_color_rom Foreground_Color_Rom (
	// Inputs
	.clk					(clk),
	.clk_en				(stream_ready),

	.color_index		(cur_char_for_display[10: 7]),
	
	// Bidirectionals

	// Outputs
	.red					(fg_R),
	.green				(fg_G),
	.blue					(fg_B)
);
`endif

`ifdef USE_BACKGROUND_COLOR
altera_up_video_fb_color_rom Background_Color_Rom (
	// Inputs
	.clk					(clk),
	.clk_en				(stream_ready),

	.color_index		(cur_char_for_display[14:11]),
	
	// Bidirectionals

	// Outputs
	.red					(bg_R),
	.green				(bg_G),
	.blue					(bg_B)
);
`endif

endmodule

