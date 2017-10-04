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
-- * This module perform Canny Edge Detection on video streams on the DE        *
-- *  boards.                                                                   *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_edge_detection IS 

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
	clk					:IN		STD_LOGIC;
	reset					:IN		STD_LOGIC;

	in_data				:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	in_startofpacket	:IN		STD_LOGIC;
	in_endofpacket		:IN		STD_LOGIC;
	in_empty				:IN		STD_LOGIC;
	in_valid				:IN		STD_LOGIC;

	out_ready			:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	in_ready				:BUFFER	STD_LOGIC;

	out_data				:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	out_startofpacket	:BUFFER	STD_LOGIC;
	out_endofpacket	:BUFFER	STD_LOGIC;
	out_empty			:BUFFER	STD_LOGIC;
	out_valid			:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_edge_detection;

ARCHITECTURE Behaviour OF altera_up_avalon_video_edge_detection IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	transfer_data		:STD_LOGIC;
	
	SIGNAL	filter_1_data_out	:STD_LOGIC_VECTOR( 8 DOWNTO  0);		-- Gaussian_Smoothing
	SIGNAL	filter_2_data_out	:STD_LOGIC_VECTOR( 9 DOWNTO  0);		-- Sobel operator
	SIGNAL	filter_3_data_out	:STD_LOGIC_VECTOR( 7 DOWNTO  0);		-- Nonmaximum Suppression
	SIGNAL	filter_4_data_out	:STD_LOGIC_VECTOR( 7 DOWNTO  0);		-- Hyteresis
	SIGNAL	final_value			:STD_LOGIC_VECTOR( 7 DOWNTO  0);		-- Intensity Correction
	
	SIGNAL	pixel_info_in		:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	SIGNAL	pixel_info_out		:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	
	-- Internal Registers
	SIGNAL	data					:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	startofpacket		:STD_LOGIC;
	SIGNAL	endofpacket			:STD_LOGIC;
	SIGNAL	empty					:STD_LOGIC;
	SIGNAL	valid					:STD_LOGIC;
	
	SIGNAL	flush_pipeline		:STD_LOGIC;
	
	-- State Machine Registers
	
	-- Integers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altera_up_edge_detection_gaussian_smoothing_filter
	GENERIC ( 
		WIDTH		:INTEGER
	);
	PORT (
		-- Inputs
		clk		:IN		STD_LOGIC;
		reset		:IN		STD_LOGIC;

		data_in	:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);
		data_en	:IN		STD_LOGIC;

		-- Bidirectionals

		-- Outputs
		data_out	:BUFFER	STD_LOGIC_VECTOR( 8 DOWNTO  0)
	);
	END COMPONENT;

	COMPONENT altera_up_edge_detection_sobel_operator
	GENERIC ( 
		WIDTH		:INTEGER
	);
	PORT (
		-- Inputs
		clk		:IN		STD_LOGIC;
		reset		:IN		STD_LOGIC;

		data_in	:IN		STD_LOGIC_VECTOR( 8 DOWNTO  0);
		data_en	:IN		STD_LOGIC;

		-- Bidirectionals

		-- Outputs
		data_out	:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0)
	);
	END COMPONENT;

	COMPONENT altera_up_edge_detection_nonmaximum_suppression
	GENERIC ( 
		WIDTH		:INTEGER
	);
	PORT (
		-- Inputs
		clk		:IN		STD_LOGIC;
		reset		:IN		STD_LOGIC;

		data_in	:IN		STD_LOGIC_VECTOR( 9 DOWNTO  0);
		data_en	:IN		STD_LOGIC;

		-- Bidirectionals

		-- Outputs
		data_out	:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0)
	);
	END COMPONENT;

	COMPONENT altera_up_edge_detection_hysteresis
	GENERIC ( 
		WIDTH		:INTEGER
	);
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
	END COMPONENT;

	COMPONENT altera_up_edge_detection_pixel_info_shift_register
	GENERIC ( 
		SIZE		:INTEGER
	);
	PORT (
		-- Inputs
		clock		:IN		STD_LOGIC;
		clken		:IN		STD_LOGIC;
	
		shiftin	:IN		STD_LOGIC_VECTOR( 1 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		shiftout	:BUFFER	STD_LOGIC_VECTOR( 1 DOWNTO  0)
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
				out_data				<= B"00000000";
				out_startofpacket	<= '0';
				out_endofpacket	<= '0';
				out_empty			<= '0';
				out_valid			<= '0';
			ELSIF (transfer_data = '1') THEN
				out_data				<= final_value;
				out_startofpacket	<= pixel_info_out(1) AND NOT (AND_REDUCE (pixel_info_out));
				out_endofpacket	<= pixel_info_out(0) AND NOT (AND_REDUCE (pixel_info_out));
				out_empty			<= '0';
				out_valid			<= (OR_REDUCE (pixel_info_out));
			ELSIF (out_ready = '1') THEN
				out_valid			<= '0';
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				data					<= B"00000000";
				startofpacket		<= '0';
				endofpacket			<= '0';
				empty					<= '0';
				valid					<= '0';
			ELSIF (in_ready = '1') THEN
				data					<= in_data;
				startofpacket		<= in_startofpacket;
				endofpacket			<= in_endofpacket;
				empty					<= in_empty;
				valid					<= in_valid;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				flush_pipeline		<= '0';
			ELSIF ((in_ready = '1') AND (in_endofpacket = '1')) THEN
				flush_pipeline		<= '1';
			ELSIF ((in_ready = '1') AND (in_startofpacket = '1')) THEN
				flush_pipeline		<= '0';
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************
	-- Output Assignments
	in_ready <= in_valid AND (out_ready OR NOT out_valid);

	-- Internal Assignments
	transfer_data <= in_ready OR 
			(flush_pipeline AND (out_ready OR NOT out_valid));

--`ifdef USE_DOUBLE_INTENSITY
	final_value <= (filter_4_data_out(6 DOWNTO 0) & '0') OR
						(filter_4_data_out(7) & filter_4_data_out(7) & filter_4_data_out(7) &
						 filter_4_data_out(7) & filter_4_data_out(7) & filter_4_data_out(7) &
						 filter_4_data_out(7) & filter_4_data_out(7));
--`else
--	final_value <= filter_4_data_out;
--`endif

	pixel_info_in(1) <= in_valid AND NOT in_endofpacket; 
	pixel_info_in(0) <= in_valid AND NOT in_startofpacket;

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Filter_1 : altera_up_edge_detection_gaussian_smoothing_filter 
	GENERIC MAP ( 
		WIDTH		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clk		=> clk,
		reset		=> reset,
	
		data_in	=> data,
		data_en	=> transfer_data,
	
		-- Bidirectionals
	
		-- Outputs
		data_out	=> filter_1_data_out
	);

	Filter_2 : altera_up_edge_detection_sobel_operator 
	GENERIC MAP ( 
		WIDTH		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clk		=> clk,
		reset		=> reset,
	
		data_in	=> filter_1_data_out,
		data_en	=> transfer_data,
	
		-- Bidirectionals
	
		-- Outputs
		data_out	=> filter_2_data_out
	);

	Filter_3 : altera_up_edge_detection_nonmaximum_suppression 
	GENERIC MAP ( 
		WIDTH		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clk		=> clk,
		reset		=> reset,
	
		data_in	=> filter_2_data_out,
		data_en	=> transfer_data,
	
		-- Bidirectionals
	
		-- Outputs
		data_out	=> filter_3_data_out
	);

	Filter_4 : altera_up_edge_detection_hysteresis 
	GENERIC MAP ( 
		WIDTH		=> WIDTH
	)
	PORT MAP (
		-- Inputs
		clk		=> clk,
		reset		=> reset,
	
		data_in	=> filter_3_data_out,
		data_en	=> transfer_data,
	
		-- Bidirectionals
	
		-- Outputs
		data_out	=> filter_4_data_out
	);

	-- defparam Pixel_Info_Shift_Register.SIZE = WIDTH;	
	Pixel_Info_Shift_Register : altera_up_edge_detection_pixel_info_shift_register 
	GENERIC MAP ( 
		SIZE		=> (WIDTH * 5) + 28
	)
	PORT MAP (
		-- Inputs
		clock		=> clk,
		clken		=> transfer_data,
		
		shiftin	=> pixel_info_in,
	
		-- Bidirectionals
	
		-- Outputs
		shiftout	=> pixel_info_out
	);


END Behaviour;
