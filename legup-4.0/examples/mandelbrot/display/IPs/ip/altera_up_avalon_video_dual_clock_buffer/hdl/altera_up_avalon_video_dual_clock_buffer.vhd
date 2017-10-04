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
-- * This module is a buffer that can be used the transfer streaming data from  *
-- *  one clock domain to another.                                              *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_dual_clock_buffer IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW	:INTEGER									:= 15; -- Frame's data width
	EW	:INTEGER									:= 0 -- Frame's empty width
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk_stream_in					:IN		STD_LOGIC;
	reset_stream_in				:IN		STD_LOGIC;
	clk_stream_out					:IN		STD_LOGIC;
	reset_stream_out				:IN		STD_LOGIC;

	stream_in_data					:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_in_startofpacket		:IN		STD_LOGIC;
	stream_in_endofpacket		:IN		STD_LOGIC;
	stream_in_empty				:IN		STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_in_valid				:IN		STD_LOGIC;

	stream_out_ready				:IN		STD_LOGIC;

	-- Bi-Directional

	-- Outputs
	stream_in_ready				:BUFFER	STD_LOGIC;

	stream_out_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_empty				:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_dual_clock_buffer;

ARCHITECTURE Behaviour OF altera_up_avalon_video_dual_clock_buffer IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	fifo_wr_used	:STD_LOGIC_VECTOR( 6 DOWNTO  0);	
	SIGNAL	fifo_empty		:STD_LOGIC;
	
	SIGNAL	q					:STD_LOGIC_VECTOR((DW + 2) DOWNTO  0);
	
	-- Internal Registers
	
	-- State Machine Registers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT dcfifo
	GENERIC (
		intended_device_family	:STRING;
		lpm_hint						:STRING;
		lpm_numwords				:INTEGER;
		lpm_showahead				:STRING;
		lpm_type						:STRING;
		lpm_width					:INTEGER;
		lpm_widthu					:INTEGER;
		overflow_checking			:STRING;
		rdsync_delaypipe			:INTEGER;
		underflow_checking		:STRING;
		use_eab						:STRING;
		wrsync_delaypipe			:INTEGER
	);
	PORT (
		-- Inputs
		wrclk		:IN		STD_LOGIC;
		wrreq		:IN		STD_LOGIC;
		data		:IN		STD_LOGIC_VECTOR((DW + 2) DOWNTO  0);

		rdclk		:IN		STD_LOGIC;
		rdreq		:IN		STD_LOGIC;

		-- Outputs
		wrusedw	:BUFFER	STD_LOGIC_VECTOR( 6 DOWNTO  0);
		
		rdempty	:BUFFER	STD_LOGIC;
		q			:BUFFER	STD_LOGIC_VECTOR((DW + 2) DOWNTO  0)
		-- synopsys translate_off
	
		-- synopsys translate_on
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output assignments
	stream_in_ready 	<= NOT (AND_REDUCE (fifo_wr_used( 6 DOWNTO  4)));

	stream_out_empty 	<= (OTHERS => '0');
	stream_out_valid 	<= NOT fifo_empty;
	
	stream_out_data				<= q((DW + 2) DOWNTO  2);
	stream_out_endofpacket		<= q(1);
	stream_out_startofpacket	<= q(0);

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Data_FIFO : dcfifo 
	GENERIC MAP (
		intended_device_family	=> "Cyclone II",
		lpm_hint						=> "MAXIMIZE_SPEED=7,",
		lpm_numwords				=> 128,
		lpm_showahead				=> "ON",
		lpm_type						=> "dcfifo",
		lpm_width					=> DW + 3,
		lpm_widthu					=> 7,
		overflow_checking			=> "OFF",
		rdsync_delaypipe			=> 5,
		underflow_checking		=> "OFF",
		use_eab						=> "ON",
		wrsync_delaypipe			=> 5
	)
	PORT MAP (
		-- Inputs
		wrclk		=> clk_stream_in,
		wrreq		=> stream_in_ready AND stream_in_valid,
		data		=> stream_in_data & stream_in_endofpacket & stream_in_startofpacket,
	
		rdclk		=> clk_stream_out,
		rdreq		=> stream_out_ready AND NOT fifo_empty,
	
		-- Outputs
		wrusedw	=> fifo_wr_used,
			
		rdempty	=> fifo_empty,
		q			=> q
		-- synopsys translate_off
		
		-- synopsys translate_on
	);


END Behaviour;
