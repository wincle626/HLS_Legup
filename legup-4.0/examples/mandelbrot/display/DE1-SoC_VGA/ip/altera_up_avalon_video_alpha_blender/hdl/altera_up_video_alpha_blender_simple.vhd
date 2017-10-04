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
-- * IMPLIED, INCLUDING BUTNOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
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
-- * This module combines two video streams by overlaying one onto the          *
-- *  other using alpha blending.  The foreground image must include alpha      *
-- *  bits to be used in the blending formula: Cn = (a < 0.5) ? Cb : Cf;        *
-- *  Cn - new color                                                            *
-- *  a  - alpha                                                                *
-- *  Cb - background colour                                                    *
-- *  Cf - foreground colour                                                    *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_alpha_blender_simple IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************

-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	background_data	:IN		STD_LOGIC_VECTOR(29 DOWNTO  0);	
	foreground_data	:IN		STD_LOGIC_VECTOR(39 DOWNTO  0);	

	-- Bidirectionals

	-- Outputs
	new_red				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	new_green			:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	new_blue				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0)	

);

END altera_up_video_alpha_blender_simple;

ARCHITECTURE Behaviour OF altera_up_video_alpha_blender_simple IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************

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

	-- Internal Registers

-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************
	-- Output Assignments
	new_red 		<= foreground_data(29 DOWNTO 20) WHEN (foreground_data(39) = '1') ELSE
						background_data(29 DOWNTO 20);

	new_green 	<= foreground_data(19 DOWNTO 10) WHEN (foreground_data(39) = '1') ELSE
						background_data(19 DOWNTO 10);
					
	new_blue 	<= foreground_data( 9 DOWNTO  0) WHEN (foreground_data(39) = '1') ELSE
						background_data( 9 DOWNTO  0);

	-- Internal Assignments

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
