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
-- * This module contains a character map for 128 different characters.         *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_128_character_rom IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************

-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk				:IN		STD_LOGIC;
	clk_en			:IN		STD_LOGIC;

	character		:IN		STD_LOGIC_VECTOR( 6 DOWNTO  0);	
	x_coordinate	:IN		STD_LOGIC_VECTOR( 2 DOWNTO  0);	
	y_coordinate	:IN		STD_LOGIC_VECTOR( 2 DOWNTO  0);	

	-- Bidirectionals

	-- Outputs
	character_data	:BUFFER	STD_LOGIC

);

END altera_up_video_128_character_rom;

ARCHITECTURE Behaviour OF altera_up_video_128_character_rom IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	character_address	:STD_LOGIC_VECTOR(12 DOWNTO  0);	
	
	-- Internal Registers
	
	-- State Machine Registers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altsyncram
	GENERIC (
		clock_enable_input_a						:STRING;
		clock_enable_output_a					:STRING;
		init_file									:STRING;
		intended_device_family					:STRING;
		lpm_hint										:STRING;
		lpm_type										:STRING;
		numwords_a									:NATURAL;
		operation_mode								:STRING;
		outdata_aclr_a								:STRING;
		outdata_reg_a								:STRING;
		power_up_uninitialized					:STRING;
		read_during_write_mode_mixed_ports	:STRING;
		widthad_a									:NATURAL;
		width_a										:NATURAL;
		width_byteena_a							:NATURAL
	);
	PORT (
		-- Inputs
		clock0			:IN		STD_LOGIC;
		clocken0			:IN		STD_LOGIC;

		address_a		:IN		STD_LOGIC_VECTOR(12 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		q_a				:BUFFER	STD_LOGIC
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	character_address <= character & y_coordinate & x_coordinate;

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	character_data_rom : altsyncram 
	GENERIC MAP (
		clock_enable_input_a						=> "NORMAL",
		clock_enable_output_a					=> "NORMAL",
		init_file									=> "altera_up_video_char_mode_rom_128.mif",
		intended_device_family					=> "Cyclone II",
		lpm_hint										=> "ENABLE_RUNTIME_MOD=NO",
		lpm_type										=> "altsyncram",
		numwords_a									=> 8192,
		operation_mode								=> "ROM",
		outdata_aclr_a								=> "NONE",
		outdata_reg_a								=> "CLOCK0",
		power_up_uninitialized					=> "FALSE",
		read_during_write_mode_mixed_ports	=> "DONT_CARE",
		widthad_a									=> 13,
		width_a										=> 1,
		width_byteena_a							=> 1
	)
	PORT MAP (
		-- Inputs
		clock0			=> clk,
		clocken0			=> clk_en,
	
		address_a		=> character_address,
	
		-- Bidirectionals
	
		-- Outputs
		q_a				=> character_data
	);
	
END Behaviour;
