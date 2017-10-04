LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_unsigned.all;
USE ieee.std_logic_misc.all;
USE ieee.numeric_std;

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
-- * This module is a buffer that holds characters to be displayed on a         *
-- *  VGA or LCD screen.                                                        *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_character_buffer_with_dma IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	DW					:INTEGER									:= 8;
	
	ENLARGE_CHAR	:INTEGER									:= 0;
	
`ifdef USE_TRDB_LCM
	AW					:INTEGER									:= 11;
	BUFFER_SIZE		:INTEGER									:= 2048;
	
	PIXELS			:INTEGER									:= 320;
	LINES				:INTEGER									:= 240
`elsif USE_TRDB_LTM
	AW					:INTEGER									:= 11;
	BUFFER_SIZE		:INTEGER									:= 2048;
	
	PIXELS			:INTEGER									:= 800;
	LINES				:INTEGER									:= 480
`else
	AW					:INTEGER									:= 13;
	BUFFER_SIZE		:INTEGER									:= 8192;
	
	PIXELS			:INTEGER									:= 640;
	LINES				:INTEGER									:= 480
`endif
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk						:IN		STD_LOGIC;
	reset						:IN		STD_LOGIC;

	ctrl_address			:IN		STD_LOGIC;
	ctrl_byteenable		:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	ctrl_chipselect		:IN		STD_LOGIC;
	ctrl_read				:IN		STD_LOGIC;
	ctrl_write				:IN		STD_LOGIC;
	ctrl_writedata			:IN		STD_LOGIC_VECTOR(31 DOWNTO  0);	

	buf_address				:IN		STD_LOGIC_VECTOR((AW - 1) DOWNTO  0);	
	buf_byteenable			:IN		STD_LOGIC;
	buf_chipselect			:IN		STD_LOGIC;
	buf_read					:IN		STD_LOGIC;
	buf_write				:IN		STD_LOGIC;
	buf_writedata			:IN		STD_LOGIC_VECTOR( 7 DOWNTO  0);	

	stream_ready			:IN		STD_LOGIC;

	-- Bidirectionals

	-- Outputs
	ctrl_readdata			:BUFFER	STD_LOGIC_VECTOR(31 DOWNTO  0);	

	buf_readdata			:BUFFER	STD_LOGIC_VECTOR( 7 DOWNTO  0);	
	buf_waitrequest		:BUFFER	STD_LOGIC;

`ifdef USE_TRANSPARENCY
	stream_data				:BUFFER	STD_LOGIC_VECTOR(39 DOWNTO  0);	
`else
	stream_data				:BUFFER	STD_LOGIC_VECTOR(29 DOWNTO  0);	
`endif
	stream_startofpacket	:BUFFER	STD_LOGIC;
	stream_endofpacket	:BUFFER	STD_LOGIC;
	stream_empty			:BUFFER	STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	stream_valid			:BUFFER	STD_LOGIC

);

END altera_up_avalon_video_character_buffer_with_dma;

ARCHITECTURE Behaviour OF altera_up_avalon_video_character_buffer_with_dma IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	char_data_to_buffer		:STD_LOGIC_VECTOR((DW - 1) DOWNTO  0);	
	SIGNAL	char_data_from_buffer	:STD_LOGIC_VECTOR((DW - 1) DOWNTO  0);	
	
	SIGNAL	cur_char_position			:STD_LOGIC_VECTOR((AW - 1) DOWNTO  0);	
	SIGNAL	cur_char_for_display		:STD_LOGIC_VECTOR(15 DOWNTO  0);	
	
	SIGNAL	cur_char_data				:STD_LOGIC;
	
`ifdef USE_FOREGROUND_COLOR
	SIGNAL	fg_R							:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	fg_G							:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	fg_B							:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
`endif
	
`ifdef USE_BACKGROUND_COLOR
	SIGNAL	bg_R							:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	bg_G							:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	bg_B							:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
`endif
	
	SIGNAL	char_red						:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	char_green					:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	char_blue					:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	
	-- Internal Registers
	SIGNAL	control_reg					:STD_LOGIC_VECTOR(31 DOWNTO  0);	
	
	SIGNAL	delayed_buf_waitrequest	:STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	
	SIGNAL	clear_screen				:STD_LOGIC;
	
	SIGNAL	x_position					:STD_LOGIC_VECTOR( 9 DOWNTO  0);	
	SIGNAL	y_position					:STD_LOGIC_VECTOR( 8 DOWNTO  0);	
	SIGNAL	delayed_x_position		:STD_LOGIC_VECTOR( 5 DOWNTO  0);	
	SIGNAL	delayed_y_position		:STD_LOGIC_VECTOR( 5 DOWNTO  0);	
	
	SIGNAL	delayed_startofpacket	:STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	SIGNAL	delayed_endofpacket		:STD_LOGIC_VECTOR( 3 DOWNTO  0);	
	
`ifdef USE_9BIT_COLOR
	SIGNAL	delayed_char_color		:STD_LOGIC_VECTOR(17 DOWNTO  0);	
`endif
	
	-- State Machine Registers
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altsyncram
	GENERIC (
		init_file									:STRING;
		intended_device_family					:STRING;
		lpm_type										:STRING;
		operation_mode								:STRING;
		read_during_write_mode_mixed_ports	:STRING;
		power_up_uninitialized					:STRING;
		address_reg_b								:STRING;
		indata_reg_b								:STRING;
		wrcontrol_wraddress_reg_b				:STRING;
		clock_enable_input_a						:STRING;
		clock_enable_input_b						:STRING;
		clock_enable_output_a					:STRING;
		clock_enable_output_b					:STRING;
		numwords_a									:NATURAL;
		numwords_b									:NATURAL;
		outdata_aclr_a								:STRING;
		outdata_aclr_b								:STRING;
		outdata_reg_a								:STRING;
		outdata_reg_b								:STRING;
		widthad_a									:NATURAL;
		widthad_b									:NATURAL;
		width_a										:NATURAL;
		width_b										:NATURAL;
		width_byteena_a							:NATURAL;
		width_byteena_b							:NATURAL
	);
	PORT (
		-- Inputs
		clock0			:IN		STD_LOGIC;
		address_a		:IN		STD_LOGIC_VECTOR((AW - 1) DOWNTO  0);
		wren_a			:IN		STD_LOGIC;
		data_a			:IN		STD_LOGIC_VECTOR((DW - 1) DOWNTO  0);

		clock1			:IN		STD_LOGIC;
		clocken1			:IN		STD_LOGIC;
		address_b		:IN		STD_LOGIC_VECTOR((AW - 1) DOWNTO  0);
		wren_b			:IN		STD_LOGIC;
		data_b			:IN		STD_LOGIC_VECTOR((DW - 1) DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		q_a				:BUFFER	STD_LOGIC_VECTOR((DW - 1) DOWNTO  0);
		q_b				:BUFFER	STD_LOGIC_VECTOR(15 DOWNTO  0)
	);
	END COMPONENT;

	COMPONENT altera_up_video_128_character_rom
	PORT (
		-- Inputs
		clk				:IN		STD_LOGIC;
		clk_en			:IN		STD_LOGIC;

		character		:IN		STD_LOGIC_VECTOR( 6 DOWNTO  0);
		x_coordinate	:IN		STD_LOGIC_VECTOR( 2 DOWNTO  0);
		y_coordinate	:IN		STD_LOGIC_VECTOR( 2 DOWNTO  0);
	
		-- Bidirectionals

		-- Outputs
		character_data	:BUFFER	STD_LOGIC
	);
	END COMPONENT;

	COMPONENT altera_up_video_fb_color_rom
	PORT (
		-- Inputs
		clk				:IN		STD_LOGIC;
		clk_en			:IN		STD_LOGIC;

		color_index		:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);
	
		-- Bidirectionals

		-- Outputs
		red				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);
		green				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0);
		blue				:BUFFER	STD_LOGIC_VECTOR( 9 DOWNTO  0)
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
				ctrl_readdata <= B"00000000000000000000000000000000";
			ELSIF ((ctrl_chipselect = '1') AND (ctrl_read = '1') AND (ctrl_address = '1')) THEN
`ifdef USE_TRDB_LCM
				ctrl_readdata <= (B"0000000000011110" & B"0000000000101000");
`elsif USE_TRDB_LTM
				ctrl_readdata <= (B"0000000000011110" & B"0000000000110010");
`else
				ctrl_readdata <= (B"0000000000111100" & B"0000000001010000");
`endif
			ELSIF ((ctrl_chipselect = '1') AND (ctrl_read = '1')) THEN
				ctrl_readdata <= control_reg;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				buf_readdata <= B"00000000";
			ELSIF ((buf_chipselect = '1') AND (buf_read = '1')) THEN
				buf_readdata <= ('0' & char_data_from_buffer(6 DOWNTO 0));
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				control_reg <= B"00000000000000010000000000000000";
			ELSIF ((ctrl_chipselect = '1') AND (ctrl_write = '1') AND (ctrl_address = '0')) THEN
				IF (ctrl_byteenable(0) = '1') THEN 
					control_reg( 7 DOWNTO  0) <= ctrl_writedata( 7 DOWNTO  0);
				END IF;
				IF (ctrl_byteenable(1) = '1') THEN 
					control_reg(15 DOWNTO  8) <= ctrl_writedata(15 DOWNTO  8);
				END IF;
				IF (ctrl_byteenable(2) = '1') THEN 
					control_reg(23 DOWNTO 16) <= ctrl_writedata(23 DOWNTO 16);
				END IF;
				IF (ctrl_byteenable(3) = '1') THEN 
					control_reg(31 DOWNTO 24) <= ctrl_writedata(31 DOWNTO 24);
				END IF;
			ELSIF ((clear_screen = '1') AND (stream_ready = '1') AND 
					(x_position = (PIXELS - 1)) AND (y_position = (LINES - 1))) THEN
				control_reg(16) <= '0';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				delayed_buf_waitrequest <= B"00";
			ELSIF ((buf_chipselect = '1') AND (buf_read = '1')) THEN
				delayed_buf_waitrequest <= (delayed_buf_waitrequest(0) & '1');
			ELSE
				delayed_buf_waitrequest <= B"00";
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				clear_screen <= '1';
			ELSIF (control_reg(16) = '0') THEN
				clear_screen <= '0';
			ELSIF ((x_position = 0) AND (y_position = 0)) THEN
				clear_screen <= '1';
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				x_position <= (OTHERS => '0');
			ELSIF (stream_ready = '1') THEN
				IF (x_position = (PIXELS - 1)) THEN
					x_position <= (OTHERS => '0');
				ELSE
					x_position <= x_position + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				y_position <= (OTHERS => '0');
			ELSIF ((stream_ready = '1') AND (x_position = (PIXELS - 1))) THEN
				IF (y_position = (LINES - 1)) THEN
					y_position <= (OTHERS => '0');
				ELSE
					y_position <= y_position + 1;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				delayed_x_position <= (OTHERS => '0');
				delayed_y_position <= (OTHERS => '0');
			ELSIF (stream_ready = '1') THEN
				delayed_x_position <= (delayed_x_position(2 DOWNTO 0) & 
					x_position((ENLARGE_CHAR+2) DOWNTO ENLARGE_CHAR));
				delayed_y_position <= (delayed_y_position(2 DOWNTO 0) & 
					y_position((ENLARGE_CHAR+2) DOWNTO ENLARGE_CHAR));
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				delayed_startofpacket <= (OTHERS => '0');
			ELSIF (stream_ready = '1') THEN
				delayed_startofpacket(3 DOWNTO 1) <= delayed_startofpacket(2 DOWNTO 0);
				IF ((x_position = 0) AND (y_position = 0)) THEN
					delayed_startofpacket(0) <= '1';
				ELSE
					delayed_startofpacket(0) <= '0';
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				delayed_endofpacket <= (OTHERS => '0');
			ELSIF (stream_ready = '1') THEN
				delayed_endofpacket(3 DOWNTO 1) <= delayed_endofpacket(2 DOWNTO 0);
				IF ((x_position = (PIXELS - 1)) AND (y_position = (LINES - 1))) THEN
					delayed_endofpacket(0) <= '1';
				ELSE
					delayed_endofpacket(0) <= '0';
				END IF;
			END IF;
		END IF;
	END PROCESS;


`ifdef USE_9BIT_COLOR
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				delayed_char_color <= (OTHERS => '0');
			ELSIF (stream_ready = '1') THEN
				delayed_char_color <= (delayed_char_color(8 DOWNTO 0) & 
					cur_char_for_display(15 DOWNTO 7));
			END IF;
		END IF;
	END PROCESS;

`endif

-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	buf_waitrequest <= 
			(buf_chipselect AND buf_read) AND NOT delayed_buf_waitrequest(1);

`ifdef USE_TRANSPARENCY
	stream_data(39 DOWNTO 30) <= (cur_char_data & cur_char_data & cur_char_data & 
											cur_char_data & cur_char_data & cur_char_data & 
											cur_char_data & cur_char_data & cur_char_data & 
											cur_char_data);
`endif
	stream_data(29 DOWNTO  0) 	<= (char_red & char_green & char_blue);
	stream_startofpacket 		<= delayed_startofpacket(3);
	stream_endofpacket 			<= delayed_endofpacket(3);
	stream_empty 					<= B"00";
	stream_valid 					<= '1';

	-- Internal Assignments
	char_data_to_buffer <= (control_reg((DW-8) DOWNTO  0) & buf_writedata(6 DOWNTO 0));

`ifdef USE_TRDB_LCM
	cur_char_position <= 
		(y_position(7 DOWNTO (3 + ENLARGE_CHAR)) & x_position(8 DOWNTO (3 + ENLARGE_CHAR)));
`else
	cur_char_position <= 
		(y_position(8 DOWNTO (3 + ENLARGE_CHAR)) & x_position(9 DOWNTO (3 + ENLARGE_CHAR)));
`endif

`ifdef USE_1BIT_COLOR
	char_red 	<= (OTHERS => cur_char_data);
	char_green 	<= (OTHERS => cur_char_data);
	char_blue 	<= (OTHERS => cur_char_data);
`elsif USE_4BIT_COLOR
	char_red 	<= fg_R(9 DOWNTO 1) & '1' WHEN (cur_char_data = '1') ELSE
						(OTHERS => '0');
	char_green 	<= fg_G(9 DOWNTO 1) & '1' WHEN (cur_char_data = '1') ELSE
						(OTHERS => '0');
	char_blue 	<= fg_B(9 DOWNTO 1) & '1' WHEN (cur_char_data = '1') ELSE
						(OTHERS => '0');
`elsif USE_8BIT_COLOR
	char_red 	<= ((cur_char_data & cur_char_data & cur_char_data & cur_char_data & 
						  cur_char_data & cur_char_data & cur_char_data & cur_char_data & 
						  cur_char_data & cur_char_data) AND fg_R) OR 
						((NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data) AND bg_R);
	char_green 	<= ((cur_char_data & cur_char_data & cur_char_data & cur_char_data & 
						  cur_char_data & cur_char_data & cur_char_data & cur_char_data & 
						  cur_char_data & cur_char_data) AND fg_G) OR 
						((NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data) AND bg_G);
	char_blue 	<= ((cur_char_data & cur_char_data & cur_char_data & cur_char_data & 
						  cur_char_data & cur_char_data & cur_char_data & cur_char_data & 
						  cur_char_data & cur_char_data) AND fg_B) OR 
						 ((NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data & NOT cur_char_data & NOT cur_char_data &  
						  NOT cur_char_data) AND bg_B);
`else
	char_red 	<= ((delayed_char_color(17 DOWNTO 15) & delayed_char_color(17 DOWNTO 15) & 
						  delayed_char_color(17 DOWNTO 15)) & cur_char_data);
	char_green 	<= ((delayed_char_color(14 DOWNTO 12) & delayed_char_color(14 DOWNTO 12) & 
						  delayed_char_color(14 DOWNTO 12)) & cur_char_data);
	char_blue 	<= ((delayed_char_color(11 DOWNTO  9) & delayed_char_color(11 DOWNTO  9) & 
						  delayed_char_color(11 DOWNTO  9)) & cur_char_data);
`endif

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	Char_Buffer_Memory : altsyncram 
	GENERIC MAP (
		init_file									=> "UNUSED",
		intended_device_family					=> "Cyclone II",
		lpm_type										=> "altsyncram",
		operation_mode								=> "BIDIR_DUAL_PORT",
		read_during_write_mode_mixed_ports	=> "DONT_CARE",
		power_up_uninitialized					=> "FALSE",
		address_reg_b								=> "CLOCK1",
		indata_reg_b								=> "CLOCK1",
		wrcontrol_wraddress_reg_b				=> "CLOCK1",
		clock_enable_input_a						=> "BYPASS",
		clock_enable_input_b						=> "NORMAL",
		clock_enable_output_a					=> "BYPASS",
		clock_enable_output_b					=> "NORMAL",
		numwords_a									=> BUFFER_SIZE,
		numwords_b									=> BUFFER_SIZE,
		outdata_aclr_a								=> "NONE",
		outdata_aclr_b								=> "NONE",
		outdata_reg_a								=> "CLOCK0",
		outdata_reg_b								=> "CLOCK1",
		widthad_a									=> AW,
		widthad_b									=> AW,
		width_a										=> DW,
		width_b										=> DW,
		width_byteena_a							=> 1,
		width_byteena_b							=> 1
	)
	PORT MAP (
		-- Inputs
		clock0			=> clk,
		address_a		=> buf_address,
		wren_a			=> buf_byteenable AND buf_chipselect AND buf_write,
		data_a			=> char_data_to_buffer,
	
		clock1			=> clk,
		clocken1			=> stream_ready,
		address_b		=> cur_char_position,
		wren_b			=> clear_screen,
		data_b			=> (5 => '1', OTHERS => '0'),
	
		-- Bidirectionals
	
		-- Outputs
		q_a				=> char_data_from_buffer,
		q_b				=> cur_char_for_display
	);

	
	Character_Rom : altera_up_video_128_character_rom 
	PORT MAP (
		-- Inputs
		clk				=> clk,
		clk_en			=> stream_ready,
	
		character		=> cur_char_for_display( 6 DOWNTO  0),
		x_coordinate	=> delayed_x_position( 5 DOWNTO  3),
		y_coordinate	=> delayed_y_position( 5 DOWNTO  3),
		
		-- Bidirectionals
	
		-- Outputs
		character_data	=> cur_char_data
	);

`ifdef USE_FOREGROUND_COLOR
	Foreground_Color_Rom : altera_up_video_fb_color_rom 
	PORT MAP (
		-- Inputs
		clk				=> clk,
		clk_en			=> stream_ready,
	
		color_index		=> cur_char_for_display(10 DOWNTO  7),
		
		-- Bidirectionals
	
		-- Outputs
		red				=> fg_R,
		green				=> fg_G,
		blue				=> fg_B
	);

`endif

`ifdef USE_BACKGROUND_COLOR
	Background_Color_Rom : altera_up_video_fb_color_rom 
	PORT MAP (
		-- Inputs
		clk				=> clk,
		clk_en			=> stream_ready,
	
		color_index		=> cur_char_for_display(14 DOWNTO 11),
		
		-- Bidirectionals
	
		-- Outputs
		red				=> bg_R,
		green				=> bg_G,
		blue				=> bg_B
	);

`endif


END Behaviour;
