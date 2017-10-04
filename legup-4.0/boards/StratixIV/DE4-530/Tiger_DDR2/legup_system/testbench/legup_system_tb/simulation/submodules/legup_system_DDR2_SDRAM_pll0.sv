// (C) 2001-2013 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// ******************************************************************************************************************************** 
// This file instantiates the PLL.
// ******************************************************************************************************************************** 

`timescale 1 ps / 1 ps

(* altera_attribute = "-name IP_TOOL_NAME common; -name IP_TOOL_VERSION 13.1; -name FITTER_ADJUST_HC_SHORT_PATH_GUARDBAND 100; -name ALLOW_SYNCH_CTRL_USAGE OFF; -name AUTO_CLOCK_ENABLE_RECOGNITION OFF; -name AUTO_SHIFT_REGISTER_RECOGNITION OFF" *)


module legup_system_DDR2_SDRAM_pll0 (
	global_reset_n,
	pll_ref_clk,
	pll_mem_clk,
	pll_write_clk,
	pll_write_clk_pre_phy_clk,
	pll_addr_cmd_clk,
	pll_avl_clk,
	pll_config_clk,
	pll_locked,
	afi_clk,
	afi_half_clk
);


// ******************************************************************************************************************************** 
// BEGIN PARAMETER SECTION
// All parameters default to "" will have their values passed in from higher level wrapper with the controller and driver. 
parameter DEVICE_FAMILY = "Stratix IV";

// choose between abstract (fast) and regular model
`ifndef ALTERA_ALT_MEM_IF_PHY_FAST_SIM_MODEL
  `define ALTERA_ALT_MEM_IF_PHY_FAST_SIM_MODEL 1
`endif

parameter ALTERA_ALT_MEM_IF_PHY_FAST_SIM_MODEL = `ALTERA_ALT_MEM_IF_PHY_FAST_SIM_MODEL;

localparam FAST_SIM_MODEL = ALTERA_ALT_MEM_IF_PHY_FAST_SIM_MODEL;

// The PLL Phase counter width
parameter PLL_PHASE_COUNTER_WIDTH = 4;


// Clock settings
parameter GENERIC_PLL = "false";
parameter REF_CLK_FREQ = "50.0 MHz";
parameter REF_CLK_PERIOD_PS = 20000;

parameter PLL_AFI_CLK_FREQ_STR = "200.0 MHz";
parameter PLL_MEM_CLK_FREQ_STR = "400.0 MHz";
parameter PLL_WRITE_CLK_FREQ_STR = "400.0 MHz";
parameter PLL_ADDR_CMD_CLK_FREQ_STR = "200.0 MHz";
parameter PLL_AFI_HALF_CLK_FREQ_STR = "100.0 MHz";
parameter PLL_NIOS_CLK_FREQ_STR = "100.0 MHz";
parameter PLL_CONFIG_CLK_FREQ_STR = "25.0 MHz";
parameter PLL_P2C_READ_CLK_FREQ_STR = "";
parameter PLL_C2P_WRITE_CLK_FREQ_STR = "";
parameter PLL_HR_CLK_FREQ_STR = "";
parameter PLL_DR_CLK_FREQ_STR = "";

parameter PLL_AFI_CLK_FREQ_SIM_STR = "5000 ps";
parameter PLL_MEM_CLK_FREQ_SIM_STR = "2500 ps";
parameter PLL_WRITE_CLK_FREQ_SIM_STR = "2500 ps";
parameter PLL_ADDR_CMD_CLK_FREQ_SIM_STR = "5000 ps";
parameter PLL_AFI_HALF_CLK_FREQ_SIM_STR = "10000 ps";
parameter PLL_NIOS_CLK_FREQ_SIM_STR = "10000 ps";
parameter PLL_CONFIG_CLK_FREQ_SIM_STR = "40000 ps";
parameter PLL_P2C_READ_CLK_FREQ_SIM_STR = "0 ps";
parameter PLL_C2P_WRITE_CLK_FREQ_SIM_STR = "0 ps";
parameter PLL_HR_CLK_FREQ_SIM_STR = "0 ps";
parameter PLL_DR_CLK_FREQ_SIM_STR = "0 ps";

parameter AFI_CLK_PHASE      = "0 ps";
parameter MEM_CLK_PHASE      = "0 ps";
parameter WRITE_CLK_PHASE    = "625 ps";
parameter ADDR_CMD_CLK_PHASE = "3750 ps";
parameter AFI_HALF_CLK_PHASE = "0 ps";
parameter AVL_CLK_PHASE      = "0 ps";
parameter CONFIG_CLK_PHASE   = "0 ps";



parameter ABSTRACT_REAL_COMPARE_TEST = "false";

localparam SIM_FILESET = ("true" == "true");

localparam AFI_CLK_FREQ       = SIM_FILESET ? PLL_AFI_CLK_FREQ_SIM_STR : PLL_AFI_CLK_FREQ_STR;
localparam MEM_CLK_FREQ       = SIM_FILESET ? PLL_MEM_CLK_FREQ_SIM_STR : PLL_MEM_CLK_FREQ_STR;
localparam WRITE_CLK_FREQ     = SIM_FILESET ? PLL_WRITE_CLK_FREQ_SIM_STR : PLL_WRITE_CLK_FREQ_STR;
localparam ADDR_CMD_CLK_FREQ  = SIM_FILESET ? PLL_ADDR_CMD_CLK_FREQ_SIM_STR : PLL_ADDR_CMD_CLK_FREQ_STR;
localparam AFI_HALF_CLK_FREQ  = SIM_FILESET ? PLL_AFI_HALF_CLK_FREQ_SIM_STR : PLL_AFI_HALF_CLK_FREQ_STR;
localparam AVL_CLK_FREQ       = SIM_FILESET ? PLL_NIOS_CLK_FREQ_SIM_STR : PLL_NIOS_CLK_FREQ_STR;
localparam CONFIG_CLK_FREQ    = SIM_FILESET ? PLL_CONFIG_CLK_FREQ_SIM_STR : PLL_CONFIG_CLK_FREQ_STR;
localparam P2C_READ_CLK_FREQ  = SIM_FILESET ? PLL_P2C_READ_CLK_FREQ_SIM_STR : PLL_P2C_READ_CLK_FREQ_STR;
localparam C2P_WRITE_CLK_FREQ = SIM_FILESET ? PLL_C2P_WRITE_CLK_FREQ_SIM_STR : PLL_C2P_WRITE_CLK_FREQ_STR;
localparam HR_CLK_FREQ        = SIM_FILESET ? PLL_HR_CLK_FREQ_SIM_STR : PLL_HR_CLK_FREQ_STR;
localparam DR_CLK_FREQ        = SIM_FILESET ? PLL_DR_CLK_FREQ_SIM_STR : PLL_DR_CLK_FREQ_STR;

parameter PLL_AFI_CLK_DIV           = 1;
parameter PLL_MEM_CLK_DIV           = 1;
parameter PLL_WRITE_CLK_DIV         = 1;
parameter PLL_ADDR_CMD_CLK_DIV      = 1;
parameter PLL_AFI_HALF_CLK_DIV      = 1;
parameter PLL_NIOS_CLK_DIV          = 1;
parameter PLL_CONFIG_CLK_DIV        = 2;

parameter PLL_AFI_CLK_MULT          = 4;
parameter PLL_MEM_CLK_MULT          = 8;
parameter PLL_WRITE_CLK_MULT        = 8;
parameter PLL_ADDR_CMD_CLK_MULT     = 4;
parameter PLL_AFI_HALF_CLK_MULT     = 2;
parameter PLL_NIOS_CLK_MULT         = 2;
parameter PLL_CONFIG_CLK_MULT       = 1;

parameter PLL_AFI_CLK_PHASE_PS      = "0";
parameter PLL_MEM_CLK_PHASE_PS      = "0";
parameter PLL_WRITE_CLK_PHASE_PS    = "625";
parameter PLL_ADDR_CMD_CLK_PHASE_PS = "3750";
parameter PLL_AFI_HALF_CLK_PHASE_PS = "0";
parameter PLL_NIOS_CLK_PHASE_PS     = "0";
parameter PLL_CONFIG_CLK_PHASE_PS   = "0";

// END PARAMETER SECTION
// ******************************************************************************************************************************** 


// ******************************************************************************************************************************** 
// BEGIN PORT SECTION


input	pll_ref_clk;		// PLL reference clock

// When the PHY is selected to be a PLL/DLL MASTER, the PLL and DLL are instantied on this top level
wire	pll_afi_clk;		// See pll_memphy instantiation below for detailed description of each clock

output	pll_mem_clk;
output	pll_write_clk;
output	pll_write_clk_pre_phy_clk;
output	pll_addr_cmd_clk;
output	pll_avl_clk;
output	pll_config_clk;
output	pll_locked;    // When 0, PLL is out of lock
                       // should be used to reset system level afi_clk domain logic



// Reset Interface, AFI 2.0
input   global_reset_n;		// Resets (active-low) the whole system (all PHY logic + PLL)




// PLL Interface
output	afi_clk;
output	afi_half_clk;


wire	pll_mem_clk_pre_phy_clk;



// END PARAMETER SECTION
// ******************************************************************************************************************************** 

initial $display("Using %0s pll emif simulation models", FAST_SIM_MODEL ? "Fast" : "Regular");





localparam NUM_PLL = 10; 
				

generate
if (FAST_SIM_MODEL)
begin


`ifndef SIMGEN
	// synthesis translate_off
`endif
	
	wire fbout;


	wire [NUM_PLL-1:0] pll_clks;
	

	altera_pll #(
	      .reference_clock_frequency(REF_CLK_FREQ),
	      .sim_additional_refclk_cycles_to_lock(4), 
	      .number_of_clocks(NUM_PLL),
	      .output_clock_frequency0(AFI_CLK_FREQ),
	      .phase_shift0(AFI_CLK_PHASE),
	      .duty_cycle0(50),
	      .output_clock_frequency1(MEM_CLK_FREQ),
	      .phase_shift1(MEM_CLK_PHASE),
	      .duty_cycle1(50),
	      .output_clock_frequency2(WRITE_CLK_FREQ),
	      .phase_shift2(WRITE_CLK_PHASE),
	      .duty_cycle2(50),
	      .output_clock_frequency3(ADDR_CMD_CLK_FREQ),
	      .phase_shift3(ADDR_CMD_CLK_PHASE),
	      .duty_cycle3(50),
	      .output_clock_frequency4(AFI_HALF_CLK_FREQ),
	      .phase_shift4(AFI_HALF_CLK_PHASE),
	      .duty_cycle4(50),
	      .output_clock_frequency5(AVL_CLK_FREQ),
	      .phase_shift5(AVL_CLK_PHASE),
	      .duty_cycle5(50),
	      .output_clock_frequency6(CONFIG_CLK_FREQ),
	      .phase_shift6(CONFIG_CLK_PHASE),
	      .duty_cycle6(50),
	      .output_clock_frequency7(AFI_CLK_FREQ),
	      .phase_shift7("0 ps"),
	      .duty_cycle7(50),
	      .output_clock_frequency8(AFI_CLK_FREQ),
	      .phase_shift8("0 ps"),
	      .duty_cycle8(50),
	      .output_clock_frequency9(AFI_CLK_FREQ),
	      .phase_shift9("0 ps"),
	      .duty_cycle9(50)
		     ) pll_inst (
		.refclk({pll_ref_clk}),
		.rst(~global_reset_n),
		.fbclk(fbout),
		.outclk(pll_clks),
		.fboutclk(fbout),
		.locked(pll_locked)
	);

	wire delayed_pll_locked_pre;
	wire delayed_pll_locked;
`ifndef SIMGEN
	assign #1 delayed_pll_locked_pre = pll_locked;
`else
	legup_system_DDR2_SDRAM_pll0_sim_delay #(.delay(1)) sim_delay_inst(.o(delayed_pll_locked_pre), .i(pll_locked));
`endif

	reg default_pll_clk_value = 1'b0;
	
	initial
	begin
		repeat (6) @(negedge pll_ref_clk);
		default_pll_clk_value = 1'bx;
		repeat (2) @(negedge pll_ref_clk);
		default_pll_clk_value = 1'b1;
	end

	
	assign delayed_pll_locked = delayed_pll_locked_pre === 1'b1 ? 1'b1 : 1'b0;

	assign pll_afi_clk = delayed_pll_locked ? pll_clks[0] : default_pll_clk_value;
	assign pll_mem_clk_pre_phy_clk = delayed_pll_locked ? pll_clks[1] : default_pll_clk_value;
	assign pll_write_clk_pre_phy_clk = delayed_pll_locked ? pll_clks[2] : default_pll_clk_value;
	assign pll_addr_cmd_clk = delayed_pll_locked ? pll_clks[3] : default_pll_clk_value;
		
	assign pll_avl_clk = delayed_pll_locked ? pll_clks[5] : default_pll_clk_value;
	assign pll_config_clk = delayed_pll_locked ? pll_clks[6] : default_pll_clk_value;
		
		


`ifndef SIMGEN	
	assign pll_write_clk = pll_write_clk_pre_phy_clk;
	assign pll_mem_clk = pll_mem_clk_pre_phy_clk;
`else 
	assign pll_mem_clk = pll_mem_clk_pre_phy_clk;
	assign pll_write_clk = pll_write_clk_pre_phy_clk;
`endif 

`ifndef SIMGEN
	// synthesis translate_on
`endif
	
end
else 
begin


	wire [NUM_PLL-1:0] pll_clks;
	wire [1:0] inclk;

	`ifndef ALTERA_RESERVED_QIS
	//synopsys translate_off
	`endif
	tri0	  areset;
	`ifndef ALTERA_RESERVED_QIS
	//synopsys translate_on
	`endif

	assign areset = ~global_reset_n;
	assign inclk = {1'b0, pll_ref_clk};

	altpll	upll_memphy (
				.areset (areset),
				.inclk (inclk),
				.clk (pll_clks),
				.locked (pll_locked),
				.activeclock (),
				.clkbad (),
				.clkena ({6{1'b1}}),
				.clkloss (),
				.clkswitch (1'b0),
				.enable0 (),
				.enable1 (),	
				.extclk (),
				.extclkena ({PLL_PHASE_COUNTER_WIDTH{1'b1}}),
				.fbin (1'b1),
				.fbmimicbidir (),
				.fbout (),
				.fref (),
				.icdrclk (),
				.pfdena (1'b1),
				.pllena (1'b1),
				.scanaclr (1'b0),
				.scanread (1'b0),
				.scanwrite (1'b0),
				.configupdate(1'b0),
				.scandata(1'b0),
				.scandataout(),
				.scandone(),
				.phasecounterselect({PLL_PHASE_COUNTER_WIDTH{1'b1}}),
				.phasedone(),
				.phasestep(1'b1),
				.phaseupdown(1'b1),
				.scanclk(1'b0),
				.scanclkena(1'b1),
				.sclkout0 (),
				.sclkout1 (),
				.vcooverrange (),
				.vcounderrange ()
		);
	defparam upll_memphy.bandwidth_type = "AUTO";
	defparam upll_memphy.clk0_divide_by = PLL_AFI_CLK_DIV;
	defparam upll_memphy.clk0_duty_cycle = 50;
	defparam upll_memphy.clk0_multiply_by = PLL_AFI_CLK_MULT;
	defparam upll_memphy.clk0_phase_shift = PLL_AFI_CLK_PHASE_PS;
	defparam upll_memphy.clk1_divide_by = PLL_MEM_CLK_DIV;
	defparam upll_memphy.clk1_duty_cycle = 50;
	defparam upll_memphy.clk1_multiply_by = PLL_MEM_CLK_MULT;
	defparam upll_memphy.clk1_phase_shift = PLL_MEM_CLK_PHASE_PS;
	defparam upll_memphy.clk2_divide_by = PLL_WRITE_CLK_DIV;
	defparam upll_memphy.clk2_duty_cycle = 50;
	defparam upll_memphy.clk2_multiply_by = PLL_WRITE_CLK_MULT;
	defparam upll_memphy.clk2_phase_shift = PLL_WRITE_CLK_PHASE_PS;
	defparam upll_memphy.clk3_divide_by = PLL_ADDR_CMD_CLK_DIV;
	defparam upll_memphy.clk3_duty_cycle = 50;
	defparam upll_memphy.clk3_multiply_by = PLL_ADDR_CMD_CLK_MULT;
	defparam upll_memphy.clk3_phase_shift = PLL_ADDR_CMD_CLK_PHASE_PS;
	defparam upll_memphy.clk5_divide_by = PLL_NIOS_CLK_DIV;
	defparam upll_memphy.clk5_duty_cycle = 50;
	defparam upll_memphy.clk5_multiply_by = PLL_NIOS_CLK_MULT;
	defparam upll_memphy.clk5_phase_shift = PLL_NIOS_CLK_PHASE_PS;
	defparam upll_memphy.clk6_divide_by = PLL_CONFIG_CLK_DIV;
	defparam upll_memphy.clk6_duty_cycle = 50;
	defparam upll_memphy.clk6_multiply_by = PLL_CONFIG_CLK_MULT;
	defparam upll_memphy.clk6_phase_shift = PLL_CONFIG_CLK_PHASE_PS;
	defparam upll_memphy.inclk0_input_frequency = REF_CLK_PERIOD_PS;
	defparam upll_memphy.intended_device_family = DEVICE_FAMILY;
	defparam upll_memphy.lpm_type = "altpll";
	defparam upll_memphy.operation_mode = "NO_COMPENSATION";
	defparam upll_memphy.pll_type = "AUTO";
	defparam upll_memphy.port_activeclock = "PORT_UNUSED";
	defparam upll_memphy.port_areset = "PORT_USED";
	defparam upll_memphy.port_clkbad0 = "PORT_UNUSED";
	defparam upll_memphy.port_clkbad1 = "PORT_UNUSED";
	defparam upll_memphy.port_clkloss = "PORT_UNUSED";
	defparam upll_memphy.port_clkswitch = "PORT_UNUSED";
	defparam upll_memphy.port_fbin = "PORT_UNUSED";
	defparam upll_memphy.port_fbout = "PORT_UNUSED";
	defparam upll_memphy.port_inclk0 = "PORT_USED";
	defparam upll_memphy.port_inclk1 = "PORT_UNUSED";
	defparam upll_memphy.port_locked = "PORT_USED";
	defparam upll_memphy.port_pfdena = "PORT_UNUSED";
	defparam upll_memphy.port_pllena = "PORT_UNUSED";
	defparam upll_memphy.port_configupdate = "PORT_UNUSED";
	defparam upll_memphy.port_scandata = "PORT_UNUSED";
	defparam upll_memphy.port_scandataout = "PORT_UNUSED";
	defparam upll_memphy.port_scandone = "PORT_UNUSED";
	defparam upll_memphy.port_phasecounterselect = "PORT_UNUSED";
	defparam upll_memphy.port_phasedone = "PORT_UNUSED";
	defparam upll_memphy.port_phasestep = "PORT_UNUSED";
	defparam upll_memphy.port_phaseupdown = "PORT_UNUSED";
	defparam upll_memphy.port_scanclk = "PORT_UNUSED";
	defparam upll_memphy.port_scanclkena = "PORT_UNUSED";
	defparam upll_memphy.port_scanaclr = "PORT_UNUSED";
	defparam upll_memphy.port_scanread = "PORT_UNUSED";
	defparam upll_memphy.port_scanwrite = "PORT_UNUSED";
	defparam upll_memphy.port_clk0 = "PORT_USED";
	defparam upll_memphy.port_clk1 = "PORT_USED";
	defparam upll_memphy.port_clk2 = "PORT_USED";
	defparam upll_memphy.port_clk3 = "PORT_USED";
	defparam upll_memphy.port_clk4 = "PORT_USED";
	defparam upll_memphy.port_clk5 = "PORT_USED";
	defparam upll_memphy.port_clk6 = "PORT_USED";
	defparam upll_memphy.port_clk7 = "PORT_UNUSED";
    defparam upll_memphy.port_clk8 = "PORT_UNUSED";
	defparam upll_memphy.port_clk9 = "PORT_UNUSED";
	defparam upll_memphy.port_clkena0 = "PORT_UNUSED";
	defparam upll_memphy.port_clkena1 = "PORT_UNUSED";
	defparam upll_memphy.port_clkena2 = "PORT_UNUSED";
	defparam upll_memphy.port_clkena3 = "PORT_UNUSED";
	defparam upll_memphy.port_clkena4 = "PORT_UNUSED";
	defparam upll_memphy.port_clkena5 = "PORT_UNUSED";
	defparam upll_memphy.self_reset_on_loss_lock = "OFF";
	defparam upll_memphy.using_fbmimicbidir_port = "OFF";
	defparam upll_memphy.width_clock = 10;

	assign pll_afi_clk = pll_clks[0];
	assign pll_mem_clk_pre_phy_clk = pll_clks[1];
	assign pll_write_clk_pre_phy_clk = pll_clks[2];
	assign pll_addr_cmd_clk = pll_clks[3];
	assign pll_avl_clk = pll_clks[5];
	assign pll_config_clk = pll_clks[6];
	
	assign pll_mem_clk = pll_mem_clk_pre_phy_clk;	
	assign pll_write_clk = pll_write_clk_pre_phy_clk;	
end
endgenerate


	// Clock descriptions
	// pll_afi_clk: half-rate clock, 0 degree phase shift, clock for AFI interface logic
	// pll_mem_clk: full-rate clock, 0 degree phase shift, clock output to memory
	// pll_write_clk: full-rate clock, -90 degree phase shift, clocks write data out to memory
	// pll_addr_cmd_clk: half-rate clock, 270 degree phase shift, clocks address/command out to memory
	// pll_afi_half_clk: quad-rate clock, 0 degree phase shift
	// the purpose of these clock settings is so that address/command/write data are centred aligned with the output clock(s) to memory 

	assign afi_clk = pll_afi_clk;

	assign afi_half_clk = 1'b0;


endmodule

