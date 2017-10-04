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
-- * This module decodes video streams from the Terasic CCD cameras.            *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_camera_decoder IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW	:INTEGER									:= 9
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk				:IN		STD_LOGIC;
	reset				:IN		STD_LOGIC;

	PIXEL_DATA		:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	LINE_VALID		:IN		STD_LOGIC;
	FRAME_VALID		:IN		STD_LOGIC;

	ready				:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	startofpacket	:BUFFER	STD_LOGIC;
	endofpacket		:BUFFER	STD_LOGIC;
	valid				:BUFFER	STD_LOGIC

);

END altera_up_video_camera_decoder;

ARCHITECTURE Behaviour OF altera_up_video_camera_decoder IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	read_temps		:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	io_pixel_data	:STD_LOGIC_VECTOR(DW DOWNTO  0);	
	SIGNAL	io_line_valid	:STD_LOGIC;
	SIGNAL	io_frame_valid	:STD_LOGIC;
	
	SIGNAL	frame_sync		:STD_LOGIC;
	
	SIGNAL	temp_data		:STD_LOGIC_VECTOR(DW DOWNTO  0);	
	SIGNAL	temp_start		:STD_LOGIC;
	SIGNAL	temp_end			:STD_LOGIC;
	SIGNAL	temp_valid		:STD_LOGIC;
	
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
	-- Input Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			io_pixel_data		<= PIXEL_DATA;
			io_line_valid		<= LINE_VALID;
			io_frame_valid		<= FRAME_VALID;
		END IF;
	END PROCESS;


	-- Output Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				data				<= (OTHERS => '0');
				startofpacket	<= '0';
				endofpacket		<= '0';
				valid				<= '0';
			ELSIF (read_temps = '1') THEN
				data				<= temp_data;
				startofpacket	<= temp_start;
				endofpacket		<= temp_end;
				valid				<= temp_valid;
			ELSIF (ready = '1') THEN
				valid				<= '0';
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				frame_sync 		<= '0';
			ELSIF (io_frame_valid = '0') THEN
				frame_sync 		<= '1';
			ELSIF ((io_line_valid = '1') AND (io_frame_valid = '1')) THEN
				frame_sync 		<= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				temp_data 		<= (OTHERS => '0');
				temp_start		<= '0';
				temp_end			<= '0';
				temp_valid		<= '0';
			ELSIF (read_temps = '1') THEN
				temp_data 		<= io_pixel_data;
				temp_start		<= frame_sync;
				temp_end			<= NOT io_frame_valid;
				temp_valid		<= io_line_valid AND io_frame_valid;
			ELSIF (io_frame_valid = '0') THEN
				temp_end			<= NOT io_frame_valid;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************
	-- Output Assignments

	-- Internal Assignments
	read_temps <= (ready OR NOT valid) AND 
		((io_line_valid AND io_frame_valid) OR 
		 ((temp_start OR temp_end) AND temp_valid));


-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
