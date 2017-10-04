LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_unsigned.all;
USE ieee.std_logic_misc.all;

-- ******************************************************************************
-- * License Agreement                                                          *
-- *                                                                            *
-- * Copyright (c) 1991-2013 Altera Corporation, San Jose, California, USA.     *
-- * All rights reserved.                                                       *
-- *                                                                            *
-- * Any megafunction design, and related net list (encrypted or decrypted),    *
-- *  support information, device programming or simulation file, and any other *
-- *  associated documentation or information provided by Altera or a partner   *
-- *  under Altera's Megafunction Partnership Program may be used only to       *
-- *  program PLD devices (but not masked PLD devices) from Altera.  Any other  *
-- *  use of such megafunction design, net list, support information, device    *
-- *  programming or simulation file, or any other related documentation or     *
-- *  information is prohibited for any other purpose, including, but not       *
-- *  limited to modification, reverse engineering, de-compiling, or use with   *
-- *  any other silicon devices, unless such use is explicitly licensed under   *
-- *  a separate agreement with Altera or a megafunction partner.  Title to     *
-- *  the intellectual property, including patents, copyrights, trademarks,     *
-- *  trade secrets, or maskworks, embodied in any such megafunction design,    *
-- *  net list, support information, device programming or simulation file, or  *
-- *  any other related documentation or information provided by Altera or a    *
-- *  megafunction partner, remains with Altera, the megafunction partner, or   *
-- *  their respective licensors.  No other licenses, including any licenses    *
-- *  needed under any third party's intellectual property, are provided herein.*
-- *  Copying or modifying any file, or portion thereof, to which this notice   *
-- *  is attached violates this copyright.                                      *
-- *                                                                            *
-- * THIS FILE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
-- * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
-- * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
-- * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
-- * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
-- * FROM, OUT OF OR IN CONNECTION WITH THIS FILE OR THE USE OR OTHER DEALINGS  *
-- * IN THIS FILE.                                                              *
-- *                                                                            *
-- * This agreement shall be governed in all respects by the laws of the State  *
-- *  of California and by the laws of the United States of America.            *
-- *                                                                            *
-- ******************************************************************************


ENTITY altera_up_edge_detection_gaussian_smoothing_filter IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	WIDTH	:INTEGER									:= 640 -- Image width in pixels
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk		:IN		STD_LOGIC;
	reset		:IN		STD_LOGIC;

	data_in	:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	data_en	:IN		STD_LOGIC;

	-- Bidirectionals

	-- Outputs
	data_out	:BUFFER	STD_LOGIC_VECTOR( 8 DOWNTO  0)	

);

END altera_up_edge_detection_gaussian_smoothing_filter;

ARCHITECTURE Behaviour OF altera_up_edge_detection_gaussian_smoothing_filter IS

	TYPE MATRIX4x8		IS ARRAY( 3 DOWNTO  0) OF STD_LOGIC_VECTOR( 7 DOWNTO  0);
	TYPE MATRIX5x8		IS ARRAY( 4 DOWNTO  0) OF STD_LOGIC_VECTOR( 7 DOWNTO  0);
	TYPE MATRIX13x16	IS ARRAY(12 DOWNTO  0) OF STD_LOGIC_VECTOR(15 DOWNTO  0);
	TYPE MATRIX7x16	IS ARRAY( 6 DOWNTO  0) OF STD_LOGIC_VECTOR(15 DOWNTO  0);
	TYPE MATRIX5x16	IS ARRAY( 4 DOWNTO  0) OF STD_LOGIC_VECTOR(15 DOWNTO  0);
	TYPE MATRIX3x16	IS ARRAY( 2 DOWNTO  0) OF STD_LOGIC_VECTOR(15 DOWNTO  0);
	TYPE MATRIX2x16	IS ARRAY( 1 DOWNTO  0) OF STD_LOGIC_VECTOR(15 DOWNTO  0);

-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	shift_reg_out		:MATRIX4x8;
	
	-- Internal Registers
	SIGNAL	original_line_1	:MATRIX5x8;
	SIGNAL	original_line_2	:MATRIX5x8;
	SIGNAL	original_line_3	:MATRIX5x8;
	SIGNAL	original_line_4	:MATRIX5x8;
	SIGNAL	original_line_5	:MATRIX5x8;
	
	SIGNAL	sum_level_1			:MATRIX13x16;
	SIGNAL	sum_level_2			:MATRIX7x16;
	SIGNAL	sum_level_3			:MATRIX5x16;
	SIGNAL	sum_level_4			:MATRIX3x16;
	SIGNAL	sum_level_5			:MATRIX2x16;
	SIGNAL	sum_level_6			:STD_LOGIC_VECTOR(15 DOWNTO  0);	
	SIGNAL	sum_level_7			:STD_LOGIC_VECTOR( 8 DOWNTO  0);
	
	-- State Machine Registers
	
	-- Integers
	VARIABLE	i						:INTEGER;
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altera_up_edge_detection_data_shift_register
	GENERIC (
		DW			:INTEGER;
		SIZE		:INTEGER
	);
	PORT (
		-- Inputs
		clock		:IN		STD_LOGIC;
		clken		:IN		STD_LOGIC;
		shiftin	:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		shiftout	:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0)
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	-- Gaussian Smoothing Filter
	-- 
	--          [ 2  4  5  4  2 ]
	--          [ 4  9 12  9  4 ]
	-- 1 / 115  [ 5 12 15 12  5 ]
	--          [ 4  9 12  9  4 ]
	--          [ 2  4  5  4  2 ]
	--


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				FOR i IN 4 DOWNTO 0 LOOP
					original_line_1(i) <= B"00000000";
					original_line_2(i) <= B"00000000";
					original_line_3(i) <= B"00000000";
					original_line_4(i) <= B"00000000";
					original_line_5(i) <= B"00000000";
				END LOOP;
		
				FOR i IN 12 DOWNTO 0 LOOP
					sum_level_1(i) <= B"0000000000000000";
				END LOOP;
		
				FOR i IN 6 DOWNTO 0 LOOP
					sum_level_2(i) <= B"0000000000000000";
				END LOOP;
		
				FOR i IN 4 DOWNTO 0 LOOP
					sum_level_3(i) <= B"0000000000000000";
				END LOOP;
		
				sum_level_4(0) <= B"0000000000000000";
				sum_level_4(1) <= B"0000000000000000";
				sum_level_4(2) <= B"0000000000000000";
			
				sum_level_5(0) <= B"0000000000000000";
				sum_level_5(1) <= B"0000000000000000";
		
				sum_level_6    <= B"0000000000000000";
				sum_level_7    <= B"000000000";
			ELSIF (data_en = '1') THEN
				FOR i IN 4 DOWNTO 1 LOOP
					original_line_1(i) <= original_line_1(i-1);
					original_line_2(i) <= original_line_2(i-1);
					original_line_3(i) <= original_line_3(i-1);
					original_line_4(i) <= original_line_4(i-1);
					original_line_5(i) <= original_line_5(i-1);
				END LOOP;
				original_line_1(0) <= data_in;
				original_line_2(0) <= shift_reg_out(0);
				original_line_3(0) <= shift_reg_out(1);
				original_line_4(0) <= shift_reg_out(2);
				original_line_5(0) <= shift_reg_out(3);
		
				-- Add numbers that are multiplied by 2 and multiply by 2
				sum_level_1( 0) <= (B"0000000" & original_line_1(0) & '0') + (B"0000000" & original_line_1(4) & '0');
				sum_level_1( 1) <= (B"0000000" & original_line_5(0) & '0') + (B"0000000" & original_line_5(4) & '0');
				-- Add numbers that are multiplied by 4 and multiply by 4
				sum_level_1( 2) <= (B"000000" & original_line_1(1) & B"00") + (B"000000" & original_line_1(3) & B"00");
				sum_level_1( 3) <= (B"000000" & original_line_2(0) & B"00") + (B"000000" & original_line_2(4) & B"00");
				sum_level_1( 4) <= (B"000000" & original_line_4(0) & B"00") + (B"000000" & original_line_4(4) & B"00");
				sum_level_1( 5) <= (B"000000" & original_line_5(1) & B"00") + (B"000000" & original_line_5(3) & B"00");
				-- Add numbers that are multiplied by 5
				sum_level_1( 6) <= (B"00000000" & original_line_1(2)) + (B"00000000" & original_line_5(2));
				sum_level_1( 7) <= (B"00000000" & original_line_3(0)) + (B"00000000" & original_line_3(4));
				-- Add numbers that are multiplied by 9
				sum_level_1( 8) <= (B"00000000" & original_line_2(1)) + (B"00000000" & original_line_2(3));
				sum_level_1( 9) <= (B"00000000" & original_line_4(1)) + (B"00000000" & original_line_4(3));
				-- Add numbers that are multiplied by 12
				sum_level_1(10) <= (B"00000000" & original_line_2(2)) + (B"00000000" & original_line_4(2));
				sum_level_1(11) <= (B"00000000" & original_line_3(1)) + (B"00000000" & original_line_3(3));
				-- Add numbers that are multiplied by 15
				sum_level_1(12) <= (B"0000" & original_line_3(2) & B"0000") - original_line_3(2);
							
				-- Add numbers that are multiplied by 2
				sum_level_2( 0) <= sum_level_1( 0) + sum_level_1( 1);
				-- Add numbers that are multiplied by 4
				sum_level_2( 1) <= sum_level_1( 2) + sum_level_1( 3);
				sum_level_2( 2) <= sum_level_1( 4) + sum_level_1( 5);
				-- Add numbers that are multiplied by 5
				sum_level_2( 3) <= sum_level_1( 6) + sum_level_1( 7);
				-- Add numbers that are multiplied by 9
				sum_level_2( 4) <= sum_level_1( 8) + sum_level_1( 9);
				-- Add numbers that are multiplied by 12
				sum_level_2( 5) <= sum_level_1(10) + sum_level_1(11);
				-- Multiplied by 15
				sum_level_2( 6) <= sum_level_1(12);
		
				-- Add 2s and 15s
				sum_level_3( 0) <= sum_level_2( 0) + sum_level_2( 6);
				-- Add numbers that are multiplied by 4
				sum_level_3( 1) <= sum_level_2( 1) + sum_level_2( 2);
				-- Multiplied by 5
				sum_level_3( 2) <= (sum_level_2( 3) & B"00") + sum_level_2( 3);
				-- Multiplied by 9
				sum_level_3( 3) <= (sum_level_2( 4) & B"000") + sum_level_2( 4);
				-- Multiplied by 12
				sum_level_3( 4) <= (sum_level_2( 5) & B"000") + (sum_level_2( 5) & B"00");
		
				-- Add
				sum_level_4( 0) <= sum_level_3( 0) + sum_level_3( 1);
				sum_level_4( 1) <= sum_level_3( 2) + sum_level_3( 3);
				sum_level_4( 2) <= sum_level_3( 4);
		
				sum_level_5( 0) <= sum_level_4( 0) + sum_level_4( 1);
				sum_level_5( 1) <= sum_level_4( 2);
		
				sum_level_6     <= sum_level_5( 0) + sum_level_5( 1);
		
				-- Divide by 128, which is close enough to 115
				sum_level_7     <= sum_level_6(15 DOWNTO 7);
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	data_out <= sum_level_7; 

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	shift_register_1 : altera_up_edge_detection_data_shift_register 
	GENERIC MAP (
		DW			=> 8,
		SIZE		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clock		=> clk,
		clken		=> data_en,
		shiftin	=> data_in,
	
		-- Bidirectionals
	
		-- Outputs
		shiftout	=> shift_reg_out(0)
	);

	shift_register_2 : altera_up_edge_detection_data_shift_register 
	GENERIC MAP (
		DW			=> 8,
		SIZE		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clock		=> clk,
		clken		=> data_en,
		shiftin	=> shift_reg_out(0),
	
		-- Bidirectionals
	
		-- Outputs
		shiftout	=> shift_reg_out(1)
	);

	shift_register_3 : altera_up_edge_detection_data_shift_register 
	GENERIC MAP (
		DW			=> 8,
		SIZE		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clock		=> clk,
		clken		=> data_en,
		shiftin	=> shift_reg_out(1),
	
		-- Bidirectionals
	
		-- Outputs
		shiftout	=> shift_reg_out(2)
	);

	shift_register_4 : altera_up_edge_detection_data_shift_register 
	GENERIC MAP (
		DW			=> 8,
		SIZE		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clock		=> clk,
		clken		=> data_en,
		shiftin	=> shift_reg_out(2),
	
		-- Bidirectionals
	
		-- Outputs
		shiftout	=> shift_reg_out(3)
	);


END Behaviour;
