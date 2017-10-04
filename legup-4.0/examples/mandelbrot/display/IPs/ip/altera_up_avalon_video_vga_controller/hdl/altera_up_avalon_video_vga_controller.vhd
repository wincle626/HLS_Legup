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
-- * This module controls VGA output for Altera's DE1 and DE2 Boards.           *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_vga_controller IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	CW									:INTEGER									:= 9;
	DW									:INTEGER									:= 29;
	
	R_UI								:INTEGER									:= 29;
	R_LI								:INTEGER									:= 20;
	G_UI								:INTEGER									:= 19;
	G_LI								:INTEGER									:= 10;
	B_UI								:INTEGER									:= 9;
	B_LI								:INTEGER									:= 0;
	
`ifdef USE_TRDB_LCM
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 1280;
	H_FRONT_PORCH					:INTEGER									:= 40;
	H_SYNC							:INTEGER									:= 1;
	H_BACK_PORCH					:INTEGER									:= 239;
	H_TOTAL							:INTEGER									:= 1560;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 240;
	V_FRONT_PORCH					:INTEGER									:= 1;
	V_SYNC							:INTEGER									:= 1;
	V_BACK_PORCH					:INTEGER									:= 20;
	V_TOTAL							:INTEGER									:= 262;
	
	LW									:INTEGER									:= 9;		-- Number of bits for lines
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 8 DOWNTO  0)	:= B"000000001";
	
	PW									:INTEGER									:= 11;	-- Number of bits for pixels
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)	:= B"000000001"

`elsif USE_TRDB_LTM
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 800;
	H_FRONT_PORCH					:INTEGER									:= 40;
	H_SYNC							:INTEGER									:= 1;
	H_BACK_PORCH					:INTEGER									:= 215;
	H_TOTAL							:INTEGER									:= 1056;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 480;
	V_FRONT_PORCH					:INTEGER									:= 10;
	V_SYNC							:INTEGER									:= 1;
	V_BACK_PORCH					:INTEGER									:= 34;
	V_TOTAL							:INTEGER									:= 525;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)	:= B"00000000001"

`elsif USE_TPAD
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 800;
	H_FRONT_PORCH					:INTEGER									:= 40;
	H_SYNC							:INTEGER									:= 128;
	H_BACK_PORCH					:INTEGER									:= 88;
	H_TOTAL							:INTEGER									:= 1056;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 600;
	V_FRONT_PORCH					:INTEGER									:= 1;
	V_SYNC							:INTEGER									:= 4;
	V_BACK_PORCH					:INTEGER									:= 23;
	V_TOTAL							:INTEGER									:= 628;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)	:= B"00000000001"

`elsif USE_VEEK_MT
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 800;
	H_FRONT_PORCH					:INTEGER									:= 210;
	H_SYNC							:INTEGER									:= 30;
	H_BACK_PORCH					:INTEGER									:= 16;
	H_TOTAL							:INTEGER									:= 1056;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 480;
	V_FRONT_PORCH					:INTEGER									:= 22;
	V_SYNC							:INTEGER									:= 13;
	V_BACK_PORCH					:INTEGER									:= 10;
	V_TOTAL							:INTEGER									:= 525;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)	:= B"00000000001"

`elsif USE_VGA
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 640;
	H_FRONT_PORCH					:INTEGER									:= 16;
	H_SYNC							:INTEGER									:= 96;
	H_BACK_PORCH					:INTEGER									:= 48;
	H_TOTAL							:INTEGER									:= 800;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 480;
	V_FRONT_PORCH					:INTEGER									:= 10;
	V_SYNC							:INTEGER									:= 2;
	V_BACK_PORCH					:INTEGER									:= 33;
	V_TOTAL							:INTEGER									:= 525;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 10;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001"

`elsif USE_SVGA
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 800;
	H_FRONT_PORCH					:INTEGER									:= 40;
	H_SYNC							:INTEGER									:= 128;
	H_BACK_PORCH					:INTEGER									:= 88;
	H_TOTAL							:INTEGER									:= 1056;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 600;
	V_FRONT_PORCH					:INTEGER									:= 2;
	V_SYNC							:INTEGER									:= 4;
	V_BACK_PORCH					:INTEGER									:= 22;
	V_TOTAL							:INTEGER									:= 628;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 10 DOWNTO  0)	:= B"00000000001"

`elsif USE_XGA
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 1024;
	H_FRONT_PORCH					:INTEGER									:= 24;
	H_SYNC							:INTEGER									:= 136;
	H_BACK_PORCH					:INTEGER									:= 160;
	H_TOTAL							:INTEGER									:= 1344;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 768;
	V_FRONT_PORCH					:INTEGER									:= 3;
	V_SYNC							:INTEGER									:= 6;
	V_BACK_PORCH					:INTEGER									:= 29;
	V_TOTAL							:INTEGER									:= 806;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 10 DOWNTO  0)	:= B"00000000001"

`elsif USE_WXGA
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 1280;
	H_FRONT_PORCH					:INTEGER									:= 64;
	H_SYNC							:INTEGER									:= 136;
	H_BACK_PORCH					:INTEGER									:= 200;
	H_TOTAL							:INTEGER									:= 1680;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 800;
	V_FRONT_PORCH					:INTEGER									:= 2;
	V_SYNC							:INTEGER									:= 3;
	V_BACK_PORCH					:INTEGER									:= 23;
	V_TOTAL							:INTEGER									:= 828;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 10 DOWNTO  0)	:= B"00000000001"

`elsif USE_SXGA
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 1280;
	H_FRONT_PORCH					:INTEGER									:= 48;
	H_SYNC							:INTEGER									:= 112;
	H_BACK_PORCH					:INTEGER									:= 248;
	H_TOTAL							:INTEGER									:= 1688;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 1024;
	V_FRONT_PORCH					:INTEGER									:= 2;
	V_SYNC							:INTEGER									:= 3;
	V_BACK_PORCH					:INTEGER									:= 37;
	V_TOTAL							:INTEGER									:= 1066;
	
	LW									:INTEGER									:= 11;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"00000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 10 DOWNTO  0)	:= B"00000000001"

`elsif USE_WSXGA
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 1680;
	H_FRONT_PORCH					:INTEGER									:= 104;
	H_SYNC							:INTEGER									:= 184;
	H_BACK_PORCH					:INTEGER									:= 288;
	H_TOTAL							:INTEGER									:= 2256;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 1050;
	V_FRONT_PORCH					:INTEGER									:= 2;
	V_SYNC							:INTEGER									:= 3;
	V_BACK_PORCH					:INTEGER									:= 32;
	V_TOTAL							:INTEGER									:= 1087;
	
	LW									:INTEGER									:= 11;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"00000000001";
	
	PW									:INTEGER									:= 12;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 10 DOWNTO  0)	:= B"000000000001"

`elsif USE_SDTV
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 720;
	H_FRONT_PORCH					:INTEGER									:= 24;
	H_SYNC							:INTEGER									:= 40;
	H_BACK_PORCH					:INTEGER									:= 96;
	H_TOTAL							:INTEGER									:= 880;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 480;
	V_FRONT_PORCH					:INTEGER									:= 10;
	V_SYNC							:INTEGER									:= 3;
	V_BACK_PORCH					:INTEGER									:= 32;
	V_TOTAL							:INTEGER									:= 525;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 10;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 10 DOWNTO  0)	:= B"0000000001"

`elsif USE_HDTV
	-- Number of pixels 
	H_ACTIVE							:INTEGER									:= 1280;
	H_FRONT_PORCH					:INTEGER									:= 72;
	H_SYNC							:INTEGER									:= 80;
	H_BACK_PORCH					:INTEGER									:= 216;
	H_TOTAL							:INTEGER									:= 1648;
	
	-- Number of lines 
	V_ACTIVE							:INTEGER									:= 720;
	V_FRONT_PORCH					:INTEGER									:= 3;
	V_SYNC							:INTEGER									:= 5;
	V_BACK_PORCH					:INTEGER									:= 22;
	V_TOTAL							:INTEGER									:= 750;
	
	LW									:INTEGER									:= 10;
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	PW									:INTEGER									:= 11;
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 10 DOWNTO  0)	:= B"00000000001"

`endif
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk				:IN		STD_LOGIC;
	reset				:IN		STD_LOGIC;

	data				:IN		STD_LOGIC_VECTOR(DW DOWNTO  0);	
	startofpacket	:IN		STD_LOGIC;
	endofpacket		:IN		STD_LOGIC;
	empty				:IN		STD_LOGIC_VECTOR( 1 DOWNTO  0);	
	valid				:IN		STD_LOGIC;

	-- Bidirectionals

	-- Outputs
	ready				:BUFFER	STD_LOGIC;

`ifdef USE_UNDERFLOW_FLAG
	underflow_flag	:BUFFER	STD_LOGIC;
`endif

	VGA_CLK			:BUFFER	STD_LOGIC;
	VGA_BLANK		:BUFFER	STD_LOGIC;
	VGA_SYNC			:BUFFER	STD_LOGIC;
	VGA_HS			:BUFFER	STD_LOGIC;
	VGA_VS			:BUFFER	STD_LOGIC;
`ifdef USE_TRDB_LTM
	VGA_DATA_EN		:BUFFER	STD_LOGIC;
`elsif USE_TPAD
	VGA_DATA_EN		:BUFFER	STD_LOGIC;
`elsif USE_VEEK_MT
	VGA_DATA_EN		:BUFFER	STD_LOGIC;
`endif
`ifdef USE_TRDB_LCM
	VGA_COLOR		:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0)	
`else
	VGA_R				:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);	
	VGA_G				:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);	
	VGA_B				:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0)	
`endif

);

END altera_up_avalon_video_vga_controller;

ARCHITECTURE Behaviour OF altera_up_avalon_video_vga_controller IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************
	
	-- States
	TYPE State_Type IS (	STATE_0_SYNC_FRAME,
								STATE_1_DISPLAY
							);
	
-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	SIGNAL	read_enable				:STD_LOGIC;
	SIGNAL	end_of_active_frame	:STD_LOGIC;
	
	SIGNAL	vga_blank_sync			:STD_LOGIC;
	SIGNAL	vga_c_sync				:STD_LOGIC;
	SIGNAL	vga_h_sync				:STD_LOGIC;
	SIGNAL	vga_v_sync				:STD_LOGIC;
	SIGNAL	vga_data_enable		:STD_LOGIC;
	SIGNAL	vga_red					:STD_LOGIC_VECTOR(CW DOWNTO  0);	
	SIGNAL	vga_green				:STD_LOGIC_VECTOR(CW DOWNTO  0);	
	SIGNAL	vga_blue					:STD_LOGIC_VECTOR(CW DOWNTO  0);	
	SIGNAL	vga_color_data			:STD_LOGIC_VECTOR(CW DOWNTO  0);	
	
	
	-- Internal Registers
	SIGNAL	color_select			:STD_LOGIC_VECTOR( 3 DOWNTO  0);	-- Use for the TRDB_LCM
	
	-- State Machine Registers
	SIGNAL	ns_mode					:State_Type;
	SIGNAL	s_mode					:State_Type;
	
-- *****************************************************************************
-- *                          Component Declarations                           *
-- *****************************************************************************
	COMPONENT altera_up_avalon_video_vga_timing
	GENERIC (
		CW									:INTEGER;

		H_ACTIVE							:INTEGER;
		H_FRONT_PORCH					:INTEGER;
		H_SYNC							:INTEGER;
		H_BACK_PORCH					:INTEGER;
		H_TOTAL							:INTEGER;

		V_ACTIVE							:INTEGER;
		V_FRONT_PORCH					:INTEGER;
		V_SYNC							:INTEGER;
		V_BACK_PORCH					:INTEGER;
		V_TOTAL							:INTEGER;

		LW									:INTEGER;
`ifdef USE_TRDB_LCM
		LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 8 DOWNTO  0);
`elsif USE_TRDB_LTM
		LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0);
`elsif USE_TPAD
		LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0);
`elsif USE_VEEK_MT
		LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0);
`else
		LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0);
`endif

		PW									:INTEGER;
`ifdef USE_TRDB_LCM
		PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)
`elsif USE_TRDB_LTM
		PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)
`elsif USE_TPAD
		PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)
`elsif USE_VEEK_MT
		PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR(10 DOWNTO  0)
`else
		PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)
`endif
	);
	PORT (
		-- Inputs
		clk						:IN		STD_LOGIC;
		reset						:IN		STD_LOGIC;

		red_to_vga_display	:IN		STD_LOGIC_VECTOR((R_UI - R_LI) DOWNTO  0);
		green_to_vga_display	:IN		STD_LOGIC_VECTOR((G_UI - G_LI) DOWNTO  0);
		blue_to_vga_display	:IN		STD_LOGIC_VECTOR((B_UI - B_LI) DOWNTO  0);
		color_select			:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);

		-- Bidirectionals

		-- Outputs
		read_enable				:BUFFER	STD_LOGIC;

		end_of_active_frame	:BUFFER	STD_LOGIC;

		-- dac pins
		vga_blank				:BUFFER	STD_LOGIC;
		vga_c_sync				:BUFFER	STD_LOGIC;
		vga_h_sync				:BUFFER	STD_LOGIC;
		vga_v_sync				:BUFFER	STD_LOGIC;
		vga_data_enable		:BUFFER	STD_LOGIC;
		vga_red					:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);
		vga_green				:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);
		vga_blue					:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);
		vga_color_data			:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0)
	);
	END COMPONENT;

BEGIN
-- *****************************************************************************
-- *                         Finite State Machine(s)                           *
-- *****************************************************************************

	PROCESS (clk)	-- sync reset
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				s_mode <= STATE_0_SYNC_FRAME;
			ELSE
				s_mode <= ns_mode;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (ns_mode, s_mode, valid, startofpacket, end_of_active_frame)
	BEGIN
		-- Defaults
		ns_mode <= STATE_0_SYNC_FRAME;
	
	   CASE (s_mode) IS
		WHEN STATE_0_SYNC_FRAME =>
			IF ((valid = '1') AND (startofpacket = '1')) THEN
				ns_mode <= STATE_1_DISPLAY;
			ELSE
				ns_mode <= STATE_0_SYNC_FRAME;
			END IF;
		WHEN STATE_1_DISPLAY =>
			IF (end_of_active_frame = '1') THEN
				ns_mode <= STATE_0_SYNC_FRAME;
			ELSE
				ns_mode <= STATE_1_DISPLAY;
			END IF;
		WHEN OTHERS =>
			ns_mode <= STATE_0_SYNC_FRAME;
		END CASE;
	END PROCESS;


-- *****************************************************************************
-- *                             Sequential Logic                              *
-- *****************************************************************************

	-- Output Registers
`ifdef USE_UNDERFLOW_FLAG
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				underflow_flag <= '0';
			ELSIF ((ns_mode = STATE_1_DISPLAY) AND 
					 ((read_enable = '1') AND (valid = '0'))) THEN
				underflow_flag <= '1';
			END IF;
		END IF;
	END PROCESS;

`endif

	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			VGA_BLANK	<= vga_blank_sync;
			VGA_SYNC		<= '0';
			VGA_HS		<= vga_h_sync;
			VGA_VS		<= vga_v_sync;
`ifdef USE_TRDB_LTM
			VGA_DATA_EN	<= vga_data_enable;
`elsif USE_TPAD
			VGA_DATA_EN	<= vga_data_enable;
`elsif USE_VEEK_MT
			VGA_DATA_EN	<= vga_data_enable;
`endif
`ifdef USE_TRDB_LCM
			VGA_COLOR	<= vga_color_data;
`else
			VGA_R			<= vga_red;
			VGA_G			<= vga_green;
			VGA_B			<= vga_blue;
`endif
		END IF;
	END PROCESS;



	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				color_select <= B"0001";
			ELSIF (s_mode = STATE_0_SYNC_FRAME) THEN
				color_select <= B"0001";
			ELSIF (read_enable = '0') THEN
				color_select <= (color_select(2 DOWNTO 0) & color_select(3));
			END IF;
		END IF;
	END PROCESS;



-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************
	-- Output Assignments
	ready <= 
		valid AND NOT startofpacket WHEN (s_mode = STATE_0_SYNC_FRAME) ELSE 
`ifdef USE_TRDB_LCM
		read_enable AND color_select(3);
`else
		read_enable;
`endif

	VGA_CLK <= NOT clk;

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************

	VGA_Timing : altera_up_avalon_video_vga_timing 
	GENERIC MAP (
		CW									=> CW,

		H_ACTIVE							=> H_ACTIVE,
		H_FRONT_PORCH					=> H_FRONT_PORCH,
		H_SYNC							=> H_SYNC,
		H_BACK_PORCH					=> H_BACK_PORCH,
		H_TOTAL							=> H_TOTAL,

		V_ACTIVE							=> V_ACTIVE,
		V_FRONT_PORCH					=> V_FRONT_PORCH,
		V_SYNC							=> V_SYNC,
		V_BACK_PORCH					=> V_BACK_PORCH,
		V_TOTAL							=> V_TOTAL,

		LW									=> LW,
		LINE_COUNTER_INCREMENT		=> LINE_COUNTER_INCREMENT,

		PW									=> PW,
		PIXEL_COUNTER_INCREMENT		=> PIXEL_COUNTER_INCREMENT
	)
	PORT MAP (
		-- Inputs
		clk						=> clk,
		reset						=> reset,
	
		red_to_vga_display	=> data(R_UI DOWNTO R_LI),
		green_to_vga_display	=> data(G_UI DOWNTO G_LI),
		blue_to_vga_display	=> data(B_UI DOWNTO B_LI),
		color_select			=> color_select,
	
		-- Bidirectionals
	
		-- Outputs
		read_enable				=> read_enable,
	
		end_of_active_frame	=> end_of_active_frame, -- end_of_frame,
	
		-- dac pins
		vga_blank				=> vga_blank_sync,
		vga_c_sync				=> vga_c_sync,
		vga_h_sync				=> vga_h_sync,
		vga_v_sync				=> vga_v_sync,
		vga_data_enable		=> vga_data_enable,
		vga_red					=> vga_red,
		vga_green				=> vga_green,
		vga_blue					=> vga_blue,
		vga_color_data			=> vga_color_data
	);


END Behaviour;
