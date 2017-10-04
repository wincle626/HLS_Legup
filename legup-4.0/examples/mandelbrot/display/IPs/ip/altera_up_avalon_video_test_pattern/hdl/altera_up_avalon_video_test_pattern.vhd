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
-- * This module generators a sample video input stream for DE boards.          *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_test_pattern IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW							:INTEGER									:= 23;
	WW							:INTEGER									:= 8;
	HW							:INTEGER									:= 7;
	
	WIDTH						:INTEGER									:= 320;
	HEIGHT					:INTEGER									:= 240
	
`ifndef USE_HSV_VALUE
	;
	VALUE						:STD_LOGIC_VECTOR( 7 DOWNTO  0)	:= B"11000000";
	P_RATE					:STD_LOGIC_VECTOR(23 DOWNTO  0)	:= B"000000001100101110111011";
	TQ_START_RATE			:STD_LOGIC_VECTOR(24 DOWNTO  0)	:= B"0000000111001010011001100";
	TQ_RATE_DECELERATION	:STD_LOGIC_VECTOR(24 DOWNTO  0)	:= B"0000000000000001111010001"
`endif
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk				:IN		STD_LOGIC;
	reset				:IN		STD_LOGIC;

`ifdef USE_HSV_VALUE
	value				:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
`endif

	ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	startofpacket	:BUFFER	STD_LOGIC;
	endofpacket		:BUFFER	STD_LOGIC;
	empty				:BUFFER	STD_LOGIC;
	valid				:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_test_pattern;

ARCHITECTURE Behaviour OF altera_up_avalon_video_test_pattern IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
`ifdef USE_HSV_VALUE
	SIGNAL	v							:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
`endif
	
	SIGNAL	red						:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	green						:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	blue						:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	
`ifdef USE_HSV_VALUE
	SIGNAL	p_rate					:STD_LOGIC_VECTOR(15 DOWNTO  0);	
	SIGNAL	tq_start_rate			:STD_LOGIC_VECTOR(16 DOWNTO  0);	
	SIGNAL	tq_rate_deceleration	:STD_LOGIC_VECTOR( 8 DOWNTO  0);	
`endif
	
	-- Internal Registers
	SIGNAL	x							:STD_LOGIC_VECTOR(WW DOWNTO  0);	
	SIGNAL	y							:STD_LOGIC_VECTOR(HW DOWNTO  0);	
	
	SIGNAL	hue						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	hue_i						:STD_LOGIC_VECTOR( 2 DOWNTO  0);	
	
	SIGNAL	p							:STD_LOGIC_VECTOR(23 DOWNTO  0);	
	SIGNAL	q							:STD_LOGIC_VECTOR(24 DOWNTO  0);	
	SIGNAL	t							:STD_LOGIC_VECTOR(24 DOWNTO  0);	
	
	SIGNAL	rate						:STD_LOGIC_VECTOR(24 DOWNTO  0);	
	
	-- State Machine Registers
	
	-- Integers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altsyncram
	GENERIC (
		clock_enable_input_a		:STRING;
		clock_enable_output_a	:STRING;
		init_file					:STRING;
		intended_device_family	:STRING;
		lpm_hint						:STRING;
		lpm_type						:STRING;
		numwords_a					:INTEGER;
		operation_mode				:STRING;
		outdata_aclr_a				:STRING;
		outdata_reg_a				:STRING;
		widthad_a					:INTEGER;
		width_a						:INTEGER;
		width_byteena_a			:INTEGER
	);
	PORT (
		-- Inputs
		clock0			:IN		STD_LOGIC;
		address_a		:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);

		-- Outputs
		q_a				:BUFFER	STD_LOGIC_VECTOR(15 DOWNTO  0)
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	-- Color Space Conversion from HSV to RGB
	--
	-- HSV - Hue, Saturation, and Value
	--
	-- Hue			- 0 to 360
	-- Saturation	- 0 to 1
	-- Value		- 0 to 1
	--
	-- h_i	= floor (h / 60) mod 6
	-- f	= (h / 60) - floor (h / 60)
	-- p	= v * (1 - s) 
	-- q	= v * (1 - f * s) 
	-- t	= v * (1 - (1 - f) * s) 
	--
	--       { (v, t, p) if h_i = 0
	--       { (q, v, p) if h_i = 1
	-- RGB = { (p, v, t) if h_i = 2
	--       { (p, q, v) if h_i = 3
	--       { (t, p, v) if h_i = 4
	--       { (v, p, q) if h_i = 5
	--
	-- Source: http://en.wikipedia.org/wiki/HSL_color_space#Conversion_from_HSV_to_RGB
	--

	-- Output Registers

	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				x <= (OTHERS => '0');
			ELSIF (ready = '1') THEN
				IF (x = (WIDTH - 1)) THEN
					x <= (OTHERS => '0');
				ELSE
					x <= x + '1';
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				y <= (OTHERS => '0');
			ELSIF ((ready = '1') AND (x = (WIDTH - 1))) THEN
				IF (y = (HEIGHT - 1)) THEN
					y <= (OTHERS => '0');
				ELSE
					y <= y + '1';
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				hue	<= (OTHERS => '0');
				hue_i	<= (OTHERS => '0');
			ELSIF (ready = '1') THEN
				IF (x = (WIDTH - 1)) THEN
					hue	<= (OTHERS => '0');
					hue_i	<= (OTHERS => '0');
				ELSIF (hue = ((WIDTH / 6) - 1)) THEN
					hue	<= (OTHERS => '0');
					hue_i	<= hue_i + '1';
				ELSE
					hue	<= hue + '1';
				END IF;
			END IF;
		END IF;
	END PROCESS;


`ifdef USE_HSV_VALUE
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				p		<= (OTHERS => '0');
				q		<= ('0' & v & B"0000000000000000");
				t		<= (OTHERS => '0');
				rate	<= (B"00000000" & tq_start_rate);
			ELSIF (ready = '1') THEN
				IF ((x = (WIDTH - 1)) AND (y = (HEIGHT - 1))) THEN
					p		<= (OTHERS => '0');
					rate	<= (B"00000000" & tq_start_rate);
				ELSIF (x = (WIDTH - 1)) THEN
					p		<= p + (B"00000000" & p_rate);
					rate	<= rate - (B"0000000000000000" & tq_rate_deceleration);
				END IF;
			
				IF ((x = (WIDTH - 1)) AND (y = (HEIGHT - 1))) THEN
					q		<= ('0' & v & B"0000000000000000");
					t		<= (OTHERS => '0');
				ELSIF (x = (WIDTH - 1)) THEN
					q		<= ('0' & v & B"0000000000000000");
					t		<= ('0' & p) + (B"000000000" & p_rate);
				ELSIF ((hue = ((WIDTH / 6) - 1)) AND (hue_i /= 5)) THEN
					q		<= ('0' & v & B"0000000000000000");
					t		<= ('0' & p);
				ELSE
					q		<= q - rate;
					t		<= t + rate;
				END IF;
			END IF;
		END IF;
	END PROCESS;

`else
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				p		<= (OTHERS => '0');
				q		<= ('0' & VALUE & B"0000000000000000");
				t		<= (OTHERS => '0');
				rate	<= TQ_START_RATE;
			ELSIF (ready = '1') THEN
				IF ((x = (WIDTH - 1)) AND (y = (HEIGHT - 1))) THEN
					p		<= (OTHERS => '0');
					rate	<= TQ_START_RATE;
				ELSIF (x = (WIDTH - 1)) THEN
					p		<= p + P_RATE;
					rate	<= rate - TQ_RATE_DECELERATION;
				END IF;
			
				IF ((x = (WIDTH - 1)) AND (y = (HEIGHT - 1))) THEN
					q		<= ('0' & VALUE & B"0000000000000000");
					t		<= (OTHERS => '0');
				ELSIF (x = (WIDTH - 1)) THEN
					q		<= ('0' & VALUE & B"0000000000000000");
					t		<= ('0' & p) + ('0' & P_RATE);
				ELSIF ((hue = ((WIDTH / 6) - 1)) AND (hue_i /= 5)) THEN
					q		<= ('0' & VALUE & B"0000000000000000");
					t		<= ('0' & p);
				ELSE
					q		<= q - rate;
					t		<= t + rate;
				END IF;
			END IF;
		END IF;
	END PROCESS;

`endif

-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
`ifdef USE_HSV_VALUE
	v <= value;
`endif

	data 				<= (red & green & blue);
	startofpacket 	<= '1' WHEN ((x = 0) AND (y = 0)) ELSE '0';
	endofpacket 	<= '1' WHEN ((x = (WIDTH - 1)) AND (y = (HEIGHT - 1))) ELSE '0';
	empty 			<= '0';
	valid 			<= '1';

	-- Internal Assignments
`ifdef USE_HSV_VALUE
	red <= v WHEN (hue_i = 0) ELSE 
			q(23 DOWNTO 16) AND (NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24) & 
				NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24)) WHEN (hue_i = 1) ELSE 
			p(23 DOWNTO 16) WHEN (hue_i = 2) ELSE 
			p(23 DOWNTO 16) WHEN (hue_i = 3) ELSE 
			t(23 DOWNTO 16) OR (t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24)) 
				WHEN (hue_i = 4) ELSE 
			v;
	
	green <= t(23 DOWNTO 16) OR (t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24)) 
					WHEN (hue_i = 0) ELSE 
				v WHEN (hue_i = 1) ELSE 
				v WHEN (hue_i = 2) ELSE 
				q(23 DOWNTO 16) AND (NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24) & 
					NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24)) WHEN (hue_i = 3) ELSE 
				p(23 DOWNTO 16) WHEN (hue_i = 4) ELSE 
				p(23 DOWNTO 16);
	
	blue <= 	p(23 DOWNTO 16) WHEN (hue_i = 0) ELSE 
				p(23 DOWNTO 16) WHEN (hue_i = 1) ELSE 
				t(23 DOWNTO 16) OR (t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24)) 
					WHEN (hue_i = 2) ELSE 
				v WHEN (hue_i = 3) ELSE 
				v WHEN (hue_i = 4) ELSE 
				q(23 DOWNTO 16) AND (NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24) & 
					NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24));
	
`else
	red <= VALUE WHEN (hue_i = 0) ELSE 
			q(23 DOWNTO 16) AND (NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24) & 
				NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24)) WHEN (hue_i = 1) ELSE 
			p(23 DOWNTO 16) WHEN (hue_i = 2) ELSE 
			p(23 DOWNTO 16) WHEN (hue_i = 3) ELSE 
			t(23 DOWNTO 16) OR (t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24)) 
				WHEN (hue_i = 4) ELSE 
			VALUE;
	
	green <= t(23 DOWNTO 16) OR (t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24)) 
					WHEN (hue_i = 0) ELSE 
				VALUE WHEN (hue_i = 1) ELSE 
				VALUE WHEN (hue_i = 2) ELSE 
				q(23 DOWNTO 16) AND (NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24) & 
					NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24)) WHEN (hue_i = 3) ELSE 
				p(23 DOWNTO 16) WHEN (hue_i = 4) ELSE 
				p(23 DOWNTO 16);
	
	blue <= p(23 DOWNTO 16) WHEN (hue_i = 0) ELSE 
				p(23 DOWNTO 16) WHEN (hue_i = 1) ELSE 
				t(23 DOWNTO 16) OR (t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24) & t(24)) 
					WHEN (hue_i = 2) ELSE 
				VALUE WHEN (hue_i = 3) ELSE 
				VALUE WHEN (hue_i = 4) ELSE 
				q(23 DOWNTO 16) AND (NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24) & 
					NOT q(24) & NOT q(24) & NOT q(24) & NOT q(24));
`endif

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

`ifdef USE_HSV_VALUE
	P_Rate_ROM : altsyncram 
	GENERIC MAP (
		clock_enable_input_a		=> "BYPASS",
		clock_enable_output_a	=> "BYPASS",
		init_file					=> "p_rate.mif",
		intended_device_family	=> "Cyclone II",
		lpm_hint						=> "ENABLE_RUNTIME_MOD=NO",
		lpm_type						=> "altsyncram",
		numwords_a					=> 256,
		operation_mode				=> "ROM",
		outdata_aclr_a				=> "NONE",
		outdata_reg_a				=> "CLOCK0",
		widthad_a					=> 8,
		width_a						=> 16,
		width_byteena_a			=> 1
	)
	PORT MAP (
		-- Inputs
		clock0			=> clk,
		address_a		=> v,
	
		-- Outputs
		q_a				=> p_rate
	);

	TQ_Start_Rate_ROM : altsyncram 
	GENERIC MAP (
		clock_enable_input_a		=> "BYPASS",
		clock_enable_output_a	=> "BYPASS",
		init_file					=> "tq_rate.mif",
		intended_device_family	=> "Cyclone II",
		lpm_hint						=> "ENABLE_RUNTIME_MOD=NO",
		lpm_type						=> "altsyncram",
		numwords_a					=> 256,
		operation_mode				=> "ROM",
		outdata_aclr_a				=> "NONE",
		outdata_reg_a				=> "CLOCK0",
		widthad_a					=> 8,
		width_a						=> 17,
		width_byteena_a			=> 1
	)
	PORT MAP (
		-- Inputs
		clock0			=> clk,
		address_a		=> v,
	
		-- Outputs
		q_a				=> tq_start_rate
	);

	TQ_Rate_Deceleration_ROM : altsyncram 
	GENERIC MAP (
		clock_enable_input_a		=> "BYPASS",
		clock_enable_output_a	=> "BYPASS",
		init_file					=> "tq_accelerate.mif",
		intended_device_family	=> "Cyclone II",
		lpm_hint						=> "ENABLE_RUNTIME_MOD=NO",
		lpm_type						=> "altsyncram",
		numwords_a					=> 256,
		operation_mode				=> "ROM",
		outdata_aclr_a				=> "NONE",
		outdata_reg_a				=> "CLOCK0",
		widthad_a					=> 8,
		width_a						=> 9,
		width_byteena_a			=> 1
	)
	PORT MAP (
		-- Inputs
		clock0			=> clk,
		address_a		=> v,
	
		-- Outputs
		q_a				=> tq_rate_deceleration
	);
`endif


END Behaviour;
