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
-- * This module replicates lines in video data to enlarge the image.           *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_video_scaler_multiply_height IS 


-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW		:INTEGER									:= 15; -- Image's data width
	WW		:INTEGER									:= 8; -- Image width's address width
	WIDTH	:INTEGER									:= 320; -- Image's width in pixels
	
	CW		:INTEGER									:= 0 -- Multiply height's counter width
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk								:IN		STD_LOGIC;
	reset								:IN		STD_LOGIC;

	stream_in_data					:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_in_startofpacket		:IN		STD_LOGIC;
	stream_in_endofpacket		:IN		STD_LOGIC;
	stream_in_valid				:IN		STD_LOGIC;

	stream_out_ready				:IN		STD_LOGIC;

	-- Bi-Directional

	-- Outputs
	stream_in_ready				:BUFFER	STD_LOGIC;

	stream_out_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_out_startofpacket	:BUFFER	STD_LOGIC;
	stream_out_endofpacket		:BUFFER	STD_LOGIC;
	stream_out_valid				:BUFFER	STD_LOGIC

);

END altera_up_video_scaler_multiply_height;

ARCHITECTURE Behaviour OF altera_up_video_scaler_multiply_height IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************
	
	TYPE State_Type IS (	STATE_0_GET_CURRENT_LINE,
								STATE_1_LOOP_FIFO,
								STATE_2_OUTPUT_LAST_LINE
							);
	
-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	fifo_data_in				:STD_LOGIC_VECTOR((DW + 2) DOWNTO  0);	
	SIGNAL	fifo_data_out				:STD_LOGIC_VECTOR((DW + 2) DOWNTO  0);	
	
	
	SIGNAL	fifo_empty					:STD_LOGIC;
	SIGNAL	fifo_full					:STD_LOGIC;
	SIGNAL	fifo_read					:STD_LOGIC;
	SIGNAL	fifo_write					:STD_LOGIC;
	
	-- Internal Registers
	SIGNAL	width_in						:STD_LOGIC_VECTOR(WW DOWNTO  0);	
	SIGNAL	width_out					:STD_LOGIC_VECTOR(WW DOWNTO  0);	
	SIGNAL	enlarge_height_counter	:STD_LOGIC_VECTOR(CW DOWNTO  0);	
	
	-- State Machine Registers
	SIGNAL	s_multiply_height			:State_Type;	
	SIGNAL	ns_multiply_height		:State_Type;	
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT scfifo
	GENERIC (
		add_ram_output_register	:STRING;
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
   
		data				:IN		STD_LOGIC_VECTOR((DW + 2) DOWNTO  0);
		wrreq				:IN		STD_LOGIC;

		rdreq				:IN		STD_LOGIC;

		-- Outputs
		q					:BUFFER	STD_LOGIC_VECTOR((DW + 2) DOWNTO  0);
		empty				:BUFFER	STD_LOGIC;
		full				:BUFFER	STD_LOGIC
	   
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
				s_multiply_height <= STATE_0_GET_CURRENT_LINE;
			ELSE
				s_multiply_height <= ns_multiply_height;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (s_multiply_height, width_in, ns_multiply_height, fifo_read, width_out, 
				enlarge_height_counter)
	BEGIN
	   CASE (s_multiply_height) IS
		WHEN STATE_0_GET_CURRENT_LINE =>
			IF (width_in = WIDTH) THEN
				ns_multiply_height <= STATE_1_LOOP_FIFO;
			ELSE
				ns_multiply_height <= STATE_0_GET_CURRENT_LINE;
			END IF;
		WHEN STATE_1_LOOP_FIFO =>
			IF ((fifo_read = '1') AND (width_out = (WIDTH - 1)) AND 
					((AND_REDUCE(enlarge_height_counter(CW DOWNTO 1)) = '1') OR 
					(CW = 0))) THEN
				ns_multiply_height <= STATE_2_OUTPUT_LAST_LINE;
			ELSE
				ns_multiply_height <= STATE_1_LOOP_FIFO;
			END IF;
		WHEN STATE_2_OUTPUT_LAST_LINE =>
			IF ((fifo_read = '1') AND (width_out = (WIDTH - 1))) THEN
				ns_multiply_height <= STATE_0_GET_CURRENT_LINE;
			ELSE
				ns_multiply_height <= STATE_2_OUTPUT_LAST_LINE;
			END IF;
		WHEN OTHERS =>
			ns_multiply_height <= STATE_0_GET_CURRENT_LINE;
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
				stream_out_data					<=  (OTHERS => '0');
				stream_out_startofpacket		<= '0';
				stream_out_endofpacket			<= '0';
				stream_out_valid					<= '0';
			ELSIF ((stream_out_ready = '1') OR (stream_out_valid = '0')) THEN
				stream_out_data					<= fifo_data_out( DW  DOWNTO  0);
		
				IF (OR_REDUCE(enlarge_height_counter) = '1') THEN
					stream_out_startofpacket	<= '0';
				ELSE
					stream_out_startofpacket	<= fifo_data_out(DW + 1);
				END IF;
			
				IF (AND_REDUCE(enlarge_height_counter) = '1') THEN
					stream_out_endofpacket		<= fifo_data_out(DW + 2);
				ELSE
					stream_out_endofpacket		<= '0';
				END IF;
			
				IF (s_multiply_height = STATE_0_GET_CURRENT_LINE) THEN
					stream_out_valid				<= '0';
				ELSE
					stream_out_valid				<= NOT fifo_empty;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				width_in <= (OTHERS => '0');
			ELSIF (s_multiply_height = STATE_1_LOOP_FIFO) THEN
				width_in <= (OTHERS => '0');
			ELSIF ((stream_in_ready = '1') AND (stream_in_valid = '1')) THEN
				width_in <= width_in + 1;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				width_out <= (OTHERS => '0');
			ELSIF (fifo_read = '1') THEN
				IF (width_out = (WIDTH - 1)) THEN
					width_out <= (OTHERS => '0');
				ELSE
					width_out <= width_out + 1;
			END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				enlarge_height_counter <= (OTHERS => '0');
			ELSIF (s_multiply_height = STATE_0_GET_CURRENT_LINE) THEN
				enlarge_height_counter <= (OTHERS => '0');
			ELSIF ((fifo_read = '1') AND (width_out = (WIDTH - 1))) THEN
				enlarge_height_counter <= enlarge_height_counter + 1;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output assignments
	stream_in_ready <= '0' WHEN (s_multiply_height = STATE_1_LOOP_FIFO) ELSE 
							 '1' WHEN ((fifo_full = '0') AND NOT (width_in = WIDTH)) ELSE
							 '0';

	-- Internal assignments
	fifo_data_in(DW  DOWNTO  0) <= fifo_data_out(DW  DOWNTO  0) WHEN 
											(s_multiply_height = STATE_1_LOOP_FIFO) ELSE 
											stream_in_data;
	fifo_data_in(DW + 1) <= 
			fifo_data_out(DW + 1) WHEN (s_multiply_height = STATE_1_LOOP_FIFO) ELSE 
			stream_in_startofpacket;
	fifo_data_in(DW + 2) <= 
			fifo_data_out(DW + 2) WHEN (s_multiply_height = STATE_1_LOOP_FIFO) ELSE 
			stream_in_endofpacket;

	fifo_write <= 
			fifo_read WHEN (s_multiply_height = STATE_1_LOOP_FIFO) ELSE 
			'1' WHEN ((stream_in_ready = '1') AND (stream_in_valid = '1') AND 
			(fifo_full = '0')) ELSE '0';
	fifo_read <= 
			'0' WHEN (s_multiply_height = STATE_0_GET_CURRENT_LINE) ELSE 
			'1' WHEN (((stream_out_ready = '1') OR (stream_out_valid = '0')) AND 
			(fifo_empty = '0')) ELSE '0';
		
-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Multiply_Height_FIFO : scfifo 
	GENERIC MAP (
		add_ram_output_register	=> "OFF",
		intended_device_family	=> "Cyclone II",
		lpm_numwords				=> WIDTH + 1,
		lpm_showahead				=> "ON",
		lpm_type						=> "scfifo",
		lpm_width					=> DW + 3,
		lpm_widthu					=> WW + 1,
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
		full				=> fifo_full
		   
		-- synopsys translate_off
		
		-- synopsys translate_on
	);


END Behaviour;
