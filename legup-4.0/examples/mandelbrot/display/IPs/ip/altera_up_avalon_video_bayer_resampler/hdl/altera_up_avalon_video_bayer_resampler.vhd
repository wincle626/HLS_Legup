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
-- * This module convert Bayer Pattern video in regular RGB by joining four     *
-- *  adjacent pixels in one. This effectively halfs both the width and height  *
-- *  of the incoming video frame.                                              *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_bayer_resampler IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	WW				:INTEGER									:= 11;
	
	IMAGE_WIDTH	:INTEGER									:= 2592
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk								:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

	stream_in_data					:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	stream_in_startofpacket		:IN		STD_LOGIC;
	stream_in_endofpacket		:IN		STD_LOGIC;
	stream_in_empty				:IN		STD_LOGIC;
	stream_in_valid				:IN		STD_LOGIC;

	stream_out_ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	stream_in_ready				:BUFFER	STD_LOGIC;

	stream_out_data				:BUFFER	STD_LOGIC_VECTOR(23 DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_empty				:BUFFER	STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_bayer_resampler;

ARCHITECTURE Behaviour OF altera_up_avalon_video_bayer_resampler IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	shift_reg_data						:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	transfer_data						:STD_LOGIC;
	
	SIGNAL	red									:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	green									:STD_LOGIC_VECTOR( 8 DOWNTO  0);	
	SIGNAL	blue									:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	
	
	-- Internal Registers
	SIGNAL	saved_stream_in_startofpacket	:STD_LOGIC;
	
	SIGNAL	last_stream_in_data				:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	last_shift_reg_data				:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	
	SIGNAL	width									:STD_LOGIC_VECTOR(WW DOWNTO  0);	
	SIGNAL	pixel_toggle						:STD_LOGIC;
	SIGNAL	line_toggle							:STD_LOGIC;
	
	-- State Machine Registers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altshift_taps
	GENERIC (
		lpm_hint			:STRING;
		lpm_type			:STRING;
		number_of_taps	:INTEGER;
		tap_distance	:INTEGER;
		width				:INTEGER
	);
	PORT (
		-- Inputs
		clock		:IN		STD_LOGIC;

		clken		:IN		STD_LOGIC;
		shiftin	:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);
	
	
		-- Bidirectionals
	
		-- Outputs
		shiftout	:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0)

		-- synopsys translate_off
	
		-- synopsys translate_on
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	-- Output Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				stream_out_data				<= (OTHERS => '0');
				stream_out_startofpacket	<= '0';
				stream_out_endofpacket		<= '0';
				stream_out_empty				<= (OTHERS => '0');
				stream_out_valid				<= '0';
			ELSIF ((stream_out_ready = '1') OR (stream_out_valid = '0')) THEN
				stream_out_data				<= red & green(8 DOWNTO 1) & blue;
				stream_out_startofpacket	<= saved_stream_in_startofpacket;
				stream_out_endofpacket		<= stream_in_endofpacket;
				stream_out_empty				<= '0' & stream_in_empty;
				stream_out_valid				<= stream_in_valid AND 
														pixel_toggle AND line_toggle;
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				saved_stream_in_startofpacket <= '0';
			ELSIF ((stream_in_startofpacket = '1') AND (stream_in_valid = '1')) THEN
				saved_stream_in_startofpacket <= '1';
			ELSIF (stream_out_valid = '1') THEN
				saved_stream_in_startofpacket <= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				last_stream_in_data	<= (OTHERS => '0');
				last_shift_reg_data	<= (OTHERS => '0');
			ELSIF (transfer_data = '1') THEN
				last_stream_in_data	<= stream_in_data;
				last_shift_reg_data	<= shift_reg_data;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				width <= (OTHERS => '0');
			ELSIF ((transfer_data = '1') AND (stream_in_startofpacket = '1')) THEN
				width <= (OTHERS => '1');
			ELSIF ((transfer_data = '1') AND (width = (IMAGE_WIDTH - 1))) THEN
				width <= (OTHERS => '0');
			ELSIF (transfer_data = '1') THEN
				width <= width + 1;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				pixel_toggle <= '0';
			ELSIF ((transfer_data = '1') AND (stream_in_startofpacket = '1')) THEN
				pixel_toggle <= '1';
			ELSIF ((transfer_data = '1') AND (width = (IMAGE_WIDTH - 1))) THEN
				pixel_toggle <= '0';
			ELSIF (transfer_data = '1') THEN
				pixel_toggle <= pixel_toggle XOR '1';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				line_toggle <= '0';
			ELSIF ((transfer_data = '1') AND ((stream_in_startofpacket = '1') OR 
					 (stream_in_endofpacket = '1'))) THEN
				line_toggle <= '0';
			ELSIF ((transfer_data = '1') AND (width = (IMAGE_WIDTH - 1))) THEN
				line_toggle <= line_toggle XOR '1';
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	stream_in_ready <= stream_out_ready OR NOT stream_out_valid;

	-- Internal Assignments
	transfer_data <= stream_in_valid AND stream_in_ready;

`ifdef USE_RED_GREEN_LINE_FIRST
	red 	<= last_stream_in_data;
	green <= ('0' & last_shift_reg_data) + ('0' & stream_in_data);
	blue 	<= shift_reg_data;
`else
	red 	<= shift_reg_data;
	green <= ('0' & last_shift_reg_data) + ('0' & stream_in_data);
	blue 	<= last_stream_in_data;
`endif

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	bayern_pattern_shift_reg : altshift_taps 
	GENERIC MAP (
		lpm_hint			=> "RAM_BLOCK_TYPE=M4K",
		lpm_type			=> "altshift_taps",
		number_of_taps	=> 1,
		tap_distance	=> IMAGE_WIDTH,
		width				=> 8
	)
	PORT MAP (
		-- Inputs
		clock		=> clk,
	
		clken		=> transfer_data,
		shiftin	=> stream_in_data,
		
		
		-- Bidirectionals
		
		-- Outputs
		shiftout	=> shift_reg_data
	
		-- synopsys translate_off
		
		-- synopsys translate_on
	);


END Behaviour;
