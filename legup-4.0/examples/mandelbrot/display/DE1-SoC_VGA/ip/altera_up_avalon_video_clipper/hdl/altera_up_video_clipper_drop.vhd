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
-- * This module clips video streams on the DE boards. If the incoming image is *
-- *   the wrong size, the circuit will discard or add pixels as necessary.     *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_clipper_drop IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW							:INTEGER									:= 15; -- Image's Data Width
	EW							:INTEGER									:= 0; -- Image's Empty Width
	
	IMAGE_WIDTH				:INTEGER									:= 640; -- Image in width in pixels
	IMAGE_HEIGHT			:INTEGER									:= 480; -- Image in height in lines
	WW							:INTEGER									:= 9; -- Image in width address width
	HW							:INTEGER									:= 8; -- Image in height address width
	
	DROP_PIXELS_AT_START	:INTEGER									:= 0;
	DROP_PIXELS_AT_END	:INTEGER									:= 0;
	DROP_LINES_AT_START	:INTEGER									:= 0;
	DROP_LINES_AT_END		:INTEGER									:= 0;
	
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

END altera_up_video_clipper_drop;

ARCHITECTURE Behaviour OF altera_up_video_clipper_drop IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************
	
	TYPE State_Type IS (	STATE_0_WAIT_FOR_START,
								STATE_1_RUN_CLIPPER,
								STATE_2_ADD_MISSING_PART
							);
	
-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	increment_counters		:STD_LOGIC;
	
	SIGNAL	full_frame_endofpacket	:STD_LOGIC;
	SIGNAL	new_startofpacket			:STD_LOGIC;
	SIGNAL	new_endofpacket			:STD_LOGIC;
	SIGNAL	pass_inner_frame			:STD_LOGIC;
	
	-- Internal Registers
	
	-- State Machine Registers
	SIGNAL	s_video_clipper_drop		:State_Type;	
	SIGNAL	ns_video_clipper_drop	:State_Type;	
	
	-- Integers
	
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

		-- Bi-Directional

		-- Outputs
		end_of_outer_frame	:BUFFER	STD_LOGIC;

		start_of_inner_frame	:BUFFER	STD_LOGIC;
		end_of_inner_frame	:BUFFER	STD_LOGIC;
		inner_frame_valid		:BUFFER	STD_LOGIC
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				s_video_clipper_drop <= STATE_0_WAIT_FOR_START;
			ELSE
				s_video_clipper_drop <= ns_video_clipper_drop;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (s_video_clipper_drop, stream_in_startofpacket, stream_in_valid, 
				ns_video_clipper_drop, increment_counters, full_frame_endofpacket, 
				stream_in_endofpacket)
	BEGIN
	   CASE (s_video_clipper_drop) IS
		WHEN STATE_0_WAIT_FOR_START =>
			IF ((stream_in_startofpacket = '1') AND (stream_in_valid = '1')) THEN
				ns_video_clipper_drop <= STATE_1_RUN_CLIPPER;
			ELSE
				ns_video_clipper_drop <= STATE_0_WAIT_FOR_START;
			END IF;
		WHEN STATE_1_RUN_CLIPPER =>
			IF ((increment_counters = '1') AND (full_frame_endofpacket = '1')) THEN
				ns_video_clipper_drop <= STATE_0_WAIT_FOR_START;
			ELSIF ((increment_counters = '1') AND (stream_in_endofpacket = '1')) THEN
				ns_video_clipper_drop <= STATE_2_ADD_MISSING_PART;
			ELSE
				ns_video_clipper_drop <= STATE_1_RUN_CLIPPER;
			END IF;
		WHEN STATE_2_ADD_MISSING_PART =>
			IF ((increment_counters = '1') AND (full_frame_endofpacket = '1')) THEN
				ns_video_clipper_drop <= STATE_0_WAIT_FOR_START;
			ELSE
				ns_video_clipper_drop <= STATE_2_ADD_MISSING_PART;
			END IF;
		WHEN OTHERS =>
			ns_video_clipper_drop <= STATE_0_WAIT_FOR_START;
		END CASE;
	END PROCESS;



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
				IF (s_video_clipper_drop = STATE_2_ADD_MISSING_PART) THEN
					stream_out_data			<= ADD_DATA;
				ELSE
					stream_out_data			<= stream_in_data;
				END IF;
				stream_out_startofpacket	<= new_startofpacket;
				stream_out_endofpacket		<= new_endofpacket;
				stream_out_empty				<= stream_in_empty;
				IF (s_video_clipper_drop = STATE_1_RUN_CLIPPER) THEN
					stream_out_valid			<= pass_inner_frame AND stream_in_valid;
				ELSIF (s_video_clipper_drop = STATE_2_ADD_MISSING_PART) THEN
					stream_out_valid			<= pass_inner_frame;
				ELSE
					stream_out_valid			<= '0';
				END IF;
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers

-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	stream_in_ready <= 
		NOT (stream_in_startofpacket AND stream_in_valid) 
			WHEN (s_video_clipper_drop = STATE_0_WAIT_FOR_START) ELSE 
		NOT pass_inner_frame OR stream_out_ready OR NOT stream_out_valid 
			WHEN (s_video_clipper_drop = STATE_1_RUN_CLIPPER) ELSE 
		'0';

	-- Internal Assignments
	increment_counters <= 
		stream_in_valid AND stream_in_ready WHEN (s_video_clipper_drop = STATE_1_RUN_CLIPPER) ELSE 
		NOT pass_inner_frame OR stream_out_ready OR NOT stream_out_valid 
			WHEN (s_video_clipper_drop = STATE_2_ADD_MISSING_PART) ELSE 
		'0';

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Clipper_Drop_Counters : altera_up_video_clipper_counters 
	GENERIC MAP (
		IMAGE_WIDTH				=> IMAGE_WIDTH,
		IMAGE_HEIGHT			=> IMAGE_HEIGHT,
		WW							=> WW,
		HW							=> HW,

		LEFT_OFFSET				=> DROP_PIXELS_AT_START,
		RIGHT_OFFSET			=> DROP_PIXELS_AT_END,
		TOP_OFFSET				=> DROP_LINES_AT_START,
		BOTTOM_OFFSET			=> DROP_LINES_AT_END
	)
	PORT MAP (
		-- Inputs
		clk						=> clk,
		reset						=> reset,
	
		increment_counters	=> increment_counters,
	
		-- Bi-Directional
	
		-- Outputs
		end_of_outer_frame	=> full_frame_endofpacket,
	
		start_of_inner_frame	=> new_startofpacket,
		end_of_inner_frame	=> new_endofpacket,
		inner_frame_valid		=> pass_inner_frame
	);

END Behaviour;
