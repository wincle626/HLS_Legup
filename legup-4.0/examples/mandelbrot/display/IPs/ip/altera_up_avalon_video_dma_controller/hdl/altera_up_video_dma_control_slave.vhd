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

ENTITY altera_up_video_dma_control_slave IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	-- Parameters
	DEFAULT_BUFFER_ADDRESS		:STD_LOGIC_VECTOR(31 DOWNTO  0)	:= B"00000000000000000000000000000000";
	DEFAULT_BACK_BUF_ADDRESS	:STD_LOGIC_VECTOR(31 DOWNTO  0)	:= B"00000000000000000000000000000000";
	
	WIDTH								:INTEGER									:= 640; 	-- Frame's width in pixels
	HEIGHT							:INTEGER									:= 480; 	-- Frame's height in lines
	
	ADDRESSING_BITS				:STD_LOGIC_VECTOR(15 DOWNTO  0)	:= B"0000100000001001";
	COLOR_BITS						:STD_LOGIC_VECTOR( 3 DOWNTO  0)	:= B"0111"; -- Bits per color plane minus 1 
	COLOR_PLANES					:STD_LOGIC_VECTOR( 1 DOWNTO  0)	:= B"10"; -- Color planes per pixel minus 1
	ADDRESSING_MODE				:STD_LOGIC								:= '1'; 	-- 0: X-Y or 1: Consecutive
	
	DEFAULT_DMA_ENABLED			:STD_LOGIC								:= B"1" -- 0: OFF or 1: ON
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk							:IN		STD_LOGIC;
	reset							:IN		STD_LOGIC;

	address						:IN		STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	byteenable					:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	read							:IN		STD_LOGIC;
	write							:IN		STD_LOGIC;
	writedata					:IN		STD_LOGIC_VECTOR(31 DOWNTO  0);	

	swap_addresses_enable	:IN		STD_LOGIC;

	-- Bi-Directional

	-- Outputs
	readdata						:BUFFER	STD_LOGIC_VECTOR(31 DOWNTO  0);	

	current_start_address	:BUFFER	STD_LOGIC_VECTOR(31 DOWNTO  0);	
	dma_enabled					:BUFFER	STD_LOGIC

);

END altera_up_video_dma_control_slave;

ARCHITECTURE Behaviour OF altera_up_video_dma_control_slave IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	
	-- Internal Registers
	SIGNAL	buffer_start_address		:STD_LOGIC_VECTOR(31 DOWNTO  0);	
	SIGNAL	back_buf_start_address	:STD_LOGIC_VECTOR(31 DOWNTO  0);	
	
	SIGNAL	buffer_swap					:STD_LOGIC;
	
	-- State Machine Registers
	
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
				readdata <= B"00000000000000000000000000000000";
			
			ELSIF ((read = '1') AND (address = B"00")) THEN
				readdata <= buffer_start_address;
			
			ELSIF ((read = '1') AND (address = B"01")) THEN
				readdata <= back_buf_start_address;
			
			ELSIF ((read = '1') AND (address = B"10")) THEN
				readdata(31 DOWNTO 16) <= STD_LOGIC_VECTOR(TO_UNSIGNED(HEIGHT, 16));
				readdata(15 DOWNTO  0) <= STD_LOGIC_VECTOR(TO_UNSIGNED(WIDTH, 16));
			
			ELSIF (read = '1') THEN
				readdata(31 DOWNTO 16) <= ADDRESSING_BITS;
				readdata(15 DOWNTO 12) <= B"0000";
				readdata(11 DOWNTO  8) <= COLOR_BITS;
				readdata( 7 DOWNTO  6) <= COLOR_PLANES;
				readdata( 5 DOWNTO  3) <= B"000";
				readdata(			  2) <= dma_enabled;
				readdata(			  1) <= ADDRESSING_MODE;
				readdata(			  0) <= buffer_swap;
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				buffer_start_address	<= DEFAULT_BUFFER_ADDRESS;
				back_buf_start_address	<= DEFAULT_BACK_BUF_ADDRESS;
			ELSIF ((write = '1') AND (address = B"01")) THEN
				IF (byteenable(0) = '1') THEN
					back_buf_start_address( 7 DOWNTO  0) <= writedata( 7 DOWNTO  0);
			END IF;
				IF (byteenable(1) = '1') THEN
					back_buf_start_address(15 DOWNTO  8) <= writedata(15 DOWNTO  8);
			END IF;
				IF (byteenable(2) = '1') THEN
					back_buf_start_address(23 DOWNTO 16) <= writedata(23 DOWNTO 16);
			END IF;
				IF (byteenable(3) = '1') THEN
					back_buf_start_address(31 DOWNTO 24) <= writedata(31 DOWNTO 24);
			END IF;
			ELSIF ((buffer_swap = '1') AND (swap_addresses_enable = '1')) THEN
				buffer_start_address 	<= back_buf_start_address;
				back_buf_start_address 	<= buffer_start_address;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				buffer_swap <= '0';
			ELSIF ((write = '1') AND (address = B"00")) THEN
				buffer_swap <= '1';
			ELSIF (swap_addresses_enable = '1') THEN
				buffer_swap <= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				dma_enabled <= DEFAULT_DMA_ENABLED;
			ELSIF ((write = '1') AND (address = B"11") AND (byteenable(3) = '1')) THEN
				dma_enabled <= writedata(2);
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	current_start_address <= buffer_start_address;

	-- Internal Assignments

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
