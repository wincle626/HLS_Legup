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
-- * This module drives the vga dac on Altera's DE2 Board.                      *
-- *                                                                            *
-- ******************************************************************************

ENTITY altera_up_avalon_video_vga_timing IS 

-- *****************************************************************************
-- *                             Generic Declarations                          *
-- *****************************************************************************
	
GENERIC (
	
	CW									:INTEGER									:= 9;
	
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
	
	PW									:INTEGER									:= 10;	-- Number of bits for pixels
	PIXEL_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001";
	
	LW									:INTEGER									:= 10;	-- Number of bits for lines
	LINE_COUNTER_INCREMENT		:STD_LOGIC_VECTOR( 9 DOWNTO  0)	:= B"0000000001"
	
);
-- *****************************************************************************
-- *                             Port Declarations                             *
-- *****************************************************************************
PORT (
	-- Inputs
	clk						:IN		STD_LOGIC;
	reset						:IN		STD_LOGIC;

	red_to_vga_display	:IN		STD_LOGIC_VECTOR(CW DOWNTO  0);	
	green_to_vga_display	:IN		STD_LOGIC_VECTOR(CW DOWNTO  0);	
	blue_to_vga_display	:IN		STD_LOGIC_VECTOR(CW DOWNTO  0);	
	color_select			:IN		STD_LOGIC_VECTOR( 3 DOWNTO  0);	

	-- Bidirectionals

	-- Outputs
	read_enable				:BUFFER	STD_LOGIC;

	end_of_active_frame	:BUFFER	STD_LOGIC;
	end_of_frame			:BUFFER	STD_LOGIC;

	-- dac pins
	vga_blank				:BUFFER	STD_LOGIC;								--	VGA BLANK
	vga_c_sync				:BUFFER	STD_LOGIC;								--	VGA COMPOSITE SYNC
	vga_h_sync				:BUFFER	STD_LOGIC;								--	VGA H_SYNC
	vga_v_sync				:BUFFER	STD_LOGIC;								--	VGA V_SYNC
	vga_data_enable		:BUFFER	STD_LOGIC;								-- VGA DEN
	vga_red					:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);	--	VGA Red(9 DOWNTO 0)
	vga_green				:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);	--	VGA Green(9 DOWNTO 0)
	vga_blue					:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0);	--	VGA Blue(9 DOWNTO 0)
	vga_color_data			:BUFFER	STD_LOGIC_VECTOR(CW DOWNTO  0)	--	VGA Color(9 DOWNTO 0) for TRDB_LCM

);

END altera_up_avalon_video_vga_timing;

ARCHITECTURE Behaviour OF altera_up_avalon_video_vga_timing IS
-- *****************************************************************************
-- *                           Constant Declarations                           *
-- *****************************************************************************

-- *****************************************************************************
-- *                       Internal Signals Declarations                       *
-- *****************************************************************************
	-- Internal Wires
	
	-- Internal Registers
	SIGNAL	pixel_counter		:STD_LOGIC_VECTOR(PW DOWNTO  1);	
	SIGNAL	line_counter		:STD_LOGIC_VECTOR(LW DOWNTO  1);	
	
	SIGNAL	early_hsync_pulse	:STD_LOGIC;
	SIGNAL	early_vsync_pulse	:STD_LOGIC;
	SIGNAL	hsync_pulse			:STD_LOGIC;
	SIGNAL	vsync_pulse			:STD_LOGIC;
	SIGNAL	csync_pulse			:STD_LOGIC;
	
	SIGNAL	hblanking_pulse	:STD_LOGIC;
	SIGNAL	vblanking_pulse	:STD_LOGIC;
	SIGNAL	blanking_pulse		:STD_LOGIC;
	
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

	-- Output Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				vga_c_sync			<= '1';
				vga_blank			<= '1';
				vga_h_sync			<= '1';
				vga_v_sync			<= '1';
		
				vga_red				<= (OTHERS => '0');
				vga_green			<= (OTHERS => '0');
				vga_blue				<= (OTHERS => '0');
				vga_color_data		<= (OTHERS => '0');
			ELSE
				vga_blank			<= NOT blanking_pulse;
				vga_c_sync			<= NOT csync_pulse;
				vga_h_sync			<= NOT hsync_pulse;
				vga_v_sync			<= NOT vsync_pulse;
				vga_data_enable	<= NOT blanking_pulse;
		
				IF (blanking_pulse = '1') THEN
					vga_red			<= (OTHERS => '0');
					vga_green		<= (OTHERS => '0');
					vga_blue			<= (OTHERS => '0');
					vga_color_data	<= (OTHERS => '0');
				ELSE
					vga_red			<= red_to_vga_display;
					vga_green		<= green_to_vga_display;
					vga_blue			<= blue_to_vga_display;
					IF color_select(0) = '1' THEN
						vga_color_data	<= red_to_vga_display;
					ELSIF color_select(1) = '1' THEN
						vga_color_data	<= green_to_vga_display;
					ELSIF color_select(2) = '1' THEN
						vga_color_data	<= blue_to_vga_display;
					ELSE
						vga_color_data	<= (OTHERS => '0');
					END IF;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	-- Internal Registers
	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				pixel_counter	<= STD_LOGIC_VECTOR(TO_UNSIGNED(H_TOTAL - 3, PW)); -- {PW{1'b0}};
				line_counter	<= STD_LOGIC_VECTOR(TO_UNSIGNED(V_TOTAL - 1, LW)); -- {LW{1'b0}};
			ELSE
				-- last pixel in the line
				IF (pixel_counter = (H_TOTAL - 1)) THEN
					pixel_counter <= (OTHERS => '0');
					
					-- last pixel in last line of frame
					IF (line_counter = (V_TOTAL - 1)) THEN
						line_counter <= (OTHERS => '0');
					-- last pixel but not last line
					ELSE
						line_counter <= line_counter + LINE_COUNTER_INCREMENT;
					END IF;
				ELSE 
					pixel_counter <= pixel_counter + PIXEL_COUNTER_INCREMENT;
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk) 
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				end_of_active_frame <= '0';
				end_of_frame		<= '0';
			ELSE
				IF ((line_counter = (V_ACTIVE - 1)) AND 
					(pixel_counter = (H_ACTIVE - 2))) THEN
					end_of_active_frame <= '1';
				ELSE
					end_of_active_frame <= '0';
				END IF;
			
				IF ((line_counter = (V_TOTAL - 1)) AND 
					(pixel_counter = (H_TOTAL - 2))) THEN
					end_of_frame <= '1';
				ELSE
					end_of_frame <= '0';
				END IF;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk) 
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				early_hsync_pulse <= '0';
				early_vsync_pulse <= '0';
				
				hsync_pulse <= '0';
				vsync_pulse <= '0';
				
				csync_pulse	<= '0';
			ELSE
				-- start of horizontal sync
				IF (pixel_counter = (H_ACTIVE + H_FRONT_PORCH - 2)) THEN
					early_hsync_pulse <= '1';	
				-- end of horizontal sync
				ELSIF (pixel_counter = (H_TOTAL - H_BACK_PORCH - 2)) THEN
					early_hsync_pulse <= '0';	
				END IF;
				
				-- start of vertical sync
				IF ((line_counter = (V_ACTIVE + V_FRONT_PORCH - 1)) AND 
						(pixel_counter = (H_TOTAL - 2))) THEN
					early_vsync_pulse <= '1';
				-- end of vertical sync
				ELSIF ((line_counter = (V_TOTAL - V_BACK_PORCH - 1)) AND 
						(pixel_counter = (H_TOTAL - 2))) THEN
					early_vsync_pulse <= '0';
				END IF;		
			
				hsync_pulse <= early_hsync_pulse;
				vsync_pulse <= early_vsync_pulse;
		
				csync_pulse <= early_hsync_pulse XOR early_vsync_pulse;
			END IF;
		END IF;
	END PROCESS;


	PROCESS (clk) 
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF (reset = '1') THEN
				hblanking_pulse	<= '1';
				vblanking_pulse	<= '1';
				
				blanking_pulse	<= '1';
			ELSE
				IF (pixel_counter = (H_ACTIVE - 2)) THEN
					hblanking_pulse	<= '1';
				ELSIF (pixel_counter = (H_TOTAL - 2)) THEN
					hblanking_pulse	<= '0';
				END IF;
			
				IF ((line_counter = (V_ACTIVE - 1)) AND 
						(pixel_counter = (H_TOTAL - 2))) THEN 
					vblanking_pulse	<= '1';
				ELSIF ((line_counter = (V_TOTAL - 1)) AND 
						(pixel_counter = (H_TOTAL - 2))) THEN
					vblanking_pulse	<= '0';
				END IF;	
			
				blanking_pulse		<= hblanking_pulse OR vblanking_pulse;
			END IF;
		END IF;
	END PROCESS;


-- *****************************************************************************
-- *                            Combinational Logic                            *
-- *****************************************************************************

	-- Output Assignments
	read_enable <= NOT blanking_pulse;

-- *****************************************************************************
-- *                          Component Instantiations                         *
-- *****************************************************************************



END Behaviour;
