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
-- * This module constantly performs a DMA transfer from a memory device        *
-- *  containing pixel data to the VGA Controller IP Core.                      *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_pixel_buffer_dma IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	-- Parameters
	DEFAULT_BUFFER_ADDRESS		:STD_LOGIC_VECTOR(31 DOWNTO  0)	:= B"00000000000000000000000000000000";
	DEFAULT_BACK_BUF_ADDRESS	:STD_LOGIC_VECTOR(31 DOWNTO  0)	:= B"00000000000000000000000000000000";
	
`ifdef USE_CONSECUTIVE_ADDRESSING
	AW									:INTEGER									:= 17; -- Image size's address width
`else
	WW									:INTEGER									:= 8;  -- Image width's address width
	HW									:INTEGER									:= 7;  -- Image height's address width
`endif
	
	MW									:INTEGER									:= 15; -- Avalon master's data width
	DW									:INTEGER									:= 15; -- Image pixel width
	EW									:INTEGER									:= 0;  -- Streaming empty signel width
	
	PIXELS							:INTEGER									:= 320; -- Image width - number of pixels
	LINES								:INTEGER									:= 240 -- Image height - number of lines
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk						:IN		STD_LOGIC;
	reset						:IN		STD_LOGIC;

	slave_address			:IN		STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	slave_byteenable		:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	slave_read				:IN		STD_LOGIC;
	slave_write				:IN		STD_LOGIC;
	slave_writedata		:IN		STD_LOGIC_VECTOR(31 DOWNTO  0);	

	master_readdata		:IN		STD_LOGIC_VECTOR(MW DOWNTO  0);	
	master_readdatavalid	:IN		STD_LOGIC;
	master_waitrequest	:IN		STD_LOGIC;

	stream_ready			:IN		STD_LOGIC;

	-- Bi-Directional

	-- Outputs
	slave_readdata			:BUFFER	STD_LOGIC_VECTOR(31 DOWNTO  0);	

	master_address			:BUFFER	STD_LOGIC_VECTOR(31 DOWNTO  0);	
	master_arbiterlock	:BUFFER	STD_LOGIC;
	master_read				:BUFFER	STD_LOGIC;

	stream_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_startofpacket	:BUFFER	STD_LOGIC;
	stream_endofpacket	:BUFFER	STD_LOGIC;
	stream_empty			:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_valid			:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_pixel_buffer_dma;

ARCHITECTURE Behaviour OF altera_up_avalon_video_pixel_buffer_dma IS
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
	
	-- Data fifo signals
	SIGNAL	fifo_data_in						:STD_LOGIC_VECTOR((DW+2) DOWNTO  0);	
	SIGNAL	fifo_read							:STD_LOGIC;
	SIGNAL	fifo_write							:STD_LOGIC;
	
	SIGNAL	fifo_data_out						:STD_LOGIC_VECTOR((DW+2) DOWNTO  0);	
	SIGNAL	fifo_empty							:STD_LOGIC;
	SIGNAL	fifo_full							:STD_LOGIC;
	SIGNAL	fifo_almost_empty					:STD_LOGIC;
	SIGNAL	fifo_almost_full					:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	buffer_start_address				:STD_LOGIC_VECTOR(31 DOWNTO  0);	
	SIGNAL	back_buf_start_address			:STD_LOGIC_VECTOR(31 DOWNTO  0);	
	
	SIGNAL	buffer_swap							:STD_LOGIC;
	
	SIGNAL	pending_reads						:STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	SIGNAL	reading_first_pixel_in_image	:STD_LOGIC;
	
`ifdef USE_CONSECUTIVE_ADDRESSING
	SIGNAL	pixel_address						:STD_LOGIC_VECTOR(AW DOWNTO  0);	
`else
	SIGNAL	pixel_address						:STD_LOGIC_VECTOR(WW DOWNTO  0);	
	SIGNAL	line_address						:STD_LOGIC_VECTOR(HW DOWNTO  0);	
`endif
	
	-- State Machine Registers
	SIGNAL	s_pixel_buffer						:State_Type;	
	SIGNAL	ns_pixel_buffer					:State_Type;	
	
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
				s_pixel_buffer <= STATE_0_IDLE;
			ELSE
				s_pixel_buffer <= ns_pixel_buffer;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (s_pixel_buffer, fifo_almost_empty, ns_pixel_buffer, pending_reads, 
				master_waitrequest, pixel_address, 
`ifndef USE_CONSECUTIVE_ADDRESSING
				line_address, 
`endif
				fifo_almost_full)
	BEGIN
	   CASE (s_pixel_buffer) IS
		WHEN STATE_0_IDLE =>
			IF (fifo_almost_empty = '1') THEN
				ns_pixel_buffer <= STATE_2_READ_BUFFER;
			ELSE
				ns_pixel_buffer <= STATE_0_IDLE;
			END IF;
		WHEN STATE_1_WAIT_FOR_LAST_PIXEL =>
			IF (pending_reads = B"0000") THEN 
				ns_pixel_buffer <= STATE_0_IDLE;
			ELSE
				ns_pixel_buffer <= STATE_1_WAIT_FOR_LAST_PIXEL;
			END IF;
		WHEN STATE_2_READ_BUFFER =>
			IF (master_waitrequest = '0') THEN
`ifdef USE_CONSECUTIVE_ADDRESSING
				IF (pixel_address = ((PIXELS * LINES)  - 1)) THEN
`else
				IF ((pixel_address = (PIXELS - 1)) AND 
					(line_address = (LINES - 1))) THEN
`endif
					ns_pixel_buffer <= STATE_1_WAIT_FOR_LAST_PIXEL;
				ELSIF (fifo_almost_full = '1') THEN 
					ns_pixel_buffer <= STATE_0_IDLE;
				ELSIF (pending_reads >= B"1100") THEN 
					ns_pixel_buffer <= STATE_3_MAX_PENDING_READS_STALL;
				ELSE
					ns_pixel_buffer <= STATE_2_READ_BUFFER;
				END IF;
			ELSE
				ns_pixel_buffer <= STATE_2_READ_BUFFER;
			END IF;
		WHEN STATE_3_MAX_PENDING_READS_STALL =>
			IF (pending_reads <= B"0111") THEN 
				ns_pixel_buffer <= STATE_2_READ_BUFFER;
			ELSE
				ns_pixel_buffer <= STATE_3_MAX_PENDING_READS_STALL;
			END IF;
		WHEN OTHERS =>
			ns_pixel_buffer <= STATE_0_IDLE;
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
				slave_readdata <= B"00000000000000000000000000000000";
			
			ELSIF ((slave_read = '1') AND (slave_address = B"00")) THEN
				slave_readdata <= buffer_start_address;
			
			ELSIF ((slave_read = '1') AND (slave_address = B"01")) THEN
				slave_readdata <= back_buf_start_address;
			
			ELSIF ((slave_read = '1') AND (slave_address = B"10")) THEN
				slave_readdata(31 DOWNTO 16) <= STD_LOGIC_VECTOR(TO_UNSIGNED(LINES, 16));
				slave_readdata(15 DOWNTO  0) <= STD_LOGIC_VECTOR(TO_UNSIGNED(PIXELS, 16));
			
			ELSIF (slave_read = '1') THEN
`ifdef USE_CONSECUTIVE_ADDRESSING
				slave_readdata(31 DOWNTO 16) <= AW + B"0000000000000001";
`else
				slave_readdata(31 DOWNTO 24) <= HW + B"00000001";
				slave_readdata(23 DOWNTO 16) <= WW + B"00000001";
`endif
				slave_readdata(15 DOWNTO  8) <= B"00000000";
`ifdef USE_GRAY_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0001";
`elsif USE_8BIT_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0001";
`elsif USE_9BIT_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0001";
`elsif USE_16BIT_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0010";
`elsif USE_24BIT_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0011";
`elsif USE_30BIT_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0100";
`elsif USE_A_16BIT_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0010";
`elsif USE_A_32BIT_COLOR
				slave_readdata( 7 DOWNTO  4) <= B"0100";
`else
				slave_readdata( 7 DOWNTO  4) <= B"1000";
`endif
				slave_readdata( 3 DOWNTO  2) <= B"00";
`ifdef USE_CONSECUTIVE_ADDRESSING
				slave_readdata(			  1) <= '1';
`else
				slave_readdata(			  1) <= '0';
`endif
				slave_readdata(			  0) <= buffer_swap;
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				buffer_start_address <= DEFAULT_BUFFER_ADDRESS;
				back_buf_start_address <= DEFAULT_BACK_BUF_ADDRESS;
			ELSIF ((slave_write = '1') AND (slave_address = B"01")) THEN
				IF (slave_byteenable(0) = '1') THEN
					back_buf_start_address( 7 DOWNTO  0) <= slave_writedata( 7 DOWNTO  0);
				END IF;
				IF (slave_byteenable(1) = '1') THEN
					back_buf_start_address(15 DOWNTO  8) <= slave_writedata(15 DOWNTO  8);
				END IF;
				IF (slave_byteenable(2) = '1') THEN
					back_buf_start_address(23 DOWNTO 16) <= slave_writedata(23 DOWNTO 16);
				END IF;
				IF (slave_byteenable(3) = '1') THEN
					back_buf_start_address(31 DOWNTO 24) <= slave_writedata(31 DOWNTO 24);
				END IF;
			ELSIF ((buffer_swap = '1') AND (master_read = '1') AND 
					 (master_waitrequest = '0') AND 
`ifdef USE_CONSECUTIVE_ADDRESSING
					(pixel_address = ((PIXELS * LINES)  - 1))) THEN
`else
					((pixel_address = (PIXELS - 1)) AND 
					(line_address = (LINES - 1)))) THEN
`endif
				buffer_start_address <= back_buf_start_address;
				back_buf_start_address <= buffer_start_address;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				buffer_swap <= '0';
			ELSIF ((slave_write = '1') AND (slave_address = B"00")) THEN
				buffer_swap <= '1';
`ifdef USE_CONSECUTIVE_ADDRESSING
			ELSIF (pixel_address = 0) THEN
`else
			ELSIF ((pixel_address = 0) AND (line_address = 0)) THEN
`endif
				buffer_swap <= '0';
			END IF;
		END IF;
	END PROCESS;


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
				reading_first_pixel_in_image <= '0';
			ELSIF ((s_pixel_buffer = STATE_0_IDLE) AND
`ifdef USE_CONSECUTIVE_ADDRESSING
					(pixel_address = 0)) THEN
`else
					((pixel_address = 0) AND (line_address = 0))) THEN
`endif
				reading_first_pixel_in_image <= '1';
			ELSIF (master_readdatavalid = '1') THEN
				reading_first_pixel_in_image <= '0';
			END IF;
		END IF;
	END PROCESS;


`ifdef USE_CONSECUTIVE_ADDRESSING
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				pixel_address <= (OTHERS => '0');
			ELSIF ((master_read = '1') AND (master_waitrequest = '0')) THEN
				IF (pixel_address = ((PIXELS * LINES)  - 1)) THEN
					pixel_address <= (OTHERS => '0');
				ELSE
					pixel_address <= pixel_address + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;

`else
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				pixel_address <= (OTHERS => '0');
			ELSIF ((master_read = '1') AND (master_waitrequest = '0')) THEN
				IF (pixel_address = (PIXELS - 1)) THEN
					pixel_address <= (OTHERS => '0');
				ELSE
					pixel_address <= pixel_address + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				line_address <= (OTHERS => '0');
			ELSIF (((master_read = '1') AND (master_waitrequest = '0')) AND (pixel_address = (PIXELS - 1))) THEN
				IF (line_address = (LINES - 1)) THEN
					line_address <= (OTHERS => '0');
				ELSE
					line_address <= line_address + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;

`endif

-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	master_address <= buffer_start_address + 
`ifdef USE_CONSECUTIVE_ADDRESSING
`ifdef USE_DOUBLEWORD_ADDRESSING_MODE
									(pixel_address & B"000");
`elsif USE_WORD_ADDRESSING_MODE
									(pixel_address & B"00");
`elsif USE_HALFWORD_ADDRESSING_MODE
									(pixel_address & '0');
`else
									pixel_address;
`endif
`else
`ifdef USE_DOUBLEWORD_ADDRESSING_MODE
									(line_address & pixel_address & B"000");
`elsif USE_WORD_ADDRESSING_MODE
									(line_address & pixel_address & B"00");
`elsif USE_HALFWORD_ADDRESSING_MODE
									(line_address & pixel_address & '0');
`else
									(line_address & pixel_address);
`endif
`endif
	master_arbiterlock <= '0' WHEN ((s_pixel_buffer = STATE_2_READ_BUFFER) OR 
			(s_pixel_buffer = STATE_3_MAX_PENDING_READS_STALL)) ELSE
			'1';
	master_read <= '1' WHEN (s_pixel_buffer = STATE_2_READ_BUFFER) ELSE
						'0';

	stream_data <= fifo_data_out(DW DOWNTO 0);
	stream_startofpacket <= fifo_data_out(DW+1);
	stream_endofpacket <= fifo_data_out(DW+2);
	stream_empty <= (OTHERS => '0');
	stream_valid <= NOT fifo_empty;

	-- Internal Assignments
	fifo_data_in(DW DOWNTO 0) <= master_readdata(DW DOWNTO 0);
	fifo_data_in(DW+1) <= reading_first_pixel_in_image;
	fifo_data_in(DW+2) <= '1' WHEN (s_pixel_buffer = STATE_1_WAIT_FOR_LAST_PIXEL) AND 
											(pending_reads = B"0001") ELSE
								 '0';
	fifo_write <= master_readdatavalid AND NOT fifo_full;

	fifo_read <= stream_ready AND stream_valid;

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
