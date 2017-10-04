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
-- * This module store and retrieves video frames to and from memory.           *
-- *                                                                            *
-- ******************************************************************************

`undef USE_TO_MEMORY
`define USE_32BIT_MASTER

ENTITY altera_up_avalon_video_dma_controller IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW									:INTEGER									:= 15; 	-- Frame's datawidth
	EW									:INTEGER									:= 0; 	-- Frame's empty width
	WIDTH								:INTEGER									:= 640; 	-- Frame's width in pixels
	HEIGHT							:INTEGER									:= 480; 	-- Frame's height in lines
	
	AW									:INTEGER									:= 17; 	-- Frame's address width
	WW									:INTEGER									:= 8; 	-- Frame width's address width
	HW									:INTEGER									:= 7; 	-- Frame height's address width
	
	MDW								:INTEGER									:= 15; 	-- Avalon master's datawidth
	
	DEFAULT_BUFFER_ADDRESS		:STD_LOGIC_VECTOR(31 DOWNTO  0)	:= B"00000000000000000000000000000000";
	DEFAULT_BACK_BUF_ADDRESS	:STD_LOGIC_VECTOR(31 DOWNTO  0)	:= B"00000000000000000000000000000000";
	
	ADDRESSING_BITS				:STD_LOGIC_VECTOR(15 DOWNTO  0)	:= B"0000100000001001";
	COLOR_BITS						:STD_LOGIC_VECTOR( 3 DOWNTO  0)	:= B"0111";
	COLOR_PLANES					:STD_LOGIC_VECTOR( 1 DOWNTO  0)	:= B"10";

	DEFAULT_DMA_ENABLED			:STD_LOGIC								:= B"1"
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (

	-- Inputs
	clk						:IN		STD_LOGIC;
	reset						:IN		STD_LOGIC;

`ifdef USE_TO_MEMORY
	stream_data				:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_startofpacket	:IN		STD_LOGIC;
	stream_endofpacket	:IN		STD_LOGIC;
	stream_empty			:IN		STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_valid			:IN		STD_LOGIC;
`else
	stream_ready			:IN		STD_LOGIC;
`endif

`ifndef USE_TO_MEMORY
	master_readdata		:IN		STD_LOGIC_VECTOR(MDW DOWNTO  0);	
	master_readdatavalid	:IN		STD_LOGIC;
`endif
	master_waitrequest	:IN		STD_LOGIC;
	
	slave_address			:IN		STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	slave_byteenable		:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	slave_read				:IN		STD_LOGIC;
	slave_write				:IN		STD_LOGIC;
	slave_writedata		:IN		STD_LOGIC_VECTOR(31 DOWNTO  0);	

	-- Bidirectional

	-- Outputs
`ifdef USE_TO_MEMORY
	stream_ready			:BUFFER	STD_LOGIC;
`else
	stream_data				:BUFFER	STD_LOGIC_VECTOR(DW DOWNTO  0);	
	stream_startofpacket	:BUFFER	STD_LOGIC;
	stream_endofpacket	:BUFFER	STD_LOGIC;
	stream_empty			:BUFFER	STD_LOGIC_VECTOR(EW DOWNTO  0);	
	stream_valid			:BUFFER	STD_LOGIC;
`endif

	master_address			:BUFFER	STD_LOGIC_VECTOR(31 DOWNTO  0);	
`ifdef USE_TO_MEMORY
	master_write			:BUFFER	STD_LOGIC;
	master_writedata		:BUFFER	STD_LOGIC_VECTOR(MDW DOWNTO  0);	
`else
	master_arbiterlock	:BUFFER	STD_LOGIC;
	master_read				:BUFFER	STD_LOGIC;
`endif

	slave_readdata			:BUFFER	STD_LOGIC_VECTOR(31 DOWNTO  0)	

);

END altera_up_avalon_video_dma_controller;

ARCHITECTURE Behaviour OF altera_up_avalon_video_dma_controller IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	
	-- Internal Wires
	SIGNAL	inc_address							:STD_LOGIC;
	SIGNAL	reset_address						:STD_LOGIC;
	
	SIGNAL	buffer_start_address				:STD_LOGIC_VECTOR(31 DOWNTO  0);
	SIGNAL	dma_enabled							:STD_LOGIC;
	
	SIGNAL	reading_last_pixel_in_frame	: STD_LOGIC;
	SIGNAL	internal_dma_reset				: STD_LOGIC;
	
	-- Internal Registers
`ifdef USE_CONSECUTIVE_ADDRESSING
	SIGNAL	pixel_address						:STD_LOGIC_VECTOR(AW DOWNTO  0);	
`else
	SIGNAL	w_address							:STD_LOGIC_VECTOR(WW DOWNTO  0);	-- Frame's width address
	SIGNAL	h_address							:STD_LOGIC_VECTOR(HW DOWNTO  0);	-- Frame's height address
`endif
	
	-- State Machine Registers
	
	-- Integers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altera_up_video_dma_control_slave
	GENERIC (
		DEFAULT_BUFFER_ADDRESS		:STD_LOGIC_VECTOR(31 DOWNTO  0);
		DEFAULT_BACK_BUF_ADDRESS	:STD_LOGIC_VECTOR(31 DOWNTO  0);

		WIDTH								:INTEGER;
		HEIGHT							:INTEGER;

		ADDRESSING_BITS				:STD_LOGIC_VECTOR(15 DOWNTO  0);
		COLOR_BITS						:STD_LOGIC_VECTOR( 3 DOWNTO  0);
		COLOR_PLANES					:STD_LOGIC_VECTOR( 1 DOWNTO  0);
		ADDRESSING_MODE				:STD_LOGIC;
		
		DEFAULT_DMA_ENABLED			:STD_LOGIC
	);
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
	END COMPONENT;

`ifdef USE_TO_MEMORY
	COMPONENT altera_up_video_dma_to_memory
	GENERIC (
		DW							:INTEGER;
		EW							:INTEGER;
		MDW						:INTEGER
	);
	PORT (
		-- Inputs
		clk						:IN		STD_LOGIC;
		reset						:IN		STD_LOGIC;

		stream_data				:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);
		stream_startofpacket	:IN		STD_LOGIC;
		stream_endofpacket	:IN		STD_LOGIC;
		stream_empty			:IN		STD_LOGIC_VECTOR(EW DOWNTO  0);
		stream_valid			:IN		STD_LOGIC;

		master_waitrequest	:IN		STD_LOGIC;
	
		-- Bidirectional

		-- Outputs
		stream_ready			:BUFFER	STD_LOGIC;

		master_write			:BUFFER	STD_LOGIC;
		master_writedata		:BUFFER	STD_LOGIC_VECTOR(MDW DOWNTO  0);

		inc_address				:BUFFER	STD_LOGIC;
		reset_address			:BUFFER	STD_LOGIC
	);
	END COMPONENT;
`else
	COMPONENT altera_up_video_dma_to_stream
	GENERIC (
		DW										:INTEGER;
		EW										:INTEGER;
		MDW									:INTEGER
	);
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
	END COMPONENT;
`endif

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	-- Output Registers

	-- Internal Registers
`ifdef USE_CONSECUTIVE_ADDRESSING
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				pixel_address 	<= (OTHERS => '0');
			ELSIF (reset_address = '1') THEN
				pixel_address 	<= (OTHERS => '0');
			ELSIF (inc_address = '1') THEN
				pixel_address 	<= pixel_address + 1;
			END IF;
		END IF;
	END PROCESS;

`else
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				w_address 	<= (OTHERS => '0');
				h_address 	<= (OTHERS => '0');
			ELSIF (reset_address = '1') THEN
				w_address 	<= (OTHERS => '0');
				h_address 	<= (OTHERS => '0');
			ELSIF (inc_address = '1') THEN
				IF (w_address = (WIDTH - 1)) THEN
					w_address 	<= (OTHERS => '0');
					h_address	<= h_address + 1;
				ELSE
					w_address 	<= w_address + 1;
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
`ifdef USE_8BIT_MASTER
									pixel_address;
`elsif USE_16BIT_MASTER
							 		pixel_address & '0';
`elsif USE_32BIT_MASTER
									pixel_address & B"00";
`else
									pixel_address & B"000";
`endif
`else
`ifdef USE_8BIT_MASTER
									h_address & w_address;
`elsif USE_16BIT_MASTER
									h_address & w_address & '0';
`elsif USE_32BIT_MASTER
									h_address & w_address & B"00";
`else
									h_address & w_address & B"000";
`endif
`endif

	-- Internal Assignments
`ifdef USE_CONSECUTIVE_ADDRESSING
	reading_last_pixel_in_frame <= '1' WHEN (pixel_address = ((WIDTH * HEIGHT)  - 1)) ELSE 
											 '0';
`else
	reading_last_pixel_in_frame <= '1' WHEN (w_address = (WIDTH - 1)) 
															AND (h_address = (HEIGHT - 1)) ELSE 
											 '0';
`endif

	internal_dma_reset <= reset OR (NOT dma_enabled);

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	DMA_Control_Slave : altera_up_video_dma_control_slave 
	GENERIC MAP (
		DEFAULT_BUFFER_ADDRESS		=> DEFAULT_BUFFER_ADDRESS,
		DEFAULT_BACK_BUF_ADDRESS	=> DEFAULT_BACK_BUF_ADDRESS,

		WIDTH								=> WIDTH,
		HEIGHT							=> HEIGHT,

		ADDRESSING_BITS				=> ADDRESSING_BITS,
		COLOR_BITS						=> COLOR_BITS,
		COLOR_PLANES					=> COLOR_PLANES,
`ifdef USE_CONSECUTIVE_ADDRESSING
		ADDRESSING_MODE				=> '1',
`else
		ADDRESSING_MODE				=> '0',
`endif		

		DEFAULT_DMA_ENABLED			=> DEFAULT_DMA_ENABLED
	)
	PORT MAP (
		-- Inputs
		clk							=> clk,
		reset							=> reset,
	
		address						=> slave_address,
		byteenable					=> slave_byteenable,
		read							=> slave_read,
		write							=> slave_write,
		writedata					=> slave_writedata,
	
		swap_addresses_enable	=> reset_address,
	
		-- Bi-Directional
	
		-- Outputs
		readdata						=> slave_readdata,
	
		current_start_address	=> buffer_start_address,
		dma_enabled					=> dma_enabled
	);



`ifdef USE_TO_MEMORY
	From_Stream_to_Memory : altera_up_video_dma_to_memory 
	GENERIC MAP (
		DW							=> DW,
		EW							=> EW,
		MDW						=> MDW
	)
	PORT MAP (
		-- Inputs
		clk						=> clk,
		reset						=> internal_dma_reset,
	
		stream_data				=> stream_data,
		stream_startofpacket	=> stream_startofpacket,
		stream_endofpacket	=> stream_endofpacket,
		stream_empty			=> stream_empty,
		stream_valid			=> stream_valid,
	
		master_waitrequest	=> master_waitrequest,
		
		-- Bidirectional
	
		-- Outputs
		stream_ready			=> stream_ready,
	
		master_write			=> master_write,
		master_writedata		=> master_writedata,
	
		inc_address				=> inc_address,
		reset_address			=> reset_address
	);
`else

	From_Memory_to_Stream : altera_up_video_dma_to_stream 
	GENERIC MAP (
		DW										=> DW,
		EW										=> EW,
		MDW									=> MDW
	)
	PORT MAP (
		-- Inputs
		clk									=> clk,
		reset									=> internal_dma_reset,
	
		stream_ready						=> stream_ready,
	
		master_readdata					=> master_readdata,
		master_readdatavalid				=> master_readdatavalid,
		master_waitrequest				=> master_waitrequest,
		
`ifdef USE_CONSECUTIVE_ADDRESSING
		reading_first_pixel_in_frame	=> NOR_REDUCE(pixel_address),
		reading_last_pixel_in_frame	=> reading_last_pixel_in_frame,
`else
		reading_first_pixel_in_frame	=> NOR_REDUCE(w_address) AND NOR_REDUCE(h_address),
		reading_last_pixel_in_frame	=> reading_last_pixel_in_frame,
`endif
	
		-- Bidirectional
	
		-- Outputs
		stream_data							=> stream_data,
		stream_startofpacket				=> stream_startofpacket,
		stream_endofpacket				=> stream_endofpacket,
		stream_empty						=> stream_empty,
		stream_valid						=> stream_valid,
	
		master_arbiterlock				=> master_arbiterlock,
		master_read							=> master_read,
	
		inc_address							=> inc_address,
		reset_address						=> reset_address
	);
`endif


END Behaviour;
