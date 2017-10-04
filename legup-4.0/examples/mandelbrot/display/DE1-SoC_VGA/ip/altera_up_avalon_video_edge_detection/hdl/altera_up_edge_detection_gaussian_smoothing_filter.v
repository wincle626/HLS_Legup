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


module altera_up_edge_detection_gaussian_smoothing_filter (
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

input			[ 7: 0]	data_in;
input						data_en;

// Bidirectionals

// Outputs
output		[ 8: 0]	data_out;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 7: 0]	shift_reg_out[ 3: 0];

// Internal Registers
reg			[ 7: 0]	original_line_1[ 4: 0];
reg			[ 7: 0]	original_line_2[ 4: 0];
reg			[ 7: 0]	original_line_3[ 4: 0];
reg			[ 7: 0]	original_line_4[ 4: 0];
reg			[ 7: 0]	original_line_5[ 4: 0];

reg			[15: 0]	sum_level_1[12: 0];
reg			[15: 0]	sum_level_2[ 6: 0];
reg			[15: 0]	sum_level_3[ 4: 0];
reg			[15: 0]	sum_level_4[ 2: 0];
reg			[15: 0]	sum_level_5[ 1: 0];
reg			[15: 0]	sum_level_6;
reg			[ 8: 0]	sum_level_7;

// State Machine Registers

// Integers
integer				i;

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Gaussian Smoothing Filter
// 
//          [ 2  4  5  4  2 ]
//          [ 4  9 12  9  4 ]
// 1 / 115  [ 5 12 15 12  5 ]
//          [ 4  9 12  9  4 ]
//          [ 2  4  5  4  2 ]
//


always @(posedge clk)
begin
	if (reset == 1'b1)
	begin
		for (i = 4; i >= 0; i = i-1)
		begin
			original_line_1[i] <= 8'h00;
			original_line_2[i] <= 8'h00;
			original_line_3[i] <= 8'h00;
			original_line_4[i] <= 8'h00;
			original_line_5[i] <= 8'h00;
		end

		for (i = 12; i >= 0; i = i-1)
		begin
			sum_level_1[i] <= 16'h0000;
		end

		for (i = 6; i >= 0; i = i-1)
		begin
			sum_level_2[i] <= 16'h0000;
		end

		for (i = 4; i >= 0; i = i-1)
		begin
			sum_level_3[i] <= 16'h0000;
		end

		sum_level_4[0] <= 16'h0000;
		sum_level_4[1] <= 16'h0000;
		sum_level_4[2] <= 16'h0000;
	
		sum_level_5[0] <= 16'h0000;
		sum_level_5[1] <= 16'h0000;

		sum_level_6    <= 16'h0000;
		sum_level_7    <= 9'h000;
	end
	else if (data_en == 1'b1)
	begin	
		for (i = 4; i > 0; i = i-1)
		begin
			original_line_1[i] <= original_line_1[i-1];
			original_line_2[i] <= original_line_2[i-1];
			original_line_3[i] <= original_line_3[i-1];
			original_line_4[i] <= original_line_4[i-1];
			original_line_5[i] <= original_line_5[i-1];
		end
		original_line_1[0] <= data_in;
		original_line_2[0] <= shift_reg_out[0];
		original_line_3[0] <= shift_reg_out[1];
		original_line_4[0] <= shift_reg_out[2];
		original_line_5[0] <= shift_reg_out[3];

		// Add numbers that are multiplied by 2 and multiply by 2
		sum_level_1[ 0] <= {7'h00,original_line_1[0], 1'b0} + {7'h00,original_line_1[4], 1'b0};
		sum_level_1[ 1] <= {7'h00,original_line_5[0], 1'b0} + {7'h00,original_line_5[4], 1'b0};
		// Add numbers that are multiplied by 4 and multiply by 4
		sum_level_1[ 2] <= {6'h00,original_line_1[1], 2'h0} + {6'h00,original_line_1[3], 2'h0};
		sum_level_1[ 3] <= {6'h00,original_line_2[0], 2'h0} + {6'h00,original_line_2[4], 2'h0};
		sum_level_1[ 4] <= {6'h00,original_line_4[0], 2'h0} + {6'h00,original_line_4[4], 2'h0};
		sum_level_1[ 5] <= {6'h00,original_line_5[1], 2'h0} + {6'h00,original_line_5[3], 2'h0};
		// Add numbers that are multiplied by 5
		sum_level_1[ 6] <= {8'h00,original_line_1[2]} + {8'h00,original_line_5[2]};
		sum_level_1[ 7] <= {8'h00,original_line_3[0]} + {8'h00,original_line_3[4]};
		// Add numbers that are multiplied by 9
		sum_level_1[ 8] <= {8'h00,original_line_2[1]} + {8'h00,original_line_2[3]};
		sum_level_1[ 9] <= {8'h00,original_line_4[1]} + {8'h00,original_line_4[3]};
		// Add numbers that are multiplied by 12
		sum_level_1[10] <= {8'h00,original_line_2[2]} + {8'h00,original_line_4[2]};
		sum_level_1[11] <= {8'h00,original_line_3[1]} + {8'h00,original_line_3[3]};
		// Add numbers that are multiplied by 15
		sum_level_1[12] <= {4'h0,original_line_3[2], 4'h0} - original_line_3[2];
					
		// Add numbers that are multiplied by 2
		sum_level_2[ 0] <= sum_level_1[ 0] + sum_level_1[ 1];
		// Add numbers that are multiplied by 4
		sum_level_2[ 1] <= sum_level_1[ 2] + sum_level_1[ 3];
		sum_level_2[ 2] <= sum_level_1[ 4] + sum_level_1[ 5];
		// Add numbers that are multiplied by 5
		sum_level_2[ 3] <= sum_level_1[ 6] + sum_level_1[ 7];
		// Add numbers that are multiplied by 9
		sum_level_2[ 4] <= sum_level_1[ 8] + sum_level_1[ 9];
		// Add numbers that are multiplied by 12
		sum_level_2[ 5] <= sum_level_1[10] + sum_level_1[11];
		// Multiplied by 15
		sum_level_2[ 6] <= sum_level_1[12];

		// Add 2s and 15s
		sum_level_3[ 0] <= sum_level_2[ 0] + sum_level_2[ 6];
		// Add numbers that are multiplied by 4
		sum_level_3[ 1] <= sum_level_2[ 1] + sum_level_2[ 2];
		// Multiplied by 5
		sum_level_3[ 2] <= {sum_level_2[ 3], 2'h0} + sum_level_2[ 3];
		// Multiplied by 9
		sum_level_3[ 3] <= {sum_level_2[ 4], 3'h0} + sum_level_2[ 4];
		// Multiplied by 12
		sum_level_3[ 4] <= {sum_level_2[ 5], 3'h0} + {sum_level_2[ 5], 2'h0};

		// Add
		sum_level_4[ 0] <= sum_level_3[ 0] + sum_level_3[ 1];
		sum_level_4[ 1] <= sum_level_3[ 2] + sum_level_3[ 3];
		sum_level_4[ 2] <= sum_level_3[ 4];

		sum_level_5[ 0] <= sum_level_4[ 0] + sum_level_4[ 1];
		sum_level_5[ 1] <= sum_level_4[ 2];

		sum_level_6     <= sum_level_5[ 0] + sum_level_5[ 1];

		// Divide by 128, which is close enough to 115
		sum_level_7     <= sum_level_6[15:7];
	end
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

assign data_out = sum_level_7; 

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
	shift_register_1.DW		= 8,
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
	shift_register_2.DW		= 8,
	shift_register_2.SIZE	= WIDTH;

altera_up_edge_detection_data_shift_register shift_register_3 (
	// Inputs
	.clock		(clk),
	.clken		(data_en),
	.shiftin		(shift_reg_out[1]),

	// Bidirectionals

	// Outputs
	.shiftout	(shift_reg_out[2]),
	.taps			()
);
defparam
	shift_register_3.DW		= 8,
	shift_register_3.SIZE	= WIDTH;

altera_up_edge_detection_data_shift_register shift_register_4 (
	// Inputs
	.clock		(clk),
	.clken		(data_en),
	.shiftin		(shift_reg_out[2]),

	// Bidirectionals

	// Outputs
	.shiftout	(shift_reg_out[3]),
	.taps			()
);
defparam
	shift_register_4.DW		= 8,
	shift_register_4.SIZE	= WIDTH;

endmodule

