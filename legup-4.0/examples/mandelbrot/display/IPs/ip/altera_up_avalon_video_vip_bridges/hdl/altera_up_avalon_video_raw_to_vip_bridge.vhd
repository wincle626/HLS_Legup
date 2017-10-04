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
-- * This module convert a RAW video packet stream into a VIP video packet      *
-- *  stream.                                                                   *
-- *                                                                            *
-- ******************************************************************************

--`define USE_1_SYMBOL_PER_BEAT
--`define USE_2_SYMBOLS_PER_BEAT
--`define USE_3_SYMBOLS_PER_BEAT

ENTITY altera_up_avalon_video_raw_to_vip_bridge IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW					:INTEGER									:= 23;
	EW					:INTEGER									:= 1;
	
	CTRL_PACKET_0	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00001111";
	CTRL_PACKET_1	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00000000";
	CTRL_PACKET_2	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00000010";
	CTRL_PACKET_3	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00001000";
	CTRL_PACKET_4	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00000000";
	CTRL_PACKET_5	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00000000";
	CTRL_PACKET_6	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00000001";
	CTRL_PACKET_7	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00001110";
	CTRL_PACKET_8	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00000000";
	CTRL_PACKET_9	:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"00000000"
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk					:IN		STD_LOGIC;
	reset					:IN		STD_LOGIC;

	raw_data				:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	raw_startofpacket	:IN		STD_LOGIC;
	raw_endofpacket	:IN		STD_LOGIC;
	raw_empty			:IN		STD_LOGIC_VECTOR(EW DOWNTO  0);	
	raw_valid			:IN		STD_LOGIC;

	vip_ready			:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	raw_ready			:BUFFER	STD_LOGIC;

	vip_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	vip_startofpacket	:BUFFER	STD_LOGIC;
	vip_endofpacket	:BUFFER	STD_LOGIC;
	vip_empty			:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	vip_valid			:BUFFER	STD_LOGIC

);

END Altera_UP_Avalon_Video_RAW_to_VIP_bridge;

ARCHITECTURE Behaviour OF Altera_UP_Avalon_Video_RAW_to_VIP_bridge IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************
	
	TYPE State_Type IS (	STATE_0_CREATE_CTRL_PACKET,
								STATE_1_START_CONVERT_TO_VIP,
								STATE_2_CONVERT_TO_VIP
							);
	
-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	ctrl_packet_data		:STD_LOGIC_VECTOR(DW DOWNTO  0);	
	
	-- Internal Registers
	SIGNAL	ctrl_packet_counter	:STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	
	-- State Machine Registers
	SIGNAL	s_raw_to_vip			:State_Type;	
	SIGNAL	ns_raw_to_vip			:State_Type;	
	
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
				s_raw_to_vip <= STATE_0_CREATE_CTRL_PACKET;
			ELSE
				s_raw_to_vip <= ns_raw_to_vip;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (s_raw_to_vip, vip_valid, vip_ready, ctrl_packet_counter, ns_raw_to_vip, 
				raw_endofpacket, raw_valid)
	BEGIN
	   CASE (s_raw_to_vip) IS
		WHEN STATE_0_CREATE_CTRL_PACKET =>
`ifdef USE_1_SYMBOL_PER_BEAT
			IF (((vip_valid = '0') OR (vip_ready = '1')) AND (ctrl_packet_counter = B"1001")) THEN
`elsif USE_2_SYMBOLS_PER_BEAT
			IF (((vip_valid = '0') OR (vip_ready = '1')) AND (ctrl_packet_counter = B"0101")) THEN
`else
			IF (((vip_valid = '0') OR (vip_ready = '1')) AND (ctrl_packet_counter = B"0011")) THEN
`endif
				ns_raw_to_vip <= STATE_1_START_CONVERT_TO_VIP;
			ELSE
				ns_raw_to_vip <= STATE_0_CREATE_CTRL_PACKET;
			END IF;
		WHEN STATE_1_START_CONVERT_TO_VIP =>
			IF ((vip_valid = '0') OR (vip_ready = '1')) THEN
				ns_raw_to_vip <= STATE_2_CONVERT_TO_VIP;
			ELSE
				ns_raw_to_vip <= STATE_1_START_CONVERT_TO_VIP;
			END IF;
		WHEN STATE_2_CONVERT_TO_VIP =>
			IF (((vip_valid = '0') OR (vip_ready = '1')) AND (raw_endofpacket = '1') AND (raw_valid = '1')) THEN
				ns_raw_to_vip <= STATE_0_CREATE_CTRL_PACKET;
			ELSE
				ns_raw_to_vip <= STATE_2_CONVERT_TO_VIP;
			END IF;
		WHEN OTHERS =>
			ns_raw_to_vip <= STATE_0_CREATE_CTRL_PACKET;
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
				vip_data					<= (OTHERS => '0');
				vip_startofpacket		<= '0';
				vip_endofpacket		<= '0';
				vip_empty				<= (OTHERS => '0');
				vip_valid				<= '0';
			ELSIF ((s_raw_to_vip = STATE_0_CREATE_CTRL_PACKET) AND 
					((vip_valid = '0') OR (vip_ready = '1'))) THEN
				vip_data					<= ctrl_packet_data;
				IF (ctrl_packet_counter = B"0000") THEN
					vip_startofpacket	<= '1';
				ELSE
					vip_startofpacket	<= '0';
				END IF;
`ifdef USE_1_SYMBOL_PER_BEAT
				IF (ctrl_packet_counter = B"1001") THEN
					vip_endofpacket	<= '1';
				ELSE
					vip_endofpacket	<= '0';
				END IF;
`elsif USE_2_SYMBOLS_PER_BEAT
				IF (ctrl_packet_counter = B"0101") THEN
					vip_endofpacket	<= '1';
				ELSE
					vip_endofpacket	<= '0';
				END IF;
`else
				IF (ctrl_packet_counter = B"0011") THEN
					vip_endofpacket	<= '1';
				ELSE
					vip_endofpacket	<= '0';
				END IF;
`endif
				vip_empty				<= (OTHERS => '0');
				vip_valid				<= '1';
			ELSIF ((s_raw_to_vip = STATE_1_START_CONVERT_TO_VIP) AND 
					((vip_valid = '0') OR (vip_ready = '1'))) THEN
				vip_data					<= (OTHERS => '0');
				vip_startofpacket		<= '1';
				vip_endofpacket		<= '0';
				vip_empty				<= (OTHERS => '0');
				vip_valid				<= '1';
			ELSIF ((s_raw_to_vip = STATE_2_CONVERT_TO_VIP) AND 
					((vip_valid = '0') OR (vip_ready = '1'))) THEN
				vip_data					<= raw_data;
				vip_startofpacket		<= '0';
				vip_endofpacket		<= raw_endofpacket;
				vip_empty				<= raw_empty;
				vip_valid				<= raw_valid;
			ELSIF (vip_ready = '1') THEN
				vip_valid				<= '0';
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				ctrl_packet_counter 	<= B"0000";
			ELSIF ((s_raw_to_vip = STATE_0_CREATE_CTRL_PACKET) AND 
					((vip_valid = '0') OR (vip_ready = '1'))) THEN
				ctrl_packet_counter 	<= ctrl_packet_counter + B"0001";
			ELSIF (s_raw_to_vip /= STATE_0_CREATE_CTRL_PACKET) THEN
				ctrl_packet_counter 	<= B"0000";
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	raw_ready <= (NOT vip_valid OR vip_ready) WHEN 
						(s_raw_to_vip = STATE_2_CONVERT_TO_VIP) ELSE '0';

	-- Internal Assignments
`ifdef USE_1_SYMBOL_PER_BEAT
	ctrl_packet_data <= 
			CTRL_PACKET_0 WHEN (ctrl_packet_counter = B"0000") ELSE 
			CTRL_PACKET_1 WHEN (ctrl_packet_counter = B"0001") ELSE 
			CTRL_PACKET_2 WHEN (ctrl_packet_counter = B"0010") ELSE 
			CTRL_PACKET_3 WHEN (ctrl_packet_counter = B"0011") ELSE 
			CTRL_PACKET_4 WHEN (ctrl_packet_counter = B"0100") ELSE 
			CTRL_PACKET_5 WHEN (ctrl_packet_counter = B"0101") ELSE 
			CTRL_PACKET_6 WHEN (ctrl_packet_counter = B"0110") ELSE 
			CTRL_PACKET_7 WHEN (ctrl_packet_counter = B"0111") ELSE 
			CTRL_PACKET_8 WHEN (ctrl_packet_counter = B"1000") ELSE 
			CTRL_PACKET_9;
`elsif USE_2_SYMBOLS_PER_BEAT
	ctrl_packet_data <= 
			(CTRL_PACKET_0 & (OTHERS => '0')) WHEN (ctrl_packet_counter = B"0000") ELSE 
			(CTRL_PACKET_2 & CTRL_PACKET_1) WHEN (ctrl_packet_counter = B"0001") ELSE 
			(CTRL_PACKET_4 & CTRL_PACKET_3) WHEN (ctrl_packet_counter = B"0010") ELSE 
			(CTRL_PACKET_6 & CTRL_PACKET_5) WHEN (ctrl_packet_counter = B"0011") ELSE 
			(CTRL_PACKET_8 & CTRL_PACKET_7) WHEN (ctrl_packet_counter = B"0100") ELSE
			CTRL_PACKET_9;
`elsif USE_3_SYMBOLS_PER_BEAT
	ctrl_packet_data <= 
			(CTRL_PACKET_0 & (OTHERS => '0')) WHEN (ctrl_packet_counter = B"0000") ELSE 
			(CTRL_PACKET_3 & CTRL_PACKET_2 & CTRL_PACKET_1) WHEN 
					(ctrl_packet_counter = B"0001") ELSE 
			(CTRL_PACKET_6 & CTRL_PACKET_5 & CTRL_PACKET_4) WHEN 
					(ctrl_packet_counter = B"0010") ELSE 
			(CTRL_PACKET_9 & CTRL_PACKET_8 & CTRL_PACKET_7);
`else
	ctrl_packet_data <= 
			(CTRL_PACKET_0 & (OTHERS => '0')) WHEN (ctrl_packet_counter = B"0000") ELSE 
			(CTRL_PACKET_4 & CTRL_PACKET_3 & CTRL_PACKET_2 & CTRL_PACKET_1) WHEN 
					(ctrl_packet_counter = B"0001") ELSE 
			(CTRL_PACKET_8 & CTRL_PACKET_7 & CTRL_PACKET_6 & CTRL_PACKET_5) WHEN 
					(ctrl_packet_counter = B"0010") ELSE 
			CTRL_PACKET_9;
`endif

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
