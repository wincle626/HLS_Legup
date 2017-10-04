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

-- ******************************************************************************
-- *                                                                            *
-- * This module combines two video streams by overlaying one onto the          *
-- *  other using alpha blending.  The foreground image must include alpha      *
-- *  bits to be used in the blending formula: Cn = (1 - a)Cb + (a)Cf           *
-- *  Cn - new color                                                            *
-- *  a  - alpha                                                                *
-- *  Cb - background colour                                                    *
-- *  Cf - foreground colour                                                    *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_alpha_blender IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************

-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk								:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

	background_data				:IN		STD_LOGIC_VECTOR(29 DOWNTO  0);	
	background_startofpacket	:IN		STD_LOGIC;
	background_endofpacket		:IN		STD_LOGIC;
	background_empty				:IN		STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	background_valid				:IN		STD_LOGIC;

	foreground_data				:IN		STD_LOGIC_VECTOR(39 DOWNTO  0);	
	foreground_startofpacket	:IN		STD_LOGIC;
	foreground_endofpacket		:IN		STD_LOGIC;
	foreground_empty				:IN		STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	foreground_valid				:IN		STD_LOGIC;

	output_ready					:IN		STD_LOGIC;

	-- Bidirectionals

	-- Outputs
	background_ready				:BUFFER	STD_LOGIC;

	foreground_ready				:BUFFER	STD_LOGIC;

	output_data						:BUFFER	STD_LOGIC_VECTOR(29 DOWNTO  0);	
	output_startofpacket			:BUFFER	STD_LOGIC;
	output_endofpacket			:BUFFER	STD_LOGIC;
	output_empty					:BUFFER	STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	output_valid					:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_alpha_blender;

ARCHITECTURE Behaviour OF altera_up_avalon_video_alpha_blender IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	new_red				:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	new_green			:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	new_blue				:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	
	SIGNAL	sync_foreground	:STD_LOGIC;
	SIGNAL	sync_background	:STD_LOGIC;
	
	SIGNAL	valid					:STD_LOGIC;
	
	-- Internal Registers
	
	-- State Machine Registers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
`ifdef USE_NORMAL_MODE
	COMPONENT altera_up_video_alpha_blender_normal
	PORT (
		-- Inputs
		background_data	:IN		STD_LOGIC_VECTOR(29 DOWNTO  0);
		foreground_data	:IN		STD_LOGIC_VECTOR(39 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		new_red				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);
		new_green			:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);
		new_blue				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0)
	);
	END COMPONENT;

`else
	COMPONENT altera_up_video_alpha_blender_simple
	PORT (
		-- Inputs
		background_data	:IN		STD_LOGIC_VECTOR(29 DOWNTO  0);
		foreground_data	:IN		STD_LOGIC_VECTOR(39 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		new_red				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);
		new_green			:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);
		new_blue				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0)
	);
	END COMPONENT;
`endif
	
BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************
	-- Output Registers

	-- Internal Registers

-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************
	-- Output Assignments
	background_ready <= (output_ready AND output_valid) OR sync_background;
	foreground_ready <= (output_ready AND output_valid) OR sync_foreground;

	output_data 			<= new_red & new_green & new_blue;
	output_startofpacket <= foreground_startofpacket;
	output_endofpacket 	<= foreground_endofpacket;
	output_empty 			<= B"00";
	output_valid 			<= valid;

	-- Internal Assignments
	sync_foreground <= (foreground_valid AND background_valid AND 
				((background_startofpacket AND NOT foreground_startofpacket) OR 
				(background_endofpacket AND NOT foreground_endofpacket)));
	sync_background <= (foreground_valid AND background_valid AND 
				((foreground_startofpacket AND NOT background_startofpacket) OR 
				(foreground_endofpacket AND NOT background_endofpacket)));

	valid <= foreground_valid AND background_valid AND 
					 NOT sync_foreground AND NOT sync_background;

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

`ifdef USE_NORMAL_MODE
	alpha_blender : altera_up_video_alpha_blender_normal 
	PORT MAP (
		-- Inputs
		background_data	=> background_data,
		foreground_data	=> foreground_data,
	
		-- Bidirectionals
	
		-- Outputs
		new_red				=> new_red,
		new_green			=> new_green,
		new_blue				=> new_blue
	);

`else
	alpha_blender : altera_up_video_alpha_blender_simple 
	PORT MAP (
		-- Inputs
		background_data	=> background_data,
		foreground_data	=> foreground_data,
	
		-- Bidirectionals
	
		-- Outputs
		new_red				=> new_red,
		new_green			=> new_green,
		new_blue				=> new_blue
	);

`endif



END Behaviour;
