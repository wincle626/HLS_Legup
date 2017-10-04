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

module altera_up_edge_detection_nonmaximum_suppression (
	// Inputs
	clk,
	reset,

	data_in,
	data_en,

	// Bidirectionals

	// Outputs
	data_out
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

input			[ 9: 0]	data_in;
input						data_en;

// Bidirectionals

// Outputs
output		[ 7: 0]	data_out;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 9: 0]	shift_reg_out[ 1: 0];

// Internal Registers
reg			[ 9: 0]	original_line_1[ 2: 0];
reg			[ 9: 0]	original_line_2[ 2: 0];
reg			[ 9: 0]	original_line_3[ 2: 0];

reg			[ 7: 0]	suppress_level_1[ 4: 0];
reg			[ 8: 0]	suppress_level_2[ 2: 0];
reg			[ 7: 0]	suppress_level_3;
reg			[ 1: 0]	average_flags;

reg			[ 7: 0]	result;

// State Machine Registers

// Integers
integer					i;

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset == 1'b1)
	begin
		for (i = 2; i >= 0; i = i-1)
		begin
			original_line_1[i] 	<= 10'h000;
			original_line_2[i] 	<= 10'h000;
			original_line_3[i] 	<= 10'h000;
		end

		for (i = 4; i >= 0; i = i-1)
		begin
			suppress_level_1[i] 	<= 8'h00;
		end
		suppress_level_2[0] 		<= 9'h000;
		suppress_level_2[1] 		<= 9'h000;
		suppress_level_2[2] 		<= 9'h000;
		suppress_level_3   	 	<= 8'h00;

		average_flags				<= 2'h0;
	end
	else if (data_en == 1'b1)
	begin	
		for (i = 2; i > 0; i = i-1)
		begin
			original_line_1[i] 	<= original_line_1[i-1];
			original_line_2[i] 	<= original_line_2[i-1];
			original_line_3[i] 	<= original_line_3[i-1];
		end
		original_line_1[0] 		<= data_in;
		original_line_2[0] 		<= shift_reg_out[0];
		original_line_3[0] 		<= shift_reg_out[1];

		suppress_level_1[0] 		<= original_line_2[1][ 7: 0];
		case (original_line_2[1][9:8])
			2'b00 :
			begin
				suppress_level_1[1] <= original_line_1[0][ 7: 0];
				suppress_level_1[2] <= original_line_2[0][ 7: 0];
				suppress_level_1[3] <= original_line_2[2][ 7: 0];
				suppress_level_1[4] <= original_line_3[2][ 7: 0];
			end
			2'b01 :
			begin
				suppress_level_1[1] <= original_line_1[0][ 7: 0];
				suppress_level_1[2] <= original_line_1[1][ 7: 0];
				suppress_level_1[3] <= original_line_3[1][ 7: 0];
				suppress_level_1[4] <= original_line_3[2][ 7: 0];
			end
			2'b10 :
			begin
				suppress_level_1[1] <= original_line_1[1][ 7: 0];
				suppress_level_1[2] <= original_line_1[2][ 7: 0];
				suppress_level_1[3] <= original_line_3[0][ 7: 0];
				suppress_level_1[4] <= original_line_3[1][ 7: 0];
			end
			2'b11 :
			begin
				suppress_level_1[1] <= original_line_1[2][ 7: 0];
				suppress_level_1[2] <= original_line_2[2][ 7: 0];
				suppress_level_1[3] <= original_line_2[0][ 7: 0];
				suppress_level_1[4] <= original_line_3[0][ 7: 0];
			end
		endcase

		// Calculate Averages
		suppress_level_2[0] <= {1'b0,suppress_level_1[0]};
		suppress_level_2[1] <= {1'b0,suppress_level_1[1]} + {1'h0,suppress_level_1[2]};
		suppress_level_2[2] <= {1'b0,suppress_level_1[3]} + {1'h0,suppress_level_1[3]};

		// Calculate if pixel is nonmaximum
		suppress_level_3 <= suppress_level_2[0][ 7: 0];
		average_flags[0] <= (suppress_level_2[0][ 7: 0] >= suppress_level_2[1][ 8: 1]) ? 1'b1 : 1'b0;
		average_flags[1] <= (suppress_level_2[0][ 7: 0] >= suppress_level_2[2][ 8: 1]) ? 1'b1 : 1'b0;
	
		result <= (average_flags == 2'b11) ? suppress_level_3 : 8'h00;
	end
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

assign data_out = result; 

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altera_up_edge_detection_data_shift_register shift_register_1 (
	// Inputs
	.clock		(clk),
	.clken		(data_en),
	.shiftin		(data_in),

	// Bidirectionals

	// Outputs
	.shiftout	(shift_reg_out[0]),
	.taps			()
);
defparam
	shift_register_1.DW		= 10,
	shift_register_1.SIZE	= WIDTH;

altera_up_edge_detection_data_shift_register shift_register_2 (
	// Inputs
	.clock		(clk),
	.clken		(data_en),
	.shiftin		(shift_reg_out[0]),

	// Bidirectionals

	// Outputs
	.shiftout	(shift_reg_out[1]),
	.taps			()
);
defparam
	shift_register_2.DW		= 10,
	shift_register_2.SIZE	= WIDTH;

endmodule

