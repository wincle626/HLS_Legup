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
-- * This module removes VIP packet from a video stream and passes only RAW     *
-- *  video frame packet downstream.                                            *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_vip_to_raw_bridge IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW	:INTEGER									:= 23;
	EW	:INTEGER									:= 1
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk					:IN		STD_LOGIC;
	reset					:IN		STD_LOGIC;

	vip_data				:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	vip_startofpacket	:IN		STD_LOGIC;
	vip_endofpacket	:IN		STD_LOGIC;
	vip_empty			:IN		STD_LOGIC_VECTOR(EW DOWNTO  0);	
	vip_valid			:IN		STD_LOGIC;

	raw_ready			:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	vip_ready			:BUFFER	STD_LOGIC;

	raw_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	raw_startofpacket	:BUFFER	STD_LOGIC;
	raw_endofpacket	:BUFFER	STD_LOGIC;
	raw_empty			:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	raw_valid			:BUFFER	STD_LOGIC

);

END Altera_UP_Avalon_Video_VIP_to_RAW_bridge;

ARCHITECTURE Behaviour OF Altera_UP_Avalon_Video_VIP_to_RAW_bridge IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************
	
	TYPE State_Type IS (	STATE_0_DISCARD_PACKET,
								STATE_1_START_CONVERT_TO_RAW,
								STATE_2_CONVERT_TO_RAW
							);
	
-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	incoming_vip_packet_is_frame_data	:STD_LOGIC;
	
	-- Internal Registers
	
	-- State Machine Registers
	SIGNAL	s_vip_to_raw								:State_Type;	
	SIGNAL	ns_vip_to_raw								:State_Type;	
	
	-- Integers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				s_vip_to_raw <= STATE_0_DISCARD_PACKET;
			ELSE
				s_vip_to_raw <= ns_vip_to_raw;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (s_vip_to_raw, incoming_vip_packet_is_frame_data, ns_vip_to_raw, raw_valid, 
				raw_ready, vip_valid, vip_endofpacket)
	BEGIN
	   CASE (s_vip_to_raw) IS
		WHEN STATE_0_DISCARD_PACKET =>
			IF (incoming_vip_packet_is_frame_data = '1') THEN
				ns_vip_to_raw <= STATE_1_START_CONVERT_TO_RAW;
			ELSE
				ns_vip_to_raw <= STATE_0_DISCARD_PACKET;
			END IF;
		WHEN STATE_1_START_CONVERT_TO_RAW =>
			IF (((raw_valid = '0') OR (raw_ready = '1')) AND (vip_valid = '1')) THEN
				ns_vip_to_raw <= STATE_2_CONVERT_TO_RAW;
			ELSE
				ns_vip_to_raw <= STATE_1_START_CONVERT_TO_RAW;
			END IF;
		WHEN STATE_2_CONVERT_TO_RAW =>
			IF (((raw_valid = '0') OR (raw_ready = '1')) AND 
					(vip_endofpacket = '1') AND (vip_valid = '1')) THEN
				ns_vip_to_raw <= STATE_0_DISCARD_PACKET;
			ELSE
				ns_vip_to_raw <= STATE_2_CONVERT_TO_RAW;
			END IF;
		WHEN OTHERS =>
			ns_vip_to_raw <= STATE_0_DISCARD_PACKET;
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
				raw_data				<= (OTHERS => '0');
				raw_startofpacket	<= '0';
				raw_endofpacket	<= '0';
				raw_empty			<= (OTHERS => '0');
				raw_valid			<= '0';
			ELSIF ((s_vip_to_raw = STATE_1_START_CONVERT_TO_RAW) AND 
					((raw_valid = '0') OR (raw_ready = '1'))) THEN
				raw_data				<= vip_data;
				raw_startofpacket	<= '1';
				raw_endofpacket	<= vip_endofpacket;
				raw_empty			<= vip_empty;
				raw_valid			<= vip_valid;
			ELSIF ((s_vip_to_raw = STATE_2_CONVERT_TO_RAW) AND 
					((raw_valid = '0') OR (raw_ready = '1'))) THEN
				raw_data				<= vip_data;
				raw_startofpacket	<= '0';
				raw_endofpacket	<= vip_endofpacket;
				raw_empty			<= vip_empty;
				raw_valid			<= vip_valid;
			ELSIF (raw_ready = '1') THEN
				raw_valid			<= '0';
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	vip_ready <= '1' WHEN (s_vip_to_raw = STATE_0_DISCARD_PACKET) ELSE 
					(NOT raw_valid OR raw_ready);

	-- Internal Assignments
	incoming_vip_packet_is_frame_data <= '1' WHEN (vip_valid = '1') AND
					(vip_startofpacket = '1') AND (vip_data = (OTHERS => '0')) ELSE 
					'0';

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
