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
-- * This module decodes video input streams on the DE boards.                  *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_decoder IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	IW			:INTEGER									:= 9;
	
	OW			:INTEGER									:= 15;
	FW			:INTEGER									:= 17;
	
	PIXELS	:INTEGER									:= 1280
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk								:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

`ifdef USE_ONBOARD_VIDEO
	TD_CLK27							:IN		STD_LOGIC;
	TD_DATA							:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	TD_HS								:IN		STD_LOGIC;
	TD_VS								:IN		STD_LOGIC;
`else
	PIXEL_CLK						:IN		STD_LOGIC;
	LINE_VALID						:IN		STD_LOGIC;
	FRAME_VALID						:IN		STD_LOGIC;
	PIXEL_DATA						:IN		STD_LOGIC_VECTOR(IW DOWNTO  0);	
`endif

	stream_out_ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
`ifdef USE_ONBOARD_VIDEO
	TD_RESET							:BUFFER	STD_LOGIC;
`endif
	overflow_flag					:BUFFER	STD_LOGIC;

	stream_out_data				:BUFFER	STD_LOGIC_VECTOR(OW DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_empty				:BUFFER	STD_LOGIC;
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_decoder;

ARCHITECTURE Behaviour OF altera_up_avalon_video_decoder IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	video_clk					:STD_LOGIC;
	
	SIGNAL	decoded_pixel				:STD_LOGIC_VECTOR(OW DOWNTO  0);	
	SIGNAL	decoded_startofpacket	:STD_LOGIC;
	SIGNAL	decoded_endofpacket		:STD_LOGIC;
	SIGNAL	decoded_valid				:STD_LOGIC;
	
	SIGNAL	data_from_fifo				:STD_LOGIC_VECTOR(FW DOWNTO  0);	
	SIGNAL	fifo_used_words			:STD_LOGIC_VECTOR( 6 DOWNTO  0);	
	SIGNAL	wrusedw						:STD_LOGIC_VECTOR( 6 DOWNTO  0);	
	SIGNAL	wrfull						:STD_LOGIC;
	
	SIGNAL	rdempty						:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	reached_start_of_frame	:STD_LOGIC;
	
	-- State Machine Registers
	
	-- Integers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
`ifdef USE_ONBOARD_VIDEO
	COMPONENT altera_up_video_itu_656_decoder
	PORT (
		-- Inputs
		clk				:IN		STD_LOGIC;
		reset				:IN		STD_LOGIC;

		TD_DATA			:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);

		ready				:IN		STD_LOGIC;

		-- Bidirectionals

		-- Outputs
		data				:BUFFER	STD_LOGIC_VECTOR(OW DOWNTO  0);
		startofpacket	:BUFFER	STD_LOGIC;
		endofpacket		:BUFFER	STD_LOGIC;
		valid				:BUFFER	STD_LOGIC
	);
	END COMPONENT;
`else
	COMPONENT altera_up_video_camera_decoder
	GENERIC (
		DW					:INTEGER
	);
	PORT (
		-- Inputs
		clk				:IN		STD_LOGIC;
		reset				:IN		STD_LOGIC;

		PIXEL_DATA		:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);
		LINE_VALID		:IN		STD_LOGIC;
		FRAME_VALID		:IN		STD_LOGIC;

		ready				:IN		STD_LOGIC;

		-- Bidirectionals

		-- Outputs
		data				:BUFFER	STD_LOGIC_VECTOR(OW DOWNTO  0);
		startofpacket	:BUFFER	STD_LOGIC;
		endofpacket		:BUFFER	STD_LOGIC;
		valid				:BUFFER	STD_LOGIC
	);
	END COMPONENT;
`endif

	COMPONENT altera_up_video_dual_clock_fifo
	GENERIC ( 
		DW			:INTEGER
	);
	PORT (
		-- Inputs
		wrclk		:IN		STD_LOGIC;
		wrreq		:IN		STD_LOGIC;
		data		:IN		STD_LOGIC_VECTOR((OW + 2) DOWNTO 0);
	
		rdclk		:IN		STD_LOGIC;
		rdreq		:IN		STD_LOGIC;

		-- Bidirectionals

		-- Outputs
		wrusedw	:BUFFER	STD_LOGIC_VECTOR( 6 DOWNTO  0);
		wrfull	:BUFFER	STD_LOGIC;
		
		q			:BUFFER	STD_LOGIC_VECTOR(FW DOWNTO  0);
		rdusedw	:BUFFER	STD_LOGIC_VECTOR( 6 DOWNTO  0);
		rdempty	:BUFFER	STD_LOGIC
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
	PROCESS (video_clk)
	BEGIN
		IF video_clk'EVENT AND video_clk = '1' THEN
			IF (reset = '1') THEN
				overflow_flag <= '0';
			ELSIF ((decoded_valid = '1') AND (reached_start_of_frame = '1') AND 
					 (wrfull = '1')) THEN
				overflow_flag <= '1';
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (video_clk)
	BEGIN
		IF video_clk'EVENT AND video_clk = '1' THEN
			IF (reset = '1') THEN
				reached_start_of_frame <= '0';
			ELSIF ((decoded_valid = '1') AND (decoded_startofpacket = '1')) THEN
				reached_start_of_frame <= '1';
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
`ifdef USE_ONBOARD_VIDEO
	TD_RESET 						<= '1';
`endif

	stream_out_data 				<= data_from_fifo( OW DOWNTO   0);
	stream_out_startofpacket 	<= data_from_fifo((FW - 1));
	stream_out_endofpacket 		<= data_from_fifo(FW);
	stream_out_empty 				<= '0';
	stream_out_valid 				<= NOT rdempty;

	-- Internal Assignments
`ifdef USE_ONBOARD_VIDEO
	video_clk 						<= TD_CLK27;
`else
	video_clk 						<= PIXEL_CLK;
`endif

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

`ifdef USE_ONBOARD_VIDEO
	-- NTSC Video In Decoding
	ITU_R_656_Decoder : altera_up_video_itu_656_decoder 
	PORT MAP (
		-- Inputs
		clk				=> video_clk,
		reset				=> reset,
	
		TD_DATA			=> TD_DATA,
	
		ready				=> decoded_valid AND NOT wrfull,
	
		-- Bidirectionals
	
		-- Outputs
		data				=> decoded_pixel,
		startofpacket	=> decoded_startofpacket,
		endofpacket		=> decoded_endofpacket,
		valid				=> decoded_valid
	);

`else
	-- Camera Daughter Card Decoding
	Camera_Decoder : altera_up_video_camera_decoder 
	GENERIC MAP (
		DW					=> 7
	)
	PORT MAP (
		-- Inputs
		clk				=> video_clk,
		reset				=> reset,
	
`ifdef USE_5MP_CAMERA
		PIXEL_DATA		=> PIXEL_DATA(11 DOWNTO  4),
`else
		PIXEL_DATA		=> PIXEL_DATA( 9 DOWNTO  2),
`endif
		LINE_VALID		=> LINE_VALID,
		FRAME_VALID		=> FRAME_VALID,
	
		ready				=> decoded_valid AND NOT wrfull,
	
		-- Bidirectionals
	
		-- Outputs
		data				=> decoded_pixel,
		startofpacket	=> decoded_startofpacket,
		endofpacket		=> decoded_endofpacket,
		valid				=> decoded_valid
	);
`endif

	Video_In_Dual_Clock_FIFO : altera_up_video_dual_clock_fifo 
	GENERIC MAP ( 
		DW			=> (FW + 1)
	)
	PORT MAP (
		-- Inputs
		wrclk		=> video_clk,
`ifdef USE_ONBOARD_VIDEO
		wrreq		=> decoded_valid AND reached_start_of_frame AND NOT wrfull,
`else
		wrreq		=> decoded_valid AND NOT wrfull,
`endif
		data		=> decoded_endofpacket & decoded_startofpacket & decoded_pixel,
		
		rdclk		=> clk,
		rdreq		=> stream_out_valid AND stream_out_ready,
	
		-- Bidirectionals
	
		-- Outputs
		wrusedw	=> wrusedw,
		wrfull	=> wrfull,
			
		q			=> data_from_fifo,
		rdusedw	=> fifo_used_words,
		rdempty	=> rdempty
	);


END Behaviour;
