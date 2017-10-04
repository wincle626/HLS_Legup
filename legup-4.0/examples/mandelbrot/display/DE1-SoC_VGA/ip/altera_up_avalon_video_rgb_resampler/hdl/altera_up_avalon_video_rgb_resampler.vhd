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
-- * This module converts video streams between RGB color formats.              *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_rgb_resampler IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	IDW	:INTEGER									:= 23;
	ODW	:INTEGER									:= 23;
	
	IEW	:INTEGER									:= 0;
	OEW	:INTEGER									:= 0;
	
	ALPHA	:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"1111111111"
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk								:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

	stream_in_data					:IN		STD_LOGIC_VECTOR(IDW DOWNTO  0);	
	stream_in_startofpacket		:IN		STD_LOGIC;
	stream_in_endofpacket		:IN		STD_LOGIC;
	stream_in_empty				:IN		STD_LOGIC_VECTOR(IEW DOWNTO  0);	
	stream_in_valid				:IN		STD_LOGIC;

	stream_out_ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	stream_in_ready				:BUFFER	STD_LOGIC;

	stream_out_data				:BUFFER	STD_LOGIC_VECTOR(ODW DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_empty				:BUFFER	STD_LOGIC_VECTOR(OEW DOWNTO  0);	
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_rgb_resampler;

ARCHITECTURE Behaviour OF altera_up_avalon_video_rgb_resampler IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	r					:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	g					:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	b					:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	a					:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	
`ifdef USE_GRAY_OUT
	SIGNAL	average_color	:STD_LOGIC_VECTOR(11 DOWNTO  0);	
`endif
	
	SIGNAL	converted_data	:STD_LOGIC_VECTOR(ODW DOWNTO  0);	
	
	-- Internal Registers
	
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
				stream_out_data				<= (OTHERS => '0');
				stream_out_startofpacket	<= '0';
				stream_out_endofpacket		<= '0';
				stream_out_empty				<= (OTHERS => '0');
				stream_out_valid				<= '0';
			ELSIF ((stream_out_ready = '1') OR (stream_out_valid = '0')) THEN
				stream_out_data				<= converted_data;
				stream_out_startofpacket	<= stream_in_startofpacket;
				stream_out_endofpacket		<= stream_in_endofpacket;
			--	stream_out_empty				<= stream_in_empty;
				stream_out_empty				<= (OTHERS => '0');
				stream_out_valid				<= stream_in_valid;
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers

-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	stream_in_ready <= stream_out_ready OR NOT stream_out_valid;

	-- Internal Assignments
`ifdef USE_GRAY_IN
	r <= (stream_in_data( 7 DOWNTO  0) & stream_in_data( 7 DOWNTO  6));
	g <= (stream_in_data( 7 DOWNTO  0) & stream_in_data( 7 DOWNTO  6));
	b <= (stream_in_data( 7 DOWNTO  0) & stream_in_data( 7 DOWNTO  6));
`elsif USE_8_BITS_IN
	r <= (stream_in_data( 7 DOWNTO  5) & stream_in_data( 7 DOWNTO  5) & 
			stream_in_data( 7 DOWNTO  5) & stream_in_data( 7));
	g <= (stream_in_data( 4 DOWNTO  2) & stream_in_data( 4 DOWNTO  2) & 
			stream_in_data( 4 DOWNTO  2) & stream_in_data( 4));
	b <=  stream_in_data( 1 DOWNTO  0) & stream_in_data( 1 DOWNTO  0) & 
			stream_in_data( 1 DOWNTO  0) & stream_in_data( 1 DOWNTO  0) & 
			stream_in_data( 1 DOWNTO  0);
`elsif USE_9_BITS_IN
	r <= (stream_in_data( 8 DOWNTO  6) & stream_in_data( 8 DOWNTO  6) & 
			stream_in_data( 8 DOWNTO  6) & stream_in_data( 8));
	g <= (stream_in_data( 5 DOWNTO  3) & stream_in_data( 5 DOWNTO  3) & 
			stream_in_data( 5 DOWNTO  3) & stream_in_data( 5));
	b <= (stream_in_data( 2 DOWNTO  0) & stream_in_data( 2 DOWNTO  0) & 
			stream_in_data( 2 DOWNTO  0) & stream_in_data( 2));
`elsif USE_16_BITS_IN
	r <= (stream_in_data(15 DOWNTO 11) & stream_in_data(15 DOWNTO 11));
	g <= (stream_in_data(10 DOWNTO  5) & stream_in_data(10 DOWNTO  7));
	b <= (stream_in_data( 4 DOWNTO  0) & stream_in_data( 4 DOWNTO  0));
`elsif USE_24_BITS_IN
	r <= (stream_in_data(23 DOWNTO 16) & stream_in_data(23 DOWNTO 22));
	g <= (stream_in_data(15 DOWNTO  8) & stream_in_data(15 DOWNTO 14));
	b <= (stream_in_data( 7 DOWNTO  0) & stream_in_data( 7 DOWNTO  6));
`elsif USE_30_BITS_IN
	r <= stream_in_data(29 DOWNTO 20);
	g <= stream_in_data(19 DOWNTO 10);
	b <= stream_in_data( 9 DOWNTO  0);
`elsif USE_A_16_BITS_IN
	r <= (stream_in_data(11 DOWNTO  8) & stream_in_data(11 DOWNTO  8) & 
			stream_in_data(11 DOWNTO 10));
	g <= (stream_in_data( 7 DOWNTO  4) & stream_in_data( 7 DOWNTO  4) & 
			stream_in_data( 7 DOWNTO  6));
	b <= (stream_in_data( 3 DOWNTO  0) & stream_in_data( 3 DOWNTO  0) & 
			stream_in_data( 3 DOWNTO  2));
`elsif USE_A_32_BITS_IN
	r <= (stream_in_data(23 DOWNTO 16) & stream_in_data(23 DOWNTO 22));
	g <= (stream_in_data(15 DOWNTO  8) & stream_in_data(15 DOWNTO 14));
	b <= (stream_in_data( 7 DOWNTO  0) & stream_in_data( 7 DOWNTO  6));
`elsif USE_A_40_BITS_IN
	r <= stream_in_data(29 DOWNTO 20);
	g <= stream_in_data(19 DOWNTO 10);
	b <= stream_in_data( 9 DOWNTO  0);
`endif

`ifdef USE_A_16_BITS_IN
	a <= (stream_in_data(15 DOWNTO 12) & stream_in_data(15 DOWNTO 12) & 
			stream_in_data(15 DOWNTO 14));
`elsif USE_A_32_BITS_IN
	a <= (stream_in_data(31 DOWNTO 24) & stream_in_data(31 DOWNTO 30));
`elsif USE_A_40_BITS_IN
	a <= stream_in_data(39 DOWNTO 30);
`elsif USE_A_16_BITS_OUT
	a <= (ALPHA & B"000000");
`elsif USE_A_32_BITS_OUT
	a <= (ALPHA & B"00");
`elsif USE_A_40_BITS_OUT
	a <= ALPHA;
`else
	a <= ALPHA;
`endif

`ifdef USE_GRAY_OUT
	average_color <= (B"00" & r) + ('0' & g & '0') + (B"00" & b);

	converted_data( 7 DOWNTO  0) <= average_color(11 DOWNTO 4);
`elsif USE_8_BITS_OUT
	converted_data( 7 DOWNTO  5) <= r( 9 DOWNTO  7);
	converted_data( 4 DOWNTO  2) <= g( 9 DOWNTO  7);
	converted_data( 1 DOWNTO  0) <= b( 9 DOWNTO  8);
`elsif USE_9_BITS_OUT
	converted_data( 8 DOWNTO  6) <= r( 9 DOWNTO  7);
	converted_data( 5 DOWNTO  3) <= g( 9 DOWNTO  7);
	converted_data( 2 DOWNTO  0) <= b( 9 DOWNTO  7);
`elsif USE_16_BITS_OUT
	converted_data(15 DOWNTO 11) <= r( 9 DOWNTO  5);
	converted_data(10 DOWNTO  5) <= g( 9 DOWNTO  4);
	converted_data( 4 DOWNTO  0) <= b( 9 DOWNTO  5);
`elsif USE_24_BITS_OUT
	converted_data(23 DOWNTO 16) <= r( 9 DOWNTO  2);
	converted_data(15 DOWNTO  8) <= g( 9 DOWNTO  2);
	converted_data( 7 DOWNTO  0) <= b( 9 DOWNTO  2);
`elsif USE_30_BITS_OUT
	converted_data(29 DOWNTO 20) <= r( 9 DOWNTO  0);
	converted_data(19 DOWNTO 10) <= g( 9 DOWNTO  0);
	converted_data( 9 DOWNTO  0) <= b( 9 DOWNTO  0);
`elsif USE_A_16_BITS_OUT
	converted_data(15 DOWNTO 12) <= a( 9 DOWNTO  6);
	converted_data(11 DOWNTO  8) <= r( 9 DOWNTO  6);
	converted_data( 7 DOWNTO  4) <= g( 9 DOWNTO  6);
	converted_data( 3 DOWNTO  0) <= b( 9 DOWNTO  6);
`elsif USE_A_32_BITS_OUT
	converted_data(31 DOWNTO 24) <= a( 9 DOWNTO  2);
	converted_data(23 DOWNTO 16) <= r( 9 DOWNTO  2);
	converted_data(15 DOWNTO  8) <= g( 9 DOWNTO  2);
	converted_data( 7 DOWNTO  0) <= b( 9 DOWNTO  2);
`elsif USE_A_40_BITS_OUT
	converted_data(39 DOWNTO 30) <= a( 9 DOWNTO  0);
	converted_data(29 DOWNTO 20) <= r( 9 DOWNTO  0);
	converted_data(19 DOWNTO 10) <= g( 9 DOWNTO  0);
	converted_data( 9 DOWNTO  0) <= b( 9 DOWNTO  0);
`endif

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
