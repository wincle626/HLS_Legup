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
-- * This module decodes a NTSC video stream.                                   *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_itu_656_decoder IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************

-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk				:IN		STD_LOGIC;
	reset				:IN		STD_LOGIC;

	TD_DATA			:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	

	ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	data				:BUFFER	STD_LOGIC_VECTOR(15 DOWNTO  0);	
	startofpacket	:BUFFER	STD_LOGIC;
	endofpacket		:BUFFER	STD_LOGIC;
	valid				:BUFFER	STD_LOGIC


);

END altera_up_video_itu_656_decoder;

ARCHITECTURE Behaviour OF altera_up_video_itu_656_decoder IS

	TYPE MATRIX IS ARRAY( 5 DOWNTO  1) OF STD_LOGIC_VECTOR( 7 DOWNTO  0);

-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	timing_reference				:STD_LOGIC;		-- 4-Bytes: FF 00 00 XY
	
	SIGNAL	start_of_an_even_line		:STD_LOGIC;
	SIGNAL	start_of_an_odd_line			:STD_LOGIC;
	
	SIGNAL	last_data						:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	
	-- Internal Registers
	SIGNAL	io_register						:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	video_shift_reg				:MATRIX;
	
	SIGNAL	possible_timing_reference	:STD_LOGIC;
	
	SIGNAL	active_video					:STD_LOGIC_VECTOR( 6 DOWNTO  1);	
	
	SIGNAL	last_line						:STD_LOGIC;
	
	SIGNAL	internal_data					:STD_LOGIC_VECTOR(15 DOWNTO  0);	
	SIGNAL	internal_startofpacket		:STD_LOGIC;
	SIGNAL	internal_valid					:STD_LOGIC;
	
	-- State Machine Registers
	
	-- Integers
	VARIABLE	i									:INTEGER;
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altera_up_video_decoder_add_endofpacket
	GENERIC ( 
		DW									:INTEGER
	);
	PORT (
		-- Inputs
		clk								:IN		STD_LOGIC;
		reset								:IN		STD_LOGIC;

		stream_in_data					:IN		STD_LOGIC_VECTOR(15 DOWNTO  0);
		stream_in_startofpacket		:IN		STD_LOGIC;
		stream_in_endofpacket		:IN		STD_LOGIC;
		stream_in_valid				:IN		STD_LOGIC;

		stream_out_ready				:IN		STD_LOGIC;
	
		-- Bidirectional

		-- Outputs

		stream_out_data				:BUFFER	STD_LOGIC_VECTOR(15 DOWNTO  0);
		stream_out_startofpacket	:BUFFER	STD_LOGIC;
		stream_out_endofpacket		:BUFFER	STD_LOGIC;
		stream_out_valid				:BUFFER	STD_LOGIC
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************
	-- Input Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			io_register	<= TD_DATA;
		END IF;
	END PROCESS;

	-- Output Registers

	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			FOR i IN 5 DOWNTO 2 LOOP
				video_shift_reg(i) <= video_shift_reg((i - 1));
			END LOOP;
			video_shift_reg(1) <= io_register;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF ((video_shift_reg(3) = B"11111111") AND 
					(video_shift_reg(2) = B"00000000") AND 
					(video_shift_reg(1) = B"00000000")) THEN
				possible_timing_reference <= '1';
			ELSE
				possible_timing_reference <= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				active_video 		<= B"000000";
			ELSIF ((start_of_an_even_line = '1') OR (start_of_an_odd_line = '1')) THEN
				active_video 		<= active_video(5 DOWNTO 1) &'1';
			ELSIF (timing_reference = '1') THEN
				active_video		<= B"000000";
			ELSE
				active_video(6 DOWNTO 2)	<= active_video(5 DOWNTO 1);
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				last_line <= '0';
			ELSIF (start_of_an_odd_line = '1') THEN
				last_line <= '1';
			ELSIF (start_of_an_even_line = '1') THEN
				last_line <= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			internal_data	<= video_shift_reg(5) &video_shift_reg(4);
		END IF;
	END PROCESS;

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF ((last_line = '0') AND (start_of_an_odd_line = '1')) THEN
				internal_startofpacket	<= '1';
			ELSIF ((last_line = '1') AND (start_of_an_even_line = '1')) THEN
				internal_startofpacket	<= '1';
			ELSIF (valid = '1') THEN
				internal_startofpacket	<= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (active_video(5) = '1') THEN
				internal_valid			<= internal_valid XOR '1';
			ELSE
				internal_valid			<= '0';
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************
	-- Output Assignments

	-- Internal Assignments
	last_data <= video_shift_reg(1);

	timing_reference <= '1' WHEN
		(  (possible_timing_reference = '1') AND 
		 ( (last_data(5) XOR last_data(4)) =  last_data(3)) AND 
		 ( (last_data(6) XOR last_data(4)) =  last_data(2)) AND 
		 ( (last_data(6) XOR last_data(5)) =  last_data(1)) AND 
		 ( (last_data(6) XOR last_data(5) XOR last_data(4)) =  last_data(0))
		) ELSE '0';
	
	start_of_an_even_line <= '1' WHEN ((timing_reference = '1') AND 
			(last_data(6) = '1') AND (last_data(5) = '0') AND (last_data(4) = '0'))
			ELSE '0';

	start_of_an_odd_line <= '1' WHEN ((timing_reference = '1') AND 
			(last_data(6) = '0') AND (last_data(5) = '0') AND (last_data(4) = '0'))
			ELSE '0';

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Add_EndofPacket : altera_up_video_decoder_add_endofpacket 
	GENERIC MAP ( 
		DW									=> 15
	)
	PORT MAP (
		-- Inputs
		clk								=> clk,
		reset								=> reset,
	
		stream_in_data					=> internal_data,
		stream_in_startofpacket		=> internal_startofpacket,
		stream_in_endofpacket		=> '0',
		stream_in_valid				=> internal_valid,
	
		stream_out_ready				=> ready,
		
		-- Bidirectional
	
		-- Outputs
	
		stream_out_data				=> data,
		stream_out_startofpacket	=> startofpacket,
		stream_out_endofpacket		=> endofpacket,
		stream_out_valid				=> valid
	);



END Behaviour;
