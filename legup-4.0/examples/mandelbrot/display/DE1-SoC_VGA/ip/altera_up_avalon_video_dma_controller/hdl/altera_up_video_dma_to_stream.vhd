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

ENTITY altera_up_video_dma_to_stream IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW		:INTEGER									:= 15; -- Frame's datawidth
	EW		:INTEGER									:= 0; -- Frame's empty width
	
	MDW	:INTEGER									:= 15 -- Avalon master's datawidth
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk									:IN		STD_LOGIC;
	reset									:IN		STD_LOGIC;

	stream_ready						:IN		STD_LOGIC;

	master_readdata					:IN		STD_LOGIC_VECTOR(MDW DOWNTO  0);	
	master_readdatavalid				:IN		STD_LOGIC;
	master_waitrequest				:IN		STD_LOGIC;
	
	reading_first_pixel_in_frame	:IN		STD_LOGIC;
	reading_last_pixel_in_frame	:IN		STD_LOGIC;

	-- Bidirectional

	-- Outputs
	stream_data							:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_startofpacket				:BUFFER	STD_LOGIC;
	stream_endofpacket				:BUFFER	STD_LOGIC;
	stream_empty						:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_valid						:BUFFER	STD_LOGIC;

	master_arbiterlock				:BUFFER	STD_LOGIC;
	master_read							:BUFFER	STD_LOGIC;

	inc_address							:BUFFER	STD_LOGIC;
	reset_address						:BUFFER	STD_LOGIC

);

END altera_up_video_dma_to_stream;

ARCHITECTURE Behaviour OF altera_up_video_dma_to_stream IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************
	
	-- states
	TYPE State_Type IS (	STATE_0_IDLE,
								STATE_1_WAIT_FOR_LAST_PIXEL,
								STATE_2_READ_BUFFER,
								STATE_3_MAX_PENDING_READS_STALL
							);
	
-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	fifo_data_in		:STD_LOGIC_VECTOR((DW+2) DOWNTO  0);	
	SIGNAL	fifo_read			:STD_LOGIC;
	SIGNAL	fifo_write			:STD_LOGIC;
	
	SIGNAL	fifo_data_out		:STD_LOGIC_VECTOR((DW+2) DOWNTO  0);	
	SIGNAL	fifo_empty			:STD_LOGIC;
	SIGNAL	fifo_full			:STD_LOGIC;
	SIGNAL	fifo_almost_empty	:STD_LOGIC;
	SIGNAL	fifo_almost_full	:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	pending_reads		:STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	SIGNAL	startofpacket		:STD_LOGIC;
	
	-- State Machine Registers
	SIGNAL	s_dma_to_stream	:State_Type;	
	SIGNAL	ns_dma_to_stream	:State_Type;	
	
	-- Integers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT scfifo
	GENERIC (
		add_ram_output_register	:STRING;
		almost_empty_value		:INTEGER;
		almost_full_value			:INTEGER;
		intended_device_family	:STRING;
		lpm_numwords				:INTEGER;
		lpm_showahead				:STRING;
		lpm_type						:STRING;
		lpm_width					:INTEGER;
		lpm_widthu					:INTEGER;
		overflow_checking			:STRING;
		underflow_checking		:STRING;
		use_eab						:STRING
	);
	PORT (
		-- Inputs
		clock				:IN		STD_LOGIC;
		sclr				:IN		STD_LOGIC;
   
		data				:IN		STD_LOGIC_VECTOR((DW+2) DOWNTO  0);
		wrreq				:IN		STD_LOGIC;

		rdreq				:IN		STD_LOGIC;

		-- Outputs
		q					:BUFFER	STD_LOGIC_VECTOR((DW+2) DOWNTO  0);

		empty				:BUFFER	STD_LOGIC;
		full				:BUFFER	STD_LOGIC;
	   
		almost_empty	:BUFFER	STD_LOGIC;
		almost_full		:BUFFER	STD_LOGIC
		-- synopsys translate_off
	
		-- synopsys translate_on
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				s_dma_to_stream <= STATE_0_IDLE;
			ELSE
				s_dma_to_stream <= ns_dma_to_stream;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (s_dma_to_stream, fifo_almost_empty, ns_dma_to_stream, pending_reads, 
				master_waitrequest, reading_last_pixel_in_frame, fifo_almost_full)
	BEGIN
	   CASE (s_dma_to_stream) IS
		WHEN STATE_0_IDLE =>
			IF (fifo_almost_empty = '1') THEN
				ns_dma_to_stream <= STATE_2_READ_BUFFER;
			ELSE
				ns_dma_to_stream <= STATE_0_IDLE;
			END IF;
		WHEN STATE_1_WAIT_FOR_LAST_PIXEL =>
			IF (pending_reads = B"0000") THEN 
				ns_dma_to_stream <= STATE_0_IDLE;
			ELSE
				ns_dma_to_stream <= STATE_1_WAIT_FOR_LAST_PIXEL;
			END IF;
		WHEN STATE_2_READ_BUFFER =>
			IF (master_waitrequest = '0') THEN
				IF (reading_last_pixel_in_frame = '1') THEN
					ns_dma_to_stream <= STATE_1_WAIT_FOR_LAST_PIXEL;
				ELSIF (fifo_almost_full = '1') THEN 
					ns_dma_to_stream <= STATE_0_IDLE;
				ELSIF (pending_reads >= B"1100") THEN
					ns_dma_to_stream <= STATE_3_MAX_PENDING_READS_STALL;
				ELSE
					ns_dma_to_stream <= STATE_2_READ_BUFFER;
				END IF;
			ELSE
				ns_dma_to_stream <= STATE_2_READ_BUFFER;
			END IF;
		WHEN STATE_3_MAX_PENDING_READS_STALL =>
			IF (pending_reads <= B"0111") THEN 
				ns_dma_to_stream <= STATE_2_READ_BUFFER;
			ELSIF (fifo_almost_full = '1') THEN 
				ns_dma_to_stream <= STATE_0_IDLE;
			ELSE
				ns_dma_to_stream <= STATE_3_MAX_PENDING_READS_STALL;
			END IF;
		WHEN OTHERS =>
			ns_dma_to_stream <= STATE_0_IDLE;
		END CASE;
	END PROCESS;


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	-- Output Registers

	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				pending_reads <= B"0000";
			ELSIF ((master_read = '1') AND (master_waitrequest = '0')) THEN
				IF (master_readdatavalid = '0') THEN
					pending_reads <= pending_reads + '1';
			END IF;
			ELSIF ((master_readdatavalid = '1') AND (pending_reads /= B"0000")) THEN
				pending_reads <= pending_reads - '1';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				startofpacket <= '0';
			ELSIF ((s_dma_to_stream = STATE_0_IDLE) AND (reading_first_pixel_in_frame = '1')) THEN
				startofpacket <= '1';
			ELSIF (master_readdatavalid = '1') THEN
				startofpacket <= '0';
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************
	-- Output Assignments
	stream_data <= fifo_data_out( DW DOWNTO  0);
	stream_startofpacket <= fifo_data_out(DW+1);
	stream_endofpacket <= fifo_data_out(DW+2);
	stream_empty <= (OTHERS => '0');
	stream_valid <= NOT fifo_empty;

	master_arbiterlock <= '0' WHEN ((s_dma_to_stream = STATE_2_READ_BUFFER) OR 
			(s_dma_to_stream = STATE_3_MAX_PENDING_READS_STALL)) ELSE '1';
	master_read <= '1' WHEN (s_dma_to_stream = STATE_2_READ_BUFFER) ELSE '0';

	inc_address <= '1' WHEN ((master_read = '1') AND (master_waitrequest = '0'))
						ELSE '0';
	reset_address <= '1' WHEN ((inc_address = '1') AND 
							(reading_last_pixel_in_frame = '1')) ELSE '0';

	-- Internal Assignments
	fifo_data_in( DW DOWNTO  0) <= master_readdata( DW DOWNTO  0);
	fifo_data_in(DW+1) <= startofpacket;
	fifo_data_in(DW+2) <= '1' WHEN ((s_dma_to_stream = STATE_1_WAIT_FOR_LAST_PIXEL) AND 
												(pending_reads = B"0001")) ELSE '0';
	fifo_write <= '1' WHEN ((master_readdatavalid = '1') AND (fifo_full = '0'))
						ELSE '0';

	fifo_read <= '1' WHEN ((stream_ready = '1') AND (stream_valid = '1')) ELSE '0';

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Image_Buffer : scfifo 
	GENERIC MAP (
		add_ram_output_register	=> "OFF",
		almost_empty_value		=> 32,
		almost_full_value			=> 96,
		intended_device_family	=> "Cyclone II",
		lpm_numwords				=> 128,
		lpm_showahead				=> "ON",
		lpm_type						=> "scfifo",
		lpm_width					=> DW + 3,
		lpm_widthu					=> 7,
		overflow_checking			=> "OFF",
		underflow_checking		=> "OFF",
		use_eab						=> "ON"
	)
	PORT MAP (
		-- Inputs
		clock				=> clk,
		sclr				=> reset,
	   
		data				=> fifo_data_in,
		wrreq				=> fifo_write,
	
		rdreq				=> fifo_read,
	
		-- Outputs
		q					=> fifo_data_out,
	
		empty				=> fifo_empty,
		full				=> fifo_full,
		   
		almost_empty	=> fifo_almost_empty,
		almost_full		=> fifo_almost_full
		-- synopsys translate_off
		
		-- synopsys translate_on
	);


END Behaviour;
