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


ENTITY altera_up_edge_detection_hysteresis IS 

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
	data_out	:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0)	

);

END altera_up_edge_detection_hysteresis;

ARCHITECTURE Behaviour OF altera_up_edge_detection_hysteresis IS

	TYPE MATRIX2x9		IS ARRAY( 1 DOWNTO  0) OF STD_LOGIC_VECTOR( 8 DOWNTO  0);
	TYPE MATRIX3x3		IS ARRAY( 2 DOWNTO  0) OF STD_LOGIC_VECTOR( 2 DOWNTO  0);

-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	shift_reg_out					:MATRIX2x9;
	
	SIGNAL	data_above_high_threshold	:STD_LOGIC;
	SIGNAL	data_to_shift_register_1	:STD_LOGIC_VECTOR( 8 DOWNTO  0);	
	
	SIGNAL	above_threshold				:STD_LOGIC;
	
	SIGNAL	overflow							:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	data_line_2						:MATRIX2x9;
	
	SIGNAL	thresholds						:MATRIX3x3;
	
	SIGNAL	result							:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	
	-- State Machine Registers
	
	-- Integers
	VARIABLE	i									:INTEGER;
	
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
		shiftin	:IN		STD_LOGIC_VECTOR( 8 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		shiftout	:BUFFER	STD_LOGIC_VECTOR( 8 DOWNTO  0)
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
				data_line_2(0) <= B"00000000";
				data_line_2(1) <= B"00000000";
		
				FOR i IN 2 DOWNTO 0 LOOP
					thresholds(i) <= B"000";
			END LOOP;
			ELSIF (data_en = '1') THEN
				-- Increase edge visibility by 32 and saturate at 255 
				data_line_2(1) <= data_line_2(0) OR (data_line_2(0)(8) & data_line_2(0)(8) &
										data_line_2(0)(8) & data_line_2(0)(8) & data_line_2(0)(8) & 
										data_line_2(0)(8) & data_line_2(0)(8) & data_line_2(0)(8) & 
										data_line_2(0)(8));
				data_line_2(0) <= ('0' & shift_reg_out(0)(7 DOWNTO 0)) + 32;
		
				thresholds(0) <= thresholds(0)(1 DOWNTO 0) & data_above_high_threshold;
				thresholds(1) <= thresholds(1)(1 DOWNTO 0) & shift_reg_out(0)(8);
				thresholds(2) <= thresholds(2)(1 DOWNTO 0) & shift_reg_out(1)(8);
				
				IF (above_threshold = '1') THEN
					result <= data_line_2(1)(7 DOWNTO 0);
				ELSE
					result <= B"00000000";
				END IF;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- External Assignments
	data_out <= result; 

	-- Internal Assignments
	data_above_high_threshold <= '1' WHEN (data_in >= B"00001010") ELSE '0';
	data_to_shift_register_1 <= data_above_high_threshold & data_in;

	above_threshold <= ((OR_REDUCE (thresholds(0))) OR (OR_REDUCE (thresholds(1))) 
								OR (OR_REDUCE (thresholds(2))));

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	shift_register_1 : altera_up_edge_detection_data_shift_register 
	GENERIC MAP (
		DW			=> 9,
		SIZE		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clock		=> clk,
		clken		=> data_en,
		shiftin	=> data_to_shift_register_1,
	
		-- Bidirectionals
	
		-- Outputs
		shiftout	=> shift_reg_out(0)
	);

	shift_register_2 : altera_up_edge_detection_data_shift_register 
	GENERIC MAP (
		DW			=> 9,
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
