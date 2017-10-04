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
-- * This module contains counter for the width and height of frames for the    *
-- *   video clipper core.                                                      *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_clipper_counters IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	IMAGE_WIDTH		:INTEGER									:= 640; 	-- Final image width in pixels
	IMAGE_HEIGHT	:INTEGER									:= 480; 	-- Final image height in lines
	WW					:INTEGER									:= 9; 	-- Final image width address width
	HW					:INTEGER									:= 8; 	-- Final image height address width
	
	LEFT_OFFSET		:INTEGER									:= 0;
	RIGHT_OFFSET	:INTEGER									:= 0;
	TOP_OFFSET		:INTEGER									:= 0;
	BOTTOM_OFFSET	:INTEGER									:= 0
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk						:IN		STD_LOGIC;
	reset						:IN		STD_LOGIC;

	increment_counters	:IN		STD_LOGIC;

	-- Bi-Directional

	-- Outputs
	start_of_outer_frame	:BUFFER	STD_LOGIC;
	end_of_outer_frame	:BUFFER	STD_LOGIC;

	start_of_inner_frame	:BUFFER	STD_LOGIC;
	end_of_inner_frame	:BUFFER	STD_LOGIC;
	inner_frame_valid		:BUFFER	STD_LOGIC

);

END altera_up_video_clipper_counters;

ARCHITECTURE Behaviour OF altera_up_video_clipper_counters IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	
	-- Internal Registers
	SIGNAL	width						:STD_LOGIC_VECTOR(WW DOWNTO  0);	
	SIGNAL	height					:STD_LOGIC_VECTOR(HW DOWNTO  0);	
	
	SIGNAL	inner_width_valid		:STD_LOGIC;
	SIGNAL	inner_height_valid	:STD_LOGIC;
	
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

	-- Output registers

	-- Internal registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				width <= (OTHERS => '0');
			ELSIF ((increment_counters = '1') AND (width = (IMAGE_WIDTH - 1))) THEN
				width <= (OTHERS => '0');
			ELSIF (increment_counters = '1') THEN
				width <= width + 1;	
			END IF;
		END IF;
	END PROCESS;

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				height <= (OTHERS => '0');
			ELSIF ((increment_counters = '1') AND (width = (IMAGE_WIDTH - 1))) THEN
				IF (height = (IMAGE_HEIGHT - 1)) THEN
					height <= (OTHERS => '0');
				ELSE
					height <= height + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				IF (LEFT_OFFSET = 0) THEN
					inner_width_valid <= '1';
				ELSE
					inner_width_valid <= '0';
				END IF;
			ELSIF (increment_counters = '1') THEN
				IF (width = (IMAGE_WIDTH - 1)) THEN
					IF (LEFT_OFFSET = 0) THEN
						inner_width_valid <= '1';
					ELSE
						inner_width_valid <= '0';
					END IF;
				
				ELSIF (width = (IMAGE_WIDTH - RIGHT_OFFSET - 1)) THEN
					inner_width_valid <= '0';
				
				ELSIF (width = (LEFT_OFFSET - 1)) THEN
					inner_width_valid <= '1';
				END IF;
			END IF;
		END IF;
	END PROCESS;

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				IF (TOP_OFFSET = 0) THEN
					inner_height_valid <= '1';
				ELSE
					inner_height_valid <= '0';
				END IF;
			ELSIF ((increment_counters = '1') AND (width = (IMAGE_WIDTH - 1))) THEN
				IF (height = (IMAGE_HEIGHT - 1)) THEN
					IF (TOP_OFFSET = 0) THEN
						inner_height_valid <= '1';
					ELSE
						inner_height_valid <= '0';
					END IF;
				
				ELSIF (height = (IMAGE_HEIGHT - BOTTOM_OFFSET - 1)) THEN
					inner_height_valid <= '0';
				
				ELSIF (height = (TOP_OFFSET - 1)) THEN
					inner_height_valid <= '1';
				END IF;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output assignments
	start_of_outer_frame <= '1' WHEN ((width = (OTHERS => '0')) AND
												 (height = (OTHERS => '0'))) ELSE '0';
	end_of_outer_frame 	<= '1' WHEN ((width = (IMAGE_WIDTH - 1)) AND 
												 (height = (IMAGE_HEIGHT - 1))) ELSE	'0';

	start_of_inner_frame <= '1' WHEN ((width = LEFT_OFFSET) AND 
												 (height = TOP_OFFSET)) ELSE '0';
	end_of_inner_frame 	<= '1' WHEN ((width = (IMAGE_WIDTH - RIGHT_OFFSET - 1)) AND 
												 (height = (IMAGE_HEIGHT - BOTTOM_OFFSET - 1)))
										 ELSE '0';
	inner_frame_valid 	<= inner_width_valid AND inner_height_valid;

	-- Internal assignments


-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
