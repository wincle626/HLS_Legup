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
-- * This module performs colour space conversion from RGB to YCrCb.            *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_RGB_to_YCrCb_converter IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************

-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk								:IN		STD_LOGIC;
	clk_en							:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

	R									:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	G									:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	B									:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	stream_in_startofpacket		:IN		STD_LOGIC;
	stream_in_endofpacket		:IN		STD_LOGIC;
	stream_in_empty				:IN		STD_LOGIC;
	stream_in_valid				:IN		STD_LOGIC;

	-- Bidirectionals

	-- Outputs
	Y									:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	Cr									:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	Cb									:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_empty				:BUFFER	STD_LOGIC;
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_RGB_to_YCrCb_converter;

ARCHITECTURE Behaviour OF altera_up_RGB_to_YCrCb_converter IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	product_0					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_1					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_2					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_3					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_4					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_5					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_6					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_7					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	SIGNAL	product_8					:STD_LOGIC_VECTOR(35 DOWNTO  0);	
	
	SIGNAL	Y_sum							:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	Cr_sum						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	Cb_sum						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	
	-- Internal Registers
	SIGNAL	R_in							:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	G_in							:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	SIGNAL	B_in							:STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	
	SIGNAL	R_0d257						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	G_0d504						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	B_0d098						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	R_0d148						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	G_0d291						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	B_0d439						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	R_0d439						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	G_0d368						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	SIGNAL	B_0d071						:STD_LOGIC_VECTOR(10 DOWNTO  0);	
	
	SIGNAL	startofpacket_shift_reg	:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	SIGNAL	endofpacket_shift_reg	:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	SIGNAL	empty_shift_reg			:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	SIGNAL	valid_shift_reg			:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	
	-- State Machine Registers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT lpm_mult
	GENERIC (
		lpm_widtha				:INTEGER;
		lpm_widthb				:INTEGER;
		lpm_widthp				:INTEGER;
		lpm_widths				:INTEGER;
		lpm_type					:STRING;
		lpm_representation	:STRING;
		lpm_hint					:STRING
	);
	PORT (
		-- Inputs
		dataa		:IN		STD_LOGIC_VECTOR(17 DOWNTO  0);
		datab		:IN		STD_LOGIC_VECTOR(17 DOWNTO  0);
		aclr		:IN		STD_LOGIC;
		clken		:IN		STD_LOGIC;
		clock		:IN		STD_LOGIC;
	
		-- Bidirectionals
	
		-- Outputs
		result	:BUFFER	STD_LOGIC_VECTOR(35 DOWNTO  0);
		sum		:BUFFER	STD_LOGIC
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
				Y  <= B"00000000";
				Cr <= B"00000000";
				Cb <= B"00000000";
			ELSIF (clk_en = '1') THEN
				IF (Y_sum(10) = '1') THEN -- Negative number
					Y <= B"00000000";
				ELSIF ((Y_sum(9) OR Y_sum(8)) = '1') THEN -- Number greater than 255
					Y <= B"11111111";
				ELSE
					Y <= Y_sum( 7 DOWNTO  0);
				END IF;
			
				IF (Cr_sum(10) = '1') THEN -- Negative number
					Cr <= B"00000000";
				ELSIF ((Cr_sum(9) OR Cr_sum(8)) = '1') THEN -- Number greater than 255
					Cr <= B"11111111";
				ELSE
					Cr <= Cr_sum( 7 DOWNTO  0);
				END IF;
			
				IF (Cb_sum(10) = '1') THEN -- Negative number
					Cb <= B"00000000";
				ELSIF ((Cb_sum(9) OR Cb_sum(8)) = '1') THEN -- Number greater than 255
					Cb <= B"11111111";
				ELSE
					Cb <= Cb_sum( 7 DOWNTO  0);
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (clk_en = '1') THEN
				stream_out_startofpacket	<= startofpacket_shift_reg(1);
				stream_out_endofpacket		<= endofpacket_shift_reg(1);
				stream_out_empty				<= empty_shift_reg(1);
				stream_out_valid				<= valid_shift_reg(1);
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	-- ---------------------------------------------------------------------------
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				R_in	<= B"00000000";
				G_in	<= B"00000000";
				B_in	<= B"00000000";
			ELSIF (clk_en = '1') THEN
				R_in	<= R;
				G_in	<= G;
				B_in	<= B;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				R_0d257 <= B"00000000000";
				G_0d504 <= B"00000000000";
				B_0d098 <= B"00000000000";
				R_0d148 <= B"00000000000";
				G_0d291 <= B"00000000000";
				B_0d439 <= B"00000000000";
				R_0d439 <= B"00000000000";
				G_0d368 <= B"00000000000";
				B_0d071 <= B"00000000000";
			ELSIF (clk_en = '1') THEN
				R_0d257 <= product_0(25 DOWNTO 15);
				G_0d504 <= product_1(25 DOWNTO 15);
				B_0d098 <= product_2(25 DOWNTO 15);
				R_0d148 <= product_3(25 DOWNTO 15);
				G_0d291 <= product_4(25 DOWNTO 15);
				B_0d439 <= product_5(25 DOWNTO 15);
				R_0d439 <= product_6(25 DOWNTO 15);
				G_0d368 <= product_7(25 DOWNTO 15);
				B_0d071 <= product_8(25 DOWNTO 15);
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				startofpacket_shift_reg	<= B"00";
				endofpacket_shift_reg	<= B"00";
				empty_shift_reg			<= B"00";
				valid_shift_reg			<= B"00";
			ELSIF (clk_en = '1') THEN
				startofpacket_shift_reg(1)	<= startofpacket_shift_reg(0);
				endofpacket_shift_reg(1)	<= endofpacket_shift_reg(0);
				empty_shift_reg(1)			<= empty_shift_reg(0);
				valid_shift_reg(1)			<= valid_shift_reg(0);
		
				startofpacket_shift_reg(0)	<= stream_in_startofpacket;
				endofpacket_shift_reg(0)	<= stream_in_endofpacket;
				empty_shift_reg(0)			<= stream_in_empty;
				valid_shift_reg(0)			<= stream_in_valid;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments

	-- Internal Assignments
	-- ---------------------------------------------------------------------------
	--
	-- Sum the proper outputs from the multiply to form YCrCb
	--
	Y_sum <= B""  + R_0d257 + G_0d504 + B_0d098;
	Cr_sum <= B"" + R_0d439 - G_0d368 - B_0d071;
	Cb_sum <= B"" - R_0d148 - G_0d291 + B_0d439;

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************


	-- Formula Set #1 (Corrected for 0 to 255 Color Range)
	-- ---------------------------------------------------------------------------
	-- Y  =  0.257R + 0.504G + 0.098B + 16
	-- Cr =  0.439R - 0.368G - 0.071B + 128
	-- Cb = -0.148R - 0.291G + 0.439B + 128
	-- 
	-- use full precision of multiply to experiment with coefficients
	-- 0.257 -> I[1:0].F[14:0]  .257 X 2^15 = 020E5
	-- 0.504 -> I[1:0].F[14:0]  .504 X 2^15 = 04083
	-- 0.098 -> I[1:0].F[14:0]  .098 X 2^15 = 00C8D
	-- 0.148 -> I[1:0].F[14:0]  .148 X 2^15 = 012F2
	-- 0.291 -> I[1:0].F[14:0]  .291 X 2^15 = 0253F
	-- 0.439 -> I[1:0].F[14:0]  .439 X 2^15 = 03831
	-- 0.368 -> I[1:0].F[14:0]  .368 X 2^15 = 02F1B
	-- 0.071 -> I[1:0].F[14:0]  .071 X 2^15 = 00917

	lpm_mult_component_0 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & R_in,
		datab		=> B"000010000011100101",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_0,
		sum		=> '0'
	);


	lpm_mult_component_1 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & G_in,
		datab		=> B"000100000010000011",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_1,
		sum		=> '0'
	);


	lpm_mult_component_2 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & B_in,
		datab		=> B"000000110010001101",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',	
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_2,
		sum		=> '0'
	);


	lpm_mult_component_6 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & R_in,
		datab		=> B"000011100000110001",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_6,
		sum		=> '0'
	);


	lpm_mult_component_7 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & G_in,
		datab		=> B"000010111100011011",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_7,
		sum		=> '0'
	);


	lpm_mult_component_8 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & B_in,
		datab		=> B"000000100100010111",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_8,
		sum		=> '0'
	);


	lpm_mult_component_3 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & R_in,
		datab		=> B"000001001011110010",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_3,
		sum		=> '0'
	);


	lpm_mult_component_4 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & G_in,
		datab		=> B"000010010100111111",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_4,
		sum		=> '0'
	);


	lpm_mult_component_5 : lpm_mult 
	GENERIC MAP (
		lpm_widtha				=> 18,
		lpm_widthb				=> 18,
		lpm_widthp				=> 36,
		lpm_widths				=> 1,
		lpm_type					=> "LPM_MULT",
		lpm_representation	=> "SIGNED",
		lpm_hint					=> "INPUT_B_IS_CONSTANT=YES,MAXIMIZE_SPEED=5"
	)
	PORT MAP (
		-- Inputs
		dataa		=> B"0000000000" & B_in,
		datab		=> B"000011100000110001",
		aclr		=> '0',
		clken		=> '1',
		clock		=> '0',
		
		-- Bidirectionals
		
		-- Outputs
		result	=> product_5,
		sum		=> '0'
	);

	-- Formula Set #2 (BT.601 Gamma Corrected Color Conversion)
	-- ---------------------------------------------------------------------------
	-- Y  =  ( 77 / 256)R + (150 / 256)G + ( 29 / 256)B
	-- Cr = -( 44 / 256)R - ( 87 / 256)G + (131 / 256)B + 128
	-- Cb =  (131 / 256)R - (110 / 256)G - ( 21 / 256)B + 128
	-- 
	-- use full precision of multiply to experiment with coefficients
	-- ( 77 / 256) -> I[1:0].F[14:0]  0.30078 X 2^15 = 02680  
	-- (150 / 256) -> I[1:0].F[14:0]  0.58594 X 2^15 = 04B00 
	-- ( 29 / 256) -> I[1:0].F[14:0]  0.11328 X 2^15 = 00E80 
	-- ( 44 / 256) -> I[1:0].F[14:0]  0.17188 X 2^15 = 01600 
	-- ( 87 / 256) -> I[1:0].F[14:0]  0.33984 X 2^15 = 02B80 
	-- (131 / 256) -> I[1:0].F[14:0]  0.51172 X 2^15 = 04180 
	-- (110 / 256) -> I[1:0].F[14:0]  0.42969 X 2^15 = 03700 
	-- ( 21 / 256) -> I[1:0].F[14:0]  0.08103 X 2^15 = 00A80 


END Behaviour;
