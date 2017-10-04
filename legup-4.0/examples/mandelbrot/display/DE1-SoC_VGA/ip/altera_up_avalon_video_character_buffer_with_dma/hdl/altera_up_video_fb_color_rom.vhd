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
-- * This module contains a color map for foreground and background of          *
-- *  characters.                                                               *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_fb_color_rom IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************

-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk			:IN		STD_LOGIC;
	clk_en		:IN		STD_LOGIC;

	color_index	:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);	

	-- Bidirectionals

	-- Outputs
	red			:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	green			:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	blue			:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0)	

);

END altera_up_video_fb_color_rom;

ARCHITECTURE Behaviour OF altera_up_video_fb_color_rom IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	color_data	:STD_LOGIC_VECTOR(29 DOWNTO  0);	
	
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

		address_a		:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		q_a				:BUFFER	STD_LOGIC_VECTOR(29 DOWNTO  0)
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

	red 	<= color_data(29 DOWNTO 20);
	green <= color_data(19 DOWNTO 10);
	blue 	<= color_data( 9 DOWNTO  0);

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	color_data_rom : altsyncram 
	GENERIC MAP (
		clock_enable_input_a						=> "NORMAL",
		clock_enable_output_a					=> "NORMAL",
		init_file									=> "altera_up_video_fb_color_rom.mif",
		intended_device_family					=> "Cyclone II",
		lpm_hint										=> "ENABLE_RUNTIME_MOD=NO",
		lpm_type										=> "altsyncram",
		numwords_a									=> 16,
		operation_mode								=> "ROM",
		outdata_aclr_a								=> "NONE",
		outdata_reg_a								=> "CLOCK0",
		power_up_uninitialized					=> "FALSE",
		read_during_write_mode_mixed_ports	=> "DONT_CARE",
		widthad_a									=> 4,
		width_a										=> 30,
		width_byteena_a							=> 1
	)
	PORT MAP (
		-- Inputs
		clock0			=> clk,
		clocken0			=> clk_en,
	
		address_a		=> color_index,
	
		-- Bidirectionals
	
		-- Outputs
		q_a				=> color_data
	);
	

END Behaviour;
