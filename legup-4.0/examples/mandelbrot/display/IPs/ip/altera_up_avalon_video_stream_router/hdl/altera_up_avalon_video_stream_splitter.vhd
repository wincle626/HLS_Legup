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
-- * This module splits video in streams for the DE-series boards.              *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_stream_splitter IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW	:INTEGER									:= 15; -- Frame's data width
	EW	:INTEGER									:= 0 -- Frame's empty width
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk								:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

`ifdef USE_SYNC
	sync_ready						:IN		STD_LOGIC;

`endif
	stream_in_data					:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_in_startofpacket		:IN		STD_LOGIC;
	stream_in_endofpacket		:IN		STD_LOGIC;
	stream_in_empty				:IN		STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_in_valid				:IN		STD_LOGIC;

	stream_out_ready_0			:IN		STD_LOGIC;

	stream_out_ready_1			:IN		STD_LOGIC;

	stream_select					:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
`ifdef USE_SYNC
	sync_data						:BUFFER	STD_LOGIC;
	sync_valid						:BUFFER	STD_LOGIC;

`endif
	stream_in_ready				:BUFFER	STD_LOGIC;

	stream_out_data_0				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_out_startofpacket_0	:BUFFER	STD_LOGIC;
	stream_out_endofpacket_0	:BUFFER	STD_LOGIC;
	stream_out_empty_0			:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_out_valid_0			:BUFFER	STD_LOGIC;

	stream_out_data_1				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_out_startofpacket_1	:BUFFER	STD_LOGIC;
	stream_out_endofpacket_1	:BUFFER	STD_LOGIC;
	stream_out_empty_1			:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_out_valid_1			:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_stream_splitter;

ARCHITECTURE Behaviour OF altera_up_avalon_video_stream_splitter IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	enable_setting_stream_select	:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	between_frames						:STD_LOGIC;
	SIGNAL	stream_select_reg					:STD_LOGIC;
	
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
`ifdef USE_SYNC
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				sync_data	<= '0';
				sync_valid	<= '0';
			ELSIF (enable_setting_stream_select = '1') THEN
				sync_data	<= stream_select;
				sync_valid	<= '1';
			ELSIF (sync_ready = '1') THEN
				sync_valid	<= '0';
			END IF;
		END IF;
	END PROCESS;

`endif

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				stream_out_data_0				<=  (OTHERS => '0');
				stream_out_startofpacket_0	<= '0';
				stream_out_endofpacket_0	<= '0';
				stream_out_empty_0			<=  (OTHERS => '0');
				stream_out_valid_0			<= '0';
			ELSIF ((stream_in_ready = '1') AND (stream_select_reg = '0')) THEN
				stream_out_data_0				<= stream_in_data;
				stream_out_startofpacket_0	<= stream_in_startofpacket;
				stream_out_endofpacket_0	<= stream_in_endofpacket;
				stream_out_empty_0			<= stream_in_empty;
				stream_out_valid_0			<= stream_in_valid;
			ELSIF (stream_out_ready_0 = '1') THEN
				stream_out_valid_0			<= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				stream_out_data_1				<=  (OTHERS => '0');
				stream_out_startofpacket_1	<= '0';
				stream_out_endofpacket_1	<= '0';
				stream_out_empty_1			<=  (OTHERS => '0');
				stream_out_valid_1			<= '0';
			ELSIF ((stream_in_ready = '1') AND (stream_select_reg = '1')) THEN
				stream_out_data_1				<= stream_in_data;
				stream_out_startofpacket_1	<= stream_in_startofpacket;
				stream_out_endofpacket_1	<= stream_in_endofpacket;
				stream_out_empty_1			<= stream_in_empty;
				stream_out_valid_1			<= stream_in_valid;
			ELSIF (stream_out_ready_1 = '1') THEN
				stream_out_valid_1			<= '0';
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				between_frames <= '1';
			ELSIF ((stream_in_ready = '1') AND (stream_in_endofpacket = '1')) THEN
				between_frames <= '1';
			ELSIF ((stream_in_ready = '1') AND (stream_in_startofpacket = '1')) THEN
				between_frames <= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				stream_select_reg <= '0';
			ELSIF (enable_setting_stream_select = '1') THEN
				stream_select_reg <= stream_select;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	stream_in_ready <= stream_in_valid AND (NOT stream_out_valid_1 OR stream_out_ready_1) 
							WHEN (stream_select_reg = '1') ELSE 
							stream_in_valid AND (NOT stream_out_valid_0 OR stream_out_ready_0);

	-- Internal Assignments
	enable_setting_stream_select <= 
			  (stream_in_ready AND stream_in_endofpacket) OR 
			(NOT (stream_in_ready AND stream_in_startofpacket) AND between_frames);

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
