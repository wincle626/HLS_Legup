LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_unsigned.all;
USE ieee.std_logic_misc.all;
USE ieee.numeric_std.all;

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
-- * This module increases the frame size of video streams.                     *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_clipper_add IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW							:INTEGER									:= 15; 	-- Image's Data Width
	EW							:INTEGER									:= 0; 	-- Image's Empty Width
	
	IMAGE_WIDTH				:INTEGER									:= 640; 	-- Final image width in pixels
	IMAGE_HEIGHT			:INTEGER									:= 480; 	-- Final image height in lines
	WW							:INTEGER									:= 9; 	-- Final image width address width
	HW							:INTEGER									:= 8; 	-- Final image height address width
	
	ADD_PIXELS_AT_START	:INTEGER									:= 0;
	ADD_PIXELS_AT_END		:INTEGER									:= 0;
	ADD_LINES_AT_START	:INTEGER									:= 0;
	ADD_LINES_AT_END		:INTEGER									:= 0;
	
	ADD_DATA					:STD_LOGIC_VECTOR(15 DOWNTO  0)	:= B"0000000000000000" -- Data for added pixels
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk								:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

	stream_in_data					:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_in_startofpacket		:IN		STD_LOGIC;
	stream_in_endofpacket		:IN		STD_LOGIC;
	stream_in_empty				:IN		STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_in_valid				:IN		STD_LOGIC;

	stream_out_ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	stream_in_ready				:BUFFER	STD_LOGIC;

	stream_out_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_empty				:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_video_clipper_add;

ARCHITECTURE Behaviour OF altera_up_video_clipper_add IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	increment_counters	:STD_LOGIC;
	
	SIGNAL	new_startofpacket		:STD_LOGIC;
	SIGNAL	new_endofpacket		:STD_LOGIC;
	SIGNAL	pass_inner_frame		:STD_LOGIC;
	
	-- Internal Registers
	
	-- State Machine Registers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altera_up_video_clipper_counters
	GENERIC (	
		IMAGE_WIDTH				:INTEGER;
		IMAGE_HEIGHT			:INTEGER;
		WW							:INTEGER;
		HW							:INTEGER;

		LEFT_OFFSET				:INTEGER;
		RIGHT_OFFSET			:INTEGER;
		TOP_OFFSET				:INTEGER;
		BOTTOM_OFFSET			:INTEGER
	);
	PORT (
		-- Inputs
		clk						:IN		STD_LOGIC;
		reset						:IN		STD_LOGIC;

		increment_counters	:IN		STD_LOGIC;

		-- Bidirectional

		-- Outputs
		start_of_outer_frame	:BUFFER	STD_LOGIC;
		end_of_outer_frame	:BUFFER	STD_LOGIC;
		inner_frame_valid		:BUFFER	STD_LOGIC
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	-- Output registers
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
				IF (pass_inner_frame = '1') THEN
					stream_out_data			<= stream_in_data;
				ELSE
					stream_out_data			<= ADD_DATA;
				END IF;
				stream_out_startofpacket	<= new_startofpacket;
				stream_out_endofpacket		<= new_endofpacket;
				stream_out_empty				<= (OTHERS => '0');
				IF (pass_inner_frame = '1') THEN
					stream_out_valid			<= stream_in_valid;
				ELSE
					stream_out_valid			<= '1';
				END IF;
			END IF;
		END IF;
	END PROCESS;



	-- Internal registers


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output assignments
	stream_in_ready <= pass_inner_frame AND (NOT stream_out_valid OR stream_out_ready);

	-- Internal assignments
	increment_counters <= (NOT stream_out_valid OR stream_out_ready) AND 
										  (NOT pass_inner_frame OR stream_in_valid);

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Clipper_Add_Counters : altera_up_video_clipper_counters 
	GENERIC MAP (	
		IMAGE_WIDTH				=> IMAGE_WIDTH,
		IMAGE_HEIGHT			=> IMAGE_HEIGHT,
		WW							=> WW,
		HW							=> HW,

		LEFT_OFFSET				=> ADD_PIXELS_AT_START,
		RIGHT_OFFSET			=> ADD_PIXELS_AT_END,
		TOP_OFFSET				=> ADD_LINES_AT_START,
		BOTTOM_OFFSET			=> ADD_LINES_AT_END
	)
	PORT MAP (
		-- Inputs
		clk						=> clk,
		reset						=> reset,
	
		increment_counters	=> increment_counters,
	
		-- Bidirectional
	
		-- Outputs
		start_of_outer_frame	=> new_startofpacket,
		end_of_outer_frame	=> new_endofpacket,
	
		inner_frame_valid		=> pass_inner_frame
	);


END Behaviour;
