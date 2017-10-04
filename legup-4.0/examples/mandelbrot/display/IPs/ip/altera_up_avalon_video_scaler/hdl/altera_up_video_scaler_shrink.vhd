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
-- * This module scales down the frame size of video streams for the DE boards. *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_scaler_shrink IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW						:INTEGER									:= 15; -- Image's Data Width
	WW						:INTEGER									:= 9; -- Image In: width's address width
	HW						:INTEGER									:= 9; -- Image In: height's address width
	
	WIDTH_IN				:INTEGER									:= 640; -- Image In's width in pixels
	
	WIDTH_DROP_MASK	:STD_LOGIC_VECTOR( 3 DOWNTO  0)	:= B"0101";
	HEIGHT_DROP_MASK	:STD_LOGIC_VECTOR( 3 DOWNTO  0)	:= B"0000"
	
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
	stream_in_valid				:IN		STD_LOGIC;

	stream_out_ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	stream_in_ready				:BUFFER	STD_LOGIC;

	stream_out_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_video_scaler_shrink;

ARCHITECTURE Behaviour OF altera_up_video_scaler_shrink IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	drop						:STD_LOGIC;
	
	SIGNAL	capture_inputs			:STD_LOGIC;
	
	SIGNAL	transfer_data			:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	saved_startofpacket	:STD_LOGIC;
	
	SIGNAL	data						:STD_LOGIC_VECTOR(DW DOWNTO  0);	
	SIGNAL	startofpacket			:STD_LOGIC;
	SIGNAL	endofpacket				:STD_LOGIC;
	SIGNAL	valid						:STD_LOGIC;
	
	SIGNAL	width_counter			:STD_LOGIC_VECTOR(WW DOWNTO  0);	
	SIGNAL	height_counter			:STD_LOGIC_VECTOR(HW DOWNTO  0);	
	SIGNAL	drop_pixel				:STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	SIGNAL	drop_line				:STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	
	-- State Machine Registers
	
	-- Integers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
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
				stream_out_data				<=  (OTHERS => '0');
				stream_out_startofpacket	<= '0';
				stream_out_endofpacket		<= '0';
				stream_out_valid				<= '0';
			ELSIF (transfer_data = '1') THEN
				stream_out_data				<= data;
				stream_out_startofpacket	<= startofpacket;
				stream_out_endofpacket		<= endofpacket;
				stream_out_valid				<= valid;
			ELSIF ((stream_out_ready = '1') AND (stream_out_valid = '1')) THEN
				stream_out_data				<=  (OTHERS => '0');
				stream_out_startofpacket	<= '0';
				stream_out_endofpacket		<= '0';
				stream_out_valid				<= '0';
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				saved_startofpacket	<= '0';
			ELSIF (capture_inputs = '1') THEN
				saved_startofpacket	<= '0';
			ELSIF (stream_in_ready = '1') THEN
				saved_startofpacket	<= saved_startofpacket OR stream_in_startofpacket;
			END IF;
		END IF;
	END PROCESS;

		
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				data				<=  (OTHERS => '0');
				startofpacket	<= '0';
				endofpacket		<= '0';
				valid				<= '0';
			ELSIF (capture_inputs = '1') THEN
				data				<= stream_in_data;
				startofpacket	<= stream_in_startofpacket OR saved_startofpacket;
				endofpacket		<= stream_in_endofpacket;
				valid				<= stream_in_valid;
			ELSIF (stream_in_ready = '1') THEN
				endofpacket		<= endofpacket OR stream_in_endofpacket;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				width_counter		<= (OTHERS => '0');
			ELSIF (stream_in_ready = '1') THEN
				IF ((stream_in_startofpacket = '1') OR (width_counter = (WIDTH_IN - 1))) THEN
					width_counter	<= (OTHERS => '0');
				ELSE
					width_counter	<= width_counter + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				height_counter		<= (OTHERS => '0');
			ELSIF (stream_in_ready = '1') THEN
				IF (stream_in_startofpacket = '1') THEN
					height_counter	<= (OTHERS => '0');
				ELSIF (width_counter = (WIDTH_IN - 1)) THEN
					height_counter	<= height_counter + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				drop_pixel		<= B"0000";
			ELSIF (stream_in_ready = '1') THEN
				IF (stream_in_startofpacket = '1') THEN
					drop_pixel	<= WIDTH_DROP_MASK;
				ELSIF (width_counter = (WIDTH_IN - 1)) THEN
					drop_pixel	<= WIDTH_DROP_MASK;
				ELSE
					drop_pixel	<= (drop_pixel(2 DOWNTO 0) & drop_pixel(3));		
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				drop_line		<= B"0000";
			ELSIF (stream_in_ready = '1') THEN
				IF (stream_in_startofpacket = '1') THEN
					drop_line	<= HEIGHT_DROP_MASK;
				ELSIF (width_counter = (WIDTH_IN - 1)) THEN
					drop_line	<= (drop_line(2 DOWNTO 0) & drop_line(3));
				END IF;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	stream_in_ready <= '1' WHEN (stream_in_valid = '1') AND 
							((drop = '1') OR (valid = '0') OR (transfer_data = '1')) ELSE
							'0';

	-- Internal Assignments
	drop <= drop_pixel(0) OR drop_line(0);

	capture_inputs <= stream_in_ready AND NOT drop;

	transfer_data <= NOT stream_out_valid AND stream_in_valid AND NOT drop;

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
