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

ENTITY altera_up_edge_detection_nonmaximum_suppression IS 

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

	data_in	:IN		STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	data_en	:IN		STD_LOGIC;

	-- Bidirectionals

	-- Outputs
	data_out	:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0)	

);

END altera_up_edge_detection_nonmaximum_suppression;

ARCHITECTURE Behaviour OF altera_up_edge_detection_nonmaximum_suppression IS

	TYPE MATRIX2x10	IS ARRAY( 1 DOWNTO  0) OF STD_LOGIC_VECTOR( 9 DOWNTO  0);
	TYPE MATRIX3x10	IS ARRAY( 2 DOWNTO  0) OF STD_LOGIC_VECTOR( 9 DOWNTO  0);
	TYPE MATRIX5x8		IS ARRAY( 4 DOWNTO  0) OF STD_LOGIC_VECTOR( 7 DOWNTO  0);
	TYPE MATRIX3x9		IS ARRAY( 2 DOWNTO  0) OF STD_LOGIC_VECTOR( 8 DOWNTO  0);

-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	shift_reg_out		:MATRIX2x10;
	
	-- Internal Registers
	SIGNAL	original_line_1	:MATRIX3x10;
	SIGNAL	original_line_2	:MATRIX3x10;
	SIGNAL	original_line_3	:MATRIX3x10;
	
	SIGNAL	suppress_level_1	:MATRIX5x8;
	SIGNAL	suppress_level_2	:MATRIX3x9;
	SIGNAL	suppress_level_3	:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	average_flags		:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	
	SIGNAL	result				:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	
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
		shiftin	:IN		STD_LOGIC_VECTOR( 9 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		shiftout	:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0)
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				FOR i IN 2 DOWNTO 0 LOOP
					original_line_1(i) 	<= B"0000000000";
					original_line_2(i) 	<= B"0000000000";
					original_line_3(i) 	<= B"0000000000";
				END LOOP;
		
				FOR i IN 4 DOWNTO 0 LOOP
					suppress_level_1(i) 	<= B"00000000";
				END LOOP;
				suppress_level_2(0) 		<= B"000000000";
				suppress_level_2(1) 		<= B"000000000";
				suppress_level_2(2) 		<= B"000000000";
				suppress_level_3   	 	<= B"00000000";
		
				average_flags				<= B"00";
			ELSIF (data_en = '1') THEN
				FOR i IN 2 DOWNTO 1 LOOP
					original_line_1(i) 	<= original_line_1(i-1);
					original_line_2(i) 	<= original_line_2(i-1);
					original_line_3(i) 	<= original_line_3(i-1);
				END LOOP;
				original_line_1(0) 		<= data_in;
				original_line_2(0) 		<= shift_reg_out(0);
				original_line_3(0) 		<= shift_reg_out(1);
		
				suppress_level_1(0) 		<= original_line_2(1)( 7 DOWNTO  0);
				CASE (original_line_2(1)(9 DOWNTO 8)) IS
					WHEN B"00" =>
						suppress_level_1(1) <= original_line_1(0)( 7 DOWNTO  0);
						suppress_level_1(2) <= original_line_2(0)( 7 DOWNTO  0);
						suppress_level_1(3) <= original_line_2(2)( 7 DOWNTO  0);
						suppress_level_1(4) <= original_line_3(2)( 7 DOWNTO  0);
					WHEN B"01" =>
						suppress_level_1(1) <= original_line_1(0)( 7 DOWNTO  0);
						suppress_level_1(2) <= original_line_1(1)( 7 DOWNTO  0);
						suppress_level_1(3) <= original_line_3(1)( 7 DOWNTO  0);
						suppress_level_1(4) <= original_line_3(2)( 7 DOWNTO  0);
					WHEN B"10" =>
						suppress_level_1(1) <= original_line_1(1)( 7 DOWNTO  0);
						suppress_level_1(2) <= original_line_1(2)( 7 DOWNTO  0);
						suppress_level_1(3) <= original_line_3(0)( 7 DOWNTO  0);
						suppress_level_1(4) <= original_line_3(1)( 7 DOWNTO  0);
					WHEN B"11" =>
						suppress_level_1(1) <= original_line_1(2)( 7 DOWNTO  0);
						suppress_level_1(2) <= original_line_2(2)( 7 DOWNTO  0);
						suppress_level_1(3) <= original_line_2(0)( 7 DOWNTO  0);
						suppress_level_1(4) <= original_line_3(0)( 7 DOWNTO  0);
				END CASE;
		
				-- Calculate Averages
				suppress_level_2(0) <= '0' & suppress_level_1(0);
				suppress_level_2(1) <= ('0' & suppress_level_1(1)) + ('0' & suppress_level_1(2));
				suppress_level_2(2) <= ('0' & suppress_level_1(3)) + ('0' & suppress_level_1(3));
		
				-- Calculate if pixel is nonmaximum
				suppress_level_3 <= suppress_level_2(0)( 7 DOWNTO  0);
				IF (suppress_level_2(0)( 7 DOWNTO  0) >= suppress_level_2(1)( 8 DOWNTO  1)) THEN
					average_flags(0) <= '1';
				ELSE
					average_flags(0) <= '0';
				END IF;
				
				IF (suppress_level_2(0)( 7 DOWNTO  0) >= suppress_level_2(2)( 8 DOWNTO  1)) THEN
					average_flags(1) <= '1';
				ELSE
					average_flags(1) <= '0';
				END IF;
			
				IF (average_flags = B"11") THEN
					result <= suppress_level_3;
				ELSE
					result <= B"00000000";
				END IF;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	data_out <= result; 

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	shift_register_1 : altera_up_edge_detection_data_shift_register 
	GENERIC MAP (
		DW			=> 10,
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
		DW			=> 10,
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


END Behaviour;
