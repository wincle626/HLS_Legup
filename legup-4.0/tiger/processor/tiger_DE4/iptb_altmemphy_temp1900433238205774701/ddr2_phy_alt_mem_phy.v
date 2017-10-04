//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
(* altera_attribute = "-name MESSAGE_DISABLE 14130", altera_attribute = "-name SYNCHRONIZER_IDENTIFICATION OFF" *) module ddr2_phy_alt_mem_phy (
                        //Clock and reset inputs:
                        pll_ref_clk,
                        global_reset_n,
                        soft_reset_n,

                        // Used to indicate PLL loss of lock for system reset management
                        reset_request_n,

                        // Clock and reset for the controller interface
                        ctl_clk,
                        ctl_reset_n,

                        // Write data interface
                        ctl_dqs_burst,
                        ctl_wdata_valid,
                        ctl_wdata,
                        ctl_dm,
                        ctl_wlat,

                        // Address and command interface:
                        ctl_addr,
                        ctl_ba,
                        ctl_cas_n,
                        ctl_cke,
                        ctl_cs_n,
                        ctl_odt,
                        ctl_ras_n,
                        ctl_we_n,
                        ctl_rst_n,
                        ctl_mem_clk_disable,

                        // Read data interface:
                        ctl_doing_rd,
                        ctl_rdata,
                        ctl_rdata_valid,
                        ctl_rlat,

                        //re-calibration request & configuration
                        ctl_cal_req,
                        ctl_cal_byte_lane_sel_n,

                        //Calibration status interface:
                        ctl_cal_success,
                        ctl_cal_fail,
                        ctl_cal_warning,

                        //Parity signals for registered DIMMs
                        mem_ac_parity,
                        mem_err_out_n,
                        parity_error_n,

                        //ports to memory device(s):
                        mem_addr,
                        mem_ba,
                        mem_cas_n,
                        mem_cke,
                        mem_cs_n,
                        mem_dm,
                        mem_odt,
                        mem_ras_n,
                        mem_we_n,
                        mem_clk,
                        mem_clk_n,
                        mem_reset_n,

                        // Bidirectional Memory interface signals:
                        mem_dq,
                        mem_dqs,
                        mem_dqs_n,

                        // On Chip Termination: -  dynamically updated values.
                        oct_ctl_rs_value,
                        oct_ctl_rt_value,

                        // DLL import/export ports
                        dqs_offset_delay_ctrl,
                        dqs_delay_ctrl_import,
                        dqs_delay_ctrl_export,

                        dll_reference_clk,

                        // Auxiliary clocks. Some systems may need these for debugging
                        // purposes, or for full-rate to half-rate bridge interfaces
                        aux_half_rate_clk,
                        aux_full_rate_clk,
                        aux_scan_clk,             // New for 9.0 for PLL/DLL re-config.
                        aux_scan_clk_reset_n,     // New for 9.0 for PLL/DLL re-config.

                        // Debug interface:- ALTERA USE ONLY
                        dbg_clk,
                        dbg_reset_n,
                        dbg_addr,
                        dbg_wr,
                        dbg_rd,
                        dbg_cs,
                        dbg_wr_data,
                        dbg_rd_data,
                        dbg_waitrequest,

			// PLL reconfig interfaces - new for 9.0
                        pll_reconfig_enable,
                        pll_phasecounterselect,
                        pll_phaseupdown,
                        pll_phasestep,

                        pll_phase_done,

			// Externalisation of levelled PHY's scan chain controls
			hc_scan_enable_access,
			hc_scan_enable_dq,
			hc_scan_enable_dm,
			hc_scan_enable_dqs,
			hc_scan_enable_dqs_config,
			hc_scan_ck,
			hc_scan_din,
			hc_scan_dout,
			hc_scan_update
                      );


// Default parameter values :

parameter FAMILY                          =    "Stratix III";
parameter SPEED_GRADE                     =             "C3";
parameter MEM_IF_MEMTYPE                  =           "DDR3";

parameter DLL_EXPORT_IMPORT               =           "NONE";
parameter DLL_DELAY_BUFFER_MODE           =           "HIGH";
parameter DLL_DELAY_CHAIN_LENGTH          =                8;
parameter DQS_DELAY_CTL_WIDTH             =                6;
parameter DQS_OUT_MODE                    =   "DELAY_CHAIN2";
parameter DQS_PHASE                       =             9000;
parameter DQS_PHASE_SETTING               =                2;
parameter DWIDTH_RATIO                    =                4;
parameter MEM_IF_DWIDTH                   =               64;
parameter MEM_IF_DQSN_EN                  =                1;
parameter MEM_IF_ADDR_CMD_PHASE           =            "PLL";
parameter MEM_IF_RANKS_PER_SLOT           =                1; // how ranks are arranged into slot - needed for odt setting in the sequencer
parameter MEM_IF_ADDR_WIDTH               =               13;
parameter MEM_IF_BANKADDR_WIDTH           =                3;
parameter MEM_IF_CS_WIDTH                 =                2;
parameter MEM_IF_DM_WIDTH                 =                8;
parameter MEM_IF_DM_PINS_EN               =                1;
parameter MEM_IF_DQ_PER_DQS               =                8;
parameter MEM_IF_DQS_WIDTH                =                8;
parameter MEM_IF_OCT_EN                   =                0;
parameter MEM_IF_CLK_PAIR_COUNT           =                3;
parameter MEM_IF_CLK_PS                   =             4000;
parameter MEM_IF_CLK_PS_STR               =        "4000 ps";
parameter MEM_IF_MR_0                     =                0;
parameter MEM_IF_MR_1                     =                0;
parameter MEM_IF_MR_2                     =                0;
parameter MEM_IF_MR_3                     =                0;
parameter MEM_IF_PRESET_RLAT              =                0;
parameter PLL_STEPS_PER_CYCLE             =               24;
parameter SCAN_CLK_DIVIDE_BY              =                4;
parameter REDUCE_SIM_TIME             	  =                1;
parameter FORCE_HC                        =                0;
parameter CAPABILITIES                	  =                0;
parameter WRITE_DESKEW_T10                =                0;
parameter WRITE_DESKEW_T9NI               =                0;
parameter WRITE_DESKEW_T9I                =                0;
parameter WRITE_DESKEW_RANGE              =                0;
parameter TINIT_TCK                   	  =            40000;
parameter TINIT_RST                   	  =           100000;
parameter DBG_A_WIDTH                     =               13;
parameter MEM_IF_CS_PER_RANK              =                1;    // duplicates CS, CKE, ODT, sequencer still controls 1 rank, but it is subdivided from controller perspective.
parameter MEM_IF_RDV_PER_CHIP             =                0;   // multiple chips, and which gives valid data...
parameter GENERATE_ADDITIONAL_DBG_RTL     =                0;   // Non-Levelling sequencer specific
parameter SEQ_STRING_ID                   =       "seq_name";

parameter LEVELLING                       =                1;
parameter READ_DESKEW_MODE                =           "NONE";
parameter WRITE_DESKEW_MODE               =           "NONE";
parameter PLL_RECONFIG_PORTS_EN           =                0;

parameter RANK_HAS_ADDR_SWAP              =                0;

parameter INVERT_POSTAMBLE_CLK            =          "false";

// In full-rate modes, some addr/cmd phases require an inverted phy_clk here to make the transfer
// successfully.  If core to addr/cmd clock timing is failing, then it may be that an incorrect
// default for this parameter was selected by the GUI.

// Tests have shown that for addr/cmd phases<270 degrees it should be inverted, otherwise it
// should not but this does vary dependant on frequency and loading, so for middling phases
// from 240 to 270 this parameter may need inverting.
parameter INVERT_ADDR_CMD_TXFR_CLK        =          "false";

// Params for Registered DIMMs
parameter CHIP_OR_DIMM                    = "Unbuffered DIMM";
parameter RDIMM_CONFIG_BITS               = "0000000000000000000000000000000000000000000000000000000000000000";

// Localparams :
localparam ENABLE_DDR3_SEQUENCER          =           "TRUE";
localparam MEM_IF_DQS_CAPTURE_EN          =                1;
localparam ADV_LAT_WIDTH                  =                5;
localparam CAPTURE_MIMIC_PATH             =                0;
localparam DDR_MIMIC_PATH_EN              =                1;
localparam MIMIC_DEBUG_EN                 =                0;
localparam NUM_MIMIC_SAMPLE_CYCLES        =                6;
localparam NUM_DEBUG_SAMPLES_TO_STORE     =             4096;
localparam ASYNCHRONOUS_AVALON_CLOCK      =                1;
localparam POSTAMBLE_AWIDTH               =                6;
localparam MEM_TCL                        =            "1.5";
localparam POSTAMBLE_INITIAL_LAT          =               13;
localparam RDV_INITIAL_LAT                =               23;
localparam RDP_INITIAL_LAT                = ((DWIDTH_RATIO == 2) && (MEM_IF_MEMTYPE == "DDR2")) ? 4:6;
localparam RESYNC_PIPELINE_DEPTH          =                0;
localparam CLOCK_INDEX_WIDTH              =                4;
localparam OCT_LAT_WIDTH                  =    ADV_LAT_WIDTH;
localparam USE_DLL_OFFSET                 =          "false";
localparam MEM_IF_NUM_RANKS               =  MEM_IF_CS_WIDTH / MEM_IF_CS_PER_RANK;

localparam OCT_RTT_LAUNCH_TIME_DELAY      =                0;
localparam OCT_RTT_EXTENSION              =                0;
localparam PER_GROUP_HR_RSC               =                0;

// I/O Signal definitions :

// Clock and reset I/O :
input  wire                                                    pll_ref_clk;
input  wire                                                    global_reset_n;
input  wire                                                    soft_reset_n;

// This is the PLL locked signal :
output wire                                                    reset_request_n;

// The controller must use this phy_clk to interface to the PHY.  It is
// optional as to whether the remainder of the system uses it :
output wire                                                    ctl_clk;
output wire                                                    ctl_reset_n;

// new AFI I/Os -  write data i/f:
input  wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 -1 : 0]         ctl_dqs_burst;
input  wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 -1 : 0]         ctl_wdata_valid;
input  wire [MEM_IF_DWIDTH * DWIDTH_RATIO      -1 : 0]         ctl_wdata;
input  wire [MEM_IF_DM_WIDTH * DWIDTH_RATIO    -1 : 0]         ctl_dm;
output wire [4 : 0]                                            ctl_wlat;

// new AFI I/Os - addr/cmd i/f:
input  wire [MEM_IF_ADDR_WIDTH  * DWIDTH_RATIO/2 -1 : 0]       ctl_addr;
input  wire [MEM_IF_BANKADDR_WIDTH * DWIDTH_RATIO/2 -1 : 0]    ctl_ba;
input  wire [1 * DWIDTH_RATIO/2 -1 : 0]                        ctl_cas_n;
input  wire [MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1:0]          ctl_cke;
input  wire [MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1:0]          ctl_cs_n;
input  wire [MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1:0]          ctl_odt;
input  wire [1 * DWIDTH_RATIO/2 -1 : 0]                        ctl_ras_n;
input  wire [1 * DWIDTH_RATIO/2 -1 : 0]                        ctl_we_n;
input  wire [DWIDTH_RATIO/2 - 1 : 0]                           ctl_rst_n;

// new AFI I/Os - read data i/f:
input  wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO / 2 -1 : 0]       ctl_doing_rd;
output wire [MEM_IF_DWIDTH * DWIDTH_RATIO      -1 : 0]         ctl_rdata;
output wire [DWIDTH_RATIO / 2 -1 : 0]                          ctl_rdata_valid;
output wire [4 : 0]                                            ctl_rlat;

// new AFI I/Os - update interface:
input  wire                                                    ctl_cal_req;

// new AFI I/Os - status interface:
input  wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                    ctl_mem_clk_disable;
output wire                                                    ctl_cal_success;
output wire                                                    ctl_cal_fail;
output wire                                                    ctl_cal_warning;

// Parity signals for RDIMMs
output wire                                                    mem_ac_parity;
input wire                                                     mem_err_out_n;
output wire                                                    parity_error_n;

// PHY configuration:
input  wire [MEM_IF_DQS_WIDTH * MEM_IF_NUM_RANKS - 1 : 0]       ctl_cal_byte_lane_sel_n;

//Inputs from DIMM:

//Outputs to DIMM :
output wire [MEM_IF_ADDR_WIDTH - 1 : 0]                        mem_addr;
output wire [MEM_IF_BANKADDR_WIDTH - 1 : 0]                    mem_ba;
output wire                                                    mem_cas_n;
output wire [MEM_IF_NUM_RANKS - 1 : 0]                          mem_cke;
output wire [MEM_IF_CS_WIDTH - 1 : 0]                          mem_cs_n;
wire        [MEM_IF_DWIDTH - 1 : 0]                            mem_d;
output wire [MEM_IF_DM_WIDTH - 1 : 0]                          mem_dm;
output wire [MEM_IF_NUM_RANKS - 1 : 0]                          mem_odt;
output wire                                                    mem_ras_n;
output wire                                                    mem_we_n;
output wire                                                    mem_reset_n;

//The mem_clks are outputs, but one is sometimes used for the mimic_path, so
//is looped back in.  Therefore defining as an inout ensures no errors in Quartus :
inout  wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                    mem_clk;
inout  wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                    mem_clk_n;

//Bidirectional:
inout  tri  [MEM_IF_DWIDTH - 1 : 0]                            mem_dq;
inout  tri  [MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS - 1 : 0]        mem_dqs;
inout  tri  [MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS - 1 : 0]        mem_dqs_n;


input  wire [`OCT_SERIES_TERM_CONTROL_WIDTH   -1 : 0]          oct_ctl_rs_value;
input  wire [`OCT_PARALLEL_TERM_CONTROL_WIDTH -1 : 0]          oct_ctl_rt_value;

input  wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]	                   dqs_offset_delay_ctrl; // New for 9.0, tie to zero if not required.
input  wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]                     dqs_delay_ctrl_import;
output wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]                     dqs_delay_ctrl_export;

output wire                                                    dll_reference_clk;

// Auxillary clocks. These do not have to be connected if the system
// doesn't require them :
output wire                                                    aux_half_rate_clk;
output wire                                                    aux_full_rate_clk;

// The scan clock is provided as a convenient clock for the user logic which
// drives either the DLL or PLL reprogramming interfaces
output wire                                                    aux_scan_clk;   // New for 9.0
output wire         	                                       aux_scan_clk_reset_n;      // New for 9.0

// AVALON MM Slave   -- debug IF
input  wire                                                    dbg_clk;
input  wire                                                    dbg_reset_n;
input  wire [DBG_A_WIDTH -1 : 0]                               dbg_addr;
input  wire                                                    dbg_wr;
input  wire                                                    dbg_rd;
input  wire                                                    dbg_cs;
input  wire [31 : 0]                                           dbg_wr_data;
output wire [31 : 0]                                           dbg_rd_data;
output wire                                                    dbg_waitrequest;

// PLL reconfig interface.  Made available primarily for HardCopy customers :
input  wire         	                                       pll_reconfig_enable;       // New for 9.0 - tie low if not required.
input  wire [3:0]        	                               pll_phasecounterselect;    // New for 9.0 - tie low if not required.
input  wire         	                                       pll_phaseupdown;           // New for 9.0 - tie low if not required.
input  wire         	                                       pll_phasestep;             // New for 9.0 - tie low if not required.

output wire         	                                       pll_phase_done;            // New for 9.0

// Size the interface based on DWDTH, as this will always be the largest :
input  wire                                                    hc_scan_enable_access;     // New for 9.1 - tie low if not required.
input  wire [MEM_IF_DWIDTH - 1     : 0]                        hc_scan_enable_dq;         // New for 9.1 - tie low if not required.
input  wire [MEM_IF_DM_WIDTH - 1   : 0]                        hc_scan_enable_dm;         // New for 9.1 - tie low if not required.
input  wire [MEM_IF_DQS_WIDTH - 1  : 0]                        hc_scan_enable_dqs;        // New for 9.1 - tie low if not required.
input  wire [MEM_IF_DQS_WIDTH - 1  : 0]                        hc_scan_enable_dqs_config; // New for 9.1 - tie low if not required.
input  wire [MEM_IF_DQS_WIDTH - 1  : 0]                        hc_scan_din;               // New for 9.1 - tie low if not required.
input  wire [MEM_IF_DQS_WIDTH - 1  : 0]                        hc_scan_update;            // New for 9.1 - tie low if not required.

output wire [MEM_IF_DWIDTH - 1  : 0]                           hc_scan_dout;              // New for 9.1

input wire                      	   		       hc_scan_ck;                // New for 9.1 - tie low if not required.


// Internal signal declarations :
wire                                            aux_clk;

// Clocks :

// full-rate memory clock
wire                                            mem_clk_2x;

// write_clk_2x is a full-rate write clock.  It is -90 degress aligned to the
// system clock :
wire                                            write_clk_2x;

wire                                            phy_clk_1x_src;
wire                                            phy_clk_1x;
wire                                            ac_clk_1x;
wire                                            cs_n_clk_1x;
wire                                            postamble_clk_2x;
wire                                            resync_clk_2x;
wire                                            measure_clk_1x;

wire [MEM_IF_DQS_WIDTH - 1 : 0]                 resync_clk_1x;

wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]             dqs_delay_ctrl_internal;
wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]             dqs_delay_ctrl; // Output from clk_reset block

// resets, async assert, de-assert is sync'd to each clock domain
wire                                            reset_mem_clk_2x_n;
wire [MEM_IF_DQS_WIDTH - 1 : 0]                 reset_rdp_phy_clk_1x_n;
wire                                            reset_phy_clk_1x_n;
wire                                            reset_ac_clk_1x_n;
wire                                            reset_cs_n_clk_1x_n;
wire                                            reset_mimic_2x_n;
wire [MEM_IF_DQS_WIDTH - 1 : 0]                 reset_resync_clk_1x_n;
wire                                            reset_seq_n;
wire                                            reset_measure_clk_1x_n;
wire                                            reset_resync_clk_2x_n;

// Misc signals :
wire                                            phs_shft_busy;
wire                                            pll_seq_reconfig_busy;

// Postamble signals :

wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO /2  - 1 : 0]             poa_postamble_en_preset;




// Sequencer signals
wire                                                           seq_mmc_start;
wire                                                           seq_pll_inc_dec_n;
wire                                                           seq_pll_start_reconfig;
wire [CLOCK_INDEX_WIDTH - 1 : 0]                               seq_pll_select;
wire [MEM_IF_DQS_WIDTH -1 : 0]                                 seq_rdp_dec_read_lat_1x;
wire [MEM_IF_DQS_WIDTH -1 : 0]                                 seq_rdp_inc_read_lat_1x;
wire [MEM_IF_DQS_WIDTH -1 : 0]                                 seq_poa_lat_dec_1x;
wire [MEM_IF_DQS_WIDTH -1 : 0]                                 seq_poa_lat_inc_1x;
wire                                                           seq_poa_protection_override_1x;

wire                                                           seq_rdp_reset_req_n;

wire                                                           seq_ac_sel;

wire [MEM_IF_ADDR_WIDTH * DWIDTH_RATIO/2 - 1 : 0]              seq_ac_addr;
wire [MEM_IF_BANKADDR_WIDTH * DWIDTH_RATIO/2 - 1 : 0]          seq_ac_ba;
wire [DWIDTH_RATIO/2 -1 : 0]                                   seq_ac_cas_n;
wire [DWIDTH_RATIO/2 -1 : 0]                                   seq_ac_ras_n;
wire [DWIDTH_RATIO/2 -1 : 0]                                   seq_ac_we_n;
wire [MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1 : 0]                seq_ac_cke;
wire [MEM_IF_CS_WIDTH * DWIDTH_RATIO/2 - 1 : 0]                seq_ac_cs_n;
wire [MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1 : 0]                seq_ac_odt;

wire [DWIDTH_RATIO * MEM_IF_DM_WIDTH - 1 : 0 ]                 seq_wdp_dm;
wire [MEM_IF_DQS_WIDTH * (DWIDTH_RATIO/2) - 1 : 0]             seq_wdp_dqs_burst;
wire [MEM_IF_DWIDTH * DWIDTH_RATIO - 1 : 0 ]                   seq_wdp_wdata;
wire [MEM_IF_DQS_WIDTH * (DWIDTH_RATIO/2) - 1 : 0]             seq_wdp_wdata_valid;
wire [DWIDTH_RATIO                        - 1 : 0]             seq_wdp_dqs;
wire                                                           seq_wdp_ovride;

wire [MEM_IF_DQS_WIDTH * (DWIDTH_RATIO/2) - 1 : 0]             oct_rsst_sel;

wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0]               seq_rdv_doing_rd;
wire [DWIDTH_RATIO/2 - 1 : 0]                                  int_rdata_valid;
wire                                                           seq_rdata_valid_lat_inc;
wire                                                           seq_rdata_valid_lat_dec;

wire  [DWIDTH_RATIO/2 - 1 : 0]                                 seq_rdata_valid;


// Scanchain signals
reg  [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                           scan_enable_ck;
reg  [MEM_IF_DQS_WIDTH - 1 : 0]                                scan_enable_dqs;
reg  [MEM_IF_DQS_WIDTH - 1 : 0]                                scan_enable_dqs_config;
reg  [MEM_IF_DQS_WIDTH - 1 : 0]                                scan_update;
reg  [MEM_IF_DWIDTH  - 1 : 0]                                  scan_enable_dq;
reg  [MEM_IF_DM_WIDTH - 1 : 0]                                 scan_enable_dm;
reg  [MEM_IF_DQS_WIDTH - 1 : 0]                                scan_din;
reg  [MEM_IF_DQS_WIDTH - 1 : 0]                                scan_ck;
wire [MEM_IF_DWIDTH - 1 : 0]                                   scan_dout;


// set pll clock index of resync and mimic clocks
wire [CLOCK_INDEX_WIDTH                        - 1 : 0]        pll_resync_clk_index;
wire [CLOCK_INDEX_WIDTH                        - 1 : 0]        pll_measure_clk_index;


// The clk_reset block provides the sc_clk to the sequencer and DP blocks.
wire                                               sc_clk;
wire [MEM_IF_DQS_WIDTH - 1 : 0]                    sc_clk_dp;

// Mimic signals :
wire                                               mmc_seq_done;
wire                                               mmc_seq_value;
wire                                               mimic_data;

wire                                               mux_seq_controller_ready;
wire                                               mux_seq_wdata_req;


// Read datapath signals :

// Connections from the IOE to the read datapath :
wire [MEM_IF_DWIDTH - 1 : 0]                       dio_rdata_h_2x;
wire [MEM_IF_DWIDTH - 1 : 0]                       dio_rdata_l_2x;


// Write datapath signals :



// wires from the wdp to the dpio :
wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata3_1x;
wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata2_1x;
wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata1_1x;
wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata0_1x;

wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_wdata_oe_h_1x;
wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_wdata_oe_l_1x;

wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs3_1x;
wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs2_1x;
wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs1_1x;
wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs0_1x;


wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs_oe_h_1x;
wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs_oe_l_1x;

wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm3_1x;
wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm2_1x;
wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm1_1x;
wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm0_1x;

wire [MEM_IF_DQS_WIDTH -1 : 0]               wdp_oct_h_1x;
wire [MEM_IF_DQS_WIDTH -1 : 0]               wdp_oct_l_1x;

wire [MEM_IF_DQS_WIDTH -1 : 0]               seq_dqs_add_2t_delay;

wire                                         ctl_add_1t_ac_lat_internal;
wire                                         ctl_add_1t_odt_lat_internal;
wire                                         ctl_add_intermediate_regs_internal;
wire                                         ctl_negedge_en_internal;

// These ports are tied off for DDR,DDR2,DDR3.  Registers are used to reduce Quartus warnings :
(* preserve *) reg [DWIDTH_RATIO - 1 : 0]    ctl_mem_dqs;

wire [MEM_IF_NUM_RANKS - 1 : 0]               int_rank_has_addr_swap;

//SIII declarations :

//Outputs from the dp_io block to the read_dp block :
wire [MEM_IF_DWIDTH - 1 : 0]                     dio_rdata3_1x;
wire [MEM_IF_DWIDTH - 1 : 0]                     dio_rdata2_1x;
wire [MEM_IF_DWIDTH - 1 : 0]                     dio_rdata1_1x;
wire [MEM_IF_DWIDTH - 1 : 0]                     dio_rdata0_1x;

reg [DWIDTH_RATIO/2 - 1 : 0]                     rdv_pipe_ip;
reg [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0]  merged_doing_rd;


wire [OCT_LAT_WIDTH - 1 : 0]                     seq_oct_oct_delay; // oct_lat
wire [OCT_LAT_WIDTH - 1 : 0]                     seq_oct_oct_extend; //oct_extend_duration
wire seq_oct_val;

wire seq_mem_clk_disable;


wire [DWIDTH_RATIO/2 - 1 : 0]                    seq_ac_rst_n;

wire                                             dqs_delay_update_en;

// Offset control output for internal offset control block - usupported :
wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]              dlloffset_offsetctrl_out;

// offsetctrl_out for the DQS delay chains.  This will be driven by dqs_offset_delay_ctrl when
// in external DLL mode (DLL_IMPORT_EXPORT == "IMPORT" :
wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]              dlloffset_offsetctrl_internal;

wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]             seq_scan_enable_ck;
wire [MEM_IF_DWIDTH - 1 : 0]                     seq_scan_enable_dq;
wire [MEM_IF_DM_WIDTH - 1 : 0]                   seq_scan_enable_dm;
wire [MEM_IF_DQS_WIDTH - 1 : 0]                  seq_scan_enable_dqs;
wire [MEM_IF_DQS_WIDTH - 1 : 0]                  seq_scan_enable_dqs_config;
wire [MEM_IF_DQS_WIDTH - 1 : 0]                  seq_scan_update;
wire [MEM_IF_DQS_WIDTH - 1 : 0]                  seq_scan_din;


//Generate variable :
genvar dqs_grp_num;

// continual assignments :

// The top level I/O should not have the "Nx" clock domain suffices, so this is
// assigned here.  Also note that to avoid delta delay issues both the external and
// internal phy_clks are assigned to a common 'src' clock :
assign ctl_clk         = phy_clk_1x_src;
assign phy_clk_1x      = phy_clk_1x_src;

assign ctl_reset_n     = reset_phy_clk_1x_n;

// Generate auxillary clocks:
generate

    // Half-rate mode :
    if (DWIDTH_RATIO == 4)
    begin
        assign aux_half_rate_clk = phy_clk_1x;
        assign aux_full_rate_clk = aux_clk;
    end

    // Full-rate mode - NB unsupported for DDR3 :
    else
    begin
        assign aux_half_rate_clk = aux_clk;
        assign aux_full_rate_clk = phy_clk_1x;
    end

endgenerate


assign dqs_delay_ctrl_export = dqs_delay_ctrl;
assign dll_reference_clk     = mem_clk_2x;

// If using an external DLL, DLL_EXPORT_IMPORT shall be set to "IMPORT" and the
// delay code for the DQS delay chains and the phase alignment blocks (if used)
// shall come from the dqs_delay_ctrl_import input instead.  This should be connected
// to the "delayctrlout" output of the external DLL
generate
    if (DLL_EXPORT_IMPORT == "IMPORT")
        assign dqs_delay_ctrl_internal = dqs_delay_ctrl_import;
    else
        assign dqs_delay_ctrl_internal = dqs_delay_ctrl;
endgenerate

// If DQS offsetting is required, external DLL mode should be used
// (as this is the only supported mode for offsetting), so DLL_EXPORT_IMPORT shall be
// set to "IMPORT" and the offset_delay_ctrl signal for the DQS delay chain shall come
// from the "dqs_offset_delay_ctrl" input.  This should be connected in turn to the
// "offset_ctrl_out" output of the dll_offset_ctrl block.
generate
    if (DLL_EXPORT_IMPORT == "IMPORT")
        assign dlloffset_offsetctrl_internal = dqs_offset_delay_ctrl;
    else
        assign dlloffset_offsetctrl_internal = dlloffset_offsetctrl_out;
endgenerate


//Scan control muxing :


always @*
begin

    if (hc_scan_enable_access == 1'b1)
    begin
    	scan_enable_dq         = hc_scan_enable_dq;
    	scan_enable_dm         = hc_scan_enable_dm;
    	scan_enable_dqs        = hc_scan_enable_dqs;
    	scan_enable_dqs_config = hc_scan_enable_dqs_config;
    	scan_enable_ck         = 1'b0;
    	scan_update 	       = hc_scan_update;
    	scan_din    	       = hc_scan_din;
        scan_ck                = {MEM_IF_DQS_WIDTH{hc_scan_ck}} ;
    end

    else
    begin
    	scan_enable_dq         = seq_scan_enable_dq;
    	scan_enable_dm         = seq_scan_enable_dm;
    	scan_enable_dqs        = seq_scan_enable_dqs;
    	scan_enable_dqs_config = seq_scan_enable_dqs_config;
    	scan_enable_ck         = seq_scan_enable_ck; // Actually sequencer does not use, but included for completeness.
    	scan_update 	       = seq_scan_update;
    	scan_din    	       = seq_scan_din;
        scan_ck                = sc_clk_dp;
    end

end

assign hc_scan_dout = hc_scan_enable_access ? scan_dout : 'h0 ;



// Instance I/O modules :


//
ddr2_phy_alt_mem_phy_dp_io #(
     .MEM_IF_CLK_PS               (MEM_IF_CLK_PS),
     .MEM_IF_CLK_PS_STR           (MEM_IF_CLK_PS_STR),
     .MEM_IF_BANKADDR_WIDTH       (MEM_IF_BANKADDR_WIDTH),
     .MEM_IF_CS_WIDTH             (MEM_IF_CS_WIDTH),
     .MEM_IF_DWIDTH               (MEM_IF_DWIDTH),
     .MEM_IF_DM_WIDTH             (MEM_IF_DM_WIDTH),
     .MEM_IF_DM_PINS_EN           (MEM_IF_DM_PINS_EN),
     .MEM_IF_DQ_PER_DQS           (MEM_IF_DQ_PER_DQS),
     .MEM_IF_DQS_CAPTURE_EN       (MEM_IF_DQS_CAPTURE_EN),
     .MEM_IF_DQS_WIDTH            (MEM_IF_DQS_WIDTH),
     .MEM_IF_MEMTYPE              (MEM_IF_MEMTYPE),
     .DWIDTH_RATIO                (DWIDTH_RATIO),
     .LEVELLING                   (LEVELLING),
     .READ_DESKEW_MODE            (READ_DESKEW_MODE),
     .WRITE_DESKEW_MODE           (WRITE_DESKEW_MODE),
     .MEM_IF_DQSN_EN              (MEM_IF_DQSN_EN),
     .MEM_IF_OCT_EN               (MEM_IF_OCT_EN),
     .MEM_IF_POSTAMBLE_EN_WIDTH   (8),
     .MEM_IF_ROWADDR_WIDTH        (MEM_IF_ADDR_WIDTH),
     .DLL_DELAY_BUFFER_MODE       (DLL_DELAY_BUFFER_MODE),
     .DQS_OUT_MODE                (DQS_OUT_MODE),
     .DQS_PHASE                   (DQS_PHASE),
     .DQS_PHASE_SETTING           (DQS_PHASE_SETTING),
     .MEM_TCL                     (MEM_TCL),
     .ENABLE_DDR3_SEQUENCER       (ENABLE_DDR3_SEQUENCER),
     .DQS_DELAY_CTL_WIDTH         (DQS_DELAY_CTL_WIDTH),
     .DQS_DELAY_USES_OFFSET       (USE_DLL_OFFSET),
     .INVERT_POSTAMBLE_CLK        (INVERT_POSTAMBLE_CLK)
) dpio (
     .reset_write_clk_2x_n        (1'b1),
     .phy_clk_1x                  (phy_clk_1x),
     .resync_clk_2x               (resync_clk_2x),
     .mem_clk_2x                  (mem_clk_2x),
     .write_clk_2x                (write_clk_2x),
     .resync_clk_1x               (resync_clk_1x),
     .sc_clk                      (scan_ck),
     .scan_enable_dqs_config      (scan_enable_dqs_config),
     .scan_enable_dqs             (scan_enable_dqs),
     .scan_enable_dq              (scan_enable_dq),
     .scan_enable_dm              (scan_enable_dm),
     .scan_update                 (scan_update),
     .scan_din                    (scan_din),
     .scan_dout                   (scan_dout),
     .dedicated_dll_delay_ctrl    (dqs_delay_ctrl_internal),
     .seq_dqs_delay_ctrl          (dqs_delay_ctrl_internal),
     .dll_offset_delay_ctrl       (dlloffset_offsetctrl_internal),
     .dqs_update_en               (dqs_delay_update_en),
     .mem_d                       (mem_d),
     .mem_dm                      (mem_dm),
     .mem_dq                      (mem_dq),
     .mem_dqs                     (mem_dqs),
     .mem_dqsn                    (mem_dqs_n),
     .dio_rdata3_1x               (dio_rdata3_1x),
     .dio_rdata2_1x               (dio_rdata2_1x),
     .dio_rdata1_1x               (dio_rdata1_1x),
     .dio_rdata0_1x               (dio_rdata0_1x),
     .poa_postamble_en_preset     (poa_postamble_en_preset),
     .wdp_wdata3_1x               (wdp_wdata3_1x),
     .wdp_wdata2_1x               (wdp_wdata2_1x),
     .wdp_wdata1_1x               (wdp_wdata1_1x),
     .wdp_wdata0_1x               (wdp_wdata0_1x),
     .wdp_wdata_oe_h_1x           (wdp_wdata_oe_h_1x),
     .wdp_wdata_oe_l_1x           (wdp_wdata_oe_l_1x),
     .wdp_dqs3_1x                 (wdp_dqs3_1x),
     .wdp_dqs2_1x                 (wdp_dqs2_1x),
     .wdp_dqs1_1x                 (wdp_dqs1_1x),
     .wdp_dqs0_1x                 (wdp_dqs0_1x),
     .wdp_dqs_oe_h_1x             (wdp_dqs_oe_h_1x),
     .wdp_dqs_oe_l_1x             (wdp_dqs_oe_l_1x),
     .wdp_dm3_1x                  (wdp_dm3_1x),
     .wdp_dm2_1x                  (wdp_dm2_1x),
     .wdp_dm1_1x                  (wdp_dm1_1x),
     .wdp_dm0_1x                  (wdp_dm0_1x),
     .wdp_oct_h_1x                (wdp_oct_h_1x),
     .wdp_oct_l_1x                (wdp_oct_l_1x),
     .seriesterminationcontrol    (oct_ctl_rs_value),
     .parallelterminationcontrol  (oct_ctl_rt_value)
);



// Instance the read datapath :

// This needs generating on a per DQS-group basis :

// For example, a 64bit memory with 4 DQ bits per DQS will instance this block 16
// times, each instance catering for 4 DQ bits.


generate

    if (DWIDTH_RATIO == 4)
    begin: hr_rdp

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS ; dqs_grp_num=dqs_grp_num+1)
        begin : rdp_per_group

            //
            ddr2_phy_alt_mem_phy_read_dp_group #(
                                 .ADDR_COUNT_WIDTH          (4),
                     .BIDIR_DPINS               (1),
                     .DWIDTH_RATIO              (DWIDTH_RATIO),
                     .MEM_IF_CLK_PS             (MEM_IF_CLK_PS),
                     .FAMILY                    (FAMILY),
                     .MEM_IF_DQ_PER_DQS         (MEM_IF_DQ_PER_DQS),
                     .RDP_INITIAL_LAT           (RDP_INITIAL_LAT),
                     .RDP_RESYNC_LAT_CTL_EN     (0),
                     .RESYNC_PIPELINE_DEPTH     (RESYNC_PIPELINE_DEPTH)
                   ) rdp (
                     .phy_clk_1x                (phy_clk_1x),
                     .resync_clk_1x             (resync_clk_1x[dqs_grp_num]),
                     .reset_phy_clk_1x_n        (reset_rdp_phy_clk_1x_n[dqs_grp_num]),
                     .reset_resync_clk_1x_n     (reset_resync_clk_1x_n[dqs_grp_num]),
                     .seq_rdp_dec_read_lat_1x   (seq_rdp_dec_read_lat_1x[dqs_grp_num]),
                     .seq_rdp_inc_read_lat_1x   (seq_rdp_inc_read_lat_1x[dqs_grp_num]),
                     .dio_rdata3_1x             (dio_rdata3_1x[(dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1:dqs_grp_num*MEM_IF_DQ_PER_DQS]),
                     .dio_rdata2_1x             (dio_rdata2_1x[(dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1:dqs_grp_num*MEM_IF_DQ_PER_DQS]),
                     .dio_rdata1_1x             (dio_rdata1_1x[(dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1:dqs_grp_num*MEM_IF_DQ_PER_DQS]),
                     .dio_rdata0_1x             (dio_rdata0_1x[(dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1:dqs_grp_num*MEM_IF_DQ_PER_DQS]),
                     .ctl_mem_rdata             ( { ctl_rdata[MEM_IF_DWIDTH * 3 + ((dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1) : MEM_IF_DWIDTH * 3 + (dqs_grp_num*MEM_IF_DQ_PER_DQS) ],
                                                    ctl_rdata[MEM_IF_DWIDTH * 2 + ((dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1) : MEM_IF_DWIDTH * 2 + (dqs_grp_num*MEM_IF_DQ_PER_DQS) ],
                                                    ctl_rdata[MEM_IF_DWIDTH * 1 + ((dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1) : MEM_IF_DWIDTH * 1 + (dqs_grp_num*MEM_IF_DQ_PER_DQS) ],
                                                    ctl_rdata[MEM_IF_DWIDTH * 0 + ((dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1) : MEM_IF_DWIDTH * 0 + (dqs_grp_num*MEM_IF_DQ_PER_DQS) ]
                                                })
                   );
                end

    end

    else
    begin : fr_rdp

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS ; dqs_grp_num=dqs_grp_num+1)
        begin : rdp_per_group

            //
            ddr2_phy_alt_mem_phy_read_dp_group #(
                                 .ADDR_COUNT_WIDTH          (4),
                     .BIDIR_DPINS               (1),
                     .DWIDTH_RATIO              (DWIDTH_RATIO),
                     .MEM_IF_CLK_PS             (MEM_IF_CLK_PS),
                     .FAMILY                    (FAMILY),
                     .MEM_IF_DQ_PER_DQS         (MEM_IF_DQ_PER_DQS),
                     .RDP_INITIAL_LAT           (RDP_INITIAL_LAT),
                     .RDP_RESYNC_LAT_CTL_EN     (0),
                     .RESYNC_PIPELINE_DEPTH     (RESYNC_PIPELINE_DEPTH)
                   ) rdp (
                     .phy_clk_1x                (phy_clk_1x),
                     .resync_clk_1x             (resync_clk_2x),
                     .reset_phy_clk_1x_n        (reset_rdp_phy_clk_1x_n[dqs_grp_num]),
                     .reset_resync_clk_1x_n     (reset_resync_clk_2x_n),
                     .seq_rdp_dec_read_lat_1x   (seq_rdp_dec_read_lat_1x[dqs_grp_num]),
                     .seq_rdp_inc_read_lat_1x   (seq_rdp_inc_read_lat_1x[dqs_grp_num]),
                     .dio_rdata3_1x             (),
                     .dio_rdata2_1x             (),
                     .dio_rdata1_1x             (dio_rdata1_1x[(dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1:dqs_grp_num*MEM_IF_DQ_PER_DQS]),
                     .dio_rdata0_1x             (dio_rdata0_1x[(dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1:dqs_grp_num*MEM_IF_DQ_PER_DQS]),
                     .ctl_mem_rdata             ( { ctl_rdata[MEM_IF_DWIDTH * 1 + ((dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1) : MEM_IF_DWIDTH * 1 + (dqs_grp_num*MEM_IF_DQ_PER_DQS) ],
                                                    ctl_rdata[MEM_IF_DWIDTH * 0 + ((dqs_grp_num+1)*MEM_IF_DQ_PER_DQS-1) : MEM_IF_DWIDTH * 0 + (dqs_grp_num*MEM_IF_DQ_PER_DQS) ]
                                                })
                   );
                end

    end
endgenerate



generate

    if (DWIDTH_RATIO == 4)
    begin : hr_oct_gen

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS ; dqs_grp_num=dqs_grp_num+1)
        begin : park_at_rs_oct_group

             //
             ddr2_phy_alt_mem_phy_oct_delay #(
                           .FAMILY             (FAMILY),
              .OCT_LAT_WIDTH      (OCT_LAT_WIDTH),
              .DWIDTH_RATIO       (DWIDTH_RATIO)
            ) oct (
             .phy_clk_1x          (phy_clk_1x),
             .reset_phy_clk_1x_n  (reset_phy_clk_1x_n),
             .oct_lat             (seq_oct_oct_delay),
             .oct_extend_duration (seq_oct_oct_extend),
             .ctl_doing_rd        ( { ctl_doing_rd[(dqs_grp_num + (MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS))],  ctl_doing_rd[dqs_grp_num]}),
             .oct_out             ( { oct_rsst_sel[(dqs_grp_num + (MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS))],  oct_rsst_sel[dqs_grp_num]})
            );

        end

    end

    else
    begin : fr_oct_gen

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS ; dqs_grp_num=dqs_grp_num+1)
        begin : park_at_rs_oct_group

             //
             ddr2_phy_alt_mem_phy_oct_delay #(
                           .FAMILY             (FAMILY),
              .OCT_LAT_WIDTH      (OCT_LAT_WIDTH),
              .DWIDTH_RATIO       (DWIDTH_RATIO)
            ) oct (
             .phy_clk_1x          (phy_clk_1x),
             .reset_phy_clk_1x_n  (reset_phy_clk_1x_n),
             .oct_lat             (seq_oct_oct_delay),
             .oct_extend_duration (seq_oct_oct_extend),
             .ctl_doing_rd        (ctl_doing_rd[dqs_grp_num]),
             .oct_out             (oct_rsst_sel[dqs_grp_num])
            );

        end

    end

endgenerate


// Instance the write datapath :
always @(posedge phy_clk_1x)
begin
    if (DWIDTH_RATIO == 4)
       ctl_mem_dqs <= 4'b1100;
    else
       ctl_mem_dqs <= 2'b10;
end

generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate_wdp_gen

        //
        ddr2_phy_alt_mem_phy_write_dp #(
                    .BIDIR_DPINS            (1),
            .LOCAL_IF_DRATE         ("HALF"),
            .LOCAL_IF_DWIDTH        (MEM_IF_DWIDTH * DWIDTH_RATIO),
            .MEM_IF_DQS_WIDTH       (MEM_IF_DQS_WIDTH),
            .MEM_IF_DQ_PER_DQS      (MEM_IF_DQ_PER_DQS),
            .MEM_IF_DM_WIDTH        (MEM_IF_DM_WIDTH),
            .MEM_IF_BE_WIDTH        (MEM_IF_DM_WIDTH * DWIDTH_RATIO),
            .MEM_IF_MEMTYPE         (MEM_IF_MEMTYPE),
            .MEM_IF_OCT_EN          (MEM_IF_OCT_EN),
            .GENERATE_WRITE_DQS     (1),
            .MEM_IF_DWIDTH          (MEM_IF_DWIDTH),
            .DWIDTH_RATIO           (DWIDTH_RATIO),
            .MEM_IF_DM_PINS_EN      (MEM_IF_DM_PINS_EN)
        ) wdp (
            .phy_clk_1x             (phy_clk_1x),
            .reset_phy_clk_1x_n     (reset_phy_clk_1x_n),
            .ctl_mem_be             (ctl_dm),
            .ctl_mem_dqs_burst      (ctl_dqs_burst),
            .ctl_mem_wdata          (ctl_wdata),
            .ctl_mem_wdata_valid    (ctl_wdata_valid),
            .ctl_mem_dqs            (ctl_mem_dqs),
            .ctl_mem_oct            (oct_rsst_sel),
            .seq_be                 (seq_wdp_dm),
            .seq_dqs_burst          (seq_wdp_dqs_burst),
            .seq_wdata              (seq_wdp_wdata),
            .seq_wdata_valid        (seq_wdp_wdata_valid),
            .seq_dqs                (seq_wdp_dqs),
            .seq_ctl_sel            (seq_wdp_ovride),
            .seq_oct_val            (seq_oct_val),
            .seq_dq_dm_add_2t_delay (1'b0),
            .seq_dqs_add_2t_delay   (seq_dqs_add_2t_delay),
            .wdp_wdata3_1x          (wdp_wdata3_1x),
            .wdp_wdata2_1x          (wdp_wdata2_1x),
            .wdp_wdata1_1x          (wdp_wdata1_1x),
            .wdp_wdata0_1x          (wdp_wdata0_1x),
            .wdp_wdata_oe_h_1x      (wdp_wdata_oe_h_1x),
            .wdp_wdata_oe_l_1x      (wdp_wdata_oe_l_1x),
            .wdp_dqs3_1x            (wdp_dqs3_1x),
            .wdp_dqs2_1x            (wdp_dqs2_1x),
            .wdp_dqs1_1x            (wdp_dqs1_1x),
            .wdp_dqs0_1x            (wdp_dqs0_1x),
            .wdp_dqs_oe_h_1x        (wdp_dqs_oe_h_1x),
            .wdp_dqs_oe_l_1x        (wdp_dqs_oe_l_1x),
            .wdp_dm3_1x             (wdp_dm3_1x),
            .wdp_dm2_1x             (wdp_dm2_1x),
            .wdp_dm1_1x             (wdp_dm1_1x),
            .wdp_dm0_1x             (wdp_dm0_1x),
            .wdp_oct_h_1x           (wdp_oct_h_1x),
            .wdp_oct_l_1x           (wdp_oct_l_1x)
        );

    end

    else
    begin: full_rate_wdp_gen

        //
        ddr2_phy_alt_mem_phy_write_dp_fr #(
                    .BIDIR_DPINS            (1),
            .LOCAL_IF_DRATE         ("FULL"),
            .LOCAL_IF_DWIDTH        (MEM_IF_DWIDTH * DWIDTH_RATIO),
            .MEM_IF_DQS_WIDTH       (MEM_IF_DQS_WIDTH),
            .MEM_IF_DQ_PER_DQS      (MEM_IF_DQ_PER_DQS),
            .MEM_IF_DM_WIDTH        (MEM_IF_DM_WIDTH),
            .MEM_IF_BE_WIDTH        (MEM_IF_DM_WIDTH * DWIDTH_RATIO),
            .MEM_IF_MEMTYPE         (MEM_IF_MEMTYPE),
            .MEM_IF_OCT_EN          (MEM_IF_OCT_EN),
            .GENERATE_WRITE_DQS     (1),
            .MEM_IF_DWIDTH          (MEM_IF_DWIDTH),
            .DWIDTH_RATIO           (DWIDTH_RATIO),
            .MEM_IF_DM_PINS_EN      (MEM_IF_DM_PINS_EN)
        ) wdp (
            .phy_clk_1x             (phy_clk_1x),
            .reset_phy_clk_1x_n     (reset_phy_clk_1x_n),
            .ctl_mem_be             (ctl_dm),
            .ctl_mem_dqs_burst      (ctl_dqs_burst),
            .ctl_mem_wdata          (ctl_wdata),
            .ctl_mem_wdata_valid    (ctl_wdata_valid),
            .ctl_mem_dqs            (ctl_mem_dqs),
            .ctl_mem_oct            (oct_rsst_sel),
            .seq_be                 (seq_wdp_dm),
            .seq_dqs_burst          (seq_wdp_dqs_burst),
            .seq_wdata              (seq_wdp_wdata),
            .seq_wdata_valid        (seq_wdp_wdata_valid),
            .seq_dqs                (seq_wdp_dqs),
            .seq_ctl_sel            (seq_wdp_ovride),
            .seq_oct_val            (seq_oct_val),
            .seq_dq_dm_add_2t_delay (1'b0),
            .seq_dqs_add_2t_delay   (seq_dqs_add_2t_delay),
            .wdp_wdata1_1x          (wdp_wdata1_1x),
            .wdp_wdata0_1x          (wdp_wdata0_1x),
            .wdp_wdata_oe_h_1x      (wdp_wdata_oe_h_1x),
            .wdp_wdata_oe_l_1x      (wdp_wdata_oe_l_1x),
            .wdp_dqs1_1x            (wdp_dqs1_1x),
            .wdp_dqs0_1x            (wdp_dqs0_1x),
            .wdp_dqs_oe_h_1x        (wdp_dqs_oe_h_1x),
            .wdp_dqs_oe_l_1x        (wdp_dqs_oe_l_1x),
            .wdp_dm1_1x             (wdp_dm1_1x),
            .wdp_dm0_1x             (wdp_dm0_1x),
            .wdp_oct_h_1x           (wdp_oct_h_1x),
            .wdp_oct_l_1x           (wdp_oct_l_1x)
        );

    end

endgenerate

// Instance the address and command :

generate

    if (DWIDTH_RATIO == 4)
    begin: half_rate_adc_gen

        //
        ddr2_phy_alt_mem_phy_addr_cmd #(
                     .FAMILY                       (FAMILY),
             .MEM_ADDR_CMD_BUS_COUNT       (1),
             .MEM_IF_BANKADDR_WIDTH        (MEM_IF_BANKADDR_WIDTH),
             .MEM_IF_CS_WIDTH              (MEM_IF_CS_WIDTH),
             .MEM_IF_NUM_RANKS             (MEM_IF_NUM_RANKS),
             .MEM_IF_MEMTYPE               (MEM_IF_MEMTYPE),
             .DWIDTH_RATIO                 (DWIDTH_RATIO),
             .MEM_IF_ROWADDR_WIDTH         (MEM_IF_ADDR_WIDTH),
             .INVERT_ADDR_CMD_TXFR_CLK     (INVERT_ADDR_CMD_TXFR_CLK),
             .LEVELLING                    (LEVELLING),
             .CHIP_OR_DIMM                 (CHIP_OR_DIMM)
        ) adc (
             .ac_clk_1x                    (ac_clk_1x),
             .cs_n_clk_1x                  (cs_n_clk_1x),
             .phy_clk_1x                   (phy_clk_1x),
             .reset_phy_clk_1x_n           (reset_phy_clk_1x_n),
             .reset_ac_clk_1x_n            (reset_cs_n_clk_1x_n), // cs_n reset is used instead. Basically the same.
             .ctl_add_1t_ac_lat            (ctl_add_1t_ac_lat_internal),
             .ctl_add_1t_odt_lat           (ctl_add_1t_odt_lat_internal),
             .ctl_add_intermediate_regs    (ctl_add_intermediate_regs_internal),
             .ctl_negedge_en               (ctl_negedge_en_internal),
             .ctl_mem_addr_h               (ctl_addr[MEM_IF_ADDR_WIDTH -1 : 0]),
             .ctl_mem_addr_l               (ctl_addr[MEM_IF_ADDR_WIDTH  * DWIDTH_RATIO/2 -1 : MEM_IF_ADDR_WIDTH]),
             .ctl_mem_ba_h                 (ctl_ba[MEM_IF_BANKADDR_WIDTH -1 : 0]),
             .ctl_mem_ba_l                 (ctl_ba[MEM_IF_BANKADDR_WIDTH * DWIDTH_RATIO/2 -1 : MEM_IF_BANKADDR_WIDTH]),
             .ctl_mem_cas_n_h              (ctl_cas_n[0]),
             .ctl_mem_cas_n_l              (ctl_cas_n[1]),
             .ctl_mem_cke_h                (ctl_cke[MEM_IF_NUM_RANKS - 1 : 0]),
             .ctl_mem_cke_l                (ctl_cke[MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1 : MEM_IF_NUM_RANKS]),
             .ctl_mem_cs_n_h               (ctl_cs_n[MEM_IF_NUM_RANKS - 1 : 0]),
             .ctl_mem_cs_n_l               (ctl_cs_n[MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1 : MEM_IF_NUM_RANKS]),
             .ctl_mem_odt_h                (ctl_odt[MEM_IF_NUM_RANKS - 1 : 0]),
             .ctl_mem_odt_l                (ctl_odt[MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1 : MEM_IF_NUM_RANKS]),
             .ctl_mem_ras_n_h              (ctl_ras_n[0]),
             .ctl_mem_ras_n_l              (ctl_ras_n[1]),
             .ctl_mem_we_n_h               (ctl_we_n[0]),
             .ctl_mem_we_n_l               (ctl_we_n[1]),
             .ctl_mem_wps_n                ({MEM_IF_NUM_RANKS{1'b0}}),
             .ctl_mem_rps_n                ({MEM_IF_NUM_RANKS{1'b0}}),
             .ctl_mem_rst_n_h              (ctl_rst_n[0]),
             .ctl_mem_rst_n_l              (ctl_rst_n[1]),
             .seq_addr_h                   (seq_ac_addr[MEM_IF_ADDR_WIDTH -1 : 0]),
             .seq_addr_l                   (seq_ac_addr[MEM_IF_ADDR_WIDTH  * DWIDTH_RATIO/2 -1 : MEM_IF_ADDR_WIDTH]),
             .seq_ba_h                     (seq_ac_ba[MEM_IF_BANKADDR_WIDTH -1 : 0]),
             .seq_ba_l                     (seq_ac_ba[MEM_IF_BANKADDR_WIDTH * DWIDTH_RATIO/2 -1 : MEM_IF_BANKADDR_WIDTH]),
             .seq_cas_n_h                  (seq_ac_cas_n[0]),
             .seq_cas_n_l                  (seq_ac_cas_n[1]),
             .seq_cke_h                    (seq_ac_cke[MEM_IF_NUM_RANKS - 1 : 0]),
             .seq_cke_l                    (seq_ac_cke[MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1 : MEM_IF_NUM_RANKS]),
             .seq_cs_n_h                   (seq_ac_cs_n[MEM_IF_CS_WIDTH - 1 : 0]),
             .seq_cs_n_l                   (seq_ac_cs_n[MEM_IF_CS_WIDTH * DWIDTH_RATIO/2 - 1 : MEM_IF_CS_WIDTH]),
             .seq_odt_h                    (seq_ac_odt[MEM_IF_NUM_RANKS - 1 : 0]),
             .seq_odt_l                    (seq_ac_odt[MEM_IF_NUM_RANKS * DWIDTH_RATIO/2 - 1 : MEM_IF_NUM_RANKS]),
             .seq_ras_n_h                  (seq_ac_ras_n[0]),
             .seq_ras_n_l                  (seq_ac_ras_n[1]),
             .seq_we_n_h                   (seq_ac_we_n[0]),
             .seq_we_n_l                   (seq_ac_we_n[1]),
             .seq_mem_rst_n_h              (seq_ac_rst_n[0]),
             .seq_mem_rst_n_l              (seq_ac_rst_n[1]),
             .seq_ac_sel                   (seq_ac_sel),
             .seq_wps_n                    ({MEM_IF_NUM_RANKS{1'b0}}),
             .seq_rps_n                    ({MEM_IF_NUM_RANKS{1'b0}}),
             .mem_ac_parity                (mem_ac_parity),
             .mem_addr                     (mem_addr),
             .mem_ba                       (mem_ba),
             .mem_cas_n                    (mem_cas_n),
             .mem_cke                      (mem_cke),
             .mem_cs_n                     (mem_cs_n),
             .mem_odt                      (mem_odt),
             .mem_ras_n                    (mem_ras_n),
             .mem_we_n                     (mem_we_n),
             .mem_rst_n                    (mem_reset_n),
             .mem_rps_n                    (),
             .mem_wps_n                    ()
        );

    end

    else
    begin : full_rate_adc_gen

        //
        ddr2_phy_alt_mem_phy_addr_cmd #(
                     .FAMILY                       (FAMILY),
             .MEM_ADDR_CMD_BUS_COUNT       (1),
             .MEM_IF_BANKADDR_WIDTH        (MEM_IF_BANKADDR_WIDTH),
             .MEM_IF_CS_WIDTH              (MEM_IF_CS_WIDTH),
             .MEM_IF_NUM_RANKS             (MEM_IF_NUM_RANKS),
             .MEM_IF_MEMTYPE               (MEM_IF_MEMTYPE),
             .DWIDTH_RATIO                 (DWIDTH_RATIO),
             .MEM_IF_ROWADDR_WIDTH         (MEM_IF_ADDR_WIDTH),
             .INVERT_ADDR_CMD_TXFR_CLK     (INVERT_ADDR_CMD_TXFR_CLK),
             .LEVELLING                    (LEVELLING),
             .CHIP_OR_DIMM                 (CHIP_OR_DIMM)
        ) adc (
             .ac_clk_1x                    (ac_clk_1x),
             .cs_n_clk_1x                  (cs_n_clk_1x),
             .phy_clk_1x                   (phy_clk_1x),
             .reset_phy_clk_1x_n           (reset_phy_clk_1x_n),
             .reset_ac_clk_1x_n            (reset_cs_n_clk_1x_n), // cs_n reset is used instead. Basically the same.
             .ctl_add_1t_ac_lat            (ctl_add_1t_ac_lat_internal),
             .ctl_add_1t_odt_lat           (ctl_add_1t_odt_lat_internal),
             .ctl_add_intermediate_regs    (ctl_add_intermediate_regs_internal),
             .ctl_negedge_en               (ctl_negedge_en_internal),
             .ctl_mem_addr_h               (),
             .ctl_mem_addr_l               (ctl_addr[MEM_IF_ADDR_WIDTH -1 : 0]),
             .ctl_mem_ba_h                 (),
             .ctl_mem_ba_l                 (ctl_ba[MEM_IF_BANKADDR_WIDTH -1 : 0]),
             .ctl_mem_cas_n_h              (),
             .ctl_mem_cas_n_l              (ctl_cas_n[0]),
             .ctl_mem_cke_h                (),
             .ctl_mem_cke_l                (ctl_cke[MEM_IF_NUM_RANKS - 1 : 0]),
             .ctl_mem_cs_n_h               (),
             .ctl_mem_cs_n_l               (ctl_cs_n[MEM_IF_NUM_RANKS - 1 : 0]),
             .ctl_mem_odt_h                (),
             .ctl_mem_odt_l                (ctl_odt[MEM_IF_NUM_RANKS - 1 : 0]),
             .ctl_mem_ras_n_h              (),
             .ctl_mem_ras_n_l              (ctl_ras_n[0]),
             .ctl_mem_we_n_h               (),
             .ctl_mem_we_n_l               (ctl_we_n[0]),
             .ctl_mem_wps_n                ({MEM_IF_CS_WIDTH{1'b0}}),
             .ctl_mem_rps_n                ({MEM_IF_CS_WIDTH{1'b0}}),
             .ctl_mem_rst_n_h              (),
             .ctl_mem_rst_n_l              (ctl_rst_n[0]),
             .seq_addr_h                   (),
             .seq_addr_l                   (seq_ac_addr[MEM_IF_ADDR_WIDTH -1 : 0]),
             .seq_ba_h                     (),
             .seq_ba_l                     (seq_ac_ba[MEM_IF_BANKADDR_WIDTH -1 : 0]),
             .seq_cas_n_h                  (),
             .seq_cas_n_l                  (seq_ac_cas_n[0]),
             .seq_cke_h                    (),
             .seq_cke_l                    (seq_ac_cke[MEM_IF_NUM_RANKS - 1 : 0]),
             .seq_cs_n_h                   (),
             .seq_cs_n_l                   (seq_ac_cs_n[MEM_IF_CS_WIDTH - 1 : 0]),
             .seq_odt_h                    (),
             .seq_odt_l                    (seq_ac_odt[MEM_IF_NUM_RANKS - 1 : 0]),
             .seq_ras_n_h                  (),
             .seq_ras_n_l                  (seq_ac_ras_n[0]),
             .seq_we_n_h                   (),
             .seq_we_n_l                   (seq_ac_we_n[0]),
             .seq_mem_rst_n_h              (),
             .seq_mem_rst_n_l              (seq_ac_rst_n[0]),
             .seq_ac_sel                   (seq_ac_sel),
             .seq_wps_n                    ({MEM_IF_NUM_RANKS{1'b0}}),
             .seq_rps_n                    ({MEM_IF_NUM_RANKS{1'b0}}),
             .mem_ac_parity                (mem_ac_parity),
             .mem_addr                     (mem_addr),
             .mem_ba                       (mem_ba),
             .mem_cas_n                    (mem_cas_n),
             .mem_cke                      (mem_cke),
             .mem_cs_n                     (mem_cs_n),
             .mem_odt                      (mem_odt),
             .mem_ras_n                    (mem_ras_n),
             .mem_we_n                     (mem_we_n),
             .mem_rst_n                    (mem_reset_n),
             .mem_rps_n                    (),
             .mem_wps_n                    ()
        );

    end

endgenerate


 assign int_rank_has_addr_swap = RANK_HAS_ADDR_SWAP[MEM_IF_CS_WIDTH - 1 : 0];
 assign pll_resync_clk_index   = 5;
 assign pll_measure_clk_index  = 7;

// instantiate the DDR3 sequencer:
   //
  ddr2_phy_alt_mem_phy_seq_wrapper
      //
    seq_wrapper (
        .phy_clk_1x                         (phy_clk_1x),
        .reset_phy_clk_1x_n                 (reset_phy_clk_1x_n),
        .ctl_cal_success                    (ctl_cal_success),
        .ctl_cal_fail                       (ctl_cal_fail),
        .ctl_cal_warning                    (ctl_cal_warning),
        .ctl_cal_req                        (ctl_cal_req),
        .int_RANK_HAS_ADDR_SWAP             (int_rank_has_addr_swap),
        .ctl_cal_byte_lane_sel_n            (ctl_cal_byte_lane_sel_n),
        .seq_pll_inc_dec_n                  (seq_pll_inc_dec_n),
        .seq_pll_start_reconfig             (seq_pll_start_reconfig),
        .seq_pll_select                     (seq_pll_select),
        .phs_shft_busy                      (phs_shft_busy),
        .pll_resync_clk_index               (pll_resync_clk_index),
        .pll_measure_clk_index              (pll_measure_clk_index),
        .sc_clk_dp                          (sc_clk_dp),
        .scan_enable_dqs_config             (seq_scan_enable_dqs_config),
        .scan_update                        (seq_scan_update),
        .scan_din                           (seq_scan_din),
        .scan_enable_ck                     (seq_scan_enable_ck), //NB. mem_clk scan config
        .scan_enable_dqs                    (seq_scan_enable_dqs),
        .scan_enable_dq                     (seq_scan_enable_dq),
        .scan_enable_dm                     (seq_scan_enable_dm),
        .hr_rsc_clk                         (resync_clk_1x[0]),
        .seq_ac_addr                        (seq_ac_addr),
        .seq_ac_ba                          (seq_ac_ba),
        .seq_ac_cas_n                       (seq_ac_cas_n),
        .seq_ac_ras_n                       (seq_ac_ras_n),
        .seq_ac_we_n                        (seq_ac_we_n),
        .seq_ac_cke                         (seq_ac_cke),
        .seq_ac_cs_n                        (seq_ac_cs_n),
        .seq_ac_odt                         (seq_ac_odt),
        .seq_ac_rst_n                       (seq_ac_rst_n),
        .seq_ac_sel                         (seq_ac_sel),
        .seq_mem_clk_disable                (seq_mem_clk_disable),
        .ctl_add_1t_ac_lat_internal         (ctl_add_1t_ac_lat_internal),
        .ctl_add_1t_odt_lat_internal        (ctl_add_1t_odt_lat_internal),
        .ctl_add_intermediate_regs_internal (ctl_add_intermediate_regs_internal),
        .seq_rdv_doing_rd                   (seq_rdv_doing_rd),
        .seq_rdp_reset_req_n                (seq_rdp_reset_req_n),
        .seq_rdp_inc_read_lat_1x            (seq_rdp_inc_read_lat_1x),
        .seq_rdp_dec_read_lat_1x            (seq_rdp_dec_read_lat_1x),
        .ctl_rdata                          (ctl_rdata),
        .int_rdata_valid_1t                 (seq_rdata_valid),
        .seq_rdata_valid_lat_inc            (seq_rdata_valid_lat_inc),
        .seq_rdata_valid_lat_dec            (seq_rdata_valid_lat_dec),
        .ctl_rlat                           (ctl_rlat),
        .seq_poa_lat_dec_1x                 (seq_poa_lat_dec_1x),
        .seq_poa_lat_inc_1x                 (seq_poa_lat_inc_1x),
        .seq_poa_protection_override_1x     (seq_poa_protection_override_1x),
        .seq_oct_oct_delay                  (seq_oct_oct_delay),
        .seq_oct_oct_extend                 (seq_oct_oct_extend),
        .seq_oct_val                        (seq_oct_val),
        .seq_wdp_dqs_burst                  (seq_wdp_dqs_burst),
        .seq_wdp_wdata_valid                (seq_wdp_wdata_valid),
        .seq_wdp_wdata                      (seq_wdp_wdata),
        .seq_wdp_dm                         (seq_wdp_dm),
        .seq_wdp_dqs                        (seq_wdp_dqs),
        .seq_wdp_ovride                     (seq_wdp_ovride),
        .seq_dqs_add_2t_delay               (seq_dqs_add_2t_delay),
        .ctl_wlat                           (ctl_wlat),
        .seq_mmc_start                      (seq_mmc_start),
        .mmc_seq_done                       (mmc_seq_done),
        .mmc_seq_value                      (mmc_seq_value),
        .mem_err_out_n                      (mem_err_out_n),
        .parity_error_n                     (parity_error_n),
        .dbg_clk                            (dbg_clk),
        .dbg_reset_n                        (dbg_reset_n),
        .dbg_addr                           (dbg_addr),
        .dbg_wr                             (dbg_wr),
        .dbg_rd                             (dbg_rd),
        .dbg_cs                             (dbg_cs),
        .dbg_wr_data                        (dbg_wr_data),
        .dbg_rd_data                        (dbg_rd_data),
        .dbg_waitrequest                    (dbg_waitrequest)
    );



// Instance the postamble.  Note that there is one block per DQS group, this is
// because each group generates its own half-rate resync clock and may require
// different postamble treatment :
generate

    if (DWIDTH_RATIO == 4)
    begin: hr_poa_gen

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS ; dqs_grp_num=dqs_grp_num+1)
        begin : poa_group

            //
            ddr2_phy_alt_mem_phy_postamble #(
                             .FAMILY                         (FAMILY),
                 .MEM_IF_POSTAMBLE_EN_WIDTH      (8),
                 .POSTAMBLE_AWIDTH               (POSTAMBLE_AWIDTH),
                 .POSTAMBLE_HALFT_EN             (0),
                 .POSTAMBLE_INITIAL_LAT          (POSTAMBLE_INITIAL_LAT),
                 .POSTAMBLE_RESYNC_LAT_CTL_EN    (0),
                 .DWIDTH_RATIO                   (4)
            ) poa (
                 .phy_clk_1x                     (phy_clk_1x),
                 .resync_clk_1x                  (resync_clk_1x[dqs_grp_num]),
                 .reset_phy_clk_1x_n             (reset_rdp_phy_clk_1x_n[dqs_grp_num]),
                 .reset_resync_clk_1x_n          (reset_resync_clk_1x_n[dqs_grp_num]),
                 .seq_poa_lat_dec_1x             (seq_poa_lat_dec_1x[dqs_grp_num]),
                 .seq_poa_lat_inc_1x             (seq_poa_lat_inc_1x[dqs_grp_num]),
                 .seq_poa_protection_override_1x (seq_poa_protection_override_1x),
                 .ctl_doing_rd_beat              ({merged_doing_rd[dqs_grp_num + MEM_IF_DQS_WIDTH], merged_doing_rd[dqs_grp_num + 0]}),
                 .poa_postamble_en_preset        (poa_postamble_en_preset[((dqs_grp_num + 1) * (DWIDTH_RATIO/2) - 1) : ((dqs_grp_num)*(DWIDTH_RATIO/2))])
            );

        end

    end

    else
    begin: fr_poa_gen

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS ; dqs_grp_num=dqs_grp_num+1)
        begin : poa_group

            //
            ddr2_phy_alt_mem_phy_postamble #(
                             .FAMILY                         (FAMILY),
                 .MEM_IF_POSTAMBLE_EN_WIDTH      (8),
                 .POSTAMBLE_AWIDTH               (POSTAMBLE_AWIDTH),
                 .POSTAMBLE_HALFT_EN             (0),
                 .POSTAMBLE_INITIAL_LAT          (POSTAMBLE_INITIAL_LAT),
                 .POSTAMBLE_RESYNC_LAT_CTL_EN    (0),
                 .DWIDTH_RATIO                   (2)
            ) poa (
                 .phy_clk_1x                     (phy_clk_1x),
                 .resync_clk_1x                  (resync_clk_2x),
                 .reset_phy_clk_1x_n             (reset_rdp_phy_clk_1x_n[dqs_grp_num]),
                 .reset_resync_clk_1x_n          (reset_resync_clk_2x_n),
                 .seq_poa_lat_dec_1x             (seq_poa_lat_dec_1x[dqs_grp_num]),
                 .seq_poa_lat_inc_1x             (seq_poa_lat_inc_1x[dqs_grp_num]),
                 .seq_poa_protection_override_1x (seq_poa_protection_override_1x),
                 .ctl_doing_rd_beat              (merged_doing_rd[dqs_grp_num + 0]),
                 .poa_postamble_en_preset        (poa_postamble_en_preset[((dqs_grp_num + 1) * (DWIDTH_RATIO/2) - 1) : ((dqs_grp_num)*(DWIDTH_RATIO/2))])
            );

        end

    end
endgenerate


// Generate the input to the RDV delay.
// Also determine the data for the OCT control & postamble paths (merged_doing_rd)
generate
    if (DWIDTH_RATIO == 4)
    begin : merging_doing_rd_halfrate
        always @*
        begin
            merged_doing_rd = seq_rdv_doing_rd | (ctl_doing_rd & {(2 * MEM_IF_DQS_WIDTH) {ctl_cal_success}});
            rdv_pipe_ip[0]  = | merged_doing_rd[    MEM_IF_DQS_WIDTH - 1 : 0];
            rdv_pipe_ip[1]  = | merged_doing_rd[2 * MEM_IF_DQS_WIDTH - 1 : MEM_IF_DQS_WIDTH];
        end
    end
    else  // DWIDTH_RATIO == 2    NOTE THAT THIS IS NOT CURRENTLY SUPPORTED
    begin : merging_doing_rd_fullrate
        always @*
        begin
            merged_doing_rd = seq_rdv_doing_rd | (ctl_doing_rd & { MEM_IF_DQS_WIDTH {ctl_cal_success}});
            rdv_pipe_ip[0]  = | merged_doing_rd[MEM_IF_DQS_WIDTH - 1 : 0];
        end
    end // else: !if(DWIDTH_RATIO == 4)

endgenerate


// Generate rdata_valid for sequencer and controller
//
ddr2_phy_alt_mem_phy_rdata_valid #(
     .FAMILY                    (FAMILY),
     .MEM_IF_DQS_WIDTH          (MEM_IF_DQS_WIDTH),
     .RDATA_VALID_AWIDTH        (8),
     .RDATA_VALID_INITIAL_LAT   (RDV_INITIAL_LAT),
     .DWIDTH_RATIO              (DWIDTH_RATIO)
) rdv_pipe (
     .phy_clk_1x                (phy_clk_1x),
     .reset_phy_clk_1x_n        (reset_rdp_phy_clk_1x_n[0]),
     .seq_rdata_valid_lat_dec   (seq_rdata_valid_lat_dec),
     .seq_rdata_valid_lat_inc   (seq_rdata_valid_lat_inc),
     .seq_doing_rd              (seq_rdv_doing_rd),
     .ctl_doing_rd              (ctl_doing_rd),
     .ctl_cal_success           (ctl_cal_success),
     .ctl_rdata_valid           (ctl_rdata_valid),
     .seq_rdata_valid           (seq_rdata_valid)
);


//
ddr2_phy_alt_mem_phy_clk_reset #(
    .AC_PHASE                             ("MEM_CLK"),
    .CLOCK_INDEX_WIDTH                    (CLOCK_INDEX_WIDTH),
    .DDR_MIMIC_PATH_EN                    (DDR_MIMIC_PATH_EN),
    .DLL_EXPORT_IMPORT                    (DLL_EXPORT_IMPORT),
    .LOCAL_IF_CLK_PS                      (4000),
    .MEM_IF_CLK_PAIR_COUNT                (MEM_IF_CLK_PAIR_COUNT),
    .MEM_IF_CLK_PS                        (MEM_IF_CLK_PS),
    .MEM_IF_CLK_PS_STR                    (MEM_IF_CLK_PS_STR),
    .MEM_IF_DQ_PER_DQS                    (MEM_IF_DQ_PER_DQS),
    .MEM_IF_DWIDTH                        (MEM_IF_DWIDTH),
    .MEM_IF_DQS_WIDTH                     (MEM_IF_DQS_WIDTH),
    .MEM_IF_MEMTYPE                       (MEM_IF_MEMTYPE),
    .MEM_IF_DQSN_EN                       (MEM_IF_DQSN_EN),
    .MIF_FILENAME                         ("PLL.MIF"),
    .DWIDTH_RATIO                         (DWIDTH_RATIO),
    .PLL_EXPORT_IMPORT                    ("NONE"),
    .PLL_REF_CLK_PS                       (4000),
    .PLL_TYPE                             ("ENHANCED"),
    .SPEED_GRADE                          ("C3"),
    .DLL_DELAY_BUFFER_MODE                (DLL_DELAY_BUFFER_MODE),
    .DLL_DELAY_CHAIN_LENGTH               (DLL_DELAY_CHAIN_LENGTH),
    .DQS_OUT_MODE                         (DQS_OUT_MODE),
    .DQS_PHASE                            (DQS_PHASE),
    .SCAN_CLK_DIVIDE_BY                   (SCAN_CLK_DIVIDE_BY),
    .USE_MEM_CLK_FOR_ADDR_CMD_CLK         (1),
    .DQS_DELAY_CTL_WIDTH                  (DQS_DELAY_CTL_WIDTH),
    .USE_DLL_OFFSET                       (USE_DLL_OFFSET),
    .LEVELLING                            (LEVELLING),
    .READ_DESKEW_MODE                     (READ_DESKEW_MODE),
    .WRITE_DESKEW_MODE                    (WRITE_DESKEW_MODE),
    .PLL_RECONFIG_PORTS_EN                (PLL_RECONFIG_PORTS_EN)
) clk (
    .pll_ref_clk                          (pll_ref_clk),
    .global_reset_n                       (global_reset_n),
    .soft_reset_n                         (soft_reset_n),
    .seq_rdp_reset_req_n                  ({MEM_IF_DQS_WIDTH{seq_rdp_reset_req_n}}),
    .seq_qdr_doff_req_n                   (),
    .resync_clk_1x                        (resync_clk_1x),
    .ac_clk_1x                            (ac_clk_1x),
    .measure_clk_1x                       (measure_clk_1x),
    .mem_clk_2x                           (mem_clk_2x),
    .mem_clk                              (mem_clk),
    .mem_clk_n                            (mem_clk_n),
    .phy_clk_1x                           (phy_clk_1x_src),
    .postamble_clk_2x                     (postamble_clk_2x),
    .resync_clk_2x                        (resync_clk_2x),
    .cs_n_clk_1x                          (cs_n_clk_1x),
    .write_clk_2x                         (write_clk_2x),
    .aux_clk                              (aux_clk),
    .scan_clk                             (aux_scan_clk),
    .aux_scan_clk_reset_n                 (aux_scan_clk_reset_n),       // Connection New for 9.0
    .scan_clk_in                          (scan_ck[0]),
    .scan_enable_ck                       (scan_enable_ck),
    .scan_update                          (scan_update[0]),
    .scan_din                             (scan_din[0]),
    .reset_ac_clk_1x_n                    (reset_ac_clk_1x_n),
    .reset_measure_clk_1x_n               (reset_measure_clk_1x_n),
    .reset_mem_clk_2x_n                   (reset_mem_clk_2x_n),
    .reset_phy_clk_1x_n                   (reset_phy_clk_1x_n),
    .reset_rdp_phy_clk_1x_n               (reset_rdp_phy_clk_1x_n),
    .reset_resync_clk_1x_n                (reset_resync_clk_1x_n),
    .reset_resync_clk_2x_n                (reset_resync_clk_2x_n),
    .reset_write_clk_2x_n                 (),
    .reset_cs_n_clk_1x_n                  (reset_cs_n_clk_1x_n),
    .mem_reset_n                          (),
    .mem_doff_n                           (),
    .reset_request_n                      (reset_request_n),
    .dqs_delay_ctrl                       (dqs_delay_ctrl),
    .dqs_delay_ctrl_import                (dqs_delay_ctrl_import),
    .dqs_delay_update_en                  (dqs_delay_update_en),
    .dlloffset_addnsub                    (1'b0),
    .dlloffset_offset                     ({DQS_DELAY_CTL_WIDTH{1'b0}}),
    .dlloffset_offsetctrl_out             (dlloffset_offsetctrl_out),
    .phs_shft_busy                        (phs_shft_busy),
    .seq_pll_inc_dec_n                    (seq_pll_inc_dec_n),
    .seq_pll_select                       (seq_pll_select),
    .seq_pll_start_reconfig               (seq_pll_start_reconfig),
    .mimic_data_1x                        (mimic_data),
    .seq_clk_disable                      (seq_mem_clk_disable),
    .ctrl_clk_disable                     (ctl_mem_clk_disable),
    .pll_reconfig_enable                  (pll_reconfig_enable),       // Connection New for 9.0
    .pll_phasecounterselect               (pll_phasecounterselect),    // Connection New for 9.0
    .pll_phaseupdown                      (pll_phaseupdown),           // Connection New for 9.0
    .pll_phasestep                        (pll_phasestep),             // Connection New for 9.0
    .pll_phase_done                       (pll_phase_done)             // Connection New for 9.0
);



// Instance the mimic block :

//
ddr2_phy_alt_mem_phy_mimic #(
    .NUM_MIMIC_SAMPLE_CYCLES (NUM_MIMIC_SAMPLE_CYCLES)
) mmc (
    .measure_clk             (measure_clk_1x),
    .reset_measure_clk_n     (reset_measure_clk_1x_n),
    .mimic_data_in           (mimic_data),
    .seq_mmc_start           (seq_mmc_start),
    .mmc_seq_done            (mmc_seq_done),
    .mmc_seq_value           (mmc_seq_value)
);

// If required, instance the Mimic debug block.  If the debug block is used, a top level input
// for mimic_recapture_debug_data should be created.
generate

    if (MIMIC_DEBUG_EN == 1)
    begin : create_mimic_debug_ram

        //
        ddr2_phy_alt_mem_phy_mimic_debug #(
                    .NUM_DEBUG_SAMPLES_TO_STORE (NUM_DEBUG_SAMPLES_TO_STORE),
            .PLL_STEPS_PER_CYCLE        (PLL_STEPS_PER_CYCLE)
        ) mmc_debug (
            .measure_clk                (measure_clk_1x),
            .reset_measure_clk_n        (reset_measure_clk_1x_n),
            .mmc_seq_done               (mmc_seq_done),
            .mmc_seq_value              (mmc_seq_value),
            .mimic_recapture_debug_data (1'b0)
        );

    end

endgenerate



endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_oct_delay ( // inputs
                               phy_clk_1x,
                               reset_phy_clk_1x_n,

                               oct_lat,
                               oct_extend_duration,
                               ctl_doing_rd,

                               // outputs
                               oct_out
                              );

parameter FAMILY              = "STRATIXIII";    // why needed ??
parameter OCT_LAT_WIDTH       = 5;               // note hardcoded max lat of 32 mem_clk
parameter DWIDTH_RATIO        = 4;               // scales output & input widths..


// clocks
input  wire                           phy_clk_1x;

// resets
input  wire                           reset_phy_clk_1x_n;

// control signals from sequencer
input wire [OCT_LAT_WIDTH    - 1 : 0] oct_lat;
input wire [OCT_LAT_WIDTH    - 1 : 0] oct_extend_duration;


input wire [DWIDTH_RATIO / 2 - 1 : 0] ctl_doing_rd;

// output to IOE for SIII :
output reg [DWIDTH_RATIO / 2 - 1 : 0] oct_out;




localparam DELAY_ARRAY_SIZE  = 32;
localparam MAX_EXTENSION     = 14;
localparam EXTEND_ARRAY_SIZE = MAX_EXTENSION + (DWIDTH_RATIO / 2);

// internal wires/regs       // these should probably be made 2D arrays for readability...
reg  [DELAY_ARRAY_SIZE   - 1 : 0]  delay_array;
reg  [EXTEND_ARRAY_SIZE  - 1 : 0]  extend_array;
reg  [(DWIDTH_RATIO / 2) - 1 : 0]  extended_value;
wire [(DWIDTH_RATIO / 2) - 1 : 0]  pre_extended_value;

reg  [EXTEND_ARRAY_SIZE - 1 : 0]   pre_extend_array;
reg  [MAX_EXTENSION         : 0]   extend_using_this_bit_mask;

wire  [MAX_EXTENSION : 0]   dwidth_scaled_pre_extend_array [(DWIDTH_RATIO / 2) - 1 : 0];


integer i;
integer j;
integer k;
integer l;


always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n )
begin
    if (reset_phy_clk_1x_n == 1'b0)
    begin
        extend_using_this_bit_mask <= 0;
    end
    else
    begin
        for (i = 0; i < MAX_EXTENSION + 1; i = i + 1)
        begin
            if (i > oct_extend_duration)
                extend_using_this_bit_mask[i] <= 1'b0;
            else
                extend_using_this_bit_mask[i] <= 1'b1;
        end
    end
end


// pre-extend array is re-ordered to allow this to be more easily visibly to humans.
always @*
begin
     pre_extend_array[EXTEND_ARRAY_SIZE - 1 : (DWIDTH_RATIO / 2)] = extend_array[ EXTEND_ARRAY_SIZE - (DWIDTH_RATIO / 2) - 1: 0] ;
     for (j=0; j < DWIDTH_RATIO / 2; j = j+1)
     begin
         pre_extend_array[j] = ctl_doing_rd[  (DWIDTH_RATIO / 2) -1 -j];
     end
end

reg  [DELAY_ARRAY_SIZE   - 1 : 0]  delay_array_temp;

// generate registered version
always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n )
begin
    if (reset_phy_clk_1x_n == 1'b0)
    begin
        delay_array  <= {DELAY_ARRAY_SIZE{1'b0}};
        extend_array <= {EXTEND_ARRAY_SIZE{1'b0}};
        oct_out <= {(DWIDTH_RATIO/2){1'b0}};
    end
    else
    begin
        delay_array_temp  = {delay_array[  DELAY_ARRAY_SIZE  - (DWIDTH_RATIO / 2) - 1 : 0], extended_value};
        extend_array <= pre_extend_array;
        delay_array  <= delay_array_temp;

        if (oct_lat < (DELAY_ARRAY_SIZE - (DWIDTH_RATIO / 2)) )
        begin
            for (l = 0; l < (DWIDTH_RATIO / 2); l = l + 1)
            begin
                oct_out[(DWIDTH_RATIO / 2) -1 -l] <= delay_array_temp[oct_lat + l];
            end
        end
        else
            oct_out <= 0;
    end
end


// generate different array which can be anded to give a per bit result -- this is becasue RHS sweeping based on bits won't work !!
generate
    genvar bits;
    for (bits = 0; bits < (DWIDTH_RATIO / 2); bits = bits + 1)
    begin : generate_arrays
        assign dwidth_scaled_pre_extend_array[bits] = pre_extend_array[ MAX_EXTENSION + bits : bits];
        assign pre_extended_value[bits] =  | (extend_using_this_bit_mask & dwidth_scaled_pre_extend_array[bits]);
    end
endgenerate


//for per bit in extended_vale use array (generated above) take result of bitwise and for all bits in vecotr (length of extension) and or result together(real lenght extension to 1 bit)
always @(extended_value or pre_extended_value)
begin
    if (extended_value < MAX_EXTENSION +1)
    begin
        for (k = 0; k < (DWIDTH_RATIO / 2); k = k + 1)
        begin
            extended_value [k] =  pre_extended_value[k];
        end
    end
    else
        extended_value = 0;
end

endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_ac (
                            ac_clk_1x,
                            phy_clk_1x,
                            reset_phy_clk_1x_n,
                            reset_ac_clk_1x_n,
                            ctl_add_1t_ac_lat,
                            ctl_add_intermediate_regs,
                            seq_ac_sel,
                            ctl_ac_h,
                            ctl_ac_l,
                            seq_ac_h,
                            seq_ac_l,
                            mem_ac );

parameter POWER_UP_HIGH   	   =	   1;
parameter MEM_IF_MEMTYPE  	   =   "DDR";
parameter DWIDTH_RATIO    	   =	   4;
parameter INVERT_ADDR_CMD_TXFR_CLK = "false";

input wire          ac_clk_1x;
input wire          phy_clk_1x;
input wire          reset_phy_clk_1x_n;
input wire          reset_ac_clk_1x_n;

input wire          ctl_add_1t_ac_lat;
input wire          ctl_add_intermediate_regs;
input wire          seq_ac_sel;

input wire          ctl_ac_h;
input wire          ctl_ac_l;

input wire          seq_ac_h;
input wire          seq_ac_l;

output wire         mem_ac;

reg                 ac_h_r          = POWER_UP_HIGH[0];
reg                 ac_l_r          = POWER_UP_HIGH[0];
reg                 ac_del_1x       = POWER_UP_HIGH[0];

reg                 ac_h_r_alt      = POWER_UP_HIGH[0];
reg                 ac_l_r_alt      = POWER_UP_HIGH[0];
reg                 ac_del_1x_alt   = POWER_UP_HIGH[0];

reg                 ac_h_ddio_alt_r = POWER_UP_HIGH[0];
reg                 ac_l_ddio_alt_r = POWER_UP_HIGH[0];

reg                 ac_l;
reg                 ac_h;

reg                 ac_l_ddio;
reg                 ac_h_ddio;
reg                 ac_l_ddio_r;

reg                 ac_l_ddio_alt;
reg                 ac_h_ddio_alt;

wire                reset_1x ;

reg                 ac_h_ddio_mux_op;
reg                 ac_l_ddio_datainlo;

reg                 ctl_ac_l_r      = POWER_UP_HIGH[0];
reg                 ctl_ac_h_r      = POWER_UP_HIGH[0];
reg                 seq_ac_l_r      = POWER_UP_HIGH[0];
reg                 seq_ac_h_r      = POWER_UP_HIGH[0];

// Active sense is high on atoms :
wire                reset_ac_clk_1x;

assign reset_ac_clk_1x  = ~reset_ac_clk_1x_n;


generate

    if (MEM_IF_MEMTYPE != "DDR3")
    begin : delay_ac

        always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
        begin

            if (reset_phy_clk_1x_n == 1'b0)
            begin
                ctl_ac_l_r <= POWER_UP_HIGH[0];
                ctl_ac_h_r <= POWER_UP_HIGH[0];
                seq_ac_l_r <= POWER_UP_HIGH[0];
                seq_ac_h_r <= POWER_UP_HIGH[0];
            end

            else
            begin
                ctl_ac_l_r <= ctl_ac_l;
                ctl_ac_h_r <= ctl_ac_h;
                seq_ac_l_r <= seq_ac_l;
                seq_ac_h_r <= seq_ac_h;
            end

        end //always

    end

    else
    begin : ac_passthru

        always @*
        begin

            ctl_ac_l_r = ctl_ac_l;
            ctl_ac_h_r = ctl_ac_h;


            seq_ac_l_r = seq_ac_l;
            seq_ac_h_r = seq_ac_h;

        end //always

    end

endgenerate

// Select between the sequencer inputs and the controller's :
generate

    // QDR memory types :
    if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdr_ac_mux_gen

        always @*
        begin

            casez(seq_ac_sel)

            1'b0 :
            begin
                ac_l = ctl_ac_l;
                ac_h = ctl_ac_h;
            end

            1'b1 :
            begin
                ac_l = seq_ac_l;
                ac_h = seq_ac_h;
            end

            endcase

        end //always

    end

    else
    begin : ddr_ac_mux_gen

        if (DWIDTH_RATIO == 4) //HR
        begin : hr_mux_gen

            // Half Rate DDR memory types require an extra cycle of latency :
            always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
            begin

                if (reset_phy_clk_1x_n == 1'b0)
                begin
                    ac_l <= POWER_UP_HIGH[0];
                    ac_h <= POWER_UP_HIGH[0];
                end

                else
                begin

                    casez(seq_ac_sel)

                    1'b0 :
                    begin
                        ac_l <= ctl_ac_l_r;
                        ac_h <= ctl_ac_h_r;
                    end

                    1'b1 :
                    begin
                        ac_l <= seq_ac_l_r;
                        ac_h <= seq_ac_h_r;
                    end

                    endcase

                end

            end //always

        end

        else // FR
        begin : fr_passthru_gen

            // Note that "_h" is unused in full-rate and no latency
            // is required :
            always @*
            begin

                casez(seq_ac_sel)

                1'b0:
                begin
                    ac_l = ctl_ac_l;
                end

                1'b1:
                begin
                    ac_l = seq_ac_l;
                end

                endcase

            end
        end

    end

endgenerate

generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate

        // Registering of inputs :
        always @(posedge phy_clk_1x)
        begin
            ac_h_r    <= ac_h;
            ac_l_r    <= ac_l;
            ac_del_1x <= ac_l_r;
        end

        // Create alternative negedge registered versions
        always @(negedge phy_clk_1x)
        begin
            ac_h_r_alt    <= ac_h;
            ac_l_r_alt    <= ac_l;
            ac_del_1x_alt <= ac_l_r_alt;
        end


        // Determine whether to add a cycle latency :
        always @*
        begin

            casez(ctl_add_1t_ac_lat)

            1'b0     :
            begin

                ac_l_ddio = ac_l_r;
                ac_h_ddio = ac_h_r;

                ac_l_ddio_alt = ac_l_r_alt;
                ac_h_ddio_alt = ac_h_r_alt;

            end

            1'b1     :
            begin

                ac_l_ddio = ac_h_r;
                ac_h_ddio = ac_del_1x;

                ac_l_ddio_alt = ac_h_r_alt;
                ac_h_ddio_alt = ac_del_1x_alt;

            end

            // X propagation :
            default  :
            begin

                ac_l_ddio = 1'bx;
                ac_h_ddio = 1'bx;

                ac_l_ddio_alt = 1'bx;
                ac_h_ddio_alt = 1'bx;

            end

            endcase

        end


        // If using 'alt' signals, transfer to ac_clk in the core :
        always @(posedge phy_clk_1x)
        begin
            ac_l_ddio_alt_r <= ac_l_ddio_alt;
            ac_h_ddio_alt_r <= ac_h_ddio_alt;
        end


        // Nb. Quartus shall either remove the "alt" logic, or it's posedge equivalent
        // based upon the setting of ctl_add_intermediate_regs :

        always @*
        begin

            if (ctl_add_intermediate_regs == 1'b0)
            begin
                ac_h_ddio_mux_op = ac_h_ddio;
                ac_l_ddio_datainlo = ac_l_ddio;
            end

            else
            begin
                ac_h_ddio_mux_op = ac_h_ddio_alt_r;
                ac_l_ddio_datainlo = ac_l_ddio_alt_r;
            end

        end

        if (POWER_UP_HIGH == 1)
        begin : ac_power_up_high

            stratixiii_ddio_out #(
                .async_mode("preset"),
                .power_up("high"),
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            ) addr_pin(
                .datainlo (ac_l_ddio_datainlo),
                .datainhi (ac_h_ddio_mux_op),
                .clkhi    (ac_clk_1x),
                .clklo    (ac_clk_1x),
                .muxsel   (ac_clk_1x),
                .ena      (1'b1),
                .areset   (reset_ac_clk_1x),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (mem_ac),
                .devclrn (),
                .devpor ()
            );

        end

        else
        begin : ac_power_up_low

            stratixiii_ddio_out # (
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            ) addr_pin (
                .datainlo (ac_l_ddio_datainlo),
                .datainhi (ac_h_ddio_mux_op),
                .clkhi    (ac_clk_1x),
                .clklo    (ac_clk_1x),
                .muxsel   (ac_clk_1x),
                .ena      (1'b1),
                .areset   (reset_ac_clk_1x),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (mem_ac),
                .devclrn (),
                .devpor ()
            );

        end

    end // Half-rate

    // Full-rate
    
    // In full-rate modes, some addr/cmd phases require an inverted phy_clk here to make the transfer 
    // successfully.  If core to addr/cmd clock timing is failing, then it may be that an incorrect 
    // default for this parameter was selected by the GUI.
    
    // Tests have shown that for addr/cmd phases<270 degrees it should be inverted, otherwise it 
    // should not but this does vary dependant on frequency and loading, so for middling phases 
    // from 240 to 270 this parameter may need inverting.
    
    else if (INVERT_ADDR_CMD_TXFR_CLK == "true")
    begin : full_rate_inverted_clk

        // Inverted PHY transfer clock :
        always @(negedge phy_clk_1x)
        begin
            // Registering of inputs :
            // 1t registering - only used if ctl_add_1t_ac_lat is true
            ac_l_r <= ac_l;
            // Determine whether to add a cycle latency :
            // add 1 addr_clock delay if "Add 1T" is set:
            if ( ctl_add_intermediate_regs == 1'b1)
                ac_l_ddio <= ac_l_r;
            else
                ac_l_ddio <= ac_l;

        end

        // Inverted PHY transfer clock :
        always @(negedge phy_clk_1x)
        begin
            ac_l_ddio_r <= ac_l_ddio;
        end

        if (POWER_UP_HIGH == 1)
        begin

            stratixiii_ddio_out #(
                .async_mode("preset"),
                .power_up("high"),
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            ) addr_pin(
                .datainhi (ac_l_ddio_r),
                .datainlo (ac_l_ddio),
                .clkhi    (ac_clk_1x),
                .clklo    (ac_clk_1x),
                .muxsel   (ac_clk_1x),
                .ena      (1'b1),
                .areset   (reset_ac_clk_1x),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
		.clk(),
                // synopsys translate_on
                .dataout (mem_ac),
                .devclrn (),
                .devpor ()
            );

        end

        else
        begin

            stratixiii_ddio_out # (
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            ) addr_pin (
                .datainhi (ac_l_ddio_r),
                .datainlo (ac_l_ddio),
                .clkhi    (ac_clk_1x),
                .clklo    (ac_clk_1x),
                .muxsel   (ac_clk_1x),
                .ena      (1'b1),
                .areset   (reset_ac_clk_1x),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
		.clk(),
                // synopsys translate_on
                .dataout (mem_ac),
                .devclrn (),
                .devpor ()
            );
        end // else: !if(POWER_UP_HIGH == 1)

    end
    
    
    else // the non-inverted case :
    begin : full_rate_non_inverted_clk 

        // Non-Inverted PHY transfer clock :
        always @(posedge phy_clk_1x)
        begin
            // Registering of inputs :
            // 1t registering - only used if ctl_add_1t_ac_lat is true
            ac_l_r <= ac_l;
            // Determine whether to add a cycle latency :
            // add 1 addr_clock delay if "Add 1T" is set:
            if ( ctl_add_intermediate_regs == 1'b1)
                ac_l_ddio <= ac_l_r;
            else
                ac_l_ddio <= ac_l;

        end

        // Non-Inverted PHY transfer clock :
        always @(posedge phy_clk_1x)
        begin
            ac_l_ddio_r <= ac_l_ddio;
        end

        if (POWER_UP_HIGH == 1)
        begin

            stratixiii_ddio_out #(
                .async_mode("preset"),
                .power_up("high"),
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            ) addr_pin(
                .datainhi (ac_l_ddio_r),
                .datainlo (ac_l_ddio),
                .clkhi    (ac_clk_1x),
                .clklo    (ac_clk_1x),
                .muxsel   (ac_clk_1x),
                .ena      (1'b1),
                .areset   (reset_ac_clk_1x),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
		.clk(),
                // synopsys translate_on
                .dataout (mem_ac),
                .devclrn (),
                .devpor ()
            );

        end

        else
        begin

            stratixiii_ddio_out # (
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            ) addr_pin (
                .datainhi (ac_l_ddio_r),
                .datainlo (ac_l_ddio),
                .clkhi    (ac_clk_1x),
                .clklo    (ac_clk_1x),
                .muxsel   (ac_clk_1x),
                .ena      (1'b1),
                .areset   (reset_ac_clk_1x),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
		.clk(),
                // synopsys translate_on
                .dataout (mem_ac),
                .devclrn (),
                .devpor ()
            );
        end // else: !if(POWER_UP_HIGH == 1)
    end // block: full_rate

endgenerate

endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_addr_cmd (
                                  ac_clk_1x,
                                  cs_n_clk_1x,
                                  phy_clk_1x,
                                  reset_phy_clk_1x_n,
                                  reset_ac_clk_1x_n,

                                  // Usual Addr/cmd interface from controller :
                                  ctl_add_1t_ac_lat,
                                  ctl_add_1t_odt_lat,
                                  ctl_add_intermediate_regs,

                                  ctl_negedge_en,
                                  ctl_mem_addr_h,
                                  ctl_mem_addr_l,
                                  ctl_mem_ba_h,
                                  ctl_mem_ba_l,
                                  ctl_mem_cas_n_h,
                                  ctl_mem_cas_n_l,
                                  ctl_mem_cke_h,
                                  ctl_mem_cke_l,
                                  ctl_mem_cs_n_h,
                                  ctl_mem_cs_n_l,
                                  ctl_mem_odt_h,
                                  ctl_mem_odt_l,
                                  ctl_mem_ras_n_h,
                                  ctl_mem_ras_n_l,
                                  ctl_mem_we_n_h,
                                  ctl_mem_we_n_l,

                                  // DDR3 Signals
                                  ctl_mem_rst_n_h,
                                  ctl_mem_rst_n_l,

                                  //QDRII signals :
                                  ctl_mem_wps_n,
                                  ctl_mem_rps_n,

                                  // Interface from Sequencer, used for DDR3 calibration
                                  // as the MRS registers need to be controlled :
                                  seq_addr_h,
                                  seq_addr_l,
                                  seq_ba_h,
                                  seq_ba_l,
                                  seq_cas_n_h,
                                  seq_cas_n_l,
                                  seq_cke_h,
                                  seq_cke_l,
                                  seq_cs_n_h,
                                  seq_cs_n_l,
                                  seq_odt_h,
                                  seq_odt_l,
                                  seq_ras_n_h,
                                  seq_ras_n_l,
                                  seq_we_n_h,
                                  seq_we_n_l,

                                  // DDR3 Signals
                                  seq_mem_rst_n_h,
                                  seq_mem_rst_n_l,

                                  seq_ac_sel,

                                  seq_wps_n,
                                  seq_rps_n,

                                  mem_ac_parity,
                                  mem_addr,
                                  mem_ba,
                                  mem_cas_n,
                                  mem_cke,
                                  mem_cs_n,
                                  mem_odt,
                                  mem_ras_n,
                                  mem_we_n,
                                  mem_rst_n,

                                  mem_rps_n,
                                  mem_wps_n
                                  );


parameter FAMILY                   = "Stratix III";
parameter MEM_ADDR_CMD_BUS_COUNT   =	  	 1;
parameter MEM_IF_BANKADDR_WIDTH    =	  	 3;
parameter MEM_IF_CS_WIDTH          =	  	 2;
parameter MEM_IF_NUM_RANKS         =    	 2;
parameter MEM_IF_MEMTYPE           =	     "DDR";
parameter MEM_IF_ROWADDR_WIDTH     =	  	13;
parameter DWIDTH_RATIO             =	  	 4;
parameter INVERT_ADDR_CMD_TXFR_CLK =       "false";
parameter LEVELLING                =       0;
parameter CHIP_OR_DIMM             = "Unregistered DIMM";

input wire                                  cs_n_clk_1x;
input wire                                  ac_clk_1x;
input wire                                  phy_clk_1x;

input wire                                  reset_phy_clk_1x_n;
input wire                                  reset_ac_clk_1x_n;

input wire [MEM_IF_ROWADDR_WIDTH -1:0]      ctl_mem_addr_h;
input wire [MEM_IF_ROWADDR_WIDTH -1:0]      ctl_mem_addr_l;
input wire                                  ctl_add_1t_ac_lat;
input wire                                  ctl_add_1t_odt_lat;
input wire                                  ctl_add_intermediate_regs;
input wire                                  ctl_negedge_en;
input wire [MEM_IF_BANKADDR_WIDTH - 1:0]    ctl_mem_ba_h;
input wire [MEM_IF_BANKADDR_WIDTH - 1:0]    ctl_mem_ba_l;
input wire                                  ctl_mem_cas_n_h;
input wire                                  ctl_mem_cas_n_l;
input wire [MEM_IF_NUM_RANKS - 1:0]         ctl_mem_cke_h;
input wire [MEM_IF_NUM_RANKS - 1:0]         ctl_mem_cke_l;
input wire [MEM_IF_NUM_RANKS - 1:0]         ctl_mem_cs_n_h;
input wire [MEM_IF_NUM_RANKS - 1:0]         ctl_mem_cs_n_l;
input wire [MEM_IF_NUM_RANKS - 1:0]         ctl_mem_odt_h;
input wire [MEM_IF_NUM_RANKS - 1:0]         ctl_mem_odt_l;
input wire                                  ctl_mem_ras_n_h;
input wire                                  ctl_mem_ras_n_l;
input wire                                  ctl_mem_we_n_h;
input wire                                  ctl_mem_we_n_l;

input wire                                   ctl_mem_rst_n_h;
input wire                                   ctl_mem_rst_n_l;

// QDRII has seperate read and write addresses that are multiplexed
// onto the ctl_mem_addr output :
input wire [MEM_IF_NUM_RANKS -1:0]           ctl_mem_wps_n;
input wire [MEM_IF_NUM_RANKS -1:0]           ctl_mem_rps_n;

input wire [MEM_IF_ROWADDR_WIDTH -1:0]      seq_addr_h;
input wire [MEM_IF_ROWADDR_WIDTH -1:0]      seq_addr_l;
input wire [MEM_IF_BANKADDR_WIDTH - 1:0]    seq_ba_h;
input wire [MEM_IF_BANKADDR_WIDTH - 1:0]    seq_ba_l;
input wire                                  seq_cas_n_h;
input wire                                  seq_cas_n_l;
input wire [MEM_IF_NUM_RANKS - 1:0]          seq_cke_h;
input wire [MEM_IF_NUM_RANKS - 1:0]          seq_cke_l;
input wire [MEM_IF_CS_WIDTH - 1:0]          seq_cs_n_h;
input wire [MEM_IF_CS_WIDTH - 1:0]          seq_cs_n_l;
input wire [MEM_IF_NUM_RANKS - 1:0]          seq_odt_h;
input wire [MEM_IF_NUM_RANKS - 1:0]          seq_odt_l;
input wire                                  seq_ras_n_h;
input wire                                  seq_ras_n_l;
input wire                                  seq_we_n_h;
input wire                                  seq_we_n_l;

input wire                                  seq_mem_rst_n_h;
input wire                                  seq_mem_rst_n_l;

input wire                                  seq_ac_sel;

// Sequencer QDR signals :
input wire [MEM_IF_NUM_RANKS -1:0]           seq_wps_n;
input wire [MEM_IF_NUM_RANKS -1:0]           seq_rps_n;

output wire                                 mem_ac_parity;
output wire [MEM_IF_ROWADDR_WIDTH - 1 : 0]  mem_addr;
output wire [MEM_IF_BANKADDR_WIDTH - 1 : 0] mem_ba;
output wire                                 mem_cas_n;
output wire [MEM_IF_NUM_RANKS - 1 : 0]       mem_cke;
output wire [MEM_IF_CS_WIDTH - 1 : 0]       mem_cs_n;
output wire [MEM_IF_NUM_RANKS - 1 : 0]       mem_odt;
output wire                                 mem_ras_n;
output wire                                 mem_we_n;
output wire                                 mem_rst_n;

output wire [MEM_IF_NUM_RANKS - 1 : 0]       mem_rps_n;
output wire [MEM_IF_NUM_RANKS - 1 : 0]       mem_wps_n;

genvar ia;
genvar ib;
genvar ic;
genvar id;
genvar ie;

// Create the ADDR I/O structure :
generate

    // QDR memory types :
    if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdrii

        // The address is multiplexed read/write addresses :
        for (ia=0; ia<MEM_IF_ROWADDR_WIDTH; ia=ia+1)
        begin : addr

        //
        ddr2_phy_alt_mem_phy_ac # (
                    .POWER_UP_HIGH   	       (0),
            .MEM_IF_MEMTYPE  	       (MEM_IF_MEMTYPE),
            .DWIDTH_RATIO    	       (DWIDTH_RATIO),
            .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
        ) addr_struct (
            .ac_clk_1x                 (ac_clk_1x),
            .phy_clk_1x                (phy_clk_1x),
            .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
            .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
            .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
            .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
            .seq_ac_sel                (seq_ac_sel),
            .ctl_ac_h                  (ctl_mem_addr_h[ia]), // NB. Write == High
            .ctl_ac_l                  (ctl_mem_addr_l[ia]),
            .seq_ac_h                  (seq_addr_h[ia]),
            .seq_ac_l                  (seq_addr_l[ia]),
            .mem_ac                    (mem_addr[ia])
        );

        end

        // Create the WPS_N I/O structure:
        for (ib=0; ib<MEM_IF_CS_WIDTH; ib=ib+1)
        begin : wps_n

            //
            ddr2_phy_alt_mem_phy_ac # (
                            .POWER_UP_HIGH   	   (1),
                .MEM_IF_MEMTYPE  	   (MEM_IF_MEMTYPE),
                .DWIDTH_RATIO    	   (DWIDTH_RATIO),
                .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
            ) wps_n_struct (
                .ac_clk_1x                 (ac_clk_1x),
                .phy_clk_1x                (phy_clk_1x),
                .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
                .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
                .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
                .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
                .seq_ac_sel                (seq_ac_sel),
                .ctl_ac_h                  (ctl_mem_wps_n[ib]),
                .ctl_ac_l                  (1'b1),
                .seq_ac_h                  (seq_wps_n[ib]),
                .seq_ac_l                  (1'b1),
                .mem_ac                    (mem_wps_n[ib])
        );

        end

        // Create the RPS_N I/O structure:
        for (ic=0; ic<MEM_IF_CS_WIDTH; ic=ic+1)
        begin : rps_n

            //
            ddr2_phy_alt_mem_phy_ac # (
                            .POWER_UP_HIGH   	   (1),
                .MEM_IF_MEMTYPE  	   (MEM_IF_MEMTYPE),
                .DWIDTH_RATIO     	   (DWIDTH_RATIO),     
                .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
            ) rps_n_struct (
                .ac_clk_1x                 (ac_clk_1x),
                .phy_clk_1x                (phy_clk_1x),
                .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
                .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
                .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
                .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
                .seq_ac_sel                (seq_ac_sel),
                .ctl_ac_h                  (1'b1),
                .ctl_ac_l                  (ctl_mem_rps_n[ic]),
                .seq_ac_h                  (1'b1),
                .seq_ac_l                  (seq_rps_n[ic]),
                .mem_ac                    (mem_rps_n[ic])
        );

        end

    end // QDRII



    // DDR memory types :
    else
    begin : ddr


        for (ia=0; ia<MEM_IF_ROWADDR_WIDTH; ia=ia+1)
        begin : addr

            //
            ddr2_phy_alt_mem_phy_ac # (
                            .POWER_UP_HIGH   	   (0), 	        
                .MEM_IF_MEMTYPE  	   (MEM_IF_MEMTYPE),    
                .DWIDTH_RATIO      	   (DWIDTH_RATIO),      
                .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
            ) addr_struct (
                .ac_clk_1x                 (ac_clk_1x),
                .phy_clk_1x                (phy_clk_1x),
                .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
                .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
                .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
                .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
                .seq_ac_sel                (seq_ac_sel),
                .ctl_ac_h                  (ctl_mem_addr_h[ia]),
                .ctl_ac_l                  (ctl_mem_addr_l[ia]),
                .seq_ac_h                  (seq_addr_h[ia]),
                .seq_ac_l                  (seq_addr_l[ia]),
                .mem_ac                    (mem_addr[ia])
            );

        end


        // Create the BANK_ADDR I/O structure :
        for (ib=0; ib<MEM_IF_BANKADDR_WIDTH; ib=ib+1)
        begin : ba

            //
            ddr2_phy_alt_mem_phy_ac # (
                            .POWER_UP_HIGH   	   (0),
                .MEM_IF_MEMTYPE  	   (MEM_IF_MEMTYPE),
                .DWIDTH_RATIO    	   (DWIDTH_RATIO),	 
                .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
            ) ba_struct (
                .ac_clk_1x                 (ac_clk_1x),
                .phy_clk_1x                (phy_clk_1x),
                .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
                .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
                .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
                .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
                .seq_ac_sel                (seq_ac_sel),
                .ctl_ac_h                  (ctl_mem_ba_h[ib]),
                .ctl_ac_l                  (ctl_mem_ba_l[ib]),
                .seq_ac_h                  (seq_ba_h[ib]),
                .seq_ac_l                  (seq_ba_l[ib]),
                .mem_ac                    (mem_ba[ib])
            );

        end

        // Create the CAS_N I/O structure :

        //
        ddr2_phy_alt_mem_phy_ac # (
                    .POWER_UP_HIGH   	       (1),		 
            .MEM_IF_MEMTYPE  	       (MEM_IF_MEMTYPE), 
            .DWIDTH_RATIO    	       (DWIDTH_RATIO),   
            .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
        ) cas_n_struct (
            .ac_clk_1x                 (ac_clk_1x),
            .phy_clk_1x                (phy_clk_1x),
            .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
            .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
            .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
            .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
            .seq_ac_sel                (seq_ac_sel),
            .ctl_ac_h                  (ctl_mem_cas_n_h),
            .ctl_ac_l                  (ctl_mem_cas_n_l),
            .seq_ac_h                  (seq_cas_n_h),
            .seq_ac_l                  (seq_cas_n_l),
            .mem_ac                    (mem_cas_n)
        );

        // Create the CKE I/O structure:
        for (ic=0; ic<MEM_IF_NUM_RANKS; ic=ic+1)
        begin : cke

            //
            ddr2_phy_alt_mem_phy_ac # (
                            .POWER_UP_HIGH   	   (0),
                .MEM_IF_MEMTYPE  	   (MEM_IF_MEMTYPE),
                .DWIDTH_RATIO    	   (DWIDTH_RATIO),     
                .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
            ) cke_struct (
                .ac_clk_1x                 (ac_clk_1x),
                .phy_clk_1x                (phy_clk_1x),
                .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
                .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
                .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
                .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
                .seq_ac_sel                (seq_ac_sel),
                .ctl_ac_h                  (ctl_mem_cke_h[ic]),
                .ctl_ac_l                  (ctl_mem_cke_l[ic]),
                .seq_ac_h                  (seq_cke_h[ic]),
                .seq_ac_l                  (seq_cke_l[ic]),
                .mem_ac                    (mem_cke[ic])
        );

        end

        // Create the CS_N I/O structure.  Note that the 2x clock is different.
        
        // In RDIMMs, single-rank DIMMs has 2 chip selects but only one rank. The high-order bit is only used to configure the
        // register but otherwise is unused. Lock any unused CS lines to high.
        wire [MEM_IF_CS_WIDTH-1:0] ctl_mem_cs_n_h_padded = {{MEM_IF_CS_WIDTH-MEM_IF_NUM_RANKS{1'b1}}, ctl_mem_cs_n_h};
        wire [MEM_IF_CS_WIDTH-1:0] ctl_mem_cs_n_l_padded = {{MEM_IF_CS_WIDTH-MEM_IF_NUM_RANKS{1'b1}}, ctl_mem_cs_n_l};
        for (id=0; id<MEM_IF_CS_WIDTH; id=id+1)
        begin : cs_n

            //
            ddr2_phy_alt_mem_phy_ac # (
                            .POWER_UP_HIGH  	   (1),
                .MEM_IF_MEMTYPE 	   (MEM_IF_MEMTYPE),
                .DWIDTH_RATIO   	   (DWIDTH_RATIO),
                .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
            ) cs_n_struct (
                .ac_clk_1x                 (ac_clk_1x),
                .phy_clk_1x                (phy_clk_1x),
                .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
                .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
                .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
                .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
                .seq_ac_sel                (seq_ac_sel),
                .ctl_ac_h                  (ctl_mem_cs_n_h_padded[id]),
                .ctl_ac_l                  (ctl_mem_cs_n_l_padded[id]),
                .seq_ac_h                  (seq_cs_n_h[id]),
                .seq_ac_l                  (seq_cs_n_l[id]),
                .mem_ac                    (mem_cs_n[id])
            );

        end

        // Create the ODT I/O structure for DDR2 and 3 only :
        if (MEM_IF_MEMTYPE != "DDR")
	begin : gen_odt
	
            for (ie=0; ie<MEM_IF_NUM_RANKS; ie=ie+1)
            begin : odt

            	//
            	ddr2_phy_alt_mem_phy_ac # (
            	            	    .POWER_UP_HIGH  	       (0),
            	    .MEM_IF_MEMTYPE 	       (MEM_IF_MEMTYPE),
            	    .DWIDTH_RATIO   	       (DWIDTH_RATIO),
                    .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
            	)odt_struct (
            	    .ac_clk_1x  	       (ac_clk_1x),
            	    .phy_clk_1x 	       (phy_clk_1x),
                    .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
                    .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
            	    .ctl_add_1t_ac_lat         (ctl_add_1t_odt_lat),
            	    .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
            	    .seq_ac_sel 	       (seq_ac_sel),
            	    .ctl_ac_h		       (ctl_mem_odt_h[ie]),
            	    .ctl_ac_l		       (ctl_mem_odt_l[ie]),
            	    .seq_ac_h		       (seq_odt_h[ie]),
            	    .seq_ac_l		       (seq_odt_l[ie]),
            	    .mem_ac		       (mem_odt[ie])
            	);

            end
	
	end

        // Create the RAS_N I/O structure :

        //
        ddr2_phy_alt_mem_phy_ac # (
                    .POWER_UP_HIGH  	       (1),
            .MEM_IF_MEMTYPE 	       (MEM_IF_MEMTYPE),
            .DWIDTH_RATIO   	       (DWIDTH_RATIO),
            .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
        ) ras_n_struct (
            .ac_clk_1x                 (ac_clk_1x),
            .phy_clk_1x                (phy_clk_1x),
            .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
            .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
            .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
            .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
            .seq_ac_sel                (seq_ac_sel),
            .ctl_ac_h                  (ctl_mem_ras_n_h),
            .ctl_ac_l                  (ctl_mem_ras_n_l),
            .seq_ac_h                  (seq_ras_n_h),
            .seq_ac_l                  (seq_ras_n_l),
            .mem_ac                    (mem_ras_n)
        );


        // Create the WE_N I/O structure :

        //
        ddr2_phy_alt_mem_phy_ac # (
                    .POWER_UP_HIGH  	       (1),
            .MEM_IF_MEMTYPE 	       (MEM_IF_MEMTYPE),
            .DWIDTH_RATIO   	       (DWIDTH_RATIO),
            .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
        ) we_n_struct (
            .ac_clk_1x                 (ac_clk_1x),
            .phy_clk_1x                (phy_clk_1x),
            .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
            .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
            .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
            .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
            .seq_ac_sel                (seq_ac_sel),
            .ctl_ac_h                  (ctl_mem_we_n_h),
            .ctl_ac_l                  (ctl_mem_we_n_l),
            .seq_ac_h                  (seq_we_n_h),
            .seq_ac_l                  (seq_we_n_l),
            .mem_ac                    (mem_we_n)
        );

        assign mem_rps_n = {MEM_IF_NUM_RANKS{1'b0}};
        assign mem_wps_n = {MEM_IF_NUM_RANKS{1'b0}};

    end

endgenerate


generate

    if (MEM_IF_MEMTYPE == "DDR3")
    begin : ddr3_rst

        // generate rst_n for DDR3
        //
        ddr2_phy_alt_mem_phy_ac # (
                    .POWER_UP_HIGH  	       (0),
            .MEM_IF_MEMTYPE 	       (MEM_IF_MEMTYPE),
            .DWIDTH_RATIO   	       (DWIDTH_RATIO),
            .INVERT_ADDR_CMD_TXFR_CLK  (INVERT_ADDR_CMD_TXFR_CLK)
        )ddr3_rst_struct (
            .ac_clk_1x                 (ac_clk_1x),
            .phy_clk_1x                (phy_clk_1x),
            .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
            .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
            .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
            .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
            .seq_ac_sel                (seq_ac_sel),
            .ctl_ac_h                  (ctl_mem_rst_n_h),
            .ctl_ac_l                  (ctl_mem_rst_n_l),
            .seq_ac_h                  (seq_mem_rst_n_h),
            .seq_ac_l                  (seq_mem_rst_n_l),
            .mem_ac                    (mem_rst_n)
        );
    
    end
    
    else
    begin : no_ddr3_rst
        assign mem_rst_n = 1'b1;
    end
    
    // Generate address/command parity signal
    // Delay a bunch of config and select lines by 1 cycle

    wire ctl_parity_h = ^({ctl_mem_addr_h, ctl_mem_ba_h, ctl_mem_cas_n_h, ctl_mem_ras_n_h, ctl_mem_we_n_h});
    reg ctl_parity_l;
    wire seq_parity_h = ^({seq_addr_h, seq_ba_h, seq_cas_n_h, seq_ras_n_h, seq_we_n_h});
    reg seq_parity_l;
    
		always @(posedge phy_clk_1x)
		begin
			ctl_parity_l <= ^({ctl_mem_addr_l, ctl_mem_ba_l, ctl_mem_cas_n_l, ctl_mem_ras_n_l, ctl_mem_we_n_l});
			seq_parity_l <= ^({seq_addr_l, seq_ba_l, seq_cas_n_l, seq_ras_n_l, seq_we_n_l});
		end

		if (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 1 && CHIP_OR_DIMM == "Registered DIMM")
    begin : ddr3_parity

        // generate parity for DDR3
        //
        ddr2_phy_alt_mem_phy_ac # (
                    .POWER_UP_HIGH (0),
            .MEM_IF_MEMTYPE (MEM_IF_MEMTYPE),
            .DWIDTH_RATIO   (DWIDTH_RATIO)
        )ddr3_parity_struct (
            .ac_clk_1x                 (ac_clk_1x),
            .phy_clk_1x                (phy_clk_1x),
            .reset_phy_clk_1x_n        (reset_phy_clk_1x_n),
            .reset_ac_clk_1x_n         (reset_ac_clk_1x_n),
            .ctl_add_1t_ac_lat         (ctl_add_1t_ac_lat),
            .ctl_add_intermediate_regs (ctl_add_intermediate_regs),
            .seq_ac_sel                (seq_ac_sel),
            .ctl_ac_h                  (ctl_parity_l),
            .ctl_ac_l                  (ctl_parity_h),
            .seq_ac_h                  (seq_parity_l),
            .seq_ac_l                  (seq_parity_h),
            .mem_ac                    (mem_ac_parity)
        );
    
    
    end
		
		
endgenerate

endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_dp_io (
                                reset_write_clk_2x_n,
                                phy_clk_1x,
                                resync_clk_2x,
                                mem_clk_2x,
                                write_clk_2x,
                                resync_clk_1x,
                                sc_clk,

                                scan_enable_dqs_config,
                                scan_enable_dqs,
                                scan_enable_dq,
                                scan_enable_dm,
                                scan_update,
                                scan_din,
                                scan_dout,

                                dedicated_dll_delay_ctrl,
                                seq_dqs_delay_ctrl,
                                dll_offset_delay_ctrl,
                                dqs_update_en,

                                mem_d,
                                mem_dm,
                                mem_dq,
                                mem_dqs,
                                mem_dqsn,
                                dio_rdata3_1x,
                                dio_rdata2_1x,
                                dio_rdata1_1x,
                                dio_rdata0_1x,
                                poa_postamble_en_preset,

                                wdp_wdata3_1x,
                                wdp_wdata2_1x,
                                wdp_wdata1_1x,
                                wdp_wdata0_1x,

                                wdp_wdata_oe_h_1x,
                                wdp_wdata_oe_l_1x,

                                wdp_dqs3_1x,
                                wdp_dqs2_1x,
                                wdp_dqs1_1x,
                                wdp_dqs0_1x,

                                wdp_dqs_oe_h_1x,
                                wdp_dqs_oe_l_1x,

                                wdp_dm3_1x,
                                wdp_dm2_1x,
                                wdp_dm1_1x,
                                wdp_dm0_1x,

                                wdp_oct_h_1x,
                                wdp_oct_l_1x,

                                seriesterminationcontrol,
                                parallelterminationcontrol

                              ) /* synthesis altera_attribute="SUPPRESS_DA_RULE_INTERNAL=C105" */ ;

parameter MEM_IF_CLK_PS             =           4000;
parameter MEM_IF_CLK_PS_STR         =      "4000 ps";
parameter MEM_IF_BANKADDR_WIDTH     =              3;
parameter MEM_IF_CS_WIDTH           =              2;
parameter MEM_IF_DWIDTH             =             64;
parameter MEM_IF_DM_PINS_EN         =              1;
parameter MEM_IF_DM_WIDTH           =              8;
parameter MEM_IF_DQ_PER_DQS         =              8;
parameter MEM_IF_DQS_CAPTURE_EN     =              1;
parameter MEM_IF_DQS_WIDTH          =              8;
parameter MEM_IF_MEMTYPE            =          "DDR";
parameter LEVELLING                 =              1;
parameter READ_DESKEW_MODE          =         "NONE";
parameter WRITE_DESKEW_MODE         =         "NONE";
parameter MEM_IF_DQSN_EN            =              1;
parameter MEM_IF_OCT_EN             =              0;
parameter MEM_IF_POSTAMBLE_EN_WIDTH =              8;
parameter MEM_IF_ROWADDR_WIDTH      =             13;
parameter DLL_DELAY_BUFFER_MODE     =         "HIGH";
parameter DQS_OUT_MODE              = "DELAY_CHAIN2";
parameter DQS_PHASE                 =           9000;
parameter DQS_PHASE_SETTING         =              2;
parameter DWIDTH_RATIO              =              4;

parameter ENABLE_DDR3_SEQUENCER     =        "FALSE";
parameter DQS_DELAY_CTL_WIDTH       =              6;
parameter MEM_TCL                   =          "1.5";

parameter DQS_DELAY_USES_OFFSET     =        "false";
parameter INVERT_POSTAMBLE_CLK      =        "false";

localparam DM_PER_DQS               = MEM_IF_DM_WIDTH  / (MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS);

// To determine which DQS_CONFIG to connect the DM pins to, this depends upon the number
// of DQ pins per DQS and whether the memory type is DDR or QDR :
localparam DDR_BY8_OR_BY9                          = ((MEM_IF_MEMTYPE != "QDRII") && (MEM_IF_DQ_PER_DQS != 4));
localparam QDRII_NORMAL_CQ_CQN_SWAPPING            = 1;

localparam NUM_CLKDIVS_PER_GRP                     = cal_num_clkdivs_per_grp(MEM_IF_DQ_PER_DQS);

// Read.
localparam MEM_IF_CAPT_T1_DESKEW_EN                = uses_dekew_delay(MEM_IF_MEMTYPE, LEVELLING, READ_DESKEW_MODE);
// Write.
localparam MEM_IF_WR_T9_DESKEW_EN                  = uses_delay(MEM_IF_MEMTYPE, LEVELLING);
localparam MEM_IF_WR_T10_DESKEW_EN                 = 0;
// Strobe.
localparam MEM_IF_STR_T9_DESKEW_EN                 = uses_delay(MEM_IF_MEMTYPE, LEVELLING);
localparam MEM_IF_STR_T10_DESKEW_EN                = uses_dekew_delay(MEM_IF_MEMTYPE, LEVELLING, WRITE_DESKEW_MODE);
// OCT.
localparam MEM_IF_OCT_T9_DESKEW_EN                 = uses_delay(MEM_IF_MEMTYPE, LEVELLING);
localparam MEM_IF_OCT_T10_DESKEW_EN                = 0;
// DQSLB.
localparam MEM_IF_USE_T7                           = uses_dekew_delay(MEM_IF_MEMTYPE, LEVELLING, READ_DESKEW_MODE);
localparam MEM_IF_USE_T11                          = uses_delay(MEM_IF_MEMTYPE, LEVELLING);

localparam MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL = "false";

localparam DQS_DELAY_CODE_FROM_DLL                 =  "true";
localparam USE_DQS_DELAY_LATCHES                   = "false";
localparam DQS_USE_PHASECTRL_IN                    = "false";
localparam OPA_USES_DELAYED_CLK                    =  "true";
localparam SINGLE_LEVELLING_DELAY_CHAIN            =  "true";

input  wire                                         reset_write_clk_2x_n;
input  wire                                         phy_clk_1x;
input  wire                                         resync_clk_2x;
input  wire                                         mem_clk_2x;
input  wire                                         write_clk_2x;

// The SIII half-rate resync clock is produced from clock dividers, 1 or 2 per
// DQS group (1 for 4 DQ per DQS, 2 for >4 DQ per DQS).  These are output to the
// read_dp block to be used to re-sync the read data :

// NB. 'wire' omitted so that attributes can be applied later :
output [MEM_IF_DQS_WIDTH - 1 : 0]                   resync_clk_1x;

input  wire [MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS - 1 : 0] sc_clk;

input  wire [MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS - 1 : 0] scan_enable_dqs_config;
input  wire [MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS - 1 : 0] scan_enable_dqs;
input  wire [MEM_IF_DWIDTH                     - 1 : 0] scan_enable_dq;
input  wire [MEM_IF_DM_WIDTH                   - 1 : 0] scan_enable_dm;
input  wire [MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS - 1 : 0] scan_din;
input  wire [MEM_IF_DWIDTH / MEM_IF_DQ_PER_DQS - 1 : 0] scan_update;
output reg  [MEM_IF_DWIDTH                     - 1 : 0] scan_dout;

// This goes to the output phase aligns
input  wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]          dedicated_dll_delay_ctrl;

// Modified DLL delay control from the core - only used when MEM_IF_DQS_CAPTURE is true.
// The sequencer shall phase-shift the DQS delay to optimise the capture window.
// This goes to the DQS delay chains/enable

input wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]           seq_dqs_delay_ctrl;

// the below wire is used to connect up the DLL phase offset primaive when connected.
input wire [DQS_DELAY_CTL_WIDTH-1:0]                dll_offset_delay_ctrl;

// Used for controlling the DQS delay updates from the core or DLL
input wire                                          dqs_update_en;

output wire [MEM_IF_DWIDTH - 1 : 0]                 mem_d;
output wire [MEM_IF_DM_WIDTH - 1 : 0]               mem_dm;
inout  wire [MEM_IF_DWIDTH - 1 : 0]                 mem_dq;
inout  wire [MEM_IF_DQS_WIDTH - 1 : 0]              mem_dqs;
inout  wire [MEM_IF_DQS_WIDTH - 1 : 0]              mem_dqsn;

(* preserve *) output  wire [MEM_IF_DWIDTH - 1 : 0] dio_rdata3_1x;
(* preserve *) output  wire [MEM_IF_DWIDTH - 1 : 0] dio_rdata2_1x;
(* preserve *) output  wire [MEM_IF_DWIDTH - 1 : 0] dio_rdata1_1x;
(* preserve *) output  wire [MEM_IF_DWIDTH - 1 : 0] dio_rdata0_1x;

input  wire [((DWIDTH_RATIO/2 )* MEM_IF_DQS_WIDTH) - 1 : 0] poa_postamble_en_preset;

input  wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata3_1x;
input  wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata2_1x;
input  wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata1_1x;
input  wire [MEM_IF_DWIDTH - 1 : 0]                 wdp_wdata0_1x;

input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_wdata_oe_h_1x;
input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_wdata_oe_l_1x;

input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs3_1x;
input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs2_1x;
input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs1_1x;
input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs0_1x;

input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs_oe_h_1x;
input  wire [MEM_IF_DQS_WIDTH - 1 : 0]              wdp_dqs_oe_l_1x;

input  wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm3_1x;
input  wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm2_1x;
input  wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm1_1x;
input  wire [MEM_IF_DM_WIDTH -1 : 0]                wdp_dm0_1x;

input  wire [MEM_IF_DQS_WIDTH -1 : 0]               wdp_oct_h_1x;
input  wire [MEM_IF_DQS_WIDTH -1 : 0]               wdp_oct_l_1x;

input  wire [`OCT_SERIES_TERM_CONTROL_WIDTH   -1 : 0] seriesterminationcontrol;
input  wire [`OCT_PARALLEL_TERM_CONTROL_WIDTH -1 : 0] parallelterminationcontrol;

wire [MEM_IF_DWIDTH - 1 : 0]                        rdata_n_captured;
wire [MEM_IF_DWIDTH - 1 : 0]                        rdata_p_captured;

(* preserve *) reg  [MEM_IF_DWIDTH - 1 : 0]         rdata_n_ams;
(* preserve *) reg  [MEM_IF_DWIDTH - 1 : 0]         rdata_p_ams;

// Use DQS clock to register DQ read data
(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] dqs_clk;
(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] dq_capture_clk;

// Create one divider clock output per clock divider.  There shall be "cal_num_clkdivs_per_grp" clock dividers per DQS group :
(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] div_clk [0 : NUM_CLKDIVS_PER_GRP - 1] ;

// Each DQS group has it's own DQS enable signal :
(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] dqs_enable_src;
(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] dqs_enable;

// QDRII uses pseudo-differential DQS/DQSN, therefore the DQSN path needs to be defined :
(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] dqsn_enable_src;
(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] dqsn_enable;


// This must be global, so that the clock dividers can access it :
wire [MEM_IF_DQS_WIDTH - 1 : 0] dividerphasesetting  [0 : NUM_CLKDIVS_PER_GRP - 1] ;

(* altera_attribute = "-name global_signal off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0] resync_clk_1x   ;


wire [MEM_IF_DQS_WIDTH - 1 : 0] resync_clk_array   [0 : NUM_CLKDIVS_PER_GRP - 1] ;
wire [MEM_IF_DQS_WIDTH - 1 : 0] resync_clk_array_n [0 : NUM_CLKDIVS_PER_GRP - 1] ;


wire [((DWIDTH_RATIO/2) * MEM_IF_DQS_WIDTH) - 1 : 0] delayed_poa_postamble_en_preset_sim;
reg  [((DWIDTH_RATIO/2) * MEM_IF_DQS_WIDTH) - 1 : 0] delayed_poa_postamble_en_preset;

wire                            ddr_div_clk_source;
wire [MEM_IF_DQS_WIDTH - 1 : 0] qdrii_div_clk_source;

wire [MEM_IF_DQS_WIDTH - 1 : 0] i_mem_dqs;
wire [MEM_IF_DQS_WIDTH - 1 : 0] i_mem_dqsn;

wire [DQS_DELAY_CTL_WIDTH - 1 : 0] offset_dqs_delay_code;

wire [NUM_CLKDIVS_PER_GRP * MEM_IF_DQS_WIDTH - 1 : 0] scan_dout_dqs_config;
wire [MEM_IF_DQS_WIDTH    		     - 1 : 0] scan_dout_dqs;  
wire [MEM_IF_DWIDTH       		     - 1 : 0] scan_dout_dq;   
wire [MEM_IF_DM_WIDTH     		     - 1 : 0] scan_dout_dm;   

genvar num_clkdivs;
genvar div_clk_num;
genvar dqs_grp_num;
genvar dm_num;
genvar dq_num;
genvar index;

// Delay postamble enable signal to prevent false failures in RTL simulation
//
generate
    for (index=0; index<(DWIDTH_RATIO/2) * MEM_IF_DQS_WIDTH; index=index+1)
    begin : delay_gen
        //
        ddr2_phy_alt_mem_phy_delay # (
                    .WIDTH     (1),
            .DELAY_PS  (MEM_IF_CLK_PS/4)
        ) postamble_preset_delay(
            .s_in      (poa_postamble_en_preset[index]),
            .s_out     (delayed_poa_postamble_en_preset_sim[index])
        );
    end
endgenerate

    always @*
    begin
      delayed_poa_postamble_en_preset  = delayed_poa_postamble_en_preset_sim;
    end

// The resync clock is chosen for the read path and postamble RAMs.
generate

    for (dqs_grp_num=0; dqs_grp_num < MEM_IF_DQS_WIDTH; dqs_grp_num = dqs_grp_num + 1)
    begin : resync_clks_gen
        assign resync_clk_1x  [dqs_grp_num] =  resync_clk_array[0][dqs_grp_num];
    end

endgenerate


// Clock dividers :













function integer cal_num_clkdivs_per_grp (input integer dq_per_dqs);
begin
    casez (dq_per_dqs)
        4       : cal_num_clkdivs_per_grp = 1;
        8       : cal_num_clkdivs_per_grp = 2;
        9       : cal_num_clkdivs_per_grp = 2;
        18      : cal_num_clkdivs_per_grp = 4;
        default : cal_num_clkdivs_per_grp = 8;
    endcase
end
endfunction



function integer calc_resync_index (input integer dq_num);
begin




    if (MEM_IF_DQ_PER_DQS%9 == 0)
    begin
        casez (dq_num)
        0,1,2,3        : calc_resync_index = 0;
        4,5,6,7,8      : calc_resync_index = 1;
        9,10,11,12     : calc_resync_index = 2;
        13,14,15,16,17 : calc_resync_index = 3;
        18,19,20,21    : calc_resync_index = 4;
        22,23,24,25,26 : calc_resync_index = 5;
        27,28,29,30    : calc_resync_index = 6;
        default        : calc_resync_index = 7;
        endcase
    end
    else
        calc_resync_index = dq_num/4;
end
endfunction


function integer uses_delay (input [31:0] mem_type,
                             input levelling);
begin
    if (((mem_type == "DDR3") || (mem_type == "ddr3")) && levelling == 1)
        uses_delay = 1;
    else
        uses_delay = 0;
end
endfunction

function integer uses_dekew_delay (input [31:0] mem_type,
                                   input levelling,
                                   input [31:0] mode);
begin
    if (((mem_type == "DDR3") || (mem_type == "ddr3")) && levelling == 1 && ((mode != "NONE") && (mode != "none")))
        uses_dekew_delay = 1;
    else
        uses_dekew_delay = 0;
end
endfunction





generate
    if (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 1)
    begin : ddr_clkdiv_gen


        assign ddr_div_clk_source =  ~resync_clk_2x;

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DQS_WIDTH ; dqs_grp_num=dqs_grp_num+1)
        begin : grp_gen

            stratixiii_io_clock_divider # (
                  .use_phasectrlin   ("false"),
                  .phase_setting     (0),
                  .delay_buffer_mode ("high"),
                  .use_masterin      ("false"),
                  .invert_phase      ("false")
              ) clk_div_group_master (
                  .clk               (ddr_div_clk_source),
                  .phaseselect       (dividerphasesetting[0][dqs_grp_num]),
                  .delayctrlin       (),
                  .phasectrlin       (),
                  .masterin          (),
                  .phaseinvertctrl   (),
                  .devclrn           (),
                  .devpor            (),
                  .clkout            (resync_clk_array[0][dqs_grp_num]),
                  .slaveout          (div_clk[0][dqs_grp_num])
              );

            //Generate the necessary number of slaves per dqs group.if we are a group wider than x4
            if (MEM_IF_DQ_PER_DQS > 4)
               for (num_clkdivs=1; num_clkdivs < MEM_IF_DQ_PER_DQS/4; num_clkdivs = num_clkdivs + 1)
               begin : slave_gen

                  // Drive masterin from the above divider :
                  stratixiii_io_clock_divider # (
                      .use_phasectrlin   ("false"),
                      .phase_setting     (0),
                      .delay_buffer_mode ("high"),
                      .use_masterin      ("true"),
                      .invert_phase      ("false")
                  ) clk_div_slave (
                      .clk               (ddr_div_clk_source),
                      .phaseselect       (dividerphasesetting[num_clkdivs][dqs_grp_num]),
                      .delayctrlin       (),
                      .phasectrlin       (),
                      .masterin          (div_clk[num_clkdivs-1][dqs_grp_num]),
                      .phaseinvertctrl   (),
                      .devclrn           (),
                      .devpor            (),
                      .clkout            (resync_clk_array[num_clkdivs][dqs_grp_num]),
                      .slaveout          (div_clk[num_clkdivs][dqs_grp_num])
                  );

               end
        end
    end

    else if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdr_clkdiv_gen

        // QDRII uses the DQS enable signal for the source of the clock dividers :
        assign qdrii_div_clk_source = dqsn_enable_src;

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DQS_WIDTH ; dqs_grp_num=dqs_grp_num+1)
        begin : grp_gen

            stratixiii_io_clock_divider # (
                  .use_phasectrlin   ("false"),
                  .phase_setting     (0),
                  .delay_buffer_mode ("high"),
                  .use_masterin      ("false"),
                  .invert_phase      ("false")
              ) clk_div_group_master (
                  .clk               (qdrii_div_clk_source[dqs_grp_num]),
                  .phaseselect       (dividerphasesetting[0][dqs_grp_num]),
                  .delayctrlin       (),
                  .phasectrlin       (),
                  .masterin          (),
                  .phaseinvertctrl   (),
                  .devclrn           (),
                  .devpor            (),
                  .clkout            (resync_clk_array[0][dqs_grp_num]),
                  .slaveout          (div_clk[0][dqs_grp_num])
              );

            // Create any necessary slaves :
            // In lieu of the old "_l" and "_h" system, we now generate indices 0,1,2.. to support wide QDRII groups :
            for (num_clkdivs=1; num_clkdivs < NUM_CLKDIVS_PER_GRP; num_clkdivs = num_clkdivs + 1)
            begin : slave_gen

                // Drive masterin from the above divider :
                stratixiii_io_clock_divider # (
                    .use_phasectrlin   ("false"),
                    .phase_setting     (0),
                    .delay_buffer_mode ("high"),
                    .use_masterin      ("true"),
                    .invert_phase      ("false")
                ) clk_div_slave (
                    .clk               (qdrii_div_clk_source[dqs_grp_num]),
                    .phaseselect       (dividerphasesetting[num_clkdivs][dqs_grp_num]),
                    .delayctrlin       (),
                    .phasectrlin       (),
                    .masterin          (div_clk[num_clkdivs-1][dqs_grp_num]),
                    .phaseinvertctrl   (),
                    .devclrn           (),
                    .devpor            (),
                    .clkout            (resync_clk_array[num_clkdivs][dqs_grp_num]),
                    .slaveout          (div_clk[num_clkdivs][dqs_grp_num])
                );

            end
        end
    end

    else
    begin : ddr_clkdiv_gen

        assign ddr_div_clk_source   = ~resync_clk_2x;

        // Create first master and first slave (if > x4 group)
        stratixiii_io_clock_divider # (
            .use_phasectrlin   ("false"),
            .phase_setting     (0),
            .delay_buffer_mode ("high"),
            .use_masterin      ("false"),
            .invert_phase      ("false")
        ) clk_div_master(
            .clk                (ddr_div_clk_source),
            .phaseselect        (dividerphasesetting[0][0]),
            .delayctrlin        (),
            .phasectrlin        (),
            .masterin           (),
            .phaseinvertctrl    (),
            .devclrn            (),
            .devpor             (),
            .clkout             (resync_clk_array[0][0]),
            .slaveout           (div_clk[0][0])
         );

        for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DQS_WIDTH ; dqs_grp_num=dqs_grp_num+1)
        begin : grp_gen

            // Create any necessary slaves :
            // In lieu of the old "_l" and "_h" system, we now generate indices 0,1,2.. to support wide QDRII groups :
            // For group 0 we already have element [0][0], as our master, so start incrementing from 1
            for (num_clkdivs= (dqs_grp_num==0 ? 1:0); num_clkdivs < MEM_IF_DQ_PER_DQS/4; num_clkdivs = num_clkdivs + 1)
            begin : slaves_gen

            wire masterin; // Localvar for slave masterin clk feed

                // If this is the first slave in the group, drive from slave in previous DQS group :
                if (num_clkdivs==0) // NB. Therefore we are not in DQS group 0.
                    if (MEM_IF_DQ_PER_DQS == 8 || MEM_IF_DQ_PER_DQS == 9)
                        assign masterin = div_clk[1][dqs_grp_num-1]; // for dq_per_dqs==8
                    else
                        assign masterin = div_clk[0][dqs_grp_num-1]; // for dq_per_dqs==4

                // Otherwise drive from previous slave (For dq_per_dqs>4 only):
                else
                    assign masterin = div_clk[num_clkdivs-1][dqs_grp_num];

                // Drive masterin from the above divider :
                stratixiii_io_clock_divider # (
                    .use_phasectrlin   ("false"),
                    .phase_setting     (0),
                    .delay_buffer_mode ("high"),
                    .use_masterin      ("true"),
                    .invert_phase      ("false")
                ) clk_div_slave (
                    .clk               (ddr_div_clk_source),
                    .phaseselect       (dividerphasesetting[num_clkdivs][dqs_grp_num]),
                    .delayctrlin       (),
                    .phasectrlin       (),
                    .masterin          (masterin),
                    .phaseinvertctrl   (),
                    .devclrn           (),
                    .devpor            (),
                    .clkout            (resync_clk_array[num_clkdivs][dqs_grp_num]),
                    .slaveout          (div_clk[num_clkdivs][dqs_grp_num])
                );

            end
        end
    end

endgenerate



// Now create the DQ, DM and DQS/DQSN I/O paths themselves :
generate

    for (dqs_grp_num=0; dqs_grp_num<MEM_IF_DQS_WIDTH ; dqs_grp_num=dqs_grp_num+1)
    begin : dqs_group

        // Local routing from DQS_CONFIG to DQS logic :

        wire [`DQSCONFIG_DQS_BUSOUT_DELAY_SETTING_WIDTH-1 :0] dqsbusoutdelaysetting[0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [`DQSCONFIG_DQS_EN_CTRL_PHASE_SETTING_WIDTH-1:0] dqsenablectrlphasesetting[0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [`DQSCONFIG_DQS_EN_DELAY_SETTING_WIDTH-1     :0] dqsenabledelaysetting[0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [2 :0]                                           dqsinputphasesetting [0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [NUM_CLKDIVS_PER_GRP-1:0] enadqsenablephasetransferreg;
        wire [NUM_CLKDIVS_PER_GRP-1:0] dqsenablectrlphaseinvert;
        wire [NUM_CLKDIVS_PER_GRP-1:0] enaoctcycledelaysetting;
        wire [NUM_CLKDIVS_PER_GRP-1:0] enaoutputcycledelaysetting;
        wire [NUM_CLKDIVS_PER_GRP-1:0] enaoutputphasetransferreg;
        wire [NUM_CLKDIVS_PER_GRP-1:0] enaoctphasetransferreg;
        wire [`DQSCONFIG_RESYNC_IP_PHASE_SETTING_WIDTH-1  :0] resyncinputphasesetting [0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [NUM_CLKDIVS_PER_GRP-1:0] enainputcycledelaysetting;
        wire [NUM_CLKDIVS_PER_GRP-1:0] enainputphasetransferreg;
        wire [NUM_CLKDIVS_PER_GRP-1:0] resyncinputphaseinvert;
        wire [NUM_CLKDIVS_PER_GRP-1:0] enadataoutbypass;
        wire [NUM_CLKDIVS_PER_GRP-1:0] dqoutputphaseinvert;
        wire [`DQSCONFIG_DQ_OP_PHASE_SETTING_WIDTH-1      :0] dqoutputphasesetting[0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [`DQSCONFIG_DQS_OCT_DELAY_SETTING1_WIDTH-1   :0] octdelaysetting1[0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [`DQSCONFIG_DQS_OCT_DELAY_SETTING2_WIDTH-1   :0] octdelaysetting2[0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [`DQSCONFIG_DQS_OUTPUT_PHASE_SETTING_WIDTH-1 :0] dqsoutputphasesetting[0 : NUM_CLKDIVS_PER_GRP - 1];
        wire [NUM_CLKDIVS_PER_GRP-1:0] dqsoutputphaseinvert;

        wire                                                  dqs_sneak;

        // First generate DQ pins for each DQS group :
        for (dq_num=0; dq_num<MEM_IF_DQ_PER_DQS ; dq_num=dq_num+1)
        begin : dq

            // Note that if MEM_IF_DQ_PER_DQS is 8 or 9, two dqs_config blocks are instanced per DQS group
            // and half the DQ pins are fed from one set of dqs_config outputs, and half from the other

            //
            ddr2_phy_alt_mem_phy_dq_io # (
                            .MEM_IF_DQS_WIDTH                        (MEM_IF_DQS_WIDTH),
                .MEM_IF_DWIDTH                           (MEM_IF_DWIDTH),
                .MEM_IF_MEMTYPE                          (MEM_IF_MEMTYPE),
                .LEVELLING                               (LEVELLING),
                .MEM_IF_OCT_EN                           (MEM_IF_OCT_EN),
                .DLL_DELAY_BUFFER_MODE                   (DLL_DELAY_BUFFER_MODE),
                .DWIDTH_RATIO                            (DWIDTH_RATIO),
                .ENABLE_DDR3_SEQUENCER                   (ENABLE_DDR3_SEQUENCER),
                .DQS_DELAY_CTL_WIDTH                     (DQS_DELAY_CTL_WIDTH),
                .MEM_IF_CAPT_T1_DESKEW_EN                (MEM_IF_CAPT_T1_DESKEW_EN),
                .MEM_IF_WR_T9_DESKEW_EN                  (MEM_IF_WR_T9_DESKEW_EN),
                .MEM_IF_WR_T10_DESKEW_EN                 (MEM_IF_WR_T10_DESKEW_EN),
                .MEM_IF_OCT_T9_DESKEW_EN                 (MEM_IF_OCT_T9_DESKEW_EN),
                .MEM_IF_OCT_T10_DESKEW_EN                (MEM_IF_OCT_T10_DESKEW_EN),
                .MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL),
                .OPA_USES_DELAYED_CLK                    (OPA_USES_DELAYED_CLK),
                .SINGLE_LEVELLING_DELAY_CHAIN            (SINGLE_LEVELLING_DELAY_CHAIN)

            ) dq_pad (
                .phy_clk_1x                 (phy_clk_1x),
                .write_clk_2x               (write_clk_2x),
                .mem_clk_2x                 (mem_clk_2x),
                .resync_clk_2x              (resync_clk_2x),
                .resync_clk_1x              (resync_clk_array[calc_resync_index(dq_num)][dqs_grp_num]),
                .sc_clk                     (sc_clk[dqs_grp_num]),
                .scan_din                   (scan_din[dqs_grp_num]),
                .scan_update                (scan_update[dqs_grp_num]),
                .scan_enable                (scan_enable_dq[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .scan_dout                  (scan_dout_dq[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .mem_d                      (mem_d[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .mem_dq                     (mem_dq[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .dqs_enable                 (dqs_enable[dqs_grp_num]),
                .dqsn_enable                (dqsn_enable[dqs_grp_num]),
                .dio_rdata3_1x              (dio_rdata3_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .dio_rdata2_1x              (dio_rdata2_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .dio_rdata1_1x              (dio_rdata1_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .dio_rdata0_1x              (dio_rdata0_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .wdp_wdata3_1x              (wdp_wdata3_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .wdp_wdata2_1x              (wdp_wdata2_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .wdp_wdata1_1x              (wdp_wdata1_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .wdp_wdata0_1x              (wdp_wdata0_1x[dqs_grp_num*MEM_IF_DQ_PER_DQS+dq_num]),
                .wdp_wdata_oe_h_1x          (wdp_wdata_oe_h_1x[dqs_grp_num]),
                .wdp_wdata_oe_l_1x          (wdp_wdata_oe_l_1x[dqs_grp_num]),
                .wdp_oct_h_1x               (wdp_oct_h_1x[dqs_grp_num]),
                .wdp_oct_l_1x               (wdp_oct_l_1x[dqs_grp_num]),
                .dedicated_dll_delay_ctrl   (dedicated_dll_delay_ctrl),
                .dqoutputphasesetting       (dqoutputphasesetting[calc_resync_index(dq_num)]),
                .dqoutputphaseinvert        (dqoutputphaseinvert[calc_resync_index(dq_num)]),
                .enaoutputcycledelaysetting (enaoutputcycledelaysetting[calc_resync_index(dq_num)]),
                .enaoutputphasetransferreg  (enaoutputphasetransferreg[calc_resync_index(dq_num)]),
                .resyncinputphasesetting    (resyncinputphasesetting[calc_resync_index(dq_num)]),
                .enainputcycledelaysetting  (enainputcycledelaysetting[calc_resync_index(dq_num)]),
                .enainputphasetransferreg   (enainputphasetransferreg[calc_resync_index(dq_num)]),
                .resyncinputphaseinvert     (resyncinputphaseinvert[calc_resync_index(dq_num)]),
                .enadataoutbypass           (enadataoutbypass[calc_resync_index(dq_num)]),
                .dqsoutputphasesetting      (dqsoutputphasesetting[calc_resync_index(dq_num)]),
                .dqsoutputphaseinvert       (dqsoutputphaseinvert[calc_resync_index(dq_num)]),
                .enaoctcycledelaysetting    (enaoctcycledelaysetting[calc_resync_index(dq_num)]),
                .enaoctphasetransferreg     (enaoctphasetransferreg[calc_resync_index(dq_num)]),
                .octdelaysetting1           (octdelaysetting1[calc_resync_index(dq_num)]),
                .octdelaysetting2           (octdelaysetting2[calc_resync_index(dq_num)]),
                .seriesterminationcontrol   (seriesterminationcontrol),
                .parallelterminationcontrol (parallelterminationcontrol)
            );

        end

        // DM Pin - if required :
        if (MEM_IF_DM_PINS_EN)
        begin : dm_gen

            // First generate DQ pins for each DQS group.  For DDR only x8 or x9 groups are supported and
            // for these just one DM pin is required.  For QDRII there may be more DMs per group :
            for (dm_num=0; dm_num<DM_PER_DQS ; dm_num=dm_num+1)
            begin : dm_pad_gen




                //
                ddr2_phy_alt_mem_phy_dm # (
                                    .MEM_IF_MEMTYPE                          (MEM_IF_MEMTYPE),
                    .LEVELLING                               (LEVELLING),
                    .DLL_DELAY_BUFFER_MODE                   (DLL_DELAY_BUFFER_MODE),
                    .DWIDTH_RATIO                            (DWIDTH_RATIO),
                    .ENABLE_DDR3_SEQUENCER                   (ENABLE_DDR3_SEQUENCER),
                    .MEM_IF_WR_T9_DESKEW_EN                  (MEM_IF_WR_T9_DESKEW_EN),
                    .MEM_IF_WR_T10_DESKEW_EN                 (MEM_IF_WR_T10_DESKEW_EN),
                    .MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL),
                    .OPA_USES_DELAYED_CLK                    (OPA_USES_DELAYED_CLK),
                    .SINGLE_LEVELLING_DELAY_CHAIN            (SINGLE_LEVELLING_DELAY_CHAIN)
                ) dm_pad (
                    .phy_clk_1x                 (phy_clk_1x),
                    .write_clk_2x               (write_clk_2x),
                    .sc_clk                     (sc_clk[dqs_grp_num]),
                    .scan_din                   (scan_din[dqs_grp_num]),
                    .scan_update                (scan_update[dqs_grp_num]),
                    .scan_enable                (scan_enable_dm[dqs_grp_num]),
                    .scan_dout                  (scan_dout_dm[dqs_grp_num]),
                    .mem_dm                     (    mem_dm[dqs_grp_num * DM_PER_DQS + dm_num]),
                    .wdp_dm3_1x                 (wdp_dm3_1x[dqs_grp_num * DM_PER_DQS + dm_num]),
                    .wdp_dm2_1x                 (wdp_dm2_1x[dqs_grp_num * DM_PER_DQS + dm_num]),
                    .wdp_dm1_1x                 (wdp_dm1_1x[dqs_grp_num * DM_PER_DQS + dm_num]),
                    .wdp_dm0_1x                 (wdp_dm0_1x[dqs_grp_num * DM_PER_DQS + dm_num]),
                    .dedicated_dll_delay_ctrl   (dedicated_dll_delay_ctrl),
                    .dqoutputphasesetting       (dqoutputphasesetting[dm_num+DDR_BY8_OR_BY9]),
                    .enaoutputcycledelaysetting (enaoutputcycledelaysetting[dm_num+DDR_BY8_OR_BY9]),
                    .enaoutputphasetransferreg  (enaoutputphasetransferreg[dm_num+DDR_BY8_OR_BY9]),
                    .dqoutputphaseinvert        (dqoutputphaseinvert[dm_num+DDR_BY8_OR_BY9]),
                    .seriesterminationcontrol   (seriesterminationcontrol),
                    .parallelterminationcontrol (parallelterminationcontrol)
                );

            end

        end

        // Instance the DQS enable path once per DQS group :

        //
        ddr2_phy_alt_mem_phy_dqs_ip # (
                    .MEM_IF_CLK_PS             (MEM_IF_CLK_PS),
            .MEM_IF_CLK_PS_STR         (MEM_IF_CLK_PS_STR),
            .MEM_IF_MEMTYPE            (MEM_IF_MEMTYPE),
            .LEVELLING                 (LEVELLING),
            .MEM_IF_DQSN_EN            (MEM_IF_DQSN_EN),
            .DLL_DELAY_BUFFER_MODE     (DLL_DELAY_BUFFER_MODE),
            .DQS_PHASE                 (DQS_PHASE),
            .DQS_PHASE_SETTING         (DQS_PHASE_SETTING),
            .DWIDTH_RATIO              (DWIDTH_RATIO),
            .ENABLE_DDR3_SEQUENCER     (ENABLE_DDR3_SEQUENCER),
            .DQS_DELAY_CTL_WIDTH       (DQS_DELAY_CTL_WIDTH),
            .MEM_TCL                   (MEM_TCL),
            .DQS_DELAY_CODE_FROM_DLL   (DQS_DELAY_CODE_FROM_DLL),
            .DQS_DELAY_USES_OFFSET     (DQS_DELAY_USES_OFFSET),
            .USE_DQS_DELAY_LATCHES     (USE_DQS_DELAY_LATCHES),
            .DQS_USE_PHASECTRL_IN      (DQS_USE_PHASECTRL_IN),
            .MEM_IF_USE_T11            (MEM_IF_USE_T11),
            .MEM_IF_USE_T7             (MEM_IF_USE_T7),
	    .INVERT_POSTAMBLE_CLK      (INVERT_POSTAMBLE_CLK)
        ) dqs_ip (
            .poa_postamble_en_preset      (delayed_poa_postamble_en_preset[((dqs_grp_num + 1) * (DWIDTH_RATIO/2) - 1) : ((dqs_grp_num)*(DWIDTH_RATIO/2))]),
            .resync_clk_1x                (resync_clk_array[0][dqs_grp_num]),
            .resync_clk_2x                (resync_clk_2x),

            .dedicated_dll_delay_ctrl     (dedicated_dll_delay_ctrl),
            .seq_dqs_delay_ctrl           (seq_dqs_delay_ctrl),
            .dll_offset_delay_ctrl        (dll_offset_delay_ctrl),
            .dqs_update_en                (dqs_update_en),
            .dqsinputphasesetting         (dqsinputphasesetting[0]),
            .dqs_pad                      (i_mem_dqs[dqs_grp_num]),
            .dqsn_pad                     (i_mem_dqsn[dqs_grp_num]),

            .dqs_enable                   (dqs_enable_src[dqs_grp_num]),
            .dqsn_enable                  (dqsn_enable_src[dqs_grp_num]),

            .dqsbusoutdelaysetting        (dqsbusoutdelaysetting[0]),
            .dqsenablectrlphasesetting    (dqsenablectrlphasesetting[0]),
            .dqsenabledelaysetting        (dqsenabledelaysetting[0]),
            .enadqsenablephasetransferreg (enadqsenablephasetransferreg[0]),
            .dqsenablectrlphaseinvert     (dqsenablectrlphaseinvert[0]),
            .enaoutputcycledelaysetting   (enaoutputcycledelaysetting[0]),
            .enaoutputphasetransferreg    (enaoutputphasetransferreg[0]),
            .dqsoutputphaseinvert         (dqsoutputphaseinvert[0]),
            .dqsoutputphasesetting        (dqsoutputphasesetting[0])
        );

        if (MEM_IF_MEMTYPE == "QDRII")
        begin : qdr_swap_cqs_based_on_mem_tcl

        //****Possible MEM_TCL latency values
        //** Altera recommendation followed: FPGA pin swap of CQ/CQn pins
        //   MEM_TCL = 2.0
        //** Altera recommendation not followed: No FPGA pin swap of CQ/CQn pins
        //   MEM_TCL = 1.5/2.5
        //These effects are only visible on the device pinout!

            if ((MEM_TCL === "1.5") || (MEM_TCL === "2.5") || (MEM_TCL === "12.0"))
            begin : qdr_normal_cq_cqn
                assign  i_mem_dqs[dqs_grp_num]  = mem_dqs [dqs_grp_num];
                assign  i_mem_dqsn[dqs_grp_num] = mem_dqsn[dqs_grp_num];
            end
            else // MEM_TCL === "2.0" || "11.5" || "12.5"
            begin : qdr_swapped_cq_cqn
                 assign  i_mem_dqs[dqs_grp_num]  = mem_dqsn[dqs_grp_num];
                 assign  i_mem_dqsn[dqs_grp_num] = mem_dqs [dqs_grp_num];
            end

            assign  dqs_enable[dqs_grp_num] = dqs_enable_src[dqs_grp_num];
            assign dqsn_enable[dqs_grp_num] = dqsn_enable_src[dqs_grp_num];

        end

        else
        begin : ddr_dqs_enable_gen

            assign  i_mem_dqs[dqs_grp_num]  = mem_dqs [dqs_grp_num];
            assign  i_mem_dqsn[dqs_grp_num] = mem_dqsn[dqs_grp_num];

            assign  dqs_enable[dqs_grp_num] =  dqs_enable_src[dqs_grp_num];
            assign dqsn_enable[dqs_grp_num] = dqsn_enable_src[dqs_grp_num];

        end

        // NB. Share the scan_din and scan_update per DQS group.

        // DQS Output path not required for QDRII :
        if (MEM_IF_MEMTYPE != "QDRII")
        begin : dqs_op_gen

            //
            ddr2_phy_alt_mem_phy_dqs_op # (
                            .DLL_DELAY_BUFFER_MODE                   (DLL_DELAY_BUFFER_MODE),
                .INVERT_OP_FOR_DQSN                      (0),
                .MEM_IF_MEMTYPE                          (MEM_IF_MEMTYPE),
                .LEVELLING                               (LEVELLING),
                .MEM_IF_DQSN_EN                          (MEM_IF_DQSN_EN),
                .MEM_IF_OCT_EN                           (MEM_IF_OCT_EN),
                .DWIDTH_RATIO                            (DWIDTH_RATIO),
                .ENABLE_DDR3_SEQUENCER                   (ENABLE_DDR3_SEQUENCER),
                .MEM_IF_STR_T9_DESKEW_EN                 (MEM_IF_STR_T9_DESKEW_EN),
                .MEM_IF_STR_T10_DESKEW_EN                (MEM_IF_STR_T10_DESKEW_EN),
                .MEM_IF_OCT_T9_DESKEW_EN                 (MEM_IF_OCT_T9_DESKEW_EN),
                .MEM_IF_OCT_T10_DESKEW_EN                (MEM_IF_OCT_T10_DESKEW_EN),
                .MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL),
                .OPA_USES_DELAYED_CLK                    (OPA_USES_DELAYED_CLK)
            ) dqs_op (
                .phy_clk_1x                 (phy_clk_1x),
                .write_clk_2x               (write_clk_2x),
                .mem_clk_2x                 (mem_clk_2x),
                .sc_clk                     (sc_clk[dqs_grp_num]),

                .scan_enable                (scan_enable_dqs[dqs_grp_num]),
                .scan_update                (scan_update[dqs_grp_num]),
                .scan_din                   (scan_din[dqs_grp_num]),
                .scan_dout                  (scan_dout_dqs[dqs_grp_num]),

                .wdp_dqs3_1x                (wdp_dqs3_1x[dqs_grp_num]),
                .wdp_dqs2_1x                (wdp_dqs2_1x[dqs_grp_num]),
                .wdp_dqs1_1x                (wdp_dqs1_1x[dqs_grp_num]),
                .wdp_dqs0_1x                (wdp_dqs0_1x[dqs_grp_num]),

                .wdp_dqs_oe_h_1x            (wdp_dqs_oe_h_1x[dqs_grp_num]),
                .wdp_dqs_oe_l_1x            (wdp_dqs_oe_l_1x[dqs_grp_num]),

                .wdp_oct_h_1x               (wdp_oct_h_1x[dqs_grp_num]),
                .wdp_oct_l_1x               (wdp_oct_l_1x[dqs_grp_num]),

                .dqs_sneak_in               (),
                .dqs_sneak_out              (dqs_sneak),

                .dqs_pad                    (mem_dqs[dqs_grp_num]),

                .dedicated_dll_delay_ctrl   (dedicated_dll_delay_ctrl),

                .enaoutputcycledelaysetting (enaoutputcycledelaysetting[0]),
                .enaoutputphasetransferreg  (enaoutputphasetransferreg[0]),
                .dqsoutputphaseinvert       (dqsoutputphaseinvert[0]),
                .dqsoutputphasesetting      (dqsoutputphasesetting[0]),

                .enaoctcycledelaysetting    (enaoctcycledelaysetting[0]),
                .enaoctphasetransferreg     (enaoctphasetransferreg[0]),
                .octdelaysetting1           (octdelaysetting1[0]),
                .octdelaysetting2           (octdelaysetting2[0]),
                .seriesterminationcontrol   (seriesterminationcontrol),
                .parallelterminationcontrol (parallelterminationcontrol)
            );

            // For DDR3 and 2 where a DQSN pin is required, instance the dqs module again,
            // but invert the output, and wire in the sneak output from the DQS pin :
            if (MEM_IF_MEMTYPE == "DDR3" || (MEM_IF_MEMTYPE == "DDR2" && (MEM_IF_DQSN_EN == 1)) )
            begin : dqsn_op_gen

                //
                ddr2_phy_alt_mem_phy_dqs_op # (
                                    .DLL_DELAY_BUFFER_MODE                   (DLL_DELAY_BUFFER_MODE),
                    .INVERT_OP_FOR_DQSN                      (1),
                    .MEM_IF_MEMTYPE                          (MEM_IF_MEMTYPE),
                    .LEVELLING                               (LEVELLING),
                    .MEM_IF_DQSN_EN                          (MEM_IF_DQSN_EN),
                    .MEM_IF_OCT_EN                           (MEM_IF_OCT_EN),
                    .DWIDTH_RATIO                            (DWIDTH_RATIO),
                    .ENABLE_DDR3_SEQUENCER                   (ENABLE_DDR3_SEQUENCER),
                    .MEM_IF_STR_T9_DESKEW_EN                 (MEM_IF_STR_T9_DESKEW_EN),
                    .MEM_IF_STR_T10_DESKEW_EN                (MEM_IF_STR_T10_DESKEW_EN),
                    .MEM_IF_OCT_T9_DESKEW_EN                 (MEM_IF_OCT_T9_DESKEW_EN),
                    .MEM_IF_OCT_T10_DESKEW_EN                (MEM_IF_OCT_T10_DESKEW_EN),
                    .MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL),
                    .OPA_USES_DELAYED_CLK                    (OPA_USES_DELAYED_CLK)
                ) dqsn_op (
                    .phy_clk_1x                 (phy_clk_1x),
                    .write_clk_2x               (write_clk_2x),
                    .mem_clk_2x                 (mem_clk_2x),
                    .sc_clk                     (sc_clk[dqs_grp_num]),

                    .scan_enable                (),
                    .scan_update                (),
                    .scan_din                   (),
                    .scan_dout                  (),

                    .wdp_dqs3_1x                (wdp_dqs3_1x[dqs_grp_num]),
                    .wdp_dqs2_1x                (wdp_dqs2_1x[dqs_grp_num]),
                    .wdp_dqs1_1x                (wdp_dqs1_1x[dqs_grp_num]),
                    .wdp_dqs0_1x                (wdp_dqs0_1x[dqs_grp_num]),

                    .wdp_dqs_oe_h_1x            (wdp_dqs_oe_h_1x[dqs_grp_num]),
                    .wdp_dqs_oe_l_1x            (wdp_dqs_oe_l_1x[dqs_grp_num]),

                    .wdp_oct_h_1x               (wdp_oct_h_1x[dqs_grp_num]),
                    .wdp_oct_l_1x               (wdp_oct_l_1x[dqs_grp_num]),

                    .dqs_sneak_in               (dqs_sneak),
                    .dqs_sneak_out              (),

                    .dqs_pad                    (mem_dqsn[dqs_grp_num]),

                    .dedicated_dll_delay_ctrl   (dedicated_dll_delay_ctrl),

                    .enaoutputcycledelaysetting (enaoutputcycledelaysetting[0]),
                    .enaoutputphasetransferreg  (enaoutputphasetransferreg[0]),
                    .dqsoutputphaseinvert       (dqsoutputphaseinvert[0]),
                    .dqsoutputphasesetting      (dqsoutputphasesetting[0]),

                    .enaoctcycledelaysetting    (enaoctcycledelaysetting[0]),
                    .enaoctphasetransferreg     (enaoctphasetransferreg[0]),
                    .octdelaysetting1           (octdelaysetting1[0]),
                    .octdelaysetting2           (octdelaysetting2[0]),
                    .seriesterminationcontrol   (seriesterminationcontrol),
                    .parallelterminationcontrol (parallelterminationcontrol)
                );

            end

        end // Not QDRII


        // Create any necessary slaves :
        // In lieu of the old "_l" and "_h" system, we now generate indices 0,1,2.. to support wide QDRII groups :
        for (num_clkdivs=0; num_clkdivs < NUM_CLKDIVS_PER_GRP; num_clkdivs = num_clkdivs + 1)
        begin : dqs_config_gen

            stratixiii_dqs_config dqs_config(
                .datain                       (scan_din[dqs_grp_num]),
                .clk                          (sc_clk[dqs_grp_num]),
                .ena                          (scan_enable_dqs_config[dqs_grp_num]),
                .update                       (scan_update[dqs_grp_num]),
                // synopsys translate_off
                .devclrn(), .devpor(),
                .dqsbusoutfinedelaysetting(),
                .dqsenablefinedelaysetting(),
                // synopsys translate_on

                .dqsbusoutdelaysetting        (dqsbusoutdelaysetting[num_clkdivs]),
                .dqsinputphasesetting         (dqsinputphasesetting [num_clkdivs]),
                .dqsenablectrlphasesetting    (dqsenablectrlphasesetting[num_clkdivs]),
                .dqsoutputphasesetting        (dqsoutputphasesetting[num_clkdivs]),
                .dqoutputphasesetting         (dqoutputphasesetting[num_clkdivs]),

                .resyncinputphasesetting      (resyncinputphasesetting[num_clkdivs]),
                .dividerphasesetting          (dividerphasesetting[num_clkdivs][dqs_grp_num]),

                .enaoctcycledelaysetting      (enaoctcycledelaysetting[num_clkdivs]),
                .enainputcycledelaysetting    (enainputcycledelaysetting[num_clkdivs]),
                .enaoutputcycledelaysetting   (enaoutputcycledelaysetting[num_clkdivs]),
                .dqsenabledelaysetting        (dqsenabledelaysetting[num_clkdivs]),
                .octdelaysetting1             (octdelaysetting1[num_clkdivs]),
                .octdelaysetting2             (octdelaysetting2[num_clkdivs]),
                .enadataoutbypass             (enadataoutbypass[num_clkdivs]),
                .enadqsenablephasetransferreg (enadqsenablephasetransferreg[num_clkdivs]),
                .enaoctphasetransferreg       (enaoctphasetransferreg[num_clkdivs]),
                .enaoutputphasetransferreg    (enaoutputphasetransferreg[num_clkdivs]),
                .enainputphasetransferreg     (enainputphasetransferreg[num_clkdivs]),
                .resyncinputphaseinvert       (resyncinputphaseinvert[num_clkdivs]),
                .dqsenablectrlphaseinvert     (dqsenablectrlphaseinvert[num_clkdivs]),
                .dqoutputphaseinvert          (dqoutputphaseinvert[num_clkdivs]),
                .dqsoutputphaseinvert         (dqsoutputphaseinvert[num_clkdivs]),

                .dataout                      (scan_dout_dqs_config[dqs_grp_num*NUM_CLKDIVS_PER_GRP + num_clkdivs])
            );

        end

    end // DQS group

endgenerate



// Scan data out muxing - this is provided purely for HCx :
generate

    if (MEM_IF_DM_PINS_EN)
    begin : hcx_scan_dout_mux

	always @*
	begin

	    casez({|scan_enable_dqs_config, |scan_enable_dqs, |scan_enable_dq, |scan_enable_dm})
	    4'b1000 : scan_dout = {{ (MEM_IF_DWIDTH - (NUM_CLKDIVS_PER_GRP * MEM_IF_DQS_WIDTH) ){1'b0}}, scan_dout_dqs_config};
	    4'b0100 : scan_dout = {{ (MEM_IF_DWIDTH - MEM_IF_DQS_WIDTH){1'b0}}, scan_dout_dqs};  
	    4'b0010 : scan_dout = scan_dout_dq; 
	    default : scan_dout = {{(MEM_IF_DWIDTH - MEM_IF_DM_WIDTH){1'b0}}, scan_dout_dm};
	    
	    endcase

	end
	
    end
    
    else
    begin : hcx_scan_dout_mux_no_dm

	always @*
	begin

	    casez({|scan_enable_dqs_config, |scan_enable_dqs, |scan_enable_dq})
	    3'b100  : scan_dout = {{ (MEM_IF_DWIDTH - (NUM_CLKDIVS_PER_GRP * MEM_IF_DQS_WIDTH) ){1'b0}}, scan_dout_dqs_config};
	    3'b010  : scan_dout = {{ (MEM_IF_DWIDTH - MEM_IF_DQS_WIDTH){1'b0}}, scan_dout_dqs};     
	    default : scan_dout = scan_dout_dq;
	    
	    endcase

	end
    
    end

endgenerate



endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_dq_io (
                                   phy_clk_1x,
                                   write_clk_2x,
                                   mem_clk_2x,
                                   resync_clk_2x,
                                   resync_clk_1x,
                                   sc_clk,
                                   scan_din,
                                   scan_dout,
                                   scan_update,
                                   scan_enable,
                                   mem_d,
                                   mem_dq,
                                   dqs_enable,
                                   dqsn_enable,
                                   dio_rdata3_1x,
                                   dio_rdata2_1x,
                                   dio_rdata1_1x,
                                   dio_rdata0_1x,
                                   wdp_wdata3_1x,
                                   wdp_wdata2_1x,
                                   wdp_wdata1_1x,
                                   wdp_wdata0_1x,
                                   wdp_wdata_oe_h_1x,
                                   wdp_wdata_oe_l_1x,
                                   wdp_oct_h_1x,
                                   wdp_oct_l_1x,
                                   dedicated_dll_delay_ctrl,
                                   dqoutputphasesetting,
                                   dqoutputphaseinvert,
                                   enaoutputcycledelaysetting,
                                   enaoutputphasetransferreg,
                                   resyncinputphasesetting,
                                   enainputcycledelaysetting,
                                   enainputphasetransferreg,
                                   resyncinputphaseinvert,
                                   enadataoutbypass,
                                   dqsoutputphasesetting,
                                   dqsoutputphaseinvert,
                                   enaoctcycledelaysetting,
                                   enaoctphasetransferreg,
                                   octdelaysetting1,
                                   octdelaysetting2,

                                   seriesterminationcontrol,
                                   parallelterminationcontrol
                             ) /* synthesis altera_attribute="SUPPRESS_DA_RULE_INTERNAL=D101" */ ;

parameter MEM_IF_DWIDTH              =      64;
parameter MEM_IF_MEMTYPE             =   "DDR";
parameter LEVELLING                  =       1;
parameter MEM_IF_OCT_EN              =       0;
parameter MEM_IF_DQS_WIDTH           =       8;
parameter DLL_DELAY_BUFFER_MODE      =  "HIGH";
parameter DWIDTH_RATIO               =       4;

parameter ENABLE_DDR3_SEQUENCER      = "FALSE";
parameter DQS_DELAY_CTL_WIDTH        =       6;

parameter MEM_IF_CAPT_T1_DESKEW_EN                = 0;
parameter MEM_IF_WR_T9_DESKEW_EN                  = 0;
parameter MEM_IF_WR_T10_DESKEW_EN                 = 0;
parameter MEM_IF_OCT_T9_DESKEW_EN                 = 0;
parameter MEM_IF_OCT_T10_DESKEW_EN                = 0;
parameter MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL = "FALSE";
parameter OPA_USES_DELAYED_CLK                    = "false";
parameter SINGLE_LEVELLING_DELAY_CHAIN            = "true";

input  wire                                                phy_clk_1x;
input  wire                                                write_clk_2x;
input  wire                                                mem_clk_2x;
input  wire                                                resync_clk_2x;
input  wire                                                resync_clk_1x;
input  wire                                                sc_clk;
input  wire                                                scan_din;
input  wire                                                scan_update;
input  wire                                                scan_enable;
output  wire                                               scan_dout;

inout  wire                                                mem_dq;
output  wire                                               mem_d; // For QDRII only

input  wire                                                dqs_enable;
input  wire                                                dqsn_enable;

output wire                                                dio_rdata3_1x;
output wire                                                dio_rdata2_1x;
output wire                                                dio_rdata1_1x;
output wire                                                dio_rdata0_1x;

input  wire                                                wdp_wdata3_1x;
input  wire                                                wdp_wdata2_1x;
input  wire                                                wdp_wdata1_1x;
input  wire                                                wdp_wdata0_1x;

input  wire                                                wdp_wdata_oe_h_1x;
input  wire                                                wdp_wdata_oe_l_1x;

input  wire                                                wdp_oct_h_1x;
input  wire                                                wdp_oct_l_1x;

input  wire [ DQS_DELAY_CTL_WIDTH - 1 : 0 ]                dedicated_dll_delay_ctrl;

input  wire [`DQSCONFIG_DQ_OP_PHASE_SETTING_WIDTH-1    :0] dqoutputphasesetting;
input  wire                                                dqoutputphaseinvert;
input  wire                                                enaoutputcycledelaysetting;
input  wire                                                enaoutputphasetransferreg;
input  wire [`DQSCONFIG_RESYNC_IP_PHASE_SETTING_WIDTH-1:0] resyncinputphasesetting;
input  wire                                                enainputcycledelaysetting;
input  wire                                                enainputphasetransferreg;
input  wire                                                resyncinputphaseinvert;
input  wire                                                enadataoutbypass;

//OCT only :
input wire [`DQSCONFIG_DQS_OUTPUT_PHASE_SETTING_WIDTH-1:0] dqsoutputphasesetting;
input wire                                                 dqsoutputphaseinvert;
input wire                                                 enaoctcycledelaysetting;
input wire                                                 enaoctphasetransferreg;
input wire [`DQSCONFIG_DQS_OCT_DELAY_SETTING1_WIDTH-1 :0]  octdelaysetting1;
input wire [`DQSCONFIG_DQS_OCT_DELAY_SETTING2_WIDTH-1 :0]  octdelaysetting2;

input wire [`OCT_SERIES_TERM_CONTROL_WIDTH   -1 : 0]       seriesterminationcontrol;
input wire [`OCT_PARALLEL_TERM_CONTROL_WIDTH -1 : 0]       parallelterminationcontrol;

wire  [`IOCONFIG_DQ_PAD_TO_IP_REG_DELAY_SETTING_WIDTH-1:0] dq_padtoinputregisterdelaysetting;
wire  [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING1_WIDTH-1      :0] dq_outputdelaysetting1;
wire  [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING2_WIDTH-1      :0] dq_outputdelaysetting2;



wire dq_output_ibuf;
wire dq_t1_delay_dataout;

wire dqi_captured_h;
wire dqi_captured_l;

wire dqi_aligned_h;
wire dqi_aligned_l;

wire dqo_l;
wire dqo_h;
wire dqo_aligned;
wire dqo_delayed;
wire dqo_delayed2;


wire dqoe;
wire dqoe_aligned;
wire dqoe_delayed;
wire dqoe_delayed2;


wire dqoct;
wire dqoct_aligned;
wire dqoct_delayed;
wire dqoct_delayed2;


// Write side :
generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate_dqo_gen

        stratixiii_ddio_out # (
            .half_rate_mode("true"),
            .use_new_clocking_model("true")
        ) dqo_ddio_in_h (
            .datainhi (wdp_wdata3_1x),
            .datainlo (wdp_wdata2_1x),
            .clkhi    (phy_clk_1x),
            .clklo    (phy_clk_1x),
            .muxsel   (phy_clk_1x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
            .clk(),
            // synopsys translate_on
            .dataout (dqo_h),
            .devclrn (),
            .devpor ()
        );

        stratixiii_ddio_out # (
            .half_rate_mode("true"),
            .use_new_clocking_model("true")
        ) dqo_ddio_in_l (
            .datainhi (wdp_wdata1_1x),
            .datainlo (wdp_wdata0_1x),
            .clkhi    (phy_clk_1x),
            .clklo    (phy_clk_1x),
            .muxsel   (phy_clk_1x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
            .clk(),
            // synopsys translate_on
            .dataout (dqo_l),
            .devclrn (),
            .devpor ()
        );

    end

    else
    begin : full_rate_dqo_gen
        assign dqo_l = wdp_wdata0_1x;
        assign dqo_h = wdp_wdata1_1x;
    end

endgenerate

// Phase alignment is either via DDIO for DDR/DDR2 or phase alignment atom for DDR3 :
generate

    if (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 1)
    begin : ddr3_dq_opa_gen

        // Note : delay_buffer_mode for output_phase_alignment atoms must always
        // be tied to "high" :
        stratixiii_output_phase_alignment #(
            .operation_mode                  ("ddio_out"),
            .use_phasectrlin                 (ENABLE_DDR3_SEQUENCER),
            .phase_setting                   (0),
            .delay_buffer_mode               ("high"),
            .power_up                        ("low"),
            .async_mode                      ("clear"),
            .sync_mode                       ("none"),
            .add_output_cycle_delay          ("dynamic"),
            .use_delayed_clock               (OPA_USES_DELAYED_CLK),
            .phase_setting_for_delayed_clock (2),
            .add_phase_transfer_reg          ("dynamic"),
            .use_phasectrl_clock             ("true"),
            .invert_phase                    ("dynamic"),
            .use_primary_clock               (SINGLE_LEVELLING_DELAY_CHAIN),
            .bypass_input_register           ("false")
        ) dq_opa_inst(
            .datain                ({dqo_h,dqo_l}),
            .clk                   (write_clk_2x),
            .delayctrlin           (dedicated_dll_delay_ctrl),
            .phasectrlin           (dqoutputphasesetting),
            .areset                (1'b0),
            .sreset                (1'b0),
            .clkena                (1'b1),
            .enaoutputcycledelay   (enaoutputcycledelaysetting),
            .enaphasetransferreg   (enaoutputphasetransferreg),
            .phaseinvertctrl       (dqoutputphaseinvert),
            // synopsys translate_off
            .delaymode(),
            .dutycycledelayctrlin(),
            .devclrn(),
	    .devpor(),
            .dffin(),
	    .dff1t(),
	    .dffddiodataout(),
            // synopsys translate_on
            .dataout               (dqo_aligned)
        );

    end

    // For DDR, DDR2, DDR3 (non-levelling solution) and QDRII, use a DDIO_OUT atom:
    else
    begin : ddr_qdr_dq_ddio_out_gen

        // DDIO output
        stratixiii_ddio_out # (
            .half_rate_mode("false"),
            .use_new_clocking_model("true")
        ) dq_ddio_inst (
            .datainhi (dqo_h),
            .datainlo (dqo_l),
            .clkhi    (write_clk_2x),
            .clklo    (write_clk_2x),
            .muxsel   (write_clk_2x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
            .clk (),
            // synopsys translate_on
            .dataout (dqo_aligned),
            .devclrn (),
            .devpor ()
        );
    end

endgenerate



// Switch to OE side :

generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate_dqoe_gen

        stratixiii_ddio_out # (
            .half_rate_mode("true"),
            .use_new_clocking_model("true")
        ) dqoe_ddio_in_h(
            .datainhi (~wdp_wdata_oe_h_1x),
            .datainlo (~wdp_wdata_oe_l_1x),
            .clkhi    (phy_clk_1x),
            .clklo    (phy_clk_1x),
            .muxsel   (phy_clk_1x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
            .clk(),
            // synopsys translate_on
            .dataout (dqoe),
            .devclrn (),
            .devpor ()
        );

    end

    else
    begin : full_rate_dqoe_gen
        assign dqoe = ~wdp_wdata_oe_l_1x;
    end

endgenerate

// Phase alignment is either via DDIO for DDR/DDR2 or phase alignment atom for DDR3 :
generate

    if (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 1)
    begin :ddr3_opa_gen

        // output_phase_alignment of oe

        // Note : delay_buffer_mode for output_phase_alignment atoms must always
        // be tied to "high" :
        stratixiii_output_phase_alignment # (
            .operation_mode                  ("oe"),
            .use_phasectrlin                 (ENABLE_DDR3_SEQUENCER),
            .phase_setting                   (0),
            .delay_buffer_mode               ("high"),
            .power_up                        ("low"),
            .async_mode                      ("clear"),
            .sync_mode                       ("none"),
            .add_output_cycle_delay          ("dynamic"),
            .use_delayed_clock               (OPA_USES_DELAYED_CLK),
            .phase_setting_for_delayed_clock (2),
            .add_phase_transfer_reg          ("dynamic"),
            .use_phasectrl_clock             ("true"),
            .invert_phase                    ("dynamic"),
            .use_primary_clock               (SINGLE_LEVELLING_DELAY_CHAIN),
            .bypass_input_register           ("false")
         ) dq_oepa_inst(
            .datain		   ({1'b0,dqoe}),
            .clk		   (write_clk_2x),
            .delayctrlin	   (dedicated_dll_delay_ctrl),
            .phasectrlin	   (dqoutputphasesetting),
            .areset		   (1'b0),
            .sreset		   (1'b0),
            .clkena		   (1'b1),
            .enaoutputcycledelay   (enaoutputcycledelaysetting),
            .enaphasetransferreg   (enaoutputphasetransferreg),
            .phaseinvertctrl	   (dqoutputphaseinvert),
            // synopsys translate_off
           .delaymode(),
           .dutycycledelayctrlin(),
           .devclrn(),
	   .devpor(),
           .dffin(),
	   .dff1t(),
	   .dffddiodataout(),
           // synopsys translate_on
           .dataout		  (dqoe_aligned)
         );

    end

    // For DDR, DDR2, DDR3 (non-levelling solution) and QDRII, use a DFF atom:
    else
    begin : ddr_dff_gen

        // The attribute here ensures the flop is not reduced, and is placed in the I/O :
        (* altera_attribute = "-name FAST_OUTPUT_ENABLE_REGISTER ON" *) dff dq_oedff_inst(
            .d          (dqoe),
            .clk        (write_clk_2x),
            .clrn       (1'b1),
            .prn        (),
            .q          (dqoe_aligned)
            );

     end

endgenerate


generate
    if (MEM_IF_WR_T9_DESKEW_EN == 1)
    begin : gen_T9_dp_deskew
        stratixiii_delay_chain dqoe_t9_delay(
            .datain             (dqoe_aligned),
            .delayctrlin        (dq_outputdelaysetting1),
            // synopsys translate_off
            .finedelayctrlin(),
            .devclrn(),
	    .devpor(),
            // synopsys translate_on
           .dataout             (dqoe_delayed)
        );

        stratixiii_delay_chain dqo_t9_delay(
            .datain             (dqo_aligned),
            .delayctrlin        (dq_outputdelaysetting1),
            // synopsys translate_off
            .finedelayctrlin(),
            .devclrn(),
	    .devpor(),
            // synopsys translate_on
           .dataout             (dqo_delayed)
        );

    end
    else
    begin : gen_T9_dp_no_deskew
        assign dqoe_delayed = dqoe_aligned;
        assign dqo_delayed  = dqo_aligned;
    end
endgenerate

generate
    if (MEM_IF_WR_T10_DESKEW_EN == 1)
    begin : gen_T10_dp_deskew
        stratixiii_delay_chain dqoe_t10_delay(
            .datain             (dqoe_delayed),
            .delayctrlin        ({1'b0, dq_outputdelaysetting2}),
            // synopsys translate_off
            .devclrn(), .devpor(),
            // synopsys translate_on
           .dataout             (dqoe_delayed2)
        );

        stratixiii_delay_chain dqo_t10_delay(
            .datain             (dqo_delayed),
            .delayctrlin        ({1'b0, dq_outputdelaysetting2}),
            // synopsys translate_off
            .devclrn(), .devpor(),
            // synopsys translate_on
           .dataout             (dqo_delayed2)
        );

    end
    else
    begin : gen_T10_dp_no_deskew
        assign dqoe_delayed2 = dqoe_delayed;
        assign dqo_delayed2  = dqo_delayed;
    end
endgenerate


// Switch to OCT if required :
generate

    if (MEM_IF_OCT_EN == 1)
    begin : oct_gen

        if (DWIDTH_RATIO == 4)
        begin : half_rate_dqoct_gen

            stratixiii_ddio_out # (
                .half_rate_mode("true"),
                .use_new_clocking_model("true")
            ) dqoct_ddio_in_h(
                .datainhi (wdp_oct_h_1x),
                .datainlo (wdp_oct_l_1x),
                .clkhi    (phy_clk_1x),
                .clklo    (phy_clk_1x),
                .muxsel   (phy_clk_1x),
                .ena      (1'b1),
                .areset   (1'b0),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (dqoct),
                .devclrn (),
                .devpor ()
            );

        end

        else
        begin : full_rate_dqoct_gen
            assign dqoct = wdp_oct_l_1x;
        end

        // Phase alignment is either via DDIO for DDR/DDR2/DDR3(non-levelling) or phase alignment atom for DDR3 :
        if (MEM_IF_MEMTYPE == "DDR" || MEM_IF_MEMTYPE == "DDR2" || (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 0))
        begin : ddio_oe_oct_gen

            // DDIO OE output
            stratixiii_ddio_oe dqoct_ddio_inst (
                .oe      (dqoct),
                .clk     (mem_clk_2x),
                .ena     (1'b1),
                .areset  (1'b0),
                .sreset  (1'b0),
                .dataout (dqoct_aligned),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                // synopsys translate_on
                .devclrn (),
                .devpor ()
            );

        end

        else
        begin : opa_oct_gen

            // output_phase_alignment of oct

            // Note : delay_buffer_mode for output_phase_alignment atoms must always
            // be tied to "high" :
            stratixiii_output_phase_alignment # (
                .operation_mode                  ("rtena"),
                .use_phasectrlin                 (ENABLE_DDR3_SEQUENCER),
                .phase_setting                   (2),                     // to match DQS
                .delay_buffer_mode               ("high"),
                .power_up                        ("low"),
                .async_mode                      ("none"),
                .sync_mode                       ("none"),
                .add_output_cycle_delay          ("dynamic"),
                .use_delayed_clock               (OPA_USES_DELAYED_CLK),
                .phase_setting_for_delayed_clock (2),
                .add_phase_transfer_reg          ("dynamic"),
                .use_phasectrl_clock             ("true"),
                .invert_phase                    ("dynamic"),
                .use_primary_clock               ("true"),
                .bypass_input_register           ("false")
             ) dq_octpa_inst(
                 .datain                ({1'b0,dqoct}),
                 .clk                   (write_clk_2x),
                 .delayctrlin           (dedicated_dll_delay_ctrl),
                 .phasectrlin           (dqsoutputphasesetting),
                 .areset                (1'b0),
                 .sreset                (1'b0),
                 .clkena                (1'b1),
                 .enaoutputcycledelay   (enaoctcycledelaysetting),
                 .enaphasetransferreg   (enaoctphasetransferreg),
                 .phaseinvertctrl       (dqsoutputphaseinvert),
                 // synopsys translate_off
                 .devclrn(), .devpor(),
                 .dffin(), .dff1t(), .dffddiodataout(),
                 // synopsys translate_on
                 .dataout               (dqoct_aligned)
             );
        end


            if (MEM_IF_OCT_T9_DESKEW_EN == 1)
            begin : gen_T9_OCT_deskew
                stratixiii_delay_chain  dqoct_t9_delay(
                    .datain             (dqoct_aligned),
                    .delayctrlin        (octdelaysetting1),
                    // synopsys translate_off
                    .devclrn(), .devpor(),
                    // synopsys translate_on
                   .dataout             (dqoct_delayed)
                );
            end
            else
            begin : gen_T9_OCT_no_deskew
                assign dqoct_delayed = dqoct_aligned;
            end

            if (MEM_IF_OCT_T10_DESKEW_EN == 1)
            begin : gen_T10_OCT_deskew
                stratixiii_delay_chain dqoct_t10_delay(
                    .datain             (dqoct_delayed),
                    .delayctrlin        ({1'b0, octdelaysetting2}),
                    // synopsys translate_off
                    .devclrn(), .devpor(),
                    // synopsys translate_on
                   .dataout             (dqoct_delayed2)
                );
            end
            else
            begin : gen_T10_OCT_no_deskew
                assign dqoct_delayed2 = dqoct_delayed;
            end

    end // if MEM_IF_OCT_EN

    // No OCT :
    else
    begin : no_oct_gen
        assign dqoct_delayed2 = 1'b0;
    end


endgenerate





generate

    // QDR has unidirectional data :
    if (MEM_IF_MEMTYPE == "QDRII")
    begin : gen_qdr_obuf

        // output buf
        stratixiii_io_obuf # (
           .bus_hold                         ( "false"),
           .open_drain_output                ( "false"),
           .shift_series_termination_control (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL)
        ) dq_obuf_inst(
            .i                         (dqo_delayed2),
            .oe                        (~dqoe_delayed2),
            .dynamicterminationcontrol (dqoct_delayed2),
            // synopsys translate_off
            .seriesterminationcontrol  (seriesterminationcontrol),
            .parallelterminationcontrol(parallelterminationcontrol),
            .obar                      (),
            // synopsys translate_on
            .o                         (mem_d)
        );

    end

    // This is the same for DDR2 and DDR3 modes - bidirectional:
    else
    begin : gen_ddr_obuf

        stratixiii_io_obuf # (
           .bus_hold                         ( "false"),
           .open_drain_output                ( "false"),
           .shift_series_termination_control (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL)
        ) dq_obuf_inst(
            .i                         (dqo_delayed2),
            .oe                        (~dqoe_delayed2),
            .dynamicterminationcontrol (dqoct_delayed2),
            // synopsys translate_off
            .seriesterminationcontrol  (seriesterminationcontrol),
            .parallelterminationcontrol(parallelterminationcontrol),
            .obar                      (),
            // synopsys translate_on
            .devoe                     (),
            .o                         (mem_dq)
        );

        assign mem_d = 1'b0;

    end

endgenerate



// DQ Read data path

// Note that this is the same for DDR2 and DDR3 modes :

// Input buf
stratixiii_io_ibuf # (
    .simulate_z_as("gnd")
    ) dqi_io_ibuf(
    .i      (mem_dq),
    .ibar   (),
    // synopsys translate_off
    .dynamicterminationcontrol(),
    // synopsys translate_on
    .o      (dq_output_ibuf)
    );


generate
    if (MEM_IF_CAPT_T1_DESKEW_EN == 1)
    begin : gen_dq_input_with_deskeq
        stratixiii_delay_chain  dqi_t1_delay (
            .datain             (dq_output_ibuf),
            .delayctrlin        (dq_padtoinputregisterdelaysetting),
            // synopsys translate_off
            .finedelayctrlin(),
            .devclrn(),
	    .devpor(),
            // synopsys translate_on
           .dataout             (dq_t1_delay_dataout)
        );
    end
    else
    begin : gen_dq_input_without_deskew
        assign dq_t1_delay_dataout = dq_output_ibuf;
    end
endgenerate


generate

    if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdr_dq_ddio_in_gen

        // Capture DQ read data
        stratixiii_ddio_in # (
            .async_mode ("clear"),
            .power_up   ("low"),
            .sync_mode  ("none"),
            .use_clkn   ("true")
        ) dqi_ddio_in (
            .datain     (dq_t1_delay_dataout),
            .clk        (dqs_enable),
            .clkn       (dqsn_enable),
            .ena        (1'b1),
            .areset     (1'b0),
            .sreset     (),
            .regoutlo   (dqi_captured_l),
            .regouthi   (dqi_captured_h),
            .dfflo      (),
            .devclrn    (),
            .devpor     ()

        );

    end

    else
    begin : ddr_dq_ddio_in_gen

        // Capture DQ read data
        stratixiii_ddio_in # (
            .async_mode ("clear"),
            .power_up   ("low"),
            .sync_mode  ("none"),
            .use_clkn   ("false")
        ) dqi_ddio_in (
            .datain     (dq_t1_delay_dataout),
            .clk        (~dqs_enable),
            .clkn       (),
            .ena        (1'b1),
            .areset     (1'b0),
            .sreset     (),
            .regoutlo   (dqi_captured_l),
            .regouthi   (dqi_captured_h),
            .dfflo      (),
            .devclrn    (),
            .devpor     ()
        );
    end

endgenerate


// The input phase align atoms on the DQ input path should have "use_phasectrlin"
// set to FALSE and "bypass_output_register" set to TRUE for DDR/DDR2 :
generate

    if (MEM_IF_MEMTYPE == "DDR" || MEM_IF_MEMTYPE == "DDR2" || (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 0))
    begin : ddr_ipa_gen

        // Resynchronize captured read data

        // Note : delay_buffer_mode for input_phase_alignment atoms must always
        // be tied to "high" :
        stratixiii_input_phase_alignment # (
            .use_phasectrlin        ("false"),
            .phase_setting          (0),
            .delay_buffer_mode      ("high"),
            .power_up               ("low"),
            .async_mode             ("clear"),
            .add_input_cycle_delay  ("false"),
            .bypass_output_register ("true"),
            .add_phase_transfer_reg ("false"),
            .invert_phase           ("false")
        ) dqi_ipa_h (
            .datain               (dqi_captured_h),
            .clk                  (resync_clk_2x),
            .delayctrlin          (dedicated_dll_delay_ctrl),
            .phasectrlin          (resyncinputphasesetting),
            .areset               (1'b0),
            .enainputcycledelay   (enainputcycledelaysetting),
            .enaphasetransferreg  (enainputphasetransferreg),
            .phaseinvertctrl      (resyncinputphaseinvert),
            // synopsys translate_off
            .devclrn              (),
            .devpor               (),
            .dffin                (),
            .dff1t                (),
            // synopsys translate_on
            .dataout              (dqi_aligned_h)
        );

        // Note : delay_buffer_mode for input_phase_alignment atoms must always
        // be tied to "high" :
        stratixiii_input_phase_alignment # (
            .use_phasectrlin           ("false"),
            .phase_setting             (0),
            .delay_buffer_mode         ("high"),
            .power_up                  ("low"),
            .async_mode                ("clear"),
            .add_input_cycle_delay     ("false"),
            .bypass_output_register    ("true"),
            .add_phase_transfer_reg    ("false"),
            .invert_phase              ("false")
        ) dqi_ipa_l (
            .datain              (dqi_captured_l),
            .clk                 (resync_clk_2x),
            .delayctrlin         (dedicated_dll_delay_ctrl),
            .phasectrlin         (resyncinputphasesetting),
            .areset              (1'b0),
            .enainputcycledelay  (enainputcycledelaysetting),
            .enaphasetransferreg (enainputphasetransferreg),
            .phaseinvertctrl     (resyncinputphaseinvert),
            // synopsys translate_off
            .devclrn             (),
            .devpor              (),
            .dffin               (),
            .dff1t               (),
            // synopsys translate_on
            .dataout             (dqi_aligned_l)
        );

    end

    else if (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 1)
    begin : ddr3_ipa_gen

        // Resynchronize captured read data

        // Note : delay_buffer_mode for input_phase_alignment atoms must always
        // be tied to "high" :
        stratixiii_input_phase_alignment # (
            .use_phasectrlin        ("true"),
            .phase_setting          (0),
            .delay_buffer_mode      ("high"),
            .power_up               ("low"),
            .async_mode             ("clear"),
            .add_input_cycle_delay  ("dynamic"), //dynamic
            .bypass_output_register ("false"),
            .add_phase_transfer_reg ("dynamic"),
            .invert_phase           ("dynamic")
        ) dqi_ipa_h (
            .datain               (dqi_captured_h),
            .clk                  (resync_clk_2x),
            .delayctrlin          (dedicated_dll_delay_ctrl),
            .phasectrlin          (resyncinputphasesetting),
            .areset               (1'b0),
            .enainputcycledelay   (enainputcycledelaysetting),
            .enaphasetransferreg  (enainputphasetransferreg),
            .phaseinvertctrl      (resyncinputphaseinvert),
            // synopsys translate_off
            .devclrn              (),
            .devpor               (),
            .dffin                (),
            .dff1t                (),
            // synopsys translate_on
            .dataout              (dqi_aligned_h)
        );

        // Note : delay_buffer_mode for input_phase_alignment atoms must always
        // be tied to "high" :
        stratixiii_input_phase_alignment # (
            .use_phasectrlin           ("true"),
            .phase_setting             (0),
            .delay_buffer_mode         ("high"),
            .power_up                  ("low"),
            .async_mode                ("clear"),
            .add_input_cycle_delay     ("dynamic"), //dynamic
            .bypass_output_register    ("false"),
            .add_phase_transfer_reg    ("dynamic"),
            .invert_phase              ("dynamic")
        ) dqi_ipa_l (
            .datain              (dqi_captured_l),
            .clk                 (resync_clk_2x),
            .delayctrlin         (dedicated_dll_delay_ctrl),
            .phasectrlin         (resyncinputphasesetting),
            .areset              (1'b0),
            .enainputcycledelay  (enainputcycledelaysetting),
            .enaphasetransferreg (enainputphasetransferreg),
            .phaseinvertctrl     (resyncinputphaseinvert),
            // synopsys translate_off
            .devclrn             (),
            .devpor              (),
            .dffin               (),
            .dff1t               (),
            // synopsys translate_on
            .dataout             (dqi_aligned_l)
        );

    end

    else // QDR-II has no input phase alignment :
    begin : qdr_no_ipa_gen
        assign dqi_aligned_l = dqi_captured_l;
        assign dqi_aligned_h = dqi_captured_h;
    end

endgenerate


generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate_dqi_gen

        if ((MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 1) && ((ENABLE_DDR3_SEQUENCER === "TRUE") || (ENABLE_DDR3_SEQUENCER === "true")))
        begin : gen_hri_with_dataoutbypass

            stratixiii_half_rate_input # (
                .power_up ("low"),
                .async_mode ("none"),
                .use_dataoutbypass ("true")
            ) dqi_hrate(
                .datain         ({dqi_aligned_h,dqi_aligned_l}),
                .directin       (dq_output_ibuf),
                .clk            (resync_clk_1x),
                .areset         (),
                .dataoutbypass  (enadataoutbypass),
                // synopsys translate_off
                .devclrn        (),
                .devpor         (),
                .dffin          (),
                // synopsys translate_on
                .dataout        ({dio_rdata3_1x, dio_rdata2_1x, dio_rdata1_1x, dio_rdata0_1x})
            );

        end

        // For DDR and QDRII dataoutpypass for write levelling is not required :
        else
        begin : gen_hri_no_dataoutbypass

            stratixiii_half_rate_input # (
                .power_up ("low"),
                .async_mode ("none"),
                .use_dataoutbypass ("false")
            ) dqi_hrate(
                .datain         ({dqi_aligned_h,dqi_aligned_l}),
                .directin       (dq_output_ibuf),
                .clk            (resync_clk_1x),
                .areset         (),
                .dataoutbypass  (enadataoutbypass),
                // synopsys translate_off
                .devclrn        (),
                .devpor         (),
                .dffin          (),
                // synopsys translate_on
                .dataout        ({dio_rdata3_1x, dio_rdata2_1x, dio_rdata1_1x, dio_rdata0_1x})
            );

        end

    end

    else
    begin : full_rate_dqi_gen
        assign dio_rdata0_1x = dqi_aligned_l;
        assign dio_rdata1_1x = dqi_aligned_h;
    end

endgenerate


generate

    // QDR has unidirectional data :
    if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdr_io_config_gen

        // IO_CONFIG - num_dq 0->16
        stratixiii_io_config d_io_config(
            .datain                        (scan_din),          // shared per DQS group
            .clk                           (sc_clk),
            .ena                           (scan_enable),
            .update                        (scan_update),       // shared per DQS group
            // synopsys translate_off
            .devclrn(), .devpor(),
            .dutycycledelaymode(),
            .dutycycledelaysettings(),
            .outputfinedelaysetting1(),
            .outputfinedelaysetting2(),
            .outputonlydelaysetting2(),
            .outputonlyfinedelaysetting2(),
            .padtoinputregisterfinedelaysetting(),
            // synopsys translate_on
            .padtoinputregisterdelaysetting(),
            .outputdelaysetting1           (dq_outputdelaysetting1),
            .outputdelaysetting2           (dq_outputdelaysetting2),
            .dataout                       ()
        );

        // IO_CONFIG - num_dq 0->16
        stratixiii_io_config dq_io_config(
            .datain                        (scan_din),          // shared per DQS group
            .clk                           (sc_clk),
            .ena                           (scan_enable),
            .update                        (scan_update),       // shared per DQS group
            // synopsys translate_off
            .devclrn(), .devpor(),
            .dutycycledelaymode(),
            .dutycycledelaysettings(),
            .outputfinedelaysetting1(),
            .outputfinedelaysetting2(),
            .outputonlydelaysetting2(),
            .outputonlyfinedelaysetting2(),
            .padtoinputregisterfinedelaysetting(),
            // synopsys translate_on
            .padtoinputregisterdelaysetting(dq_padtoinputregisterdelaysetting),
            .outputdelaysetting1           (),
            .outputdelaysetting2           (),
            .dataout                       ()
        );

    end

    // For DDR memories, where there is only 1 DQ pin per IO channel, use one IO config block :
    else
    begin : ddr_io_config_gen

        // IO_CONFIG - num_dq 0->16
        stratixiii_io_config dq_io_config(
            .datain                        (scan_din),          // shared per DQS group
            .clk                           (sc_clk),
            .ena                           (scan_enable),
            .update                        (scan_update),       // shared per DQS group
            // synopsys translate_off
            .devclrn(), .devpor(),
            .dutycycledelaymode(),
            .dutycycledelaysettings(),
            .outputfinedelaysetting1(),
            .outputfinedelaysetting2(),
            .outputonlydelaysetting2(),
            .outputonlyfinedelaysetting2(),
            .padtoinputregisterfinedelaysetting(),
            // synopsys translate_on
            .padtoinputregisterdelaysetting(dq_padtoinputregisterdelaysetting),
            .outputdelaysetting1           (dq_outputdelaysetting1),
            .outputdelaysetting2           (dq_outputdelaysetting2),
            .dataout                       (scan_dout)
        );

    end

endgenerate



endmodule


//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_clk_reset (
                               pll_ref_clk,
                               global_reset_n,
                               soft_reset_n,

                               seq_rdp_reset_req_n,
                               seq_qdr_doff_req_n,

                               resync_clk_1x,
                               ac_clk_1x,
                               measure_clk_1x,
                               mem_clk_2x,
                               mem_clk,
                               mem_clk_n,
                               phy_clk_1x,
                               postamble_clk_2x,
                               resync_clk_2x,
                               cs_n_clk_1x,
                               write_clk_2x,
                               aux_clk,
                               scan_clk,
                               aux_scan_clk_reset_n,    // Connection New for 9.0

                               scan_clk_in,

                               scan_enable_ck,
                               scan_update,
                               scan_din,

                               reset_ac_clk_1x_n,
                               reset_measure_clk_1x_n,
                               reset_mem_clk_2x_n,
                               reset_phy_clk_1x_n,
                               reset_rdp_phy_clk_1x_n,
                               reset_resync_clk_1x_n,
                               reset_resync_clk_2x_n,
                               reset_write_clk_2x_n,
                               reset_cs_n_clk_1x_n,
                               mem_reset_n,
                               mem_doff_n,

                               reset_request_n, // new output

                               dqs_delay_ctrl,
                               dqs_delay_ctrl_import,
                               dqs_delay_update_en,
                               dlloffset_addnsub,
                               dlloffset_offset,
                               dlloffset_offsetctrl_out,

                               phs_shft_busy,
                               seq_pll_inc_dec_n,
                               seq_pll_select,
                               seq_pll_start_reconfig,

                               mimic_data_1x,

                               seq_clk_disable,
                               ctrl_clk_disable,
                               
                               pll_reconfig_enable,       // Connection New for 9.0
                               pll_phasecounterselect,    // Connection New for 9.0
                               pll_phaseupdown,           // Connection New for 9.0
                               pll_phasestep,             // Connection New for 9.0
                               pll_phase_done             // Connection New for 9.0                               

                              ) /* synthesis altera_attribute=" SUPPRESS_DA_RULE_INTERNAL=\"R101,C104,C106\" ; AUTO_GLOBAL_REGISTER_CONTROLS=\"OFF\" "*/;

// Note the peculiar ranging below is necessary to use a generated CASE statement
// later in the code :
parameter AC_PHASE                     =      "MEM_CLK";
parameter CLOCK_INDEX_WIDTH            =              3;
parameter DDR_MIMIC_PATH_EN            =              1; // Only applicable for QDRII
parameter DLL_EXPORT_IMPORT            =         "NONE";
parameter LOCAL_IF_CLK_PS              =           4000;
parameter MEM_IF_CLK_PAIR_COUNT        =              3;
parameter MEM_IF_CLK_PS                =           4000;
parameter MEM_IF_CLK_PS_STR            =       "4000 ps";
parameter MEM_IF_DQ_PER_DQS            =              8;
parameter MEM_IF_DWIDTH                =             64;
parameter MEM_IF_DQS_WIDTH             =              8;
parameter MEM_IF_MEMTYPE               =          "DDR";
parameter LEVELLING                    =              1;
parameter READ_DESKEW_MODE             =         "NONE";
parameter WRITE_DESKEW_MODE            =         "NONE";
parameter MEM_IF_DQSN_EN               =              1;
parameter MIF_FILENAME                 =      "PLL.MIF";
parameter DWIDTH_RATIO                 =              4;
parameter PLL_EXPORT_IMPORT            =         "NONE";
parameter PLL_REF_CLK_PS               =           4000;
parameter PLL_TYPE                     =     "ENHANCED";
parameter SPEED_GRADE                  =           "C3";
parameter DLL_DELAY_BUFFER_MODE        =         "HIGH";
parameter DLL_DELAY_CHAIN_LENGTH       =              8;
parameter DQS_OUT_MODE                 = "DELAY_CHAIN2";
parameter DQS_PHASE                    =             72;
parameter SCAN_CLK_DIVIDE_BY           =              2;
parameter USE_MEM_CLK_FOR_ADDR_CMD_CLK =              1;
parameter DQS_DELAY_CTL_WIDTH          =              6;
parameter USE_DLL_OFFSET               =        "false";
parameter PLL_RECONFIG_PORTS_EN        =              0;

localparam RDP_RESET_PIPE_DEPTH = (MEM_IF_MEMTYPE == "QDRII") ? 1 : 3;

// Clock/reset inputs :
input                                                 global_reset_n;
input  wire                                           soft_reset_n;
input  wire                                           pll_ref_clk;

input  wire [MEM_IF_DQS_WIDTH - 1 : 0]                seq_rdp_reset_req_n;
input  wire                                           seq_qdr_doff_req_n;

input  wire [MEM_IF_DQS_WIDTH - 1 : 0]                resync_clk_1x; //NB. Input!

// Clock/reset outputs :

output                                                ac_clk_1x;
output                                                measure_clk_1x;
output                                                mem_clk_2x;
inout  [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                mem_clk;
inout  [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                mem_clk_n;
output                                                phy_clk_1x;
output                                                postamble_clk_2x;
output                                                resync_clk_2x;
output                                                cs_n_clk_1x;
output                                                write_clk_2x;
// The Aux clk shall be half-rate for full-rate mode and visa versa :
output                                                aux_clk;



output wire                                           reset_ac_clk_1x_n;
output wire                                           reset_measure_clk_1x_n;
output wire                                           reset_mem_clk_2x_n;
output reg                              	      reset_phy_clk_1x_n;
output wire [MEM_IF_DQS_WIDTH - 1 : 0]  	      reset_rdp_phy_clk_1x_n;
output wire [MEM_IF_DQS_WIDTH - 1 : 0]                reset_resync_clk_1x_n;
output wire                                           reset_resync_clk_2x_n;
output wire                                           reset_write_clk_2x_n;
output wire                                           reset_cs_n_clk_1x_n;
output wire                                           mem_reset_n;
output wire                                           mem_doff_n;

// This is the PLL locked signal :
output wire                                           reset_request_n;

// Misc I/O :
output wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]            dqs_delay_ctrl;

// For DDR3 only, when using an external DLL we need to import the code for the
// mem_clk output phase align block :
input  wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]            dqs_delay_ctrl_import;

output wire                                           dqs_delay_update_en;

// Note - although the DLL offset signals are present here, use of an offset when
// in "internal" DLL mode is unsupported (and would therefore prevent hardcopy migration).
// Customers requiring DLL offsets should use "external" DLL mode, as this is the
// supported flow :
input  wire                                           dlloffset_addnsub;        // Do not use
input  wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]            dlloffset_offset;         // Do not use
output wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]            dlloffset_offsetctrl_out; // Do not use


output wire                                           phs_shft_busy;

input  wire                                           seq_pll_inc_dec_n;
input  wire [CLOCK_INDEX_WIDTH - 1 : 0 ]              seq_pll_select;
input  wire                                           seq_pll_start_reconfig;

output wire                                           mimic_data_1x;

// Create the scan clock.  This is a divided-down version of the PLL reference clock.
// The scan chain will have an Fmax of around 100MHz, and so initially the scan clock is
// created by a divide-by 4 circuit to allow plenty of margin with the expected reference
// clock frequency of 100MHz.  This may be changed via the parameter.

output                                                scan_clk;
output                                                aux_scan_clk_reset_n;

localparam MEM_IF_STR_T9_DESKEW_EN                    = 0;
localparam MEM_IF_STR_T10_DESKEW_EN                   = 0;//uses_dekew_delay(MEM_IF_MEMTYPE, LEVELLING, WRITE_DESKEW_MODE);


input  wire                                           scan_clk_in;

input  wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]           scan_enable_ck;
input  wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]           scan_din;
input  wire                                           scan_update;

input wire                                            seq_clk_disable;
input wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]            ctrl_clk_disable;

// PLL reconfig interface.  Made available primarily for HardCopy customers :
input  wire         	                              pll_reconfig_enable;       // New for 9.0 - tie low if not required.
input  wire [3:0]        	                          pll_phasecounterselect;    // New for 9.0 - tie low if not required.
input  wire         	                              pll_phaseupdown;           // New for 9.0 - tie low if not required.
input  wire         	                              pll_phasestep;             // New for 9.0 - tie low if not required.

output wire         	                              pll_phase_done;            // New for 9.0


wire global_reset_n;

(* keep, altera_attribute = "-name global_signal  off" *) reg  scan_clk = 1'b0;

(* keep, altera_attribute = "-name global_signal dual_regional_clock" *) wire mem_clk_2x;





(* keep, altera_attribute = "-name global_signal        global_clock" *) wire phy_clk_1x;


wire aux_clk;
wire postamble_clk_2x;
(* keep, altera_attribute = "-name global_signal dual_regional_clock" *) wire resync_clk_2x;
(* keep, altera_attribute = "-name global_signal dual_regional_clock" *) wire write_clk_2x;

wire cs_n_clk_1x;

(* keep, altera_attribute = "-name global_signal dual_regional_clock" *) wire ac_clk_1x;
(* keep, altera_attribute = "-name global_signal dual_regional_clock" *) wire measure_clk_1x;

(* keep, altera_attribute = "-name global_signal                 off" *) wire phy_internal_reset_n;

wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                               mem_clk;
wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]                               mem_clk_n;


reg [2:0]                                    divider  = 3'h0;


(*preserve*) reg                             seq_pll_start_reconfig_ams;
(*preserve*) reg                             seq_pll_start_reconfig_r;
(*preserve*) reg                             seq_pll_start_reconfig_2r;
(*preserve*) reg                             seq_pll_start_reconfig_3r;

reg                                          pll_new_dir;
reg [CLOCK_INDEX_WIDTH - 1 : 0 ]             pll_new_phase;
wire                                         pll_phase_auto_calibrate_pulse;
(*preserve*) reg                             pll_reprogram_request;
wire                                         pll_locked_src;
reg                                          pll_locked;

(*preserve*) reg                             pll_reprogram_request_pulse;   // 1 scan clk cycle long
(*preserve*) reg                             pll_reprogram_request_pulse_r;
(*preserve*) reg                             pll_reprogram_request_pulse_2r;
wire                                         pll_reprogram_request_long_pulse; // 3 scan clk cycles long

(*preserve*) reg                             reset_master_ams;

wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]         mem_clk_pdiff_in;

wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]         mem_clk_buf_in;
wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]         mem_clk_n_buf_in;

reg                                          phs_shft_busy_siii;

(*preserve*) reg [2:0]                       seq_pll_start_reconfig_ccd_pipe;

(*preserve*) reg                             seq_pll_inc_dec_ccd;
(*preserve*) reg [CLOCK_INDEX_WIDTH - 1 : 0] seq_pll_select_ccd ;

(*preserve*) reg                             global_pre_clear;

wire                                         global_or_soft_reset_n;

(*preserve*) reg                             clk_div_reset_ams_n   = 1'b0;
(*preserve*) reg                             clk_div_reset_ams_n_r = 1'b0;

(*preserve*) reg                             pll_reconfig_reset_ams_n   = 1'b0;
(*preserve*) reg                             pll_reconfig_reset_ams_n_r = 1'b0;

wire                                         clk_divider_reset_n;

wire                                         qdr_doff_req_n;
(* altera_attribute = "-name global_signal  off" *) wire [MEM_IF_DQS_WIDTH - 1 : 0]              rdp_reset_req_n;

wire                                         pll_reset;

wire                                         fb_clk;

wire                                         pll_reconfig_reset_n;

wire                                         master_reset_resync_clk_1x;

wire [MEM_IF_DQS_WIDTH - 1 : 0]              reset_resync_clk_1x_pre_clear;

wire                                         dll_offset_delay_ctrl_clk;
wire [DQS_DELAY_CTL_WIDTH - 1 : 0]           dll_offset_delay_code;

wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]         mem_clk_pdiff_in_delayed1;
wire [MEM_IF_CLK_PAIR_COUNT - 1 : 0]         mem_clk_pdiff_in_delayed2;

wire [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING1_WIDTH-1:0] outputdelaysetting1[MEM_IF_CLK_PAIR_COUNT - 1 : 0];
wire [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING2_WIDTH-1:0] outputdelaysetting2[MEM_IF_CLK_PAIR_COUNT - 1 : 0];

// Hc-x requires access to the PLL phase stepping inputs, therefore they are multiplexed :
reg [3:0] 				                     muxed_pll_phasecounterselect;
reg 	  				                     muxed_pll_phasestep;     
reg 	  				                     muxed_pll_phaseupdown;   

wire                                         pll_reconfig_enable_internal;

genvar dqs_group;


function integer uses_dekew_delay (input [31:0] mem_type,
                                   input levelling,
                                   input [31:0] mode);
begin
    if (((mem_type == "DDR3") || (mem_type == "ddr3")) && levelling == 1 && ((mode != "NONE") && (mode != "none")))
        uses_dekew_delay = 1;
    else
        uses_dekew_delay = 0;
end
endfunction


// Output the PLL locked signal to be used as a reset_request_n - IE. reset when the PLL loses
// lock :
assign reset_request_n = pll_locked;



// Reset the scanclk clock divider if we either have a global_reset or the PLL loses lock
assign pll_reconfig_reset_n = global_reset_n && pll_locked;

// Delayed and re-synchronised to scan clk, reset output for external PLL reconfiguration users :
assign aux_scan_clk_reset_n = pll_reconfig_reset_ams_n_r;


// Clock divider circuit reset generation.
always @(posedge phy_clk_1x or negedge pll_reconfig_reset_n)
begin

    if (pll_reconfig_reset_n == 1'b0)
    begin
        clk_div_reset_ams_n   <= 1'b0;
        clk_div_reset_ams_n_r <= 1'b0;
    end

    else
    begin
        clk_div_reset_ams_n   <= 1'b1;
        clk_div_reset_ams_n_r <= clk_div_reset_ams_n;
    end

end


// PLL reconfig and synchronisation circuit reset generation.
always @(posedge scan_clk or negedge pll_reconfig_reset_n)
begin

    if (pll_reconfig_reset_n == 1'b0)
    begin
        pll_reconfig_reset_ams_n   <= 1'b0;
        pll_reconfig_reset_ams_n_r <= 1'b0;
    end

    else
    begin
        pll_reconfig_reset_ams_n   <= 1'b1;
        pll_reconfig_reset_ams_n_r <= pll_reconfig_reset_ams_n;
    end

end

// Create the scan clock.  Used for PLL reconfiguring in this block.

// Clock divider reset is the direct output of the AMS flops :
assign clk_divider_reset_n = clk_div_reset_ams_n_r;

generate

    if (SCAN_CLK_DIVIDE_BY == 1)
    begin : no_scan_clk_divider

        always @(phy_clk_1x)
        begin
            scan_clk = phy_clk_1x;
        end

    end

    else
    begin : gen_scan_clk

        always @(posedge phy_clk_1x or negedge clk_divider_reset_n)
        begin

            if (clk_divider_reset_n == 1'b0)
            begin
                scan_clk <= 1'b0;
                divider      <= 3'h0;
            end

            else
            begin

                // This method of clock division does not require "divider" to be used
                // as an intermediate clock:
                if (divider == (SCAN_CLK_DIVIDE_BY / 2 - 1))
                begin
                    scan_clk <= ~scan_clk; // Toggle
                    divider  <= 3'h0;
                end

                else
                begin
                    scan_clk <= scan_clk; // Do not toggle
                    divider  <= divider + 3'h1;
                end

            end

        end

    end

endgenerate


// NB. This lookup_sii table shall be different for CIII/SIII
function [3:0] lookup_siii;

input [CLOCK_INDEX_WIDTH-1:0] seq_num;
begin
    casez (seq_num)
    4'b0000  : lookup_siii = 4'b0010; // Legal code
    4'b0001  : lookup_siii = 4'b0011; // Legal code
    4'b0010  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b0011  : lookup_siii = 4'b0101; // Legal code
    4'b0100  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b0101  : lookup_siii = 4'b0110; // Legal code
    4'b0110  : lookup_siii = 4'b1000; // Legal code
    4'b0111  : lookup_siii = 4'b0111; // Legal code
    4'b1000  : lookup_siii = 4'b0100; // Legal code
    4'b1001  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b1010  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b1011  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b1100  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b1101  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b1110  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    4'b1111  : lookup_siii = 4'b1111; // illegal - return code 4'b1111
    default  : lookup_siii = 4'bxxxx; // X propagation
    endcase
end

endfunction


always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin

    if (reset_phy_clk_1x_n == 1'b0)
    begin
        seq_pll_inc_dec_ccd             <= 1'b0;
        seq_pll_select_ccd              <= {CLOCK_INDEX_WIDTH{1'b0}};
        seq_pll_start_reconfig_ccd_pipe <= 3'b000;
    end

    // Generate 'ccd' Cross Clock Domain signals :
    else
    begin

        seq_pll_start_reconfig_ccd_pipe <= {seq_pll_start_reconfig_ccd_pipe[1:0], seq_pll_start_reconfig};

        if (seq_pll_start_reconfig == 1'b1 && seq_pll_start_reconfig_ccd_pipe[0] == 1'b0)
        begin
            seq_pll_inc_dec_ccd <= seq_pll_inc_dec_n;
            seq_pll_select_ccd  <= seq_pll_select;
        end

    end

end
always @(posedge scan_clk or negedge pll_reconfig_reset_ams_n_r)
begin

    if (pll_reconfig_reset_ams_n_r == 1'b0)
    begin
        seq_pll_start_reconfig_ams     <= 1'b0;
        seq_pll_start_reconfig_r       <= 1'b0;
        seq_pll_start_reconfig_2r      <= 1'b0;
        seq_pll_start_reconfig_3r      <= 1'b0;

        pll_reprogram_request_pulse    <= 1'b0;
        pll_reprogram_request_pulse_r  <= 1'b0;
        pll_reprogram_request_pulse_2r <= 1'b0;
        pll_reprogram_request          <= 1'b0;
    end

    else
    begin
        seq_pll_start_reconfig_ams       <= seq_pll_start_reconfig_ccd_pipe[2];
        seq_pll_start_reconfig_r         <= seq_pll_start_reconfig_ams;
        seq_pll_start_reconfig_2r        <= seq_pll_start_reconfig_r;
        seq_pll_start_reconfig_3r        <= seq_pll_start_reconfig_2r;

        pll_reprogram_request_pulse      <= pll_phase_auto_calibrate_pulse;
        pll_reprogram_request_pulse_r    <= pll_reprogram_request_pulse;
        pll_reprogram_request_pulse_2r   <= pll_reprogram_request_pulse_r;
        pll_reprogram_request            <= pll_reprogram_request_long_pulse;
    end

end


// Rising-edge detect to generate a single phase shift step
assign pll_phase_auto_calibrate_pulse = ~seq_pll_start_reconfig_3r && seq_pll_start_reconfig_2r;

// extend the phase shift request pulse to be 3 scan clk cycles long.
assign pll_reprogram_request_long_pulse = pll_reprogram_request_pulse || pll_reprogram_request_pulse_r || pll_reprogram_request_pulse_2r;

// Register the Phase step settings
always @(posedge scan_clk or negedge pll_reconfig_reset_ams_n_r)
begin
    if (pll_reconfig_reset_ams_n_r == 1'b0)
    begin
        pll_new_dir   <= 1'b0;
        pll_new_phase <= 'h0;
    end

    else
    begin

        if (pll_phase_auto_calibrate_pulse)
        begin
            pll_new_dir   <= seq_pll_inc_dec_ccd;
            pll_new_phase <= seq_pll_select_ccd;
        end

    end
end


// generate the busy signal - just the inverse of the done o/p from the pll, and stretched ,
//as the initial pulse might not be long enough to be catched by the sequencer
//the same circuitry in the ciii clock and reset block

always @(posedge scan_clk or negedge pll_reconfig_reset_ams_n_r)
begin

    if (pll_reconfig_reset_ams_n_r == 1'b0)
        phs_shft_busy_siii <= 1'b0;

    else
        phs_shft_busy_siii <= pll_reprogram_request || ~pll_phase_done;

end

assign phs_shft_busy = phs_shft_busy_siii;

// Gate the soft reset input (from SOPC builder for example) with the PLL
// locked signal :
assign global_or_soft_reset_n  = soft_reset_n && global_reset_n;

// Create the PHY internal reset signal :
assign phy_internal_reset_n = pll_locked && global_or_soft_reset_n;

// The PLL resets only on a global reset :
assign pll_reset = !global_reset_n;


// If PLL reconfig ports are present, create the reconfig mux :
generate

    // Half-rate mode :
    if (PLL_RECONFIG_PORTS_EN == 1)
        assign pll_reconfig_enable_internal =  pll_reconfig_enable;
    else
        assign pll_reconfig_enable_internal =  1'b0;    
    
endgenerate

always @*
begin

    if (pll_reconfig_enable_internal == 1'b0)
    begin
        muxed_pll_phasecounterselect = lookup_siii(pll_new_phase);
        muxed_pll_phasestep          = pll_reprogram_request;
        muxed_pll_phaseupdown        = pll_new_dir;    
    end
    
    else
    begin
        muxed_pll_phasecounterselect = pll_phasecounterselect;
        muxed_pll_phasestep          = pll_phasestep;
        muxed_pll_phaseupdown        = pll_phaseupdown;    
    end
    
end

    
generate

    // Half-rate mode :
    if (DWIDTH_RATIO == 4)
    begin : half_rate

        //
        ddr2_phy_alt_mem_phy_pll pll (
                     .inclk0             (pll_ref_clk),
             .areset             (pll_reset),
             .c0                 (phy_clk_1x),     // hR
             .c1                 (mem_clk_2x),     // FR
             .c2                 (aux_clk),        // FR
             .c3                 (write_clk_2x),   // FR
             .c4                 (resync_clk_2x),  // FR
             .c5                 (measure_clk_1x), // hR
             .c6                 (ac_clk_1x),      // hR
             .phasecounterselect (muxed_pll_phasecounterselect),
             .phasestep          (muxed_pll_phasestep),
             .phaseupdown        (muxed_pll_phaseupdown),
             .scanclk            (scan_clk),
             .locked             (pll_locked_src),
             .phasedone          (pll_phase_done)
         );

    end

    // Full-rate mode :
    else
    begin : full_rate

        //
        ddr2_phy_alt_mem_phy_pll pll (
                     .inclk0             (pll_ref_clk),
             .areset             (pll_reset),
             .c0                 (aux_clk),         // hR
             .c1                 (mem_clk_2x),      // FR
             .c2                 (phy_clk_1x),      // FR
             .c3                 (write_clk_2x),    // FR
             .c4                 (resync_clk_2x),   // FR
             .c5                 (measure_clk_1x),  // hR
             .c6                 (ac_clk_1x),       // hR
             .phasecounterselect (muxed_pll_phasecounterselect),
             .phasestep          (muxed_pll_phasestep),
             .phaseupdown        (muxed_pll_phaseupdown),
             .scanclk            (scan_clk),
             .locked             (pll_locked_src),
             .phasedone          (pll_phase_done)
         );

    end

endgenerate




//synopsys translate_off
reg [19:0] pll_locked_chain  = 20'h0;

always @(posedge pll_ref_clk)
begin
    pll_locked_chain <= {pll_locked_chain[18:0],pll_locked_src};
end
//synopsys translate_on

always @*
begin
 pll_locked = pll_locked_src;
//synopsys translate_off
 pll_locked = pll_locked_chain[19];
//synopsys translate_on
end


assign cs_n_clk_1x = ac_clk_1x;

// The postamble clock is the inverse of the resync clock
assign postamble_clk_2x = ~resync_clk_2x;

generate

genvar clk_pair;

    for (clk_pair = 0 ; clk_pair < MEM_IF_CLK_PAIR_COUNT; clk_pair = clk_pair + 1)
    begin : DDR_CLK_OUT

        // For DDR/DDR2 use DDIO :
        if (MEM_IF_MEMTYPE == "DDR" || MEM_IF_MEMTYPE == "DDR2" || MEM_IF_MEMTYPE == "QDRII" || (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 0))
        begin : ddio_memclk_gen

            stratixiii_ddio_out # (
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            ) mem_clk_ddio (
                .datainlo        (1'b0),
                .datainhi        (~seq_clk_disable && ~ctrl_clk_disable[clk_pair]),
                .clkhi           (mem_clk_2x),
                .clklo           (mem_clk_2x),
                .muxsel          (mem_clk_2x),
                .clk             (),
                .ena             (1'b1),
                .areset          (1'b0),
                .sreset          (1'b0),
                .dataout         (mem_clk_pdiff_in[clk_pair]),
                .dfflo           (),
                .dffhi           (),
                .devpor          (),
                .devclrn         ()
            );


        end

        // For DDR3 use a phase align atom :
        else
        begin : phase_align_memclk_gen

            wire ck_h;
            wire ck_l;
            wire ck_n_h;
            wire ck_n_l;

            stratixiii_ddio_out # (
                .half_rate_mode("true"),
                .use_new_clocking_model("true")
            ) mem_clk_hr_ddio_h(
                .datainhi (~seq_clk_disable && ~ctrl_clk_disable[clk_pair]),
                .datainlo (~seq_clk_disable && ~ctrl_clk_disable[clk_pair]),
                .clkhi    (phy_clk_1x),
                .clklo    (phy_clk_1x),
                .muxsel   (phy_clk_1x),
                .ena      (1'b1),
                .areset   (1'b0),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (ck_h),
                .devclrn(),
                .devpor()
            );

            stratixiii_ddio_out # (
                .half_rate_mode("true"),
                .use_new_clocking_model("true")
            ) mem_clk_hr_ddio_l(
                .datainhi (1'b0),
                .datainlo (1'b0),
                .clkhi    (phy_clk_1x),
                .clklo    (phy_clk_1x),
                .muxsel   (phy_clk_1x),
                .ena      (1'b1),
                .areset   (1'b0),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (ck_l),
                .devclrn(),
                .devpor()
            );

            // Note : delay_buffer_mode for output_phase_alignment atoms must always
            // be tied to "high" :
            stratixiii_output_phase_alignment # (
                .operation_mode                  ("ddio_out"),
                .use_phasectrlin                 ("false"),
                .phase_setting                   (2),
                .delay_buffer_mode               ("high"),
                .power_up                        ("low"),
                .async_mode                      ("clear"),
                .sync_mode                       ("none"),
                .add_output_cycle_delay          ("false"),
                .use_delayed_clock               ("false"),
                .phase_setting_for_delayed_clock (2),
                .add_phase_transfer_reg          ("true"),
                .use_phasectrl_clock             ("false"),
                .invert_phase                    ("false"),
                .use_primary_clock               ("true"),
                .bypass_input_register           ("false")
            ) mem_clk_opa (
                .datain                 ({ck_h, ck_l}),
                .clk                    (write_clk_2x),
                .delayctrlin            ((DLL_EXPORT_IMPORT == "IMPORT") ? dqs_delay_ctrl_import : dqs_delay_ctrl),
                .areset                 (1'b0),
                .sreset                 (1'b0),
                .clkena                 (1'b1),
                .phaseinvertctrl        (),
                .enaphasetransferreg    (),
                .enaoutputcycledelay    (),
                .phasectrlin            (),
                // synopsys translate_off
                .devclrn(), .devpor(),
                .dffin(), .dff1t(), .dffddiodataout(),
                // synopsys translate_on
                .dataout                (mem_clk_pdiff_in[clk_pair])
            );

        end

        if (MEM_IF_STR_T9_DESKEW_EN == 1)
        begin : gen_T9
            stratixiii_delay_chain  o_pa_dc1(
                .datain             (mem_clk_pdiff_in[clk_pair]),
                .delayctrlin        (outputdelaysetting1[clk_pair]),
                // synopsys translate_off
                .devclrn(), .devpor(),
                // synopsys translate_on
               .dataout             (mem_clk_pdiff_in_delayed1[clk_pair])
            );
        end
        else
        begin : gen_no_T9
            assign mem_clk_pdiff_in_delayed1[clk_pair] = mem_clk_pdiff_in[clk_pair];
        end

        if (MEM_IF_STR_T10_DESKEW_EN == 1)
        begin : gen_T10_dqs_deskew
            stratixiii_delay_chain  o_pa_dc2(
                .datain             (mem_clk_pdiff_in_delayed1[clk_pair]),
                .delayctrlin        ({1'b0, outputdelaysetting2[clk_pair]}),
                // synopsys translate_off
                .devclrn(), .devpor(),
                // synopsys translate_on
               .dataout             (mem_clk_pdiff_in_delayed2[clk_pair])
            );
        end
        else
        begin : gen_no_T10
            assign mem_clk_pdiff_in_delayed2[clk_pair] = mem_clk_pdiff_in_delayed1[clk_pair];
        end

        // Pseudo-diff used to ensure fanout of 1 from OPA/DDIO_OUT atoms :
        stratixiii_pseudo_diff_out mem_clk_pdiff (
             .i    (mem_clk_pdiff_in_delayed2[clk_pair]),
             .o    (  mem_clk_buf_in[clk_pair]),
             .obar (mem_clk_n_buf_in[clk_pair])
         );

        // The same output buf is for both DDR2 and 3 :
        stratixiii_io_obuf # (
            .bus_hold("false"),
            .open_drain_output("false"),
            .shift_series_termination_control("false")
        ) mem_clk_obuf (
            .i                         (mem_clk_buf_in[clk_pair]),
            .oe                        (1'b1),
            .dynamicterminationcontrol (1'b0),
            // synopsys translate_off
            .seriesterminationcontrol(),
            .parallelterminationcontrol(),
            .obar(),
            // synopsys translate_on
            .o(mem_clk[clk_pair]),
            .devoe()
        );

        // The same output buf is used
        stratixiii_io_obuf # (
            .bus_hold("false"),
            .open_drain_output("false"),
            .shift_series_termination_control("false")
        ) mem_clk_n_obuf (
            .i                         (mem_clk_n_buf_in[clk_pair]),
            .oe                        (1'b1),
            .dynamicterminationcontrol (1'b0),
            // synopsys translate_off
            .seriesterminationcontrol(),
            .parallelterminationcontrol(),
            .obar(),
            // synopsys translate_on
            .o(mem_clk_n[clk_pair]),
            .devoe()
        );

        // IO CONFIG.
        stratixiii_io_config io_config(
            .datain(scan_din[clk_pair]),
            .clk(scan_clk_in),
            .ena(scan_enable_ck[clk_pair]),
            .update(scan_update),
            // synopsys translate_off
            .devclrn(), .devpor(),
            .dutycycledelaymode(),
            .dutycycledelaysettings(),
            .outputfinedelaysetting1(),
            .outputfinedelaysetting2(),
            .outputonlydelaysetting2(),
            .outputonlyfinedelaysetting2(),
            .padtoinputregisterfinedelaysetting(),
            // synopsys translate_on
            .padtoinputregisterdelaysetting(),
            .outputdelaysetting1(outputdelaysetting1[clk_pair]),
            .outputdelaysetting2(outputdelaysetting2[clk_pair]),
            .dataout()
        );

    end //for

endgenerate



// Mimic path - connect an input buffer to the pad.  Choose mem_clk[0] as
// this shall always be implemented :

generate

    // Mimic clock generation should be differential or single-ended dependant upon DQS usage :
    if (MEM_IF_MEMTYPE == "QDRII" || MEM_IF_MEMTYPE == "DDR" || (MEM_IF_MEMTYPE == "DDR2" && (MEM_IF_DQSN_EN == 0)) )
    begin : gen_mimic_se_ibuf

        stratixiii_io_ibuf fb_clk_ibuf(
            .i      (mem_clk[0]),
            // synopsys translate_off
            .ibar(),
            .dynamicterminationcontrol(),
            // synopsys translate_on
            .o      (fb_clk)

        );

    end

    // For DDR3 and DDR2 with DQSN, use a differential I/O :
    else
    begin : gen_mimic_diff_ibuf

        stratixiii_io_ibuf fb_clk_ibuf(
            .i      (mem_clk[0]),
            .ibar   (mem_clk_n[0]),
            // synopsys translate_off
            .dynamicterminationcontrol(),
            // synopsys translate_on
            .o      (fb_clk)

        );

    end

endgenerate

// DDR2 Mimic Path Generation - in effect this is just a register :
stratixiii_ddio_in ddio_mimic(
    .datain     (fb_clk),
    .clk        (measure_clk_1x),
    .clkn       (),
    // synopsys translate_off
    .devclrn(),
    .devpor(),
   // synopsys translate_on
    .ena        (1'b1),
    .areset     (1'b0),
    .sreset     (1'b0),
    .regoutlo   (),
    .regouthi   (mimic_data_1x),
    .dfflo      ()
);



generate
    if (DLL_EXPORT_IMPORT != "IMPORT")
    begin
        // Note : delay_buffer_mode for the dll atoms may be either 'high' or 'low', so it is
        // correct to propagate the DLL_DELAY_BUFFER_MODE parameter here :
        stratixiii_dll # (
            .input_frequency                 (MEM_IF_CLK_PS_STR),
            .delay_buffer_mode               (DLL_DELAY_BUFFER_MODE),
            .delay_chain_length              (DLL_DELAY_CHAIN_LENGTH),
            .delayctrlout_mode               ("normal"),
            .jitter_reduction                ("true"),
            .sim_valid_lock                  (1280),
            .sim_low_buffer_intrinsic_delay  (350),
            .sim_high_buffer_intrinsic_delay (175),
            .sim_buffer_delay_increment      (10),
            .static_delay_ctrl               (0),
            .lpm_type                        ("stratixiii_dll")
        ) dll(
            .clk                    (mem_clk_2x),
            .aload                  (~pll_locked),
            .delayctrlout           (dqs_delay_ctrl),
            .upndnout               (),
            .dqsupdate              (dqs_delay_update_en),
            .offsetdelayctrlclkout  (dll_offset_delay_ctrl_clk),
            .offsetdelayctrlout     (dll_offset_delay_code),
            .devpor                 (),
            .devclrn                (),
            .upndninclkena          (),
            .upndnin                ()
        );
    end
endgenerate


generate
    if (USE_DLL_OFFSET =="true" || USE_DLL_OFFSET == "TRUE")
        stratixiii_dll_offset_ctrl # (
            .use_offset        (USE_DLL_OFFSET),
            .static_offset     ("0"),
            .delay_buffer_mode (DLL_DELAY_BUFFER_MODE)
        ) dll_phs_offset (
            .clk                (dll_offset_delay_ctrl_clk),
            .aload              (~pll_locked),
            .offsetdelayctrlin  (dll_offset_delay_code),
            .offset             (dlloffset_offset),
            .addnsub            (dlloffset_addnsub),
            .devclrn            (),
            .devpor             (),
            .offsettestout      (),
            .offsetctrlout      (dlloffset_offsetctrl_out)
        );
     else
        assign dlloffset_offsetctrl_out = 6'b000000;
endgenerate




// Master reset generation :
always @(posedge phy_clk_1x or negedge phy_internal_reset_n)
begin

    if (phy_internal_reset_n == 1'b0)
    begin
        reset_master_ams <= 1'b0;
        global_pre_clear <= 1'b0;
    end

    else
    begin
        reset_master_ams <= 1'b1;
        global_pre_clear <= reset_master_ams;
    end

end


// phy_clk reset generation :
always @(posedge phy_clk_1x or negedge global_pre_clear)
begin

    if (global_pre_clear == 1'b0)
    begin
        reset_phy_clk_1x_n <= 1'b0;
    end

    else
    begin
        reset_phy_clk_1x_n <= global_pre_clear;
    end

end




generate

    for (dqs_group = 0 ; dqs_group < MEM_IF_DQS_WIDTH; dqs_group = dqs_group + 1)
    begin : RESET_RDP_BUS

        assign rdp_reset_req_n[dqs_group] = seq_rdp_reset_req_n[dqs_group] && global_pre_clear ;

        // phy_clk reset generation for read datapaths :
        //
        ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (RDP_RESET_PIPE_DEPTH) ) reset_rdp_phy_clk_pipe(
                     .clock     (phy_clk_1x),
             .pre_clear (rdp_reset_req_n[dqs_group]),
             .reset_out (reset_rdp_phy_clk_1x_n[dqs_group])
        );

    end

endgenerate



// NB. phy_clk reset is generated above.

// Instantiate the reset pipes.  The 4 reset signals below are family invariant
// whilst the other resets are generated on a per-family basis :


// mem_clk reset generation :
//
ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (2) )  mem_pipe(
     .clock     (mem_clk_2x),
     .pre_clear (global_pre_clear),
     .reset_out (mem_reset_n)
);

// mem_clk_2x reset generation - required for SIII DDR/DDR2 support :
//
ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (4) ) mem_clk_pipe(
    .clock     (mem_clk_2x),
    .pre_clear (global_pre_clear),
    .reset_out (reset_mem_clk_2x_n)
);

// write_clk_2x reset generation :
//
ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (4) )   write_clk_pipe(
      .clock     (write_clk_2x),
      .pre_clear (global_pre_clear),
      .reset_out (reset_write_clk_2x_n)
);

// ac_clk_1x reset generation :
//
ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (2) )   ac_clk_pipe_1x(
     .clock     (ac_clk_1x),
     .pre_clear (global_pre_clear),
     .reset_out (reset_ac_clk_1x_n)
);

// cs_clk_2x reset generation :
//
ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (4) ) cs_n_clk_pipe_1x(
      .clock     (cs_n_clk_1x),
      .pre_clear (global_pre_clear),
      .reset_out (reset_cs_n_clk_1x_n)
);

// measure_clk_1x reset generation :
//
ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (2) ) measure_clk_pipe(
      .clock     (measure_clk_1x),
      .pre_clear (global_pre_clear),
      .reset_out (reset_measure_clk_1x_n)
);

// resync_clk_2x reset generation :
//
ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (RDP_RESET_PIPE_DEPTH) ) resync_clk_2x_pipe(
      .clock     (resync_clk_2x),
      .pre_clear (rdp_reset_req_n[0]),
      .reset_out (reset_resync_clk_2x_n)
);



generate

    if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdr_doff_pipe_gen

        assign qdr_doff_req_n = seq_qdr_doff_req_n && global_pre_clear ;

        //
        ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (2) ) qdr_doff_pipe(
                    .clock     (mem_clk_2x),
            .pre_clear (qdr_doff_req_n),
            .reset_out (mem_doff_n)
        );

    end

    else
    begin
        assign mem_doff_n = 1'b0;
    end

endgenerate



generate

    for (dqs_group = 0 ; dqs_group < 1; dqs_group = dqs_group + 1)
    begin : reset_resync_clk_pipe

        //
        ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (2) ) resync_clk_pipe_1x(
                     .clock     (resync_clk_1x[0]),
             .pre_clear (rdp_reset_req_n[0]),
             .reset_out (master_reset_resync_clk_1x)
        );

    end

endgenerate

// QDRII requires individual control of each DQS group reset, whilst DDR3 requires all groups to be synchronised to reset[0] :
assign reset_resync_clk_1x_pre_clear = (MEM_IF_MEMTYPE == "QDRII") ? rdp_reset_req_n : {MEM_IF_DQS_WIDTH{master_reset_resync_clk_1x}} ;


// resync_clk_1x reset generation.  Note that the pre_clear is connected to the
// local read data path reset request signal, as these shall be reset when the
// sequencer requests :
generate

    for (dqs_group = 0 ; dqs_group < MEM_IF_DQS_WIDTH; dqs_group = dqs_group + 1)
    begin : slave_reset_resync_clk_pipe

        //
        ddr2_phy_alt_mem_phy_reset_pipe # (.PIPE_DEPTH (2) ) slave_resync_clk_pipe_1x(
                     .clock     (resync_clk_1x[dqs_group]), // Choose one resync_clk from which to generate reset
             .pre_clear (reset_resync_clk_1x_pre_clear[dqs_group]),
             .reset_out (reset_resync_clk_1x_n[dqs_group])
        );

    end

endgenerate

endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif



//
module ddr2_phy_alt_mem_phy_postamble ( // inputs
                               phy_clk_1x,
                               resync_clk_1x,
                               reset_phy_clk_1x_n,
                               reset_resync_clk_1x_n,
                               seq_poa_lat_inc_1x,
                               seq_poa_lat_dec_1x,
                               seq_poa_protection_override_1x,

                               // for 2T / 2N addr/CMD drive both of these with the same value.
                               // (DWIDTH_RATIO/2 - 1 downto 0)   (LSB represents first when changes ionto full-rate!
                               ctl_doing_rd_beat,

                               // outputs (DWIDTH_RATIO/2 - 1 downto 0)
                               poa_postamble_en_preset
                              );

parameter FAMILY                       = "STRATIXIII";
parameter POSTAMBLE_INITIAL_LAT        = 13; //13 for SIII, 16 for SII/CIII
parameter POSTAMBLE_RESYNC_LAT_CTL_EN  = 0;  // 0 means false, 1 means true
parameter POSTAMBLE_AWIDTH             = 6;
parameter POSTAMBLE_HALFT_EN           = 0;  // 0 means false, 1 means true
parameter MEM_IF_POSTAMBLE_EN_WIDTH    = 8;
parameter DWIDTH_RATIO                 = 4;

// clocks
input  wire                           phy_clk_1x;
input  wire                           resync_clk_1x;

// resets
input  wire                           reset_phy_clk_1x_n;
input  wire                           reset_resync_clk_1x_n;

// control signals from sequencer
input  wire                           seq_poa_lat_inc_1x;
input  wire                           seq_poa_lat_dec_1x;
input  wire                           seq_poa_protection_override_1x;
input  wire [DWIDTH_RATIO/2 - 1 : 0]  ctl_doing_rd_beat ;


// output to IOE for SIII :
output wire [DWIDTH_RATIO/2 - 1 : 0]  poa_postamble_en_preset;


// internal wires/regs
reg  [POSTAMBLE_AWIDTH - 1 : 0]       rd_addr;
reg  [POSTAMBLE_AWIDTH - 1 : 0]       wr_addr;
reg  [POSTAMBLE_AWIDTH - 1 : 0]       next_wr_addr;
reg  [DWIDTH_RATIO/2 - 1 : 0]         wr_data;
wire                                  wr_en;

reg                                   sync_seq_poa_lat_inc_1x;
reg                                   sync_seq_poa_lat_dec_1x;

reg                                   seq_poa_lat_inc_1x_1t;
reg                                   seq_poa_lat_dec_1x_1t;


// only for halfrate.  --- also for Quarter_rate......
reg                                   ctl_doing_rd_beat2_1x_r1;

wire [DWIDTH_RATIO/2 - 1 : 0]         postamble_en;

reg                                   bit_order_1x;

reg                                   ams_inc;
reg                                   ams_dec;



// loop variables
genvar i;



////////////////////////////////////////////////////////////////////////////////
//       Generate Statements to synchronise controls if necessary
////////////////////////////////////////////////////////////////////////////////


generate
//begin

    if (POSTAMBLE_RESYNC_LAT_CTL_EN == 0)
    begin : sync_lat_controls

        always @* // combinational logic sensitivity
        begin
            sync_seq_poa_lat_inc_1x = seq_poa_lat_inc_1x;
            sync_seq_poa_lat_dec_1x = seq_poa_lat_dec_1x;
        end

    end

    else
    begin : resynch_lat_controls


        always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
        begin

            if (reset_phy_clk_1x_n == 1'b0)
            begin
                sync_seq_poa_lat_inc_1x <= 1'b0;
                sync_seq_poa_lat_dec_1x <= 1'b0;
                ams_inc                 <= 1'b0;
                ams_dec                 <= 1'b0;
            end

            else
            begin
                sync_seq_poa_lat_inc_1x <= ams_inc;
                sync_seq_poa_lat_dec_1x <= ams_dec;
                ams_inc                 <= seq_poa_lat_inc_1x;
                ams_dec                 <= seq_poa_lat_dec_1x;
            end

        end

    end

//end
endgenerate


////////////////////////////////////////////////////////////////////////////////
//          write address controller
////////////////////////////////////////////////////////////////////////////////

// override_postamble_protection is overide onto write data.
//  otherwise use bit_order_1x to choose how word is written into RAM.

generate // based on DWIDTH RATIO...
//begin
  if (DWIDTH_RATIO == 4) // Half Rate
  begin : halfrate_wdata_gen
      always @* // combinational logic sensitivity
      begin

        if (seq_poa_protection_override_1x == 1'b1)
        begin
            wr_data  = `POA_OVERRIDE_VAL;
        end

        else if (bit_order_1x == 1'b0)
        begin
            wr_data  = {ctl_doing_rd_beat[1], ctl_doing_rd_beat[0]};
        end

        else
        begin
            wr_data  = {ctl_doing_rd_beat[0], ctl_doing_rd_beat2_1x_r1};
        end
      end
  end
else // Full-rate
 begin : fullrate_wdata_gen

  always @* // combinational logic sensitivity
    begin
      if (seq_poa_protection_override_1x == 1'b1)
        begin
          wr_data  = `POA_OVERRIDE_VAL_FULL_RATE;
        end

    else
      begin
            wr_data = ctl_doing_rd_beat;
      end

    end
  end
//end
endgenerate


always @*
begin

    next_wr_addr = wr_addr + 1'b1;

    if (sync_seq_poa_lat_dec_1x == 1'b1 && seq_poa_lat_dec_1x_1t == 1'b0)
    begin

        if ((bit_order_1x == 1'b0) || (DWIDTH_RATIO == 2))
        begin
            next_wr_addr = wr_addr;
        end

    end

    else if (sync_seq_poa_lat_inc_1x == 1'b1 && seq_poa_lat_inc_1x_1t == 1'b0)
    begin

        if ((bit_order_1x == 1'b1) || (DWIDTH_RATIO ==2))
        begin
            next_wr_addr = wr_addr + 2'h2;
        end

    end

end


always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin

    if (reset_phy_clk_1x_n == 1'b0)
    begin
        wr_addr <= POSTAMBLE_INITIAL_LAT[POSTAMBLE_AWIDTH - 1 : 0];
    end

    else
    begin
        wr_addr <= next_wr_addr;
    end

end



always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin

    if (reset_phy_clk_1x_n == 1'b0)
    begin
        ctl_doing_rd_beat2_1x_r1 <= 1'b0;
        seq_poa_lat_inc_1x_1t    <= 1'b0;
        seq_poa_lat_dec_1x_1t    <= 1'b0;
        bit_order_1x             <= 1'b1; // 1'b0
    end

    else
    begin
        ctl_doing_rd_beat2_1x_r1 <= ctl_doing_rd_beat[DWIDTH_RATIO/2 - 1];
        seq_poa_lat_inc_1x_1t    <= sync_seq_poa_lat_inc_1x;
        seq_poa_lat_dec_1x_1t    <= sync_seq_poa_lat_dec_1x;

        if (DWIDTH_RATIO == 2)
            bit_order_1x <= 1'b0;
        else if (sync_seq_poa_lat_dec_1x == 1'b1 && seq_poa_lat_dec_1x_1t == 1'b0)
        begin
            bit_order_1x <=  ~bit_order_1x;
        end

        else if (sync_seq_poa_lat_inc_1x == 1'b1 && seq_poa_lat_inc_1x_1t == 1'b0)
        begin
            bit_order_1x <= ~bit_order_1x;
        end

    end

end


///////////////////////////////////////////////////////////////////////////////////
//         Instantiate the postamble dpram
///////////////////////////////////////////////////////////////////////////////////

assign wr_en = 1'b1;                       // tied high


// For StratixIII, the read and write sides are the same width :
altsyncram #(
    .address_reg_b             ("CLOCK1"),
    .clock_enable_input_a      ("BYPASS"),
    .clock_enable_input_b      ("BYPASS"),
    .clock_enable_output_b     ("BYPASS"),
    .intended_device_family    (FAMILY),
    .lpm_type                  ("altsyncram"),
    .numwords_a                (2**POSTAMBLE_AWIDTH),
    .numwords_b                (2**POSTAMBLE_AWIDTH),
    .operation_mode            ("DUAL_PORT"),
    .outdata_aclr_b            ("NONE"),
    .outdata_reg_b             ("CLOCK1"),
    .power_up_uninitialized    ("FALSE"),
    .ram_block_type            ("MLAB"),
    .widthad_a                 (POSTAMBLE_AWIDTH),
    .widthad_b                 (POSTAMBLE_AWIDTH),
    .width_a                   (DWIDTH_RATIO/2),
    .width_b                   (DWIDTH_RATIO/2),
    .width_byteena_a           (1)
) altsyncram_component (
    .wren_a                    (wr_en),
    .clock0                    (phy_clk_1x),
    .clock1                    (resync_clk_1x),
    .address_a                 (wr_addr),
    .address_b                 (rd_addr),
    .data_a                    (wr_data),
    .q_b                       (postamble_en),
    .aclr0                     (1'b0),
    .aclr1                     (1'b0),
    .addressstall_a            (1'b0),
    .addressstall_b            (1'b0),
    .byteena_a                 (1'b1),
    .byteena_b                 (1'b1),
    .clocken0                  (1'b1),
    .clocken1                  (1'b1),
    .data_b                    ({DWIDTH_RATIO/2{1'b1}}),
    .q_a                       (),
    .rden_b                    (1'b1),
    .wren_b                    (1'b0),
    .eccstatus                 (),
    .clocken3                  (),
    .clocken2                  (),
    .rden_a                    ()
);




///////////////////////////////////////////////////////////////////////////////////
//     read address generator : just a free running counter.
///////////////////////////////////////////////////////////////////////////////////

always @(posedge resync_clk_1x or negedge reset_resync_clk_1x_n)
begin

    if (reset_resync_clk_1x_n == 1'b0)
    begin
        rd_addr <= {POSTAMBLE_AWIDTH{1'b0}};
    end

    else
    begin
        rd_addr <= rd_addr + 1'b1;     //inc address, can wrap
    end

end


assign poa_postamble_en_preset = postamble_en;



endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_read_dp_group ( phy_clk_1x,
                                  resync_clk_1x,
                                  reset_phy_clk_1x_n,
                                  reset_resync_clk_1x_n,
                                  seq_rdp_dec_read_lat_1x,
                                  seq_rdp_inc_read_lat_1x,
                                  dio_rdata3_1x,
                                  dio_rdata2_1x,
                                  dio_rdata1_1x,
                                  dio_rdata0_1x,
                                  ctl_mem_rdata
                                 );

parameter ADDR_COUNT_WIDTH         =               4;
parameter BIDIR_DPINS              =               1; // 0 for QDR only.
parameter DWIDTH_RATIO             =               4;
parameter MEM_IF_CLK_PS            =            4000;
parameter FAMILY                   =     "STRATIXII";
parameter MEM_IF_DQ_PER_DQS        =               8;
parameter RDP_INITIAL_LAT          =               6;
parameter RDP_RESYNC_LAT_CTL_EN    =               0;
parameter RESYNC_PIPELINE_DEPTH    =               1; // No pipelining in the FD spec.

input  wire                                                 phy_clk_1x;
input  wire                                                 resync_clk_1x;
input  wire                                                 reset_phy_clk_1x_n;
input  wire                                                 reset_resync_clk_1x_n;
input  wire                                                 seq_rdp_dec_read_lat_1x;
input  wire                                                 seq_rdp_inc_read_lat_1x;
input  wire [MEM_IF_DQ_PER_DQS-1 : 0]                       dio_rdata0_1x;
input  wire [MEM_IF_DQ_PER_DQS-1 : 0]                       dio_rdata1_1x;
input  wire [MEM_IF_DQ_PER_DQS-1 : 0]                       dio_rdata2_1x;
input  wire [MEM_IF_DQ_PER_DQS-1 : 0]                       dio_rdata3_1x;
output wire [DWIDTH_RATIO*MEM_IF_DQ_PER_DQS-1 : 0]          ctl_mem_rdata;


// concatonated read data :
wire [(DWIDTH_RATIO*MEM_IF_DQ_PER_DQS)-1 : 0]               dio_rdata;
reg  [ADDR_COUNT_WIDTH - 1 : 0]                             rd_ram_rd_addr;
reg  [ADDR_COUNT_WIDTH - 1 : 0]                             rd_ram_wr_addr;

reg                                                         inc_read_lat_sync_r;
reg                                                         dec_read_lat_sync_r;

// Optional AMS registers :
reg                                                         inc_read_lat_ams ;
reg                                                         inc_read_lat_sync;

reg                                                         dec_read_lat_ams ;
reg                                                         dec_read_lat_sync;

wire                                                        rd_addr_stall;
wire                                                        rd_addr_double_inc;

////////////////////////////////////////////////////////////////////////////////
//                          Write Address block
////////////////////////////////////////////////////////////////////////////////


always@ (posedge resync_clk_1x or negedge reset_resync_clk_1x_n)
begin

   if (reset_resync_clk_1x_n == 0)
   begin
       rd_ram_wr_addr  <= RDP_INITIAL_LAT[ADDR_COUNT_WIDTH - 1 : 0];
   end

   else
   begin
       rd_ram_wr_addr <= rd_ram_wr_addr + 1'b1;
   end

end



////////////////////////////////////////////////////////////////////////////////
//                          Pipeline registers
////////////////////////////////////////////////////////////////////////////////


// Concatenate the input read data :
generate
if (DWIDTH_RATIO ==4)
begin
assign dio_rdata = {dio_rdata3_1x, dio_rdata2_1x, dio_rdata1_1x, dio_rdata0_1x};
end
else
begin
    assign dio_rdata = {dio_rdata1_1x, dio_rdata0_1x};
end
endgenerate





////////////////////////////////////////////////////////////////////////////////
//         Instantiate the read_dp dpram
////////////////////////////////////////////////////////////////////////////////


    altsyncram #(
                 .address_reg_b             ("CLOCK1"),
                 .clock_enable_input_a      ("BYPASS"),
                 .clock_enable_input_b      ("BYPASS"),
                 .clock_enable_output_b     ("BYPASS"),
                 .intended_device_family    (FAMILY),
                 .lpm_type                  ("altsyncram"),
                 .numwords_a                (2**ADDR_COUNT_WIDTH),
                 .numwords_b                (2**ADDR_COUNT_WIDTH),
                 .operation_mode            ("DUAL_PORT"),
                 .outdata_aclr_b            ("NONE"),
                 .outdata_reg_b             ("CLOCK1"),
                 .power_up_uninitialized    ("FALSE"),
                 .ram_block_type            ("MLAB"),
                 .widthad_a                 (ADDR_COUNT_WIDTH),
                 .widthad_b                 (ADDR_COUNT_WIDTH),
                 .width_a                   (DWIDTH_RATIO*MEM_IF_DQ_PER_DQS),
                 .width_b                   (DWIDTH_RATIO*MEM_IF_DQ_PER_DQS),
                 .width_byteena_a           (1)
                )
    ram (
         .wren_a            (1'b1),
         .clock0            (resync_clk_1x),
         .clock1            (phy_clk_1x),
         .address_a         (rd_ram_wr_addr),
         .address_b         (rd_ram_rd_addr),
         .data_a            (dio_rdata),
         .q_b               (ctl_mem_rdata),
         .aclr0             (1'b0),
         .aclr1             (1'b0),
         .addressstall_a    (1'b0),
         .addressstall_b    (1'b0),
         .byteena_a         (1'b1),
         .byteena_b         (1'b1),
         .clocken0          (1'b1),
         .clocken1          (1'b1),
         .data_b            ({(DWIDTH_RATIO*MEM_IF_DQ_PER_DQS){1'b1}}),
         .q_a               (),
         .rden_b            (1'b1),
         .wren_b            (1'b0),
         .eccstatus         (),
         .clocken3          (),
         .clocken2          (),
         .rden_a            ()
    );





////////////////////////////////////////////////////////////////////////////////
//                          Read Address block
////////////////////////////////////////////////////////////////////////////////


// Optional Anti-metastability flops :
generate

    if (RDP_RESYNC_LAT_CTL_EN == 1)

    always@ (posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
    begin : rd_addr_ams

        if (reset_phy_clk_1x_n == 1'b0)
        begin

            inc_read_lat_ams    <= 1'b0;
            inc_read_lat_sync   <= 1'b0;
            inc_read_lat_sync_r <= 1'b0;

            // Synchronise rd_lat_inc_1x :
            dec_read_lat_ams    <= 1'b0;
            dec_read_lat_sync   <= 1'b0;
            dec_read_lat_sync_r <= 1'b0;

        end

        else
        begin

            // Synchronise rd_lat_inc_1x :
            inc_read_lat_ams    <= seq_rdp_inc_read_lat_1x;
            inc_read_lat_sync   <= inc_read_lat_ams;
            inc_read_lat_sync_r <= inc_read_lat_sync;

            // Synchronise rd_lat_inc_1x :
            dec_read_lat_ams    <= seq_rdp_dec_read_lat_1x;
            dec_read_lat_sync   <= dec_read_lat_ams;
            dec_read_lat_sync_r <= dec_read_lat_sync;

        end

    end // always

    // No anti-metastability protection required :
    else

    always@ (posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
    begin

        if (reset_phy_clk_1x_n == 1'b0)
        begin
            inc_read_lat_sync_r    <= 1'b0;
            dec_read_lat_sync_r   <= 1'b0;
        end

        else
        begin
            // No need to re-synchronise, just register for edge detect :
            inc_read_lat_sync_r <= seq_rdp_inc_read_lat_1x;
            dec_read_lat_sync_r <= seq_rdp_dec_read_lat_1x;
        end

    end

endgenerate




generate

    if (RDP_RESYNC_LAT_CTL_EN == 1)
    begin : lat_ctl_en_gen

        // 'toggle detect' logic :
        //assign rd_addr_double_inc =    !inc_read_lat_sync_r && inc_read_lat_sync;
        assign rd_addr_double_inc =    ( !dec_read_lat_sync_r && dec_read_lat_sync );
        // 'stall' logic :
       // assign rd_addr_stall      = !( !dec_read_lat_sync_r && dec_read_lat_sync );
        assign rd_addr_stall      =  !inc_read_lat_sync_r && inc_read_lat_sync;
    end

    else
    begin : no_lat_ctl_en_gen
        // 'toggle detect' logic :
        //assign rd_addr_double_inc =    !inc_read_lat_sync_r && seq_rdp_inc_read_lat_1x;
        assign rd_addr_double_inc =   ( !dec_read_lat_sync_r && seq_rdp_dec_read_lat_1x );
        // 'stall' logic :
        //assign rd_addr_stall      = !( !dec_read_lat_sync_r && seq_rdp_dec_read_lat_1x );
        assign rd_addr_stall      = !inc_read_lat_sync_r && seq_rdp_inc_read_lat_1x;
    end

endgenerate



always@ (posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin

   if (reset_phy_clk_1x_n == 0)
   begin
       rd_ram_rd_addr  <= { ADDR_COUNT_WIDTH {1'b0} };
   end

   else
   begin

       // RAM read address :
       if (rd_addr_stall == 1'b0)
       begin
           rd_ram_rd_addr <= rd_ram_rd_addr + 1'b1 + rd_addr_double_inc;
       end

   end

end


endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

`default_nettype none

//
module ddr2_phy_alt_mem_phy_rdata_valid ( // inputs
                               phy_clk_1x,
                               reset_phy_clk_1x_n,
                               seq_rdata_valid_lat_dec,
                               seq_rdata_valid_lat_inc,
                               seq_doing_rd,
                               ctl_doing_rd,
                               ctl_cal_success,
                               
                               // outputs
                               ctl_rdata_valid,
                               seq_rdata_valid
                              );

parameter FAMILY                       = "CYCLONEIII";
parameter MEM_IF_DQS_WIDTH             = 8;                              
parameter RDATA_VALID_AWIDTH           = 5;
parameter RDATA_VALID_INITIAL_LAT      = 16;
parameter DWIDTH_RATIO                 = 2;

localparam MAX_RDATA_VALID_DELAY       = 2 ** RDATA_VALID_AWIDTH;
localparam RDV_DELAY_SHR_LEN           = MAX_RDATA_VALID_DELAY*(DWIDTH_RATIO/2);

// clocks
input  wire                                              phy_clk_1x;

// resets
input  wire                                              reset_phy_clk_1x_n;

// control signals from sequencer
input  wire                                              seq_rdata_valid_lat_dec;
input  wire                                              seq_rdata_valid_lat_inc;
input  wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO / 2 -1 : 0] seq_doing_rd;
input  wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO / 2 -1 : 0] ctl_doing_rd;
input  wire                                              ctl_cal_success;

// output to IOE
output reg [DWIDTH_RATIO / 2 -1 : 0]                     ctl_rdata_valid;
output reg [DWIDTH_RATIO / 2 -1 : 0]                     seq_rdata_valid;

// Internal Signals / Variables
reg  [RDATA_VALID_AWIDTH - 1 : 0]                         rd_addr;
reg  [RDATA_VALID_AWIDTH - 1 : 0]                         wr_addr;
reg  [RDATA_VALID_AWIDTH - 1 : 0]                         next_wr_addr;
reg  [DWIDTH_RATIO/2 - 1 : 0]                             wr_data;

wire [DWIDTH_RATIO / 2 -1 : 0]                            int_rdata_valid;
reg  [DWIDTH_RATIO/2 - 1 : 0]                             rdv_pipe_ip;
reg                                                       rdv_pipe_ip_beat2_r;
reg  [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0]          merged_doing_rd;
reg                                                       seq_rdata_valid_lat_dec_1t;
reg                                                       seq_rdata_valid_lat_inc_1t;
reg                                                       bit_order_1x;

// Generate the input to the RDV delay.
// Also determine the data for the OCT control & postamble paths (merged_doing_rd)
generate
    if (DWIDTH_RATIO == 4)
    begin : merging_doing_rd_halfrate
        always @*
        begin
            merged_doing_rd = seq_doing_rd | (ctl_doing_rd & {(2 * MEM_IF_DQS_WIDTH) {ctl_cal_success}});
            rdv_pipe_ip[0]  = | merged_doing_rd[    MEM_IF_DQS_WIDTH - 1 : 0];
            rdv_pipe_ip[1]  = | merged_doing_rd[2 * MEM_IF_DQS_WIDTH - 1 : MEM_IF_DQS_WIDTH];
        end
    end
    else  // DWIDTH_RATIO == 2
    begin : merging_doing_rd_fullrate
        always @*
        begin
            merged_doing_rd = seq_doing_rd | (ctl_doing_rd & { MEM_IF_DQS_WIDTH {ctl_cal_success}});
            rdv_pipe_ip[0]  = | merged_doing_rd[MEM_IF_DQS_WIDTH - 1 : 0];
        end
    end // else: !if(DWIDTH_RATIO == 4)
endgenerate


// Register inc/dec rdata_valid signals and generate bit_order_1x
always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin
    if (reset_phy_clk_1x_n == 1'b0)
        begin
            seq_rdata_valid_lat_dec_1t <= 1'b0;
            seq_rdata_valid_lat_inc_1t <= 1'b0;
            bit_order_1x               <= 1'b1;
            
        end
    else
        begin
            rdv_pipe_ip_beat2_r <= rdv_pipe_ip[DWIDTH_RATIO/2 - 1];
            seq_rdata_valid_lat_dec_1t <= seq_rdata_valid_lat_dec;
            seq_rdata_valid_lat_inc_1t <= seq_rdata_valid_lat_inc;
            
            if (DWIDTH_RATIO == 2)
                bit_order_1x <= 1'b0;
            else if (seq_rdata_valid_lat_dec == 1'b1 && seq_rdata_valid_lat_dec_1t == 1'b0)
            begin
                bit_order_1x <=  ~bit_order_1x;
            end
            
            else if (seq_rdata_valid_lat_inc == 1'b1 && seq_rdata_valid_lat_inc_1t == 1'b0)
            begin
                bit_order_1x <= ~bit_order_1x;
            end
        end
end

// write data
generate // based on DWIDTH RATIO
  if (DWIDTH_RATIO == 4) // Half Rate
  begin : halfrate_wdata_gen
      always @* // combinational logic sensitivity
      begin

        if (bit_order_1x == 1'b0)
        begin
            wr_data  = {rdv_pipe_ip[1], rdv_pipe_ip[0]};
        end

        else
        begin
            wr_data  = {rdv_pipe_ip[0], rdv_pipe_ip_beat2_r};
        end
      end
  end
else // Full-rate
 begin : fullrate_wdata_gen

  always @* // combinational logic sensitivity
    begin
        wr_data = rdv_pipe_ip;
    end
  end
endgenerate

// write address
always @*
begin

    next_wr_addr = wr_addr + 1'b1;

    if (seq_rdata_valid_lat_dec == 1'b1 && seq_rdata_valid_lat_dec_1t == 1'b0)
    begin

        if ((bit_order_1x == 1'b0) || (DWIDTH_RATIO == 2))
        begin
            next_wr_addr = wr_addr;
        end

    end

    else if (seq_rdata_valid_lat_inc == 1'b1 && seq_rdata_valid_lat_inc_1t == 1'b0)
    begin

        if ((bit_order_1x == 1'b1) || (DWIDTH_RATIO ==2))
        begin
            next_wr_addr = wr_addr + 2'h2;
        end

    end

end


always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin

    if (reset_phy_clk_1x_n == 1'b0)
    begin
        wr_addr <= RDATA_VALID_INITIAL_LAT[RDATA_VALID_AWIDTH - 1 : 0];
    end

    else
    begin
        wr_addr <= next_wr_addr;
    end

end

//     read address generator : just a free running counter.
always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin

    if (reset_phy_clk_1x_n == 1'b0)
    begin
        rd_addr <= {RDATA_VALID_AWIDTH{1'b0}};
    end

    else
    begin
        rd_addr <= rd_addr + 1'b1;     //inc address, can wrap
    end

end

// altsyncram instance
altsyncram #(.
    address_aclr_b            ("NONE"),
	.address_reg_b            ("CLOCK0"),
	.clock_enable_input_a     ("BYPASS"),
	.clock_enable_input_b     ("BYPASS"),
	.clock_enable_output_b    ("BYPASS"),
	.intended_device_family   (FAMILY),
	.lpm_type                 ("altsyncram"),
	.numwords_a               (2**RDATA_VALID_AWIDTH),
	.numwords_b               (2**RDATA_VALID_AWIDTH),
	.operation_mode           ("DUAL_PORT"),
	.outdata_aclr_b           ("NONE"),
	.outdata_reg_b            ("CLOCK0"),
	.power_up_uninitialized   ("FALSE"),
	.widthad_a                (RDATA_VALID_AWIDTH),
	.widthad_b                (RDATA_VALID_AWIDTH),
	.width_a                  (DWIDTH_RATIO/2),
	.width_b                  (DWIDTH_RATIO/2),
	.width_byteena_a          (1)
) altsyncram_component (
	.wren_a                   (1'b1),
	.clock0                   (phy_clk_1x),
	.address_a                (wr_addr),
	.address_b                (rd_addr),
	.data_a                   (wr_data),
	.q_b                      (int_rdata_valid),
	.aclr0                    (1'b0),
	.aclr1                    (1'b0),
	.addressstall_a           (1'b0),
	.addressstall_b           (1'b0),
	.byteena_a                (1'b1),
	.byteena_b                (1'b1),
	.clock1                   (1'b1),
	.clocken0                 (1'b1),
	.clocken1                 (1'b1),
	.clocken2                 (1'b1),
	.clocken3                 (1'b1),
	.data_b                   ({(DWIDTH_RATIO/2){1'b1}}),
	.eccstatus                (),
	.q_a                      (),
	.rden_a                   (1'b1),
	.rden_b                   (1'b1),
	.wren_b                   (1'b0)
);

// Generate read data valid enable signals for controller and seqencer
always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin
    if (reset_phy_clk_1x_n == 1'b0)
        begin
            ctl_rdata_valid <= {(DWIDTH_RATIO/2){1'b0}};
            seq_rdata_valid <= {(DWIDTH_RATIO/2){1'b0}};
        end
    else
        begin
            // shift the shift register by DWIDTH_RATIO locations
            // rdv_delay_index plus (DWIDTH_RATIO/2)-1 bits counting down
            ctl_rdata_valid <= int_rdata_valid & {(DWIDTH_RATIO/2){ctl_cal_success}};
            seq_rdata_valid <= int_rdata_valid;
        end
end
 

endmodule

`default_nettype wire

//


`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif


//
module ddr2_phy_alt_mem_phy_write_dp(
                // clocks
                phy_clk_1x,

                // active-low reset, sync'd to clock domain
                reset_phy_clk_1x_n,

                // control i/f inputs
                ctl_mem_be,
                ctl_mem_dqs_burst,
                ctl_mem_wdata,
                ctl_mem_wdata_valid,
                ctl_mem_dqs,

                // from OCT path...
                ctl_mem_oct,

                // seq i/f inputs :
                seq_be,
                seq_dqs_burst,
                seq_wdata,
                seq_wdata_valid,
                seq_dqs,
                seq_ctl_sel,
                seq_oct_val,

                seq_dq_dm_add_2t_delay,
                seq_dqs_add_2t_delay,

                // outputs to IOEs
                wdp_wdata3_1x,
                wdp_wdata2_1x,
                wdp_wdata1_1x,
                wdp_wdata0_1x,

                wdp_wdata_oe_h_1x,
                wdp_wdata_oe_l_1x,

                wdp_dqs3_1x,
                wdp_dqs2_1x,
                wdp_dqs1_1x,
                wdp_dqs0_1x,

                wdp_dqs_oe_h_1x,
                wdp_dqs_oe_l_1x,

                wdp_dm3_1x,
                wdp_dm2_1x,
                wdp_dm1_1x,
                wdp_dm0_1x,

                wdp_oct_h_1x,
                wdp_oct_l_1x
                );



// Notes on parameters :
//
// BIDIR_DPINS is only required to support QDR-type memory, which has seperate DQ and D
// data buses.  The value is ignored pending QDR support.
//
// LOCAL_IF_DRATE should not be required, as SIII will always be half-rate.
//
// MEM_IF_DQS_WIDTH should always be MEM_IF_DWIDTH/MEM_IF_DQ_PER_DQS
//
// MEM_IF_DM_WIDTH should always be equal to MEM_IF_DQS_WIDTH
//
// GENERATE_WRITE_DQS is only required to support QDR-type memory, which does not
// require write DQS logic
//


parameter BIDIR_DPINS        = 1;
parameter LOCAL_IF_DRATE     = "HALF";
parameter LOCAL_IF_DWIDTH    = 256;
parameter MEM_IF_DQS_WIDTH   = 8;
parameter MEM_IF_DQ_PER_DQS  = 8;
parameter MEM_IF_DM_WIDTH    = 8;
parameter MEM_IF_BE_WIDTH    = 32;
parameter MEM_IF_OCT_EN      = 0;
parameter GENERATE_WRITE_DQS = 1;
parameter MEM_IF_DWIDTH      = 64;
parameter DWIDTH_RATIO       = 4;
parameter MEM_IF_DM_PINS_EN  = 1;
parameter MEM_IF_MEMTYPE     = "QDRII";


// Notes on I/O :
//
// ctl_mem_be is the inverse of the "DM" value.


input wire                                 phy_clk_1x;


input wire                                 reset_phy_clk_1x_n;


// control i/f inputs
input wire [MEM_IF_DM_WIDTH  * DWIDTH_RATIO   - 1 : 0] ctl_mem_be;
input wire [MEM_IF_DWIDTH    * DWIDTH_RATIO   - 1 : 0] ctl_mem_wdata;
input wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] ctl_mem_dqs_burst;
input wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] ctl_mem_wdata_valid;
input wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] ctl_mem_oct;
input wire [3 : 0]                                     ctl_mem_dqs;


// seq i/f inputs
input wire [MEM_IF_DM_WIDTH  * DWIDTH_RATIO   - 1 : 0] seq_be;
input wire [MEM_IF_DWIDTH    * DWIDTH_RATIO   - 1 : 0] seq_wdata;
input wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] seq_dqs_burst;
input wire [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] seq_wdata_valid;
input wire                                             seq_oct_val;
input wire [3 : 0]                                     seq_dqs;


input wire                                 seq_ctl_sel;
input wire                                 seq_dq_dm_add_2t_delay;
input wire  [MEM_IF_DQS_WIDTH - 1 : 0]     seq_dqs_add_2t_delay;


// outputs to the IOEs
(* preserve *) output reg  [MEM_IF_DWIDTH - 1 : 0]        wdp_wdata3_1x;
(* preserve *) output reg  [MEM_IF_DWIDTH - 1 : 0]        wdp_wdata2_1x;
(* preserve *) output reg  [MEM_IF_DWIDTH - 1 : 0]        wdp_wdata1_1x;
(* preserve *) output reg  [MEM_IF_DWIDTH - 1 : 0]        wdp_wdata0_1x;


(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_wdata_oe_h_1x;
(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_wdata_oe_l_1x;


(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_dqs3_1x;
(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_dqs2_1x;
(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_dqs1_1x;
(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_dqs0_1x;



(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_dqs_oe_h_1x;
(* preserve *) output reg  [MEM_IF_DQS_WIDTH - 1 : 0]     wdp_dqs_oe_l_1x;


// These shall be NWS/BWS for QDRII :
(* preserve *) output reg  [MEM_IF_DM_WIDTH -1 : 0]       wdp_dm3_1x;
(* preserve *) output reg  [MEM_IF_DM_WIDTH -1 : 0]       wdp_dm2_1x;
(* preserve *) output reg  [MEM_IF_DM_WIDTH -1 : 0]       wdp_dm1_1x;
(* preserve *) output reg  [MEM_IF_DM_WIDTH -1 : 0]       wdp_dm0_1x;


(* preserve *) output reg  [MEM_IF_DQS_WIDTH -1 : 0]      wdp_oct_h_1x;
(* preserve *) output reg  [MEM_IF_DQS_WIDTH -1 : 0]      wdp_oct_l_1x;


// internal reg declarations


// Note that at this point, the 'be' signal shall potentially have been
// doubled in width if in 4 DQ_PER_DQS mode :


(* preserve *) reg [3 : 0]                                        mem_dqs_r;
(* preserve *) reg [LOCAL_IF_DWIDTH - 1 : 0]                      mem_wdata_r;
(* preserve *) reg [MEM_IF_DM_WIDTH * DWIDTH_RATIO - 1 : 0]       mem_be_r;
(* preserve *) reg [MEM_IF_DQS_WIDTH * DWIDTH_RATIO / 2 - 1 : 0]  mem_dqs_burst_r;
(* preserve *) reg [MEM_IF_DQS_WIDTH * DWIDTH_RATIO / 2 - 1 : 0]  mem_wdata_valid_r;
(* preserve *) reg [MEM_IF_DQS_WIDTH * DWIDTH_RATIO / 2 - 1 : 0]  mem_oct_r;



// MUX outputs....
reg [DWIDTH_RATIO - 1 : 0]                      mem_dqs        ;
reg [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] mem_dqs_burst  ;
reg [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] mem_wdata_valid;
reg [MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2 - 1 : 0] mem_oct;
reg [MEM_IF_DM_WIDTH  * DWIDTH_RATIO   - 1 : 0] mem_be         ;
reg [MEM_IF_DWIDTH    * DWIDTH_RATIO   - 1 : 0] mem_wdata      ;
reg tie_low = 1'b0;


always @*
begin


    // Select controller or sequencer according to the select signal :
    //for QDRII, no register is needed here
    if (seq_ctl_sel)
    begin
        mem_dqs_burst     = seq_dqs_burst;
        mem_dqs           = seq_dqs;
        mem_be            = seq_be;
        mem_wdata         = seq_wdata;
        mem_wdata_valid   = seq_wdata_valid;
        mem_oct           = {(MEM_IF_DQS_WIDTH * DWIDTH_RATIO/2){seq_oct_val}};
    end


    else
    begin
        mem_dqs_burst     = ctl_mem_dqs_burst;
        mem_dqs           = ctl_mem_dqs;
        mem_be            = ctl_mem_be;
        mem_wdata         = ctl_mem_wdata;
        mem_wdata_valid   = ctl_mem_wdata_valid;
        mem_oct           = ctl_mem_oct;
    end


end




// Additional registeriong for clk delay switching in.
always @(posedge phy_clk_1x)
begin
    mem_dqs_r         <= mem_dqs;
    mem_wdata_valid_r <= mem_wdata_valid;
    mem_dqs_burst_r   <= mem_dqs_burst;
    mem_wdata_r       <= mem_wdata;
    mem_oct_r         <= mem_oct;
    mem_be_r          <= mem_be;
end






generate
   genvar ia;


   for (ia=0; ia<MEM_IF_DWIDTH; ia=ia+1)
   begin : gen_dq


       always @(posedge phy_clk_1x)
       begin


           if (seq_dq_dm_add_2t_delay)
           begin
               wdp_wdata3_1x[ia]       <= mem_wdata_r[ia+MEM_IF_DWIDTH*2];
               wdp_wdata2_1x[ia]       <= mem_wdata_r[ia+MEM_IF_DWIDTH*0];
               wdp_wdata1_1x[ia]       <= mem_wdata_r[ia+MEM_IF_DWIDTH*3];
               wdp_wdata0_1x[ia]       <= mem_wdata_r[ia+MEM_IF_DWIDTH*1];
           end


           else
           begin
               wdp_wdata3_1x[ia]       <= mem_wdata[ia+MEM_IF_DWIDTH*2];
               wdp_wdata2_1x[ia]       <= mem_wdata[ia+MEM_IF_DWIDTH*0];
               wdp_wdata1_1x[ia]       <= mem_wdata[ia+MEM_IF_DWIDTH*3];
               wdp_wdata0_1x[ia]       <= mem_wdata[ia+MEM_IF_DWIDTH*1];
            end




       end //posedge clk
   end // block: gen_dqs
endgenerate




// OEs
generate
   genvar ib;


   for (ib=0; ib<MEM_IF_DQS_WIDTH; ib=ib+1)
   begin : gen_dqoe


       always @(posedge phy_clk_1x)
       begin


           if (seq_dq_dm_add_2t_delay)
           begin
               wdp_wdata_oe_l_1x[ib]   <= mem_wdata_valid_r[ib];
               wdp_wdata_oe_h_1x[ib]   <= mem_wdata_valid_r[ib+ MEM_IF_DQS_WIDTH];
           end


           else
           begin
               wdp_wdata_oe_l_1x[ib]   <= mem_wdata_valid[ib ];
               wdp_wdata_oe_h_1x[ib]   <= mem_wdata_valid[ib+ MEM_IF_DQS_WIDTH];
           end


       end //posedge clk


   end // block: gen_dqoe


endgenerate






generate
   genvar ic;


   for (ic=0; ic<MEM_IF_DQS_WIDTH; ic=ic+1)
   begin : gen_dqs


       always @(posedge phy_clk_1x)
       begin


           if (seq_dqs_add_2t_delay[ic])
           begin
               wdp_dqs3_1x[ic]     <= mem_dqs_r[3] ^ ~mem_dqs_burst_r[ic + MEM_IF_DQS_WIDTH];
               wdp_dqs2_1x[ic]     <= mem_dqs_r[2] ^ ~mem_dqs_burst_r[ic];
               wdp_dqs1_1x[ic]     <= mem_dqs_r[1] ^ ~mem_dqs_burst_r[ic + MEM_IF_DQS_WIDTH];
               wdp_dqs0_1x[ic]     <= mem_dqs_r[0] ^ ~mem_dqs_burst_r[ic];


               wdp_dqs_oe_l_1x[ic] <= mem_dqs_burst_r[ic];
               wdp_dqs_oe_h_1x[ic] <= mem_dqs_burst_r[ic + MEM_IF_DQS_WIDTH];
           end


           else
           begin
               wdp_dqs3_1x[ic]     <= mem_dqs[3] ^ ~mem_dqs_burst[ic + MEM_IF_DQS_WIDTH];
               wdp_dqs2_1x[ic]     <= mem_dqs[2] ^ ~mem_dqs_burst[ic];
               wdp_dqs1_1x[ic]     <= mem_dqs[1] ^ ~mem_dqs_burst[ic + MEM_IF_DQS_WIDTH];
               wdp_dqs0_1x[ic]     <= mem_dqs[0] ^ ~mem_dqs_burst[ic];


               wdp_dqs_oe_l_1x[ic] <= mem_dqs_burst[ic];
               wdp_dqs_oe_h_1x[ic] <= mem_dqs_burst[ic + MEM_IF_DQS_WIDTH];
           end


       end


       if (MEM_IF_OCT_EN == 1)
       begin


           always @(posedge phy_clk_1x)
           begin


               if (seq_dqs_add_2t_delay[ic])
               begin
                   wdp_oct_l_1x[ic]    <= mem_oct_r[ic];
                   wdp_oct_h_1x[ic]    <= mem_oct_r[ic + MEM_IF_DQS_WIDTH];
               end


               else


               begin
                   wdp_oct_l_1x[ic]    <= mem_oct[ic];
                   wdp_oct_h_1x[ic]    <= mem_oct[ic + MEM_IF_DQS_WIDTH];
               end


           end //posedge clk


       end // OCT_EN


       else // Tie-off if OCT not enabled :
       begin


           always @(tie_low)
           begin
               wdp_oct_h_1x[ic] = tie_low;
               wdp_oct_l_1x[ic] = tie_low;
           end


       end


   end // block: gen_dqs


endgenerate




// Conditional generation of DM logic, based on generic
generate
    genvar ie;


    if (MEM_IF_DM_PINS_EN == 1'b1)
    begin : dm_logic


        for (ie=0; ie<MEM_IF_DM_WIDTH; ie=ie+1)
        begin : gen_dm


            always @(posedge phy_clk_1x)
            begin


                if (seq_dq_dm_add_2t_delay)
                begin
                    wdp_dm3_1x[ie] <= mem_be_r[ie+MEM_IF_DM_WIDTH*2];
                    wdp_dm2_1x[ie] <= mem_be_r[ie+MEM_IF_DM_WIDTH*0];
                    wdp_dm1_1x[ie] <= mem_be_r[ie+MEM_IF_DM_WIDTH*3];
                    wdp_dm0_1x[ie] <= mem_be_r[ie+MEM_IF_DM_WIDTH*1];
                end


                else
                begin
                    wdp_dm3_1x[ie] <= mem_be[ie+MEM_IF_DM_WIDTH*2];
                    wdp_dm2_1x[ie] <= mem_be[ie+MEM_IF_DM_WIDTH*0];
                    wdp_dm1_1x[ie] <= mem_be[ie+MEM_IF_DM_WIDTH*3];
                    wdp_dm0_1x[ie] <= mem_be[ie+MEM_IF_DM_WIDTH*1];
                end


            end //posedge clk


        end // block: gen_dm


    end // block: dm_logic_enabled
endgenerate




endmodule // alt_mem_phy_write_dp_sii



//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_dqs_op (
                                 phy_clk_1x,
                                 write_clk_2x,
                                 mem_clk_2x,
                                 sc_clk,

                                 scan_enable,
                                 scan_update,
                                 scan_din,
                                 scan_dout,

                                 wdp_dqs3_1x,
                                 wdp_dqs2_1x,
                                 wdp_dqs1_1x,
                                 wdp_dqs0_1x,

                                 wdp_dqs_oe_h_1x,
                                 wdp_dqs_oe_l_1x,

                                 wdp_oct_h_1x,
                                 wdp_oct_l_1x,

                                 dqs_sneak_in,
                                 dqs_sneak_out,

                                 dqs_pad,

                                 dedicated_dll_delay_ctrl,
                                 enaoutputcycledelaysetting,
                                 enaoutputphasetransferreg,
                                 dqsoutputphaseinvert,
                                 dqsoutputphasesetting,

                                 enaoctcycledelaysetting,
                                 enaoctphasetransferreg,
                                 octdelaysetting1,
                                 octdelaysetting2,

                                 seriesterminationcontrol,
                                 parallelterminationcontrol

                                 );

parameter DLL_DELAY_BUFFER_MODE    = "HIGH";
parameter INVERT_OP_FOR_DQSN       =      0;
parameter MEM_IF_MEMTYPE           =  "DDR";
parameter LEVELLING                =      1;
parameter MEM_IF_DQSN_EN           =      1;
parameter MEM_IF_OCT_EN            =      0;
parameter DQS_DELAY_CTL_WIDTH      =      6;
parameter DWIDTH_RATIO             =      4;

parameter MEM_IF_STR_T9_DESKEW_EN                 = 0;
parameter MEM_IF_STR_T10_DESKEW_EN                = 0;
parameter MEM_IF_OCT_T9_DESKEW_EN                 = 0;
parameter MEM_IF_OCT_T10_DESKEW_EN                = 0;
parameter MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL = "false";

parameter ENABLE_DDR3_SEQUENCER                   = "FALSE";
parameter OPA_USES_DELAYED_CLK                    = "false";

input wire                                                 phy_clk_1x;
input wire                                                 write_clk_2x;
input wire                                                 mem_clk_2x;
input wire                                                 sc_clk;

input wire                                                 scan_enable;
input wire                                                 scan_update;
input wire                                                 scan_din;
output wire                                                scan_dout;

input wire                                                 wdp_dqs3_1x;
input wire                                                 wdp_dqs2_1x;
input wire                                                 wdp_dqs1_1x;
input wire                                                 wdp_dqs0_1x;

input wire                                                 wdp_dqs_oe_h_1x;
input wire                                                 wdp_dqs_oe_l_1x;

//OCT
input wire                                                 wdp_oct_h_1x;
input wire                                                 wdp_oct_l_1x;

// If this block is being instanced as a DQSN output path, then instead of using
// DDIO/Output phase align atoms, a sneak path is used, which is an inverted version
// of the DQS output path.  The parameter INVERT_OP_FOR_DQSN is used to determine what to do.
input wire                                                 dqs_sneak_in;

// If this block is a DQS output path, then we need to tap-off the output path and
// output as the sneak path for the DQSN path to use :
output wire                                                dqs_sneak_out;

inout wire                                                 dqs_pad;

input wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]                  dedicated_dll_delay_ctrl;

// NB. These are outputs from the DQS CONFIG block :
input wire                                                 enaoutputcycledelaysetting;
input wire                                                 enaoutputphasetransferreg;
input wire                                                 dqsoutputphaseinvert;
input wire [`DQSCONFIG_DQS_OUTPUT_PHASE_SETTING_WIDTH-1:0] dqsoutputphasesetting;

//OCT only :
input wire                                                 enaoctcycledelaysetting;
input wire                                                 enaoctphasetransferreg;
input wire [`DQSCONFIG_DQS_OCT_DELAY_SETTING1_WIDTH-1 :0]  octdelaysetting1;
input wire [`DQSCONFIG_DQS_OCT_DELAY_SETTING2_WIDTH-1 :0]  octdelaysetting2;

input wire [`OCT_SERIES_TERM_CONTROL_WIDTH   -1 : 0]        seriesterminationcontrol;
input wire [`OCT_PARALLEL_TERM_CONTROL_WIDTH -1 : 0]        parallelterminationcontrol;

// Internal wires :

wire dqs_oe;
wire dqs_oe_aligned;
reg  dqs_oe_aligned_reg;
wire dqs_oe_aligned_delayed;
wire dqs_oe_aligned_delayed2;

wire dqs_oct;
wire dqs_oct_aligned;
wire dqs_oct_aligned_delayed;
wire dqs_oct_aligned_delayed2;

wire dqs_l;
wire dqs_h;

wire dqs_aligned;
wire dqs_aligned_delayed;
wire dqs_aligned_delayed2;

wire dqs_pdiff_out;

wire [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING1_WIDTH-1:0]  outputdelaysetting1;
wire [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING2_WIDTH-1:0]  outputdelaysetting2;




generate

    if (INVERT_OP_FOR_DQSN == 0)
    begin : dqs_ddio_out_gen

        if (DWIDTH_RATIO == 4)
        begin : dqs_ddio_out_half_rate_gen

            // Output path.  Instance this if a DQS path only :
            stratixiii_ddio_out # (
                .half_rate_mode("true"),
                .use_new_clocking_model("true")
            ) o_ddio_h(
                .datainhi (wdp_dqs3_1x),
                .datainlo (wdp_dqs2_1x),
                .clkhi    (phy_clk_1x),
                .clklo    (phy_clk_1x),
                .muxsel   (phy_clk_1x),
                .ena      (1'b1),
                .areset   (1'b0),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (dqs_h),
                .devpor  (),
                .devclrn ()
            );

            stratixiii_ddio_out # (
                .half_rate_mode("true"),
                .use_new_clocking_model("true")
            ) o_ddio_l(
                .datainhi (wdp_dqs1_1x),
                .datainlo (wdp_dqs0_1x),
                .clkhi    (phy_clk_1x),
                .clklo    (phy_clk_1x),
                .muxsel   (phy_clk_1x),
                .ena      (1'b1),
                .areset   (1'b0),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (dqs_l),
                .devpor  (),
                .devclrn ()
            );

        end

        else
        begin : dqs_ddio_out_full_rate_gen

            assign dqs_l = wdp_dqs0_1x;
            assign dqs_h = wdp_dqs1_1x;

        end

        // Phase alignment is either via DDIO for DDR/DDR2 or phase alignment atom for DDR3 :
        if (MEM_IF_MEMTYPE == "DDR" || MEM_IF_MEMTYPE == "DDR2" || (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 0))
        begin : ddr_ddio_phase_align_gen
        // DDIO output
            stratixiii_ddio_out # (
                .half_rate_mode("false"),
                .use_new_clocking_model("true")
            )dqs_ddio_inst(
                .datainhi (dqs_h),
                .datainlo (dqs_l),
                .clkhi    (mem_clk_2x),
                .clklo    (mem_clk_2x),
                .muxsel   (mem_clk_2x),
                .ena      (1'b1),
                .areset   (1'b0),
                .sreset   (1'b0),
                // synopsys translate_off
                .dfflo(),
                .dffhi(),
                .clk(),
                // synopsys translate_on
                .dataout (dqs_aligned),
                .devpor  (),
                .devclrn ()
            );
        end

        else
        begin : ddr3_opa_phase_align_gen

            // Output_phase_alignment of output

            // Note : delay_buffer_mode for output_phase_alignment atoms must always be tied to "high" :
            stratixiii_output_phase_alignment # (
                .operation_mode                  ("ddio_out"),
                .use_phasectrlin                 (ENABLE_DDR3_SEQUENCER),
                .delay_buffer_mode               ("high"),
                .power_up                        ("low"),
                .async_mode                      ("clear"),
                .sync_mode                       ("none"),
                .add_output_cycle_delay          ("dynamic"),
                .use_delayed_clock               (OPA_USES_DELAYED_CLK),
                .add_phase_transfer_reg          ("dynamic"),
                .use_phasectrl_clock             ("true"),
                .use_primary_clock               ("true"),
                .invert_phase                    ("dynamic"),
                //.phase_setting_for_delayed_clock (2),
                .phase_setting                   (2)
                ) o_phase_align (
                .datain                ({dqs_h,dqs_l}),
                .clk                   (write_clk_2x),
                .delayctrlin           (dedicated_dll_delay_ctrl),
                .phasectrlin           (dqsoutputphasesetting),
                .areset                (1'b0),
                .sreset                (1'b0),
                .clkena                (1'b1),
                .enaoutputcycledelay   (enaoctcycledelaysetting),
                .enaphasetransferreg   (enaoctphasetransferreg),
                .phaseinvertctrl       (dqsoutputphaseinvert),
                // synopsys translate_off
		.delaymode(),
		.dutycycledelayctrlin(),
                .devclrn(),
		.devpor(),
                .dffin(),
		.dff1t(),
		.dffddiodataout(),
                // synopsys translate_on
                .dataout               (dqs_aligned)
            );
        end

        // If DQSN is to be used, instance the pseudo-diff atom :
        if (MEM_IF_DQSN_EN == 1)
        begin : pseudo_diff_gen
            // DQS, so assign the sneak-out path :
            stratixiii_pseudo_diff_out o_pdiff (
                .i    (dqs_aligned_delayed2),
                .o    (dqs_pdiff_out),
                .obar (dqs_sneak_out)
            );
        end

        // Otherwise, output from the delay chain :
        else
        begin : no_pseudo_diff_gen
            assign dqs_pdiff_out = dqs_aligned_delayed2;
            assign dqs_sneak_out = 1'b0;
        end


    end // DQSN or not

    else // If DQSN, use the sneak path :
    begin : dqsn_sneak_path_gen

        // No need to invert sneak path as the pseudo_diff in DQS should have done this :
        assign dqs_pdiff_out = dqs_sneak_in;
        assign dqs_sneak_out = 1'b0;

    end

endgenerate




// DQS OE path :
// The OE path is always instanced, regardless of whether this is a DQS or DQSN path.


generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate_oe_ddio_gen

        stratixiii_ddio_out # (
            .half_rate_mode("true"),
            .use_new_clocking_model("true")
        ) oe_ddio (
            .datainhi (~wdp_dqs_oe_h_1x),
            .datainlo (~wdp_dqs_oe_l_1x),
            .clkhi    (phy_clk_1x),
            .clklo    (phy_clk_1x),
            .muxsel   (phy_clk_1x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
                .clk(),
            // synopsys translate_on
            .dataout (dqs_oe),
            .devpor  (),
            .devclrn ()
        );

    end

    else
    begin : full_rate_oe_ddio_gen
        assign dqs_oe = ~wdp_dqs_oe_l_1x;
    end

endgenerate

// Phase alignment is either via DDIO for DDR/DDR2 or phase alignment atom for DDR3 :
generate

    if (MEM_IF_MEMTYPE == "DDR" || MEM_IF_MEMTYPE == "DDR2")
    begin : ddr_ddio_oe_phase_align_gen

        // DDR/DDR2 DDIO_OE
        stratixiii_ddio_oe oe_phase_align (
               .oe          (dqs_oe),
               .clk         (mem_clk_2x),
               .ena         (1'b1),
               .areset      (1'b0),
               .sreset      (),
               // synopsys translate_off
               .dfflo       (),
               .dffhi       (),
               .devpor      (),
               .devclrn     (),
               // synopsys translate_on
               .dataout     (dqs_oe_aligned)
            );

     end

     else

     begin : ddr3_dqs_oe

        if (LEVELLING == 0)

         begin : ddr3_ddio_oe_phase_align_gen

            // synopsys translate_off
            initial
                dqs_oe_aligned_reg = 0;
            // synopsys translate_on
            always @ (posedge mem_clk_2x)
                dqs_oe_aligned_reg <= dqs_oe;

            assign dqs_oe_aligned = dqs_oe_aligned_reg;

         end

         else
         begin : ddr3_opa_phase_align_gen

              // Output_phase_alignment of oe

              // Note : delay_buffer_mode for output_phase_alignment atoms must always be tied to "high" :
              stratixiii_output_phase_alignment # (
                 .operation_mode         ("oe"),
                 .use_phasectrlin        (ENABLE_DDR3_SEQUENCER),
                 .delay_buffer_mode      ("high"),
                 .power_up               ("low"),
                 .async_mode             ("clear"),
                 .sync_mode              ("none"),
                 .add_output_cycle_delay ("dynamic"),
                 .use_delayed_clock      (OPA_USES_DELAYED_CLK),
                 .add_phase_transfer_reg ("dynamic"),
                 .use_phasectrl_clock    ("true"),
                 .use_primary_clock      ("true"),
                 .invert_phase           ("dynamic"),
                 //.phase_setting_for_delayed_clock (2),
                 .phase_setting                   (2)
             ) oe_phase_align (
                 .datain		({1'b0,dqs_oe}),
                 .clk			(LEVELLING == 0 ? mem_clk_2x : write_clk_2x),
                 .delayctrlin		(dedicated_dll_delay_ctrl),
                 .phasectrlin		(dqsoutputphasesetting),
                 .areset		(1'b0),
                 .sreset		(1'b0),
                 .clkena		(1'b1),
                 .enaoutputcycledelay	(enaoctcycledelaysetting),
                 .enaphasetransferreg	(enaoctphasetransferreg),
                 .phaseinvertctrl	(dqsoutputphaseinvert),
                 // synopsys translate_off
	         .delaymode(),
	         .dutycycledelayctrlin(),
                 .devclrn(),
	         .devpor(),
                 .dffin(),
	         .dff1t(),
	         .dffddiodataout(),
                 // synopsys translate_on
                 .dataout		(dqs_oe_aligned)
             );

        end

    end

endgenerate


generate
    if (MEM_IF_STR_T9_DESKEW_EN == 1)
    begin : gen_T9_dqs_deskew
        stratixiii_delay_chain  o_pa_dc1(
            .datain             (dqs_aligned),
            .delayctrlin        (outputdelaysetting1),
            // synopsys translate_off
	    .finedelayctrlin(),
            .devclrn(),
	    .devpor(),
            // synopsys translate_on
           .dataout             (dqs_aligned_delayed)
        );

        // oe delay_chain_1
        stratixiii_delay_chain  oe1_opa_dc1(
            .datain             (dqs_oe_aligned),
            .delayctrlin        (outputdelaysetting1),
            // synopsys translate_off
	    .finedelayctrlin(),
            .devclrn(),
	    .devpor(),
            // synopsys translate_on
            .dataout             (dqs_oe_aligned_delayed)
        );
    end
    else
    begin : gen_T9_dqs_no_deskew
        assign dqs_aligned_delayed    = dqs_aligned;
        assign dqs_oe_aligned_delayed = dqs_oe_aligned;
    end
endgenerate




generate
    if (MEM_IF_STR_T10_DESKEW_EN == 1)
    begin : gen_T10_dqs_deskew
        // output delay_chain_2
        stratixiii_delay_chain  o_pa_dc2(
            .datain             (dqs_aligned_delayed),
            .delayctrlin        ({1'b0, outputdelaysetting2}),
            // synopsys translate_off
	    .finedelayctrlin(),
            .devclrn(),
	    .devpor(),
            // synopsys translate_on
           .dataout             (dqs_aligned_delayed2)
        );

        // oe delay_chain_2
        stratixiii_delay_chain  oe1_opa_dc2(
            .datain             (dqs_oe_aligned_delayed),
            .delayctrlin        ({1'b0, outputdelaysetting2}),
            // synopsys translate_off
	    .finedelayctrlin(),
            .devclrn(),
	    .devpor(),
            // synopsys translate_on
            .dataout             (dqs_oe_aligned_delayed2)
        );

    end
    else
    begin : gen_T10_dqs_no_deskew
        assign dqs_aligned_delayed2    = dqs_aligned_delayed;
        assign dqs_oe_aligned_delayed2 = dqs_oe_aligned_delayed;
    end
endgenerate


// DQS OCT path :
// The OCT path is always instanced, regardless of whether this is a DQS or DQSN path.

generate

    if (MEM_IF_OCT_EN == 1)
    begin : oct_ddio_gen

        if (DWIDTH_RATIO == 4)
        begin : dqs_oct_half_rate_gen

            stratixiii_ddio_out # (
                 .half_rate_mode("true"),
                 .use_new_clocking_model("true")
            ) oct_ddio (
                 .datainhi (wdp_oct_h_1x),
                 .datainlo (wdp_oct_l_1x),
                 .clkhi    (phy_clk_1x),
                 .clklo    (phy_clk_1x),
                 .muxsel   (phy_clk_1x),
                 .ena      (1'b1),
                 .areset   (1'b0),
                 .sreset   (1'b0),
                 // synopsys translate_off
                 .dfflo(),
                 .dffhi(),
                 .clk(),
                 // synopsys translate_on
                 .dataout (dqs_oct),
                 .devpor  (),
                 .devclrn ()
            );

        end

        else
        begin : dqs_oct_full_rate_gen
            assign dqs_oct = wdp_oct_l_1x;
        end

        // Phase alignment is either via DDIO for DDR/DDR2 or phase alignment atom for DDR3 :
        if (MEM_IF_MEMTYPE == "DDR" || MEM_IF_MEMTYPE == "DDR2" || (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 0))
        begin : ddr_ddio_oe_phase_align_gen

            // DDR/DDR2 DDIO_OE
            stratixiii_ddio_oe oct_phase_align (
                   .oe          (dqs_oct),
                   .clk         (mem_clk_2x),
                   .ena         (1'b1),
                   .areset      (1'b0),
                   .sreset      (),
                   // synopsys translate_off
                   .dfflo       (),
                   .dffhi       (),
                   .devpor      (),
                   .devclrn     (),
                   // synopsys translate_on
                   .dataout     (dqs_oct_aligned)
                );

         end

         else
         begin : ddr3_opa_phase_align_gen

             // Output_phase_alignment of oct

             // Note : delay_buffer_mode for output_phase_alignment atoms must always be tied to "high" :
             stratixiii_output_phase_alignment # (
                 .operation_mode	 ("rtena"),//("extended_rtena"),
                 .use_phasectrlin	 (ENABLE_DDR3_SEQUENCER),
                 .delay_buffer_mode	 ("high"),
                 .power_up		 ("low"),
                 .async_mode		 ("none"),
                 .sync_mode		 ("none"),
                 .add_output_cycle_delay ("dynamic"),
                 .use_delayed_clock	 (OPA_USES_DELAYED_CLK),
                 .add_phase_transfer_reg ("dynamic"),
                 .use_phasectrl_clock	 ("true"),
                 .use_primary_clock	 ("true"),
                 .invert_phase  	 ("dynamic"),
                 //.phase_setting_for_delayed_clock (2),
                 .phase_setting 		  (2)
             ) oct_phase_align(
                 .datain		({1'b0,dqs_oct}),
                 .clk			(write_clk_2x),
                 .delayctrlin		(dedicated_dll_delay_ctrl),
                 .phasectrlin		(dqsoutputphasesetting),
                 .areset		(1'b0),
                 .sreset		(1'b0),
                 .clkena		(1'b1),
                 .enaoutputcycledelay	(enaoctcycledelaysetting),
                 .enaphasetransferreg	(enaoctphasetransferreg),
                 .phaseinvertctrl	(dqsoutputphaseinvert),
                 // synopsys translate_off
	         .delaymode(),
	         .dutycycledelayctrlin(),
                 .devclrn(),
	         .devpor(),
                 .dffin(),
	         .dff1t(),
	         .dffddiodataout(),
                 // synopsys translate_on
                 .dataout		(dqs_oct_aligned)
             );

         end

            if (MEM_IF_OCT_T9_DESKEW_EN == 1)
            begin : gen_T9_OCT_deskew
                stratixiii_delay_chain dqoct_t9_delay(
                    .datain             (dqs_oct_aligned),
                    .delayctrlin        (octdelaysetting1),
                    // synopsys translate_off
                    .devclrn(), .devpor(),
                    // synopsys translate_on
                   .dataout             (dqs_oct_aligned_delayed)
                );
            end
            else
            begin : gen_T9_OCT_no_deskew
                assign dqs_oct_aligned_delayed = dqs_oct_aligned;
            end

            if (MEM_IF_OCT_T10_DESKEW_EN == 1)
            begin : gen_T10_OCT_deskew
                stratixiii_delay_chain dqoct_t10_delay(
                    .datain             (dqs_oct_aligned_delayed),
                    .delayctrlin        ({1'b0, octdelaysetting2}),
                    // synopsys translate_off
                    .devclrn(), .devpor(),
                    // synopsys translate_on
                   .dataout             (dqs_oct_aligned_delayed2)
                );
            end
            else
            begin : gen_T10_OCT_no_deskew
                assign dqs_oct_aligned_delayed2 = dqs_oct_aligned_delayed;
            end

    end // if MEM_IF_OCT_EN

    // No OCT :
    else
    begin : no_oct_gen
        assign dqs_oct_aligned_delayed2 = 1'b0;
    end

endgenerate


// output buf for DQS

stratixiii_io_obuf # (
   .bus_hold                         ("false"),
   .open_drain_output                ("false"),
   .shift_series_termination_control (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL)
) obuf(
    .i                         (dqs_pdiff_out),
    .oe                        (~dqs_oe_aligned_delayed2),
    .dynamicterminationcontrol (dqs_oct_aligned_delayed2),
    // synopsys translate_off
    .seriesterminationcontrol  (seriesterminationcontrol),
    .parallelterminationcontrol(parallelterminationcontrol),
    .obar(),
    // synopsys translate_on
    .o(dqs_pad),
    .devoe()
);


// IO_CONFIG
stratixiii_io_config io_config(
    .datain(scan_din),
    .clk(sc_clk),
    .ena(scan_enable),
    .update(scan_update),
    // synopsys translate_off
    .devclrn(), .devpor(),
    .dutycycledelaymode(),
    .dutycycledelaysettings(),
    .outputfinedelaysetting1(),
    .outputfinedelaysetting2(),
    .outputonlydelaysetting2(),
    .outputonlyfinedelaysetting2(),
    .padtoinputregisterfinedelaysetting(),
    // synopsys translate_on
    .padtoinputregisterdelaysetting(),
    .outputdelaysetting1(outputdelaysetting1),
    .outputdelaysetting2(outputdelaysetting2),
    .dataout(scan_dout)
);

endmodule


//


`timescale 1 ps / 1 ps

//
module ddr2_phy_alt_mem_phy_delay (
    s_in,
    s_out
);

//parameters
parameter WIDTH                        =  1;
parameter DELAY_PS                     =  10;

//ports
input  wire [WIDTH - 1 : 0]            s_in;
output reg  [WIDTH - 1 : 0]            s_out;

// synopsys translate_off
wire [WIDTH - 1 : 0] delayed_s_in;

//model the transport delay
assign #(DELAY_PS) delayed_s_in = s_in;
// synopsys translate_on

always @*
begin
    s_out = s_in;
// synopsys translate_off
    s_out = delayed_s_in;
// synopsys translate_on
end

endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_dqs_ip (
                            poa_postamble_en_preset,
                            resync_clk_1x,
                            resync_clk_2x,

                            dedicated_dll_delay_ctrl,
                            seq_dqs_delay_ctrl,
                            dll_offset_delay_ctrl,
                            dqs_update_en,

                            dqsinputphasesetting,

                            dqs_pad,
                            dqsn_pad,

                            dqs_enable,
                            dqsn_enable,

                            dqsbusoutdelaysetting,
                            dqsenablectrlphasesetting,
                            dqsenabledelaysetting,
                            enadqsenablephasetransferreg,
                            dqsenablectrlphaseinvert,
                            enaoutputcycledelaysetting,
                            enaoutputphasetransferreg,
                            dqsoutputphaseinvert,
                            dqsoutputphasesetting

                            ) /* synthesis altera_attribute="SUPPRESS_DA_RULE_INTERNAL=\"A103,R101\"" */ ;

parameter MEM_IF_CLK_PS          =      4000;
parameter MEM_IF_CLK_PS_STR      = "4000 ps";
parameter MEM_IF_MEMTYPE         =     "DDR";
parameter LEVELLING              =         1;
parameter MEM_IF_DQSN_EN         =         1;
parameter DLL_DELAY_BUFFER_MODE  =    "HIGH";
parameter DQS_PHASE              =      9000;
parameter DQS_PHASE_SETTING      =         2;
parameter DWIDTH_RATIO           =         4;

parameter ENABLE_DDR3_SEQUENCER  =   "FALSE";
parameter DQS_DELAY_CTL_WIDTH    =         6;
parameter MEM_TCL                =    "1.5";
parameter DQS_DELAY_CODE_FROM_DLL = "FALSE";
parameter DQS_DELAY_USES_OFFSET   = "false";
parameter USE_DQS_DELAY_LATCHES   = "false";
parameter DQS_USE_PHASECTRL_IN    = "false";
parameter MEM_IF_USE_T11          =       0;
parameter MEM_IF_USE_T7           =       0;

parameter INVERT_POSTAMBLE_CLK    = "false";

localparam DQS_BUS_INSERTION_DELAY = 250;

localparam ADD_PHASE_TRANSFER_REG_SETTING = (INVERT_POSTAMBLE_CLK == "false" || INVERT_POSTAMBLE_CLK == "FALSE") ? "true" : "false";


input  wire [DWIDTH_RATIO/2 -1 : 0]                          poa_postamble_en_preset;

// Half-rate resync clock from clock dividers :
input  wire                                                  resync_clk_1x;

input  wire                                                  resync_clk_2x;

// The DLL delay control is used for the DQS Enable Control :
input  wire [DQS_DELAY_CTL_WIDTH-1:0]                        dedicated_dll_delay_ctrl;

// The sequencer can supply a unique 6bit delay control code to each DQS group or this can come straight from the DLL.
input  wire [DQS_DELAY_CTL_WIDTH-1:0]                        seq_dqs_delay_ctrl;


input wire [DQS_DELAY_CTL_WIDTH-1:0]                         dll_offset_delay_ctrl;

input wire                                                   dqs_update_en;

input wire [2 : 0]                                           dqsinputphasesetting;

inout  wire                                                  dqs_pad;
inout  wire                                                  dqsn_pad;

// NB. 'wire' omitted so that attributes can be applied later.
// DQS enable used for DDR/2/3 memories, as these either have true differential
// DQS/DQSN pins (producing one active high DQS enable) or (for DDR) just a DQS
// signal is used :
output                                                       dqs_enable;

// For QDRII devices, DQS and DQSN are pseudo-differential and as the active-low
// version of DQS is required to clock incoming DQ data, this is directly propagated :
output                                                       dqsn_enable;

// NB. These are outputs from the DQS CONFIG block :
input  wire [`DQSCONFIG_DQS_OUTPUT_PHASE_SETTING_WIDTH-1: 0] dqsoutputphasesetting;
input  wire [`DQSCONFIG_DQS_BUSOUT_DELAY_SETTING_WIDTH-1 :0] dqsbusoutdelaysetting;
input  wire [`DQSCONFIG_DQS_EN_CTRL_PHASE_SETTING_WIDTH-1:0] dqsenablectrlphasesetting;
input  wire [`DQSCONFIG_DQS_EN_DELAY_SETTING_WIDTH-1     :0] dqsenabledelaysetting;
input  wire                                                  enadqsenablephasetransferreg;
input  wire                                                  dqsenablectrlphaseinvert;
input  wire                                                  enaoutputcycledelaysetting;
input  wire                                                  enaoutputphasetransferreg;
input  wire                                                  dqsoutputphaseinvert;


(* altera_attribute = "-name global_signal off" *) wire      dqs_buffered;
(* altera_attribute = "-name global_signal off" *) wire      dqs_delayed;
(* altera_attribute = "-name global_signal off" *) wire      dqs_delayed2;

(* altera_attribute = "-name global_signal off" *) wire      dqsn_buffered;
(* altera_attribute = "-name global_signal off" *) wire      dqsn_delayed;
(* altera_attribute = "-name global_signal off" *) wire      dqsn_delayed2;

wire                                                         dqs_enable_ddio_output;
wire                                                         dqs_enable_ctrl_op;
wire                                                         dqs_enable_ctrl_op_delayed;

wire [DQS_DELAY_CTL_WIDTH-1:0]                               chosen_dqs_delay_delayctrlin;


wire                                                         dqsn_enable;
(* altera_attribute = "-name global_signal off" *) reg       dqs_enable;
(* altera_attribute = "-name global_signal off" *) wire      dqs_enable_op;
(* altera_attribute = "-name global_signal off" *) wire      dqsn_enable_op;

wire                                                         dqs_enable_sim;


function integer min (input integer a, b);
begin

    if (a < b)
        min = a;
    else
        min = b;

end
endfunction


//
ddr2_phy_alt_mem_phy_delay # (
    .WIDTH     (1),
    .DELAY_PS  (DQS_BUS_INSERTION_DELAY)
) dqs_enable_delay(
    .s_in      (dqs_enable_op),
    .s_out     (dqs_enable_sim)
);


always @*
begin
  dqs_enable = dqs_enable_sim;
end


generate

    if (MEM_IF_MEMTYPE == "QDRII")
    begin : gen_dqsn_enable_delay
        assign #DQS_BUS_INSERTION_DELAY dqsn_enable = dqsn_enable_op;
    end
    else
    begin : tie_off_dqsn_enable
        // Set dqsn_enable to zero.  The '?' is used simply to reduce warnings about dqsn_enable_op not being read :
        assign dqsn_enable = dqsn_enable_op ? 1'b0 : 1'b0;
    end

endgenerate



generate

    // For DDR, or DDR2 where DQSN is disabled, it is important to leave the DQSN
    // pad unconnected, as otherwise this path remains in the netlist even
    // though there is no intent to use DQSN, and it is unused as an output :
    if (MEM_IF_MEMTYPE == "DDR" || (MEM_IF_MEMTYPE == "DDR2" && (MEM_IF_DQSN_EN == 0)) )
    begin : ddr_no_dqsn_ibuf_gen

        // Input buf
        stratixiii_io_ibuf dqs_inpt_io_ibuf(
           .i      (dqs_pad),
           .ibar   (),
    	   // synopsys translate_off
    	   .dynamicterminationcontrol(),
    	   // synopsys translate_on
           .o      (dqs_buffered)
           );

        assign dqsn_pad = 1'b0;

    end

    // QDRII has both DQS and DQSN, but only pseudo-differential.  Both are used for DQ
    // capture in the DDIO atom :
    else if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdr_ibuf_gen
        // Input buf
        stratixiii_io_ibuf dqs_inpt_io_ibuf(
           .i      (dqs_pad),
           .ibar   (),
    	   // synopsys translate_off
    	   .dynamicterminationcontrol(),
    	   // synopsys translate_on
           .o      (dqs_buffered)
           );

        // Input buf
        stratixiii_io_ibuf dqsn_inpt_io_ibuf(
           .i      (dqsn_pad),
           .ibar   (),
    	   // synopsys translate_off
    	   .dynamicterminationcontrol(),
    	   // synopsys translate_on
           .o      (dqsn_buffered)
           );

    end

    // DDR2 (with DQSN enabled) and DDR3 have true differential DQS inputs :
    else
    begin : ddr3_2_with_dqsn_ibuf_gen

        // Input buf
        stratixiii_io_ibuf dqs_inpt_io_ibuf(
           .i      (dqs_pad),
           .ibar   (dqsn_pad),
    	   // synopsys translate_off
    	   .dynamicterminationcontrol(),
    	   // synopsys translate_on
           .o      (dqs_buffered)
           );

    end

endgenerate

// DQS delay.


generate

    if (DQS_DELAY_CODE_FROM_DLL == "FALSE" || DQS_DELAY_CODE_FROM_DLL == "false" )
        assign chosen_dqs_delay_delayctrlin = seq_dqs_delay_ctrl;
    else
        assign chosen_dqs_delay_delayctrlin = dedicated_dll_delay_ctrl;

endgenerate

// The delay control for each DQS group comes from the modified delay control
// from the sequencer.  Note that it is correct that DLL_DELAY_BUFFER_MODE should
// be propagated to delay_buffer_mode here.

generate

    if (MEM_IF_MEMTYPE == "QDRII")
    begin : gen_qdrii_dqs_delay_chain

        if (DQS_DELAY_USES_OFFSET =="true" || DQS_DELAY_USES_OFFSET == "TRUE")
        begin : gen_dqs_delay_chain_with_offset

            stratixiii_dqs_delay_chain # (
            .dqs_input_frequency     (MEM_IF_CLK_PS_STR),
            .use_phasectrlin         (DQS_USE_PHASECTRL_IN),
            .phase_setting           (DQS_PHASE_SETTING),
            .delay_buffer_mode       (DLL_DELAY_BUFFER_MODE),
            .dqs_phase_shift         (DQS_PHASE),
            .dqs_offsetctrl_enable   (DQS_DELAY_USES_OFFSET),
            .dqs_ctrl_latches_enable (USE_DQS_DELAY_LATCHES)
            ) delay_chain(
               .dqsin          (dqs_buffered),
               .delayctrlin    (chosen_dqs_delay_delayctrlin),
               .offsetctrlin   (dll_offset_delay_ctrl),
               .dqsupdateen    (dqs_update_en),
               .phasectrlin    (dqsinputphasesetting),
            // synopsys translate_off
               .devclrn        (),
               .devpor         (),
               .dffin          (),
            // synopsys translate_on
               .dqsbusout      (dqs_enable_op)
            );
        end

        else
        begin: gen_dqs_delay_chain_no_offset

            stratixiii_dqs_delay_chain # (
            .dqs_input_frequency     (MEM_IF_CLK_PS_STR),
            .use_phasectrlin         (DQS_USE_PHASECTRL_IN),
            .phase_setting           (DQS_PHASE_SETTING),
            .delay_buffer_mode       (DLL_DELAY_BUFFER_MODE),
            .dqs_phase_shift         (DQS_PHASE),
            .dqs_offsetctrl_enable   (DQS_DELAY_USES_OFFSET),
            .dqs_ctrl_latches_enable (USE_DQS_DELAY_LATCHES)
            ) delay_chain(
               .dqsin          (dqs_buffered),
               .delayctrlin    (chosen_dqs_delay_delayctrlin),
               .offsetctrlin   (), //(dll_offset_delay_ctrl),
               .dqsupdateen    (dqs_update_en),
               .phasectrlin    (dqsinputphasesetting),
            // synopsys translate_off
               .devclrn        (),
               .devpor         (),
               .dffin          (),
            // synopsys translate_on
               .dqsbusout      (dqs_enable_op)
            );

        end

    end

    else
    begin : gen_ddr_dqs_delay_chain

        if (DQS_DELAY_USES_OFFSET == "true" || DQS_DELAY_USES_OFFSET == "TRUE")
        begin : gen_dqs_delay_chain_with_offset

            stratixiii_dqs_delay_chain # (
            .dqs_input_frequency     (MEM_IF_CLK_PS_STR),
            .use_phasectrlin         (DQS_USE_PHASECTRL_IN),
            .phase_setting           (DQS_PHASE_SETTING),
            .delay_buffer_mode       (DLL_DELAY_BUFFER_MODE),
            .dqs_phase_shift         (DQS_PHASE),
            .dqs_offsetctrl_enable   ("true"),
            .dqs_ctrl_latches_enable (USE_DQS_DELAY_LATCHES)
            ) delay_chain(
               .dqsin          (dqs_buffered),
               .delayctrlin    (chosen_dqs_delay_delayctrlin),
               .offsetctrlin   (dll_offset_delay_ctrl),
               .dqsupdateen    (dqs_update_en),
               .phasectrlin    (dqsinputphasesetting),
            // synopsys translate_off
               .devclrn        (),
               .devpor         (),
               .dffin          (),
            // synopsys translate_on
               .dqsbusout      (dqs_delayed)
            );
        end

        else
        begin: gen_dqs_delay_chain_no_offset

            stratixiii_dqs_delay_chain # (
            .dqs_input_frequency     (MEM_IF_CLK_PS_STR),
            .use_phasectrlin         (DQS_USE_PHASECTRL_IN),
            .phase_setting           (DQS_PHASE_SETTING),
            .delay_buffer_mode       (DLL_DELAY_BUFFER_MODE),
            .dqs_phase_shift         (DQS_PHASE),
            .dqs_offsetctrl_enable   ("false"),
            .dqs_ctrl_latches_enable (USE_DQS_DELAY_LATCHES)
            ) delay_chain(
               .dqsin          (dqs_buffered),
               .delayctrlin    (chosen_dqs_delay_delayctrlin),
               .offsetctrlin   (), //(dll_offset_delay_ctrl),
               .dqsupdateen    (dqs_update_en),
               .phasectrlin    (dqsinputphasesetting),
            // synopsys translate_off
               .devclrn        (),
               .devpor         (),
               .dffin          (),
            // synopsys translate_on
               .dqsbusout      (dqs_delayed)
            );

        end

    end

endgenerate

generate

    if (MEM_IF_USE_T7 == 1)
    begin : gen_dynamic_dqs_T7_delay_chain_gen

        // Delayed DQS delay chain - t7
        stratixiii_delay_chain dqs_t7_delay
        (
            .datain             (dqs_delayed),
            .delayctrlin        (dqsbusoutdelaysetting),
            // synopsys translate_off
	    .finedelayctrlin(),
            .devclrn(),
            .devpor(),
            // synopsys translate_on
            .dataout             (dqs_delayed2)
        );

    end

    else
    begin : gen_ddr_no_dqs_T7_delay_chain
        assign dqs_delayed2 = dqs_delayed;
    end

endgenerate



generate

    if (MEM_IF_MEMTYPE == "QDRII")
    begin : qdr_dqsn_delay_chain_gen

        if (DQS_DELAY_USES_OFFSET == "true" || DQS_DELAY_USES_OFFSET == "TRUE")
        begin

        stratixiii_dqs_delay_chain # (
                .dqs_input_frequency     (MEM_IF_CLK_PS_STR),
                .use_phasectrlin         (DQS_USE_PHASECTRL_IN),
                .phase_setting           (DQS_PHASE_SETTING),
                .delay_buffer_mode       (DLL_DELAY_BUFFER_MODE),
                .dqs_phase_shift         (DQS_PHASE),
                .dqs_offsetctrl_enable   ("true"),
                .dqs_ctrl_latches_enable (USE_DQS_DELAY_LATCHES)
            ) dqsn_delay_chain(
               .dqsin          (dqsn_buffered),
               .delayctrlin    (chosen_dqs_delay_delayctrlin),
               .offsetctrlin   (dll_offset_delay_ctrl),
               .dqsupdateen    (dqs_update_en),
               .phasectrlin    (dqsinputphasesetting),
                // synopsys translate_off
               .devclrn        (),
               .devpor         (),
               .dffin          (),
                // synopsys translate_on
               .dqsbusout      (dqsn_enable_op)
            );

    end

        else
        begin

        stratixiii_dqs_delay_chain # (
                .dqs_input_frequency     (MEM_IF_CLK_PS_STR),
                .use_phasectrlin         (DQS_USE_PHASECTRL_IN),
                .phase_setting           (DQS_PHASE_SETTING),
                .delay_buffer_mode       (DLL_DELAY_BUFFER_MODE),
                .dqs_phase_shift         (DQS_PHASE),
                .dqs_offsetctrl_enable   ("false"),
                .dqs_ctrl_latches_enable (USE_DQS_DELAY_LATCHES)
            ) dqsn_delay_chain(
               .dqsin          (dqsn_buffered),
               .delayctrlin    (chosen_dqs_delay_delayctrlin),
               .offsetctrlin   (), //(dll_offset_delay_ctrl),
               .dqsupdateen    (dqs_update_en),
               .phasectrlin    (dqsinputphasesetting),
                // synopsys translate_off
               .devclrn        (),
               .devpor         (),
               .dffin          (),
                // synopsys translate_on
               .dqsbusout      (dqsn_enable_op)
            );

    end

    end

endgenerate


generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate_dqs_enable_gen

        stratixiii_ddio_out # (
            .half_rate_mode("true"),
            .use_new_clocking_model("true")
        ) ddio(
            .datainhi (poa_postamble_en_preset[1]),
            .datainlo (poa_postamble_en_preset[0]),
            .clkhi    (resync_clk_1x),
            .clklo    (resync_clk_1x),
            .muxsel   (resync_clk_1x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
            .devclrn(),
            .devpor(),
            .clk(),
            // synopsys translate_on
            .dataout (dqs_enable_ddio_output)
        );

    end

    else
    begin
        assign dqs_enable_ddio_output  = poa_postamble_en_preset[0];
    end

endgenerate

generate

    if (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 1)
    begin : ddr3_dqs_enable_ctrl_gen
        // DQS Enable Control


        stratixiii_dqs_enable_ctrl # (
             .use_phasectrlin                ("true"),
             .phase_setting                  (0),
             .delay_buffer_mode              ("high"),
             .level_dqs_enable               ("true"),
             .delay_dqs_enable_by_half_cycle ("true"),
             .add_phase_transfer_reg         ("dynamic"),
             .invert_phase                   ("dynamic")
         ) dqs_enable_ctrl (
            .dqsenablein            (dqs_enable_ddio_output),
            .clk                    (resync_clk_2x),
            .delayctrlin            (dedicated_dll_delay_ctrl),
            .phasectrlin            (dqsenablectrlphasesetting),
            .enaphasetransferreg    (enadqsenablephasetransferreg),
            .phaseinvertctrl        (dqsenablectrlphaseinvert),
             // synopsys translate_off
            .devclrn                (),
            .devpor                 (),
            .dffin                  (),
            .dffextenddqsenable     (),
             // synopsys translate_on
            .dqsenableout           (dqs_enable_ctrl_op)
        );

         // DQS enable delay chain
         if (MEM_IF_USE_T11 == 1)
         begin : ddr3_using_t11_delay
         stratixiii_delay_chain dqs_enable_ctrl_dc1
         (
            .datain             (dqs_enable_ctrl_op),
            .delayctrlin        ({1'b0,dqsenabledelaysetting}),
             // synopsys translate_off
	     .finedelayctrlin(),
             .devclrn(),
             .devpor(),
             // synopsys translate_on
            .dataout             (dqs_enable_ctrl_op_delayed)
         );
         end
         else
         begin : ddr3_not_using_t11_delay
             assign dqs_enable_ctrl_op_delayed = dqs_enable_ctrl_op;
         end

        // DQS Enable
        stratixiii_dqs_enable dqs_enable_atom(
            .dqsin      (dqs_delayed2),
            .dqsenable  (dqs_enable_ctrl_op_delayed),
            // synopsys translate_off
            .devclrn    (),
            .devpor     (),
            // synopsys translate_on
            .dqsbusout  (dqs_enable_op)
        );

        // Tie-off unused DQSN enable for DDR :
        assign dqsn_enable_op = 1'b0;

    end

    else if (MEM_IF_MEMTYPE == "DDR2" || MEM_IF_MEMTYPE == "DDR" || (MEM_IF_MEMTYPE == "DDR3" && LEVELLING == 0))

    begin : ddr_dqs_enable_ctrl_gen
        // DQS Enable Control


        stratixiii_dqs_enable_ctrl # (
             .use_phasectrlin                ("false"),
             .phase_setting                  (0),         //zero tap used
             .delay_buffer_mode              ("high"),    // n/a due to zero tap
             .level_dqs_enable               ("true"),
             .delay_dqs_enable_by_half_cycle ("true"),
             .add_phase_transfer_reg         (ADD_PHASE_TRANSFER_REG_SETTING),
             .invert_phase                   (INVERT_POSTAMBLE_CLK)
         ) dqs_enable_ctrl (
            .dqsenablein            (dqs_enable_ddio_output),
            .clk                    (resync_clk_2x),
            .delayctrlin            (dedicated_dll_delay_ctrl),
            .phasectrlin            (dqsenablectrlphasesetting),
            .enaphasetransferreg    (enadqsenablephasetransferreg),
            .phaseinvertctrl        (dqsenablectrlphaseinvert),
             // synopsys translate_off
            .devclrn                (),
            .devpor                 (),
            .dffin                  (),
            .dffextenddqsenable     (),
             // synopsys translate_on
            .dqsenableout           (dqs_enable_ctrl_op)
        );

         // DQS enable delay chain   If used could be inserted here
         assign dqs_enable_ctrl_op_delayed = dqs_enable_ctrl_op;

        // DQS Enable
        stratixiii_dqs_enable dqs_enable_atom(
            .dqsin      (dqs_delayed2),
            .dqsenable  (dqs_enable_ctrl_op_delayed),
            // synopsys translate_off
            .devclrn    (),
            .devpor     (),
            // synopsys translate_on
            .dqsbusout  (dqs_enable_op)
        );

        // Tie-off unused DQSN enable for DDR :
        assign dqsn_enable_op = 1'b0;

    end

endgenerate



endmodule


//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_dm (
                             phy_clk_1x,
                             write_clk_2x,
                             sc_clk,
                             scan_din,
                             scan_update,
                             scan_enable,
                             scan_dout,
                             mem_dm,
                             wdp_dm3_1x,
                             wdp_dm2_1x,
                             wdp_dm1_1x,
                             wdp_dm0_1x,
                             dedicated_dll_delay_ctrl,
                             dqoutputphasesetting,
                             enaoutputcycledelaysetting,
                             enaoutputphasetransferreg,
                             dqoutputphaseinvert,
                             seriesterminationcontrol,
                             parallelterminationcontrol
                             );

parameter MEM_IF_MEMTYPE          =   "DDR";
parameter LEVELLING               =       1;
parameter DLL_DELAY_BUFFER_MODE   =  "HIGH";

parameter ENABLE_DDR3_SEQUENCER   = "FALSE";
parameter DQS_DELAY_CTL_WIDTH     =       6;
parameter DWIDTH_RATIO            =       4;

parameter MEM_IF_WR_T9_DESKEW_EN                  = 0;
parameter MEM_IF_WR_T10_DESKEW_EN                 = 0;
parameter MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL = "FALSE";
parameter OPA_USES_DELAYED_CLK                    = "false";
parameter SINGLE_LEVELLING_DELAY_CHAIN            = "true";

input wire                                             phy_clk_1x;
input wire                                             write_clk_2x;
input wire                                             sc_clk;
input wire                                             scan_din;
input wire                                             scan_update;
input wire                                             scan_enable;
output wire                                            scan_dout;

inout wire                                             mem_dm;

input wire                                             wdp_dm3_1x;
input wire                                             wdp_dm2_1x;
input wire                                             wdp_dm1_1x;
input wire                                             wdp_dm0_1x;

input wire [DQS_DELAY_CTL_WIDTH - 1 : 0 ]              dedicated_dll_delay_ctrl;

input wire [`DQSCONFIG_DQ_OP_PHASE_SETTING_WIDTH-1 :0] dqoutputphasesetting;
input wire                                             enaoutputcycledelaysetting;
input wire                                             enaoutputphasetransferreg;
input wire                                             dqoutputphaseinvert;

input wire [`OCT_SERIES_TERM_CONTROL_WIDTH   -1 : 0]   seriesterminationcontrol;
input wire [`OCT_PARALLEL_TERM_CONTROL_WIDTH -1 : 0]   parallelterminationcontrol;


wire [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING1_WIDTH-1 :0]   dq_outputdelaysetting1;
wire [`IOCONFIG_DQ_OUTPUT_DELAY_SETTING2_WIDTH-1 :0]   dq_outputdelaysetting2;


// Internal wires :

wire dqm_l;
wire dqm_h;
wire dqm_aligned;
wire dqm_delayed;
wire dqm_delayed2;


// Write side :

// NB. These atoms should be exactly the same as the DQ write path :

generate

    if (DWIDTH_RATIO == 4)
    begin : half_rate

        stratixiii_ddio_out # (
            .half_rate_mode("true"),
            .use_new_clocking_model("true")
        ) dqm_ddio_in_h (
            .datainhi (wdp_dm3_1x),
            .datainlo (wdp_dm2_1x),
            .clkhi    (phy_clk_1x),
            .clklo    (phy_clk_1x),
            .muxsel   (phy_clk_1x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
            .clk(),
            // synopsys translate_on
            .dataout (dqm_h),
            .devclrn(),
            .devpor()
        );

        stratixiii_ddio_out # (
            .half_rate_mode("true"),
            .use_new_clocking_model("true")
        ) dqm_ddio_in_l(
            .datainhi (wdp_dm1_1x),
            .datainlo (wdp_dm0_1x),
            .clkhi    (phy_clk_1x),
            .clklo    (phy_clk_1x),
            .muxsel   (phy_clk_1x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .dfflo(),
            .dffhi(),
            .clk(),
            // synopsys translate_on
            .dataout (dqm_l),
            .devclrn(),
            .devpor()
        );

    end

    else
    begin : full_rate
        assign dqm_l =  wdp_dm0_1x;
        assign dqm_h =  wdp_dm1_1x;
    end

endgenerate

// Phase alignment is either via DDIO for DDR/DDR2 or phase alignment atom for DDR3 :

generate

    if (MEM_IF_MEMTYPE == "DDR" || MEM_IF_MEMTYPE == "DDR2" || MEM_IF_MEMTYPE == "QDRII" || (MEM_IF_MEMTYPE == "DDR3" & LEVELLING == 0))
    begin : ddr_qdr_dm_ddio_gen
       // DDIO output
        stratixiii_ddio_out # (
            .half_rate_mode("false"),
            .use_new_clocking_model("true")
        ) dm_ddio_inst (
            .datainhi (dqm_h),
            .datainlo (dqm_l),
            .clkhi    (write_clk_2x),
            .clklo    (write_clk_2x),
            .muxsel   (write_clk_2x),
            .ena      (1'b1),
            .areset   (1'b0),
            .sreset   (1'b0),
            // synopsys translate_off
            .clk (),
            .dfflo(),
            .dffhi(),
            .devclrn(),
            .devpor(),
            // synopsys translate_on
            .dataout (dqm_aligned)
        );

    end

    else
    begin : ddr3_dm_opa_gen

        // Note : delay_buffer_mode for output_phase_alignment atoms must always
        // be tied to "high" :
        stratixiii_output_phase_alignment #(
            .operation_mode                  ("ddio_out"),
            .use_phasectrlin                 ("true"),
            .phase_setting                   (0),
            .delay_buffer_mode               ("high"),
            .power_up                        ("low"),
            .async_mode                      ("clear"),
            .sync_mode                       ("none"),
            .add_output_cycle_delay          ("dynamic"),
            .use_delayed_clock               (OPA_USES_DELAYED_CLK),
            .phase_setting_for_delayed_clock (2),
            .add_phase_transfer_reg          ("dynamic"),
            .use_phasectrl_clock             ("true"),
            .invert_phase                    ("dynamic"),
            .use_primary_clock               (SINGLE_LEVELLING_DELAY_CHAIN),
            .bypass_input_register           ("false")
        ) dm_opa_inst(
            .datain                ({dqm_h,dqm_l}),
            .clk                   (write_clk_2x),
            .delayctrlin           (dedicated_dll_delay_ctrl),
            .phasectrlin           (dqoutputphasesetting),
            .areset                (1'b0),
            .sreset                (1'b0),
            .clkena                (1'b1),
            .enaoutputcycledelay   (enaoutputcycledelaysetting),
            .enaphasetransferreg   (enaoutputphasetransferreg),
            .phaseinvertctrl       (dqoutputphaseinvert),
            // synopsys translate_off
            .devclrn(), .devpor(),
            .dffin(), .dff1t(), .dffddiodataout(),
            // synopsys translate_on
            .dataout               (dqm_aligned)
        );

    end

endgenerate

generate
    if (MEM_IF_WR_T9_DESKEW_EN == 1)
    begin : gen_T9_dp_deskew
        stratixiii_delay_chain dqm_t9_delay(
            .datain             (dqm_aligned),
            .delayctrlin        (dq_outputdelaysetting1),
            // synopsys translate_off
            .devclrn(), .devpor(),
            // synopsys translate_on
           .dataout             (dqm_delayed)
        );
    end
    else
    begin : gen_T9_dp_no_deskew
        assign dqm_delayed = dqm_aligned;
    end
endgenerate

generate
    if (MEM_IF_WR_T10_DESKEW_EN == 1)
    begin : gen_T10_dp_deskew
        stratixiii_delay_chain  dqm_t10_delay(
            .datain             (dqm_delayed),
            .delayctrlin        ({1'b0, dq_outputdelaysetting2}),
            // synopsys translate_off
            .devclrn(), .devpor(),
            // synopsys translate_on
           .dataout             (dqm_delayed2)
        );
    end
    else
    begin : gen_T10_dp_no_deskew
        assign dqm_delayed2 = dqm_delayed;
    end
endgenerate



// output buf
stratixiii_io_obuf # (
   .bus_hold                         ( "false"),
   .open_drain_output                ( "false"),
   .shift_series_termination_control (MEM_IF_SHIFT_SERIES_TERMINATION_CONTROL)
) dq_obuf_inst(
    .i                         (dqm_delayed2),
    .oe                        (1'b1),
    .dynamicterminationcontrol (1'b0),
    // synopsys translate_off
    .seriesterminationcontrol(seriesterminationcontrol),
    .parallelterminationcontrol(parallelterminationcontrol),
    .obar(),
    // synopsys translate_on
    .o(mem_dm),
    .devoe ()
);



// IO_CONFIG - num_dq 0->16
stratixiii_io_config dq_io_config(
    .datain(scan_din),          // shared per DQS group
    .clk(sc_clk),
    .ena(scan_enable),
    .update(scan_update),       // shared per DQS group
    // synopsys translate_off
    .devclrn(), .devpor(),
    .dutycycledelaymode(),
    .dutycycledelaysettings(),
    .outputfinedelaysetting1(),
    .outputfinedelaysetting2(),
    .outputonlydelaysetting2(),
    .outputonlyfinedelaysetting2(),
    .padtoinputregisterfinedelaysetting(),
    // synopsys translate_on
    .padtoinputregisterdelaysetting(),
    .outputdelaysetting1(dq_outputdelaysetting1),
    .outputdelaysetting2(dq_outputdelaysetting2),
    .dataout(scan_dout)
);

endmodule


//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_mux (
                         phy_clk_1x,
                         reset_phy_clk_1x_n,

// MUX Outputs to controller :
                         ctl_address,
                         ctl_read_req,
                         ctl_wdata,
                         ctl_write_req,
                         ctl_size,
                         ctl_be,
                         ctl_refresh_req,
                         ctl_burstbegin,

// Controller inputs to the MUX :
                         ctl_ready,
                         ctl_wdata_req,
                         ctl_rdata,
                         ctl_rdata_valid,
                         ctl_refresh_ack,
                         ctl_init_done,

// MUX Select line :
                         ctl_usr_mode_rdy,

// MUX inputs from local interface :
                         local_address,
                         local_read_req,
                         local_wdata,
                         local_write_req,
                         local_size,
                         local_be,
                         local_refresh_req,
                         local_burstbegin,

// MUX outputs to sequencer :
                         mux_seq_controller_ready,
                         mux_seq_wdata_req,

// MUX inputs from sequencer :
                         seq_mux_address,
                         seq_mux_read_req,
                         seq_mux_wdata,
                         seq_mux_write_req,
                         seq_mux_size,
                         seq_mux_be,
                         seq_mux_refresh_req,
                         seq_mux_burstbegin,

// Changes made to accomodate new ports for self refresh/power-down & Auto precharge in HP Controller (User to PHY)
                         local_autopch_req,
                         local_powerdn_req,
                         local_self_rfsh_req,
                         local_powerdn_ack,
                         local_self_rfsh_ack,
			
// Changes made to accomodate new ports for self refresh/power-down & Auto precharge in HP Controller (PHY to Controller)
                         ctl_autopch_req,
                         ctl_powerdn_req,
                         ctl_self_rfsh_req,
                         ctl_powerdn_ack,
                         ctl_self_rfsh_ack,

// Also MUX some signals from the controller to the local interface :
                         local_ready,
                         local_wdata_req,
                         local_init_done,
                         local_rdata,
                         local_rdata_valid,
                         local_refresh_ack

                       );


parameter LOCAL_IF_AWIDTH       =  26;
parameter LOCAL_IF_DWIDTH       = 256;
parameter LOCAL_BURST_LEN_BITS  =   1;
parameter MEM_IF_DQ_PER_DQS     =   8;
parameter MEM_IF_DWIDTH         =  64;

input wire                                           phy_clk_1x;
input wire                                           reset_phy_clk_1x_n;

// MUX Select line :
input wire                                           ctl_usr_mode_rdy;

// MUX inputs from local interface :
input wire [LOCAL_IF_AWIDTH - 1 : 0]                 local_address;
input wire                                           local_read_req;
input wire [LOCAL_IF_DWIDTH - 1 : 0]                 local_wdata;
input wire                                           local_write_req;
input wire [LOCAL_BURST_LEN_BITS - 1 : 0]            local_size;
input wire [(LOCAL_IF_DWIDTH/8) - 1 : 0]             local_be;
input wire                                           local_refresh_req;
input wire                                           local_burstbegin;

// MUX inputs from sequencer :
input wire [LOCAL_IF_AWIDTH - 1 : 0]                 seq_mux_address;
input wire                                           seq_mux_read_req;
input wire [LOCAL_IF_DWIDTH - 1 : 0]                 seq_mux_wdata;
input wire                                           seq_mux_write_req;
input wire [LOCAL_BURST_LEN_BITS - 1 : 0]            seq_mux_size;
input wire [(LOCAL_IF_DWIDTH/8) - 1:0]               seq_mux_be;
input wire                                           seq_mux_refresh_req;
input wire                                           seq_mux_burstbegin;

// MUX Outputs to controller :
output reg [LOCAL_IF_AWIDTH - 1 : 0]                 ctl_address;
output reg                                           ctl_read_req;
output reg [LOCAL_IF_DWIDTH - 1 : 0]                 ctl_wdata;
output reg                                           ctl_write_req;
output reg [LOCAL_BURST_LEN_BITS - 1 : 0]            ctl_size;
output reg [(LOCAL_IF_DWIDTH/8) - 1:0]               ctl_be;
output reg                                           ctl_refresh_req;
output reg                                           ctl_burstbegin;


// The "ready" input from the controller shall be passed to either the
// local interface if in user mode, or the sequencer :
input wire                                           ctl_ready;
output reg                                           local_ready;
output reg                                           mux_seq_controller_ready;

// The controller's "wdata req" output is similarly passed to either
// the local interface if in user mode, or the sequencer :
input wire                                           ctl_wdata_req;
output reg                                           local_wdata_req;
output reg                                           mux_seq_wdata_req;

input wire                                           ctl_init_done;
output reg                                           local_init_done;

input wire [LOCAL_IF_DWIDTH - 1 : 0]                 ctl_rdata;
output reg  [LOCAL_IF_DWIDTH - 1 : 0]                local_rdata;

input wire                                           ctl_rdata_valid;
output reg                                           local_rdata_valid;

input wire                                           ctl_refresh_ack;
output reg                                           local_refresh_ack;

//-> Changes made to accomodate new ports for self refresh/power-down & Auto precharge in HP Controller (User to PHY)
input  wire                                          local_autopch_req;
input  wire                                          local_powerdn_req;
input  wire                                          local_self_rfsh_req;
output reg                                           local_powerdn_ack;
output reg                                           local_self_rfsh_ack;
			
// --> Changes made to accomodate new ports for self refresh/power-down & Auto precharge in HP Controller (PHY to Controller)
output reg                                           ctl_autopch_req;
output reg                                           ctl_powerdn_req;
output reg                                           ctl_self_rfsh_req;
input  wire                                          ctl_powerdn_ack;
input  wire                                          ctl_self_rfsh_ack;


wire                                                 local_burstbegin_held;
reg                                                  burstbegin_hold;

always @(posedge phy_clk_1x or negedge reset_phy_clk_1x_n)
begin

    if (reset_phy_clk_1x_n == 1'b0) 
        burstbegin_hold <= 1'b0;
        
    else
    begin
    
        if (local_ready == 1'b0 && (local_write_req == 1'b1 || local_read_req == 1'b1) && local_burstbegin == 1'b1)
            burstbegin_hold <= 1'b1;
        else if (local_ready == 1'b1 && (local_write_req == 1'b1 || local_read_req == 1'b1))
            burstbegin_hold <= 1'b0;
            
    end
end

// Gate the local burstbegin signal with the held version :
assign local_burstbegin_held = burstbegin_hold || local_burstbegin;

always @*
begin

    if (ctl_usr_mode_rdy == 1'b1)
    begin

        // Pass local interface signals to the controller if ready :
        ctl_address            = local_address;
        ctl_read_req           = local_read_req;
        ctl_wdata              = local_wdata;
        ctl_write_req          = local_write_req;
        ctl_size               = local_size;
        ctl_be                 = local_be;
        ctl_refresh_req        = local_refresh_req;
        ctl_burstbegin         = local_burstbegin_held;

        // If in user mode,  pass on the controller's ready
        // and wdata request signals to the local interface :
        local_ready         = ctl_ready;
        local_wdata_req     = ctl_wdata_req;
        local_init_done     = ctl_init_done;
        local_rdata         = ctl_rdata;
        local_rdata_valid   = ctl_rdata_valid;
        local_refresh_ack   = ctl_refresh_ack;

        // Whilst indicate to the sequencer that the controller is busy :
        mux_seq_controller_ready = 1'b0;
        mux_seq_wdata_req        = 1'b0;

        // Autopch_req & Local_power_req changes
        ctl_autopch_req     = local_autopch_req;
        ctl_powerdn_req     = local_powerdn_req;
        ctl_self_rfsh_req   = local_self_rfsh_req;
        local_powerdn_ack   = ctl_powerdn_ack;
        local_self_rfsh_ack = ctl_self_rfsh_ack;


    end

    else
    begin

        // Pass local interface signals to the sequencer if not in user mode :

        // NB. The controller will have more address bits than the sequencer, so
        // these are zero padded :
        ctl_address            = seq_mux_address;
        ctl_read_req           = seq_mux_read_req;
        ctl_wdata              = seq_mux_wdata;
        ctl_write_req          = seq_mux_write_req;
        ctl_size               = seq_mux_size;        // NB. Should be tied-off when the mux is instanced
        ctl_be                 = seq_mux_be;          // NB. Should be tied-off when the mux is instanced
        ctl_refresh_req        = local_refresh_req; // NB. Should be tied-off when the mux is instanced
        ctl_burstbegin         = seq_mux_burstbegin; // NB. Should be tied-off when the mux is instanced

        // Indicate to the local IF that the controller is busy :
        local_ready         = 1'b0;
        local_wdata_req     = 1'b0;
        local_init_done     = 1'b0;
        local_rdata         = {LOCAL_IF_DWIDTH{1'b0}};
        local_rdata_valid   = 1'b0;
        local_refresh_ack   = ctl_refresh_ack;

        // If not in user mode,  pass on the controller's ready
        // and wdata request signals to the sequencer :
        mux_seq_controller_ready   = ctl_ready;
        mux_seq_wdata_req   = ctl_wdata_req;

       // Autopch_req & Local_power_req changes
        ctl_autopch_req     = 1'b0;
        ctl_powerdn_req     = 1'b0;
        ctl_self_rfsh_req   = local_self_rfsh_req;
        local_powerdn_ack   = 1'b0;
        local_self_rfsh_ack = ctl_self_rfsh_ack;

    end

end




endmodule

//

`default_nettype none

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

/* -----------------------------------------------------------------------------
// module description                                                        
---------------------------------------------------------------------------- */
//
module ddr2_phy_alt_mem_phy_mimic(
                         //Inputs 
        
                         //Clocks 
                         measure_clk,         // full rate clock from PLL
                         mimic_data_in,       // Input against which the VT variations 
                                              // are tracked (e.g. memory clock)        

                         // Active low reset                            
                         reset_measure_clk_n, 
        
                         //Indicates that the mimic calibration sequence can start
                         seq_mmc_start,       // from sequencer

        
                         //Outputs      
                         mmc_seq_done,        // mimic calibration finished for the current PLL phase       
                         mmc_seq_value        // result value of the mimic calibration
        
        );

   input  wire measure_clk;
   input  wire mimic_data_in;
   input  wire reset_measure_clk_n;
   input  wire seq_mmc_start;
   output wire mmc_seq_done;
   output wire mmc_seq_value;
   
   function integer clogb2;
      input [31:0] value;
      for (clogb2=0; value>0; clogb2=clogb2+1)
          value = value >> 1;
   endfunction // clogb2



   
   // Parameters        
   parameter NUM_MIMIC_SAMPLE_CYCLES = 6;     

   parameter SHIFT_REG_COUNTER_WIDTH = clogb2(NUM_MIMIC_SAMPLE_CYCLES); 
   

   reg [`MIMIC_FSM_WIDTH-1:0]           mimic_state;
   
  
   reg [2:0]                            seq_mmc_start_metastable; 
   wire                                 start_edge_detected;      
   
   (* altera_attribute=" -name fast_input_register OFF"*) reg [1:0] mimic_data_in_metastable; 


   wire                                 mimic_data_in_sample;   

   wire                                 shift_reg_data_out_all_ones; 
   reg                                  mimic_done_out;         
   reg                                  mimic_value_captured;      


   reg [SHIFT_REG_COUNTER_WIDTH : 0]    shift_reg_counter;              
   reg                                  shift_reg_enable;               

   wire                                 shift_reg_data_in; 
   reg                                  shift_reg_s_clr;
   wire                                 shift_reg_a_clr;   

   reg [NUM_MIMIC_SAMPLE_CYCLES -1 : 0] shift_reg_data_out;   
    
   // shift register which contains the sampled data
   always @(posedge measure_clk or posedge shift_reg_a_clr)
   begin
      if (shift_reg_a_clr == 1'b1)
      begin
          shift_reg_data_out    <= {NUM_MIMIC_SAMPLE_CYCLES{1'b0}};
      end
     
      else
      begin
         if (shift_reg_s_clr == 1'b1)
         begin
             shift_reg_data_out <= {NUM_MIMIC_SAMPLE_CYCLES{1'b0}};
         end     

         else if (shift_reg_enable == 1'b1)
         begin
             shift_reg_data_out <= {(shift_reg_data_out[NUM_MIMIC_SAMPLE_CYCLES -2 : 0]), shift_reg_data_in};         
         end     
      end 
   end 
          
    
  // Metastable-harden mimic_start : 
  always @(posedge measure_clk or negedge reset_measure_clk_n)
  begin
  
    if (reset_measure_clk_n == 1'b0)
    begin
        seq_mmc_start_metastable    <= 0;    
    end
    else

    begin
        seq_mmc_start_metastable[0] <= seq_mmc_start;
        seq_mmc_start_metastable[1] <= seq_mmc_start_metastable[0]; 
        seq_mmc_start_metastable[2] <= seq_mmc_start_metastable[1];    
    end 
  
  end 

  assign start_edge_detected =  seq_mmc_start_metastable[1] 
                             && !seq_mmc_start_metastable[2];
                                                                
  // Metastable-harden mimic_data_in : 
  always @(posedge measure_clk or negedge reset_measure_clk_n)
  begin

    if (reset_measure_clk_n == 1'b0)
    begin
        mimic_data_in_metastable    <= 0; 
    end
      //some mimic paths configurations have another flop inside the wysiwyg ioe 
    else
      
    begin  
        mimic_data_in_metastable[0] <= mimic_data_in;    
        mimic_data_in_metastable[1] <= mimic_data_in_metastable[0];             
    end 
    
  end 
  
  assign mimic_data_in_sample =  mimic_data_in_metastable[1];
   
  // Main FSM : 
  always @(posedge measure_clk or negedge reset_measure_clk_n )
  begin
  
     if (reset_measure_clk_n == 1'b0)
     begin
  
         mimic_state           <= `MIMIC_IDLE;
      
         mimic_done_out        <= 1'b0;
         mimic_value_captured  <= 1'b0;
      
         shift_reg_counter     <= 0;       
         shift_reg_enable      <= 1'b0;
         shift_reg_s_clr       <= 1'b0;  

     end 
     
     else
     begin
  
         case (mimic_state)
      
         `MIMIC_IDLE : begin
        
                           shift_reg_counter     <= 0;       
                           mimic_done_out        <= 1'b0;
                           shift_reg_s_clr       <= 1'b1;
                           shift_reg_enable      <= 1'b1;         
            
                           if (start_edge_detected == 1'b1)
                           begin
                               mimic_state       <= `MIMIC_SAMPLE; 
                               shift_reg_counter <= shift_reg_counter + 1'b1;     
                               shift_reg_s_clr   <= 1'b0;
                           end             
                           else
                             
                           begin
                               mimic_state <= `MIMIC_IDLE; 
                           end
         end // case: MIMIC_IDLE
           
           `MIMIC_SAMPLE : begin
  
                               shift_reg_counter        <= shift_reg_counter + 1'b1;     
              
                               if (shift_reg_counter == NUM_MIMIC_SAMPLE_CYCLES + 1)

                               begin
                                   mimic_done_out       <= 1'b1; 
                                   mimic_value_captured <= shift_reg_data_out_all_ones; //captured only here
                                   shift_reg_enable     <= 1'b0;        
                                   shift_reg_counter    <= shift_reg_counter;          
                                   mimic_state          <= `MIMIC_SEND;                    
                               end 
           end // case: MIMIC_SAMPLE
           
           `MIMIC_SEND : begin
        
                             mimic_done_out  <= 1'b1; //redundant statement, here just for readibility 
                             mimic_state     <= `MIMIC_SEND1; 

            /* mimic_value_captured will not change during MIMIC_SEND
               it will change next time mimic_done_out is asserted
               mimic_done_out will be reset during MIMIC_IDLE
               the purpose of the current state is to add one clock cycle
               mimic_done_out will be active for 2 measure_clk clock cycles, i.e
               the pulses duration will be just one sequencer clock cycle
               (which is half rate) */
           end // case: MIMIC_SEND

           // MIMIC_SEND1 and MIMIC_SEND2 extend the mimic_done_out signal by another 2 measure_clk_2x cycles
           // so it is a total of 4 measure clocks long (ie 2 half-rate clock cycles long in total)
           `MIMIC_SEND1 : begin
        
                              mimic_done_out  <= 1'b1; //redundant statement, here just for readibility 
                              mimic_state     <= `MIMIC_SEND2; 

           end 

           `MIMIC_SEND2 : begin
        
                              mimic_done_out  <= 1'b1; //redundant statement, here just for readibility 
                              mimic_state     <= `MIMIC_IDLE; 

           end 

           
           default : begin
                         mimic_state <= `MIMIC_IDLE;
           end
           
            
         endcase
      
     end 
  
  end 
  
  assign shift_reg_data_out_all_ones   = (( & shift_reg_data_out) == 1'b1) ? 1'b1 
                                                                           : 1'b0;
   
  // Shift Register assignments
  assign shift_reg_data_in  =  mimic_data_in_sample;
  assign shift_reg_a_clr    =  !reset_measure_clk_n; 
  
  // Output assignments  
  assign mmc_seq_done    = mimic_done_out;   
  assign mmc_seq_value   = mimic_value_captured;         
  
  
endmodule

`default_nettype wire

//


/* -----------------------------------------------------------------------------
// module description                                                        
----------------------------------------------------------------------------- */
//
module ddr2_phy_alt_mem_phy_mimic_debug(
        // Inputs
        
        // Clocks 
        measure_clk,    // full rate clock from PLL

        // Active low reset                             
        reset_measure_clk_n, 

        mimic_recapture_debug_data, // from user board button
        
        mmc_seq_done,   // mimic calibration finished for the current PLL phase
        mmc_seq_value   // result value of the mimic calibration        

        );


   // Parameters 

   parameter NUM_DEBUG_SAMPLES_TO_STORE = 4096;   // can range from 4096 to 524288
   parameter PLL_STEPS_PER_CYCLE        = 24;     // can  range from 16 to 48  
   
   input wire measure_clk;  
   input wire reset_measure_clk_n;

   input wire mimic_recapture_debug_data;

   input wire mmc_seq_done; 
   input wire mmc_seq_value; 


   function integer clogb2;
      input [31:0] value;
      for (clogb2=0; value>0; clogb2=clogb2+1)
          value = value >> 1;
   endfunction // clogb2
   

   parameter RAM_WR_ADDRESS_WIDTH = clogb2(NUM_DEBUG_SAMPLES_TO_STORE - 1); // can range from 12 to 19 


   reg                                       s_clr_ram_wr_address_count; 

   reg [(clogb2(PLL_STEPS_PER_CYCLE)-1) : 0] mimic_sample_count;        

   reg [RAM_WR_ADDRESS_WIDTH-1 : 0 ]         ram_write_address; 
   wire                                      ram_wr_enable;             
   wire [0:0]                                debug_ram_data;            
   reg                                       clear_ram_wr_enable;               

   reg [1:0]                                 mimic_recapture_debug_data_metastable; 

   wire                                      mimic_done_in_dbg; // for internal use, just 1 measure_clk cycles long
   reg                                       mmc_seq_done_r;               
  

   // generate mimic_done_in_debug : a single clock wide pulse based on the rising edge of mmc_seq_done:

   always @ (posedge measure_clk or negedge reset_measure_clk_n)
   begin  
     if (reset_measure_clk_n == 1'b0)      // asynchronous reset (active low)
     begin
         mmc_seq_done_r <= 1'b0;
     end
     else

     begin
         mmc_seq_done_r <= mmc_seq_done;
     end
      
   end


   assign mimic_done_in_dbg   = mmc_seq_done && !mmc_seq_done_r;  

   assign ram_wr_enable       = mimic_done_in_dbg && !clear_ram_wr_enable;
   assign debug_ram_data[0]   = mmc_seq_value;

    

  altsyncram #(

                .clock_enable_input_a   ( "BYPASS"),
                .clock_enable_output_a  ( "BYPASS"),
                .intended_device_family ( "Stratix II"),
                .lpm_hint               ( "ENABLE_RUNTIME_MOD=YES, INSTANCE_NAME=MRAM"),
                .lpm_type               ( "altsyncram"),
                .maximum_depth          ( 4096),
                .numwords_a             ( 4096),
                .operation_mode         ( "SINGLE_PORT"),
                .outdata_aclr_a         ( "NONE"),
                .outdata_reg_a          ( "UNREGISTERED"),
                .power_up_uninitialized ( "FALSE"),
                .widthad_a              ( 12),
                .width_a                ( 1),
                .width_byteena_a        ( 1)
        )
         altsyncram_component (
                .wren_a                 ( ram_wr_enable),
                .clock0                 ( measure_clk),
                .address_a              ( ram_write_address),
                .data_a                 ( debug_ram_data),
                .q_a                    ( )
        );
   

   

  //  Metastability_mimic_recapture_debug_data : 
  always @(posedge measure_clk or negedge reset_measure_clk_n)
  begin

    if (reset_measure_clk_n == 1'b0)
    begin 
        mimic_recapture_debug_data_metastable    <=  2'b0;
    end
    else

    begin
        mimic_recapture_debug_data_metastable[0] <= mimic_recapture_debug_data;
        mimic_recapture_debug_data_metastable[1] <= mimic_recapture_debug_data_metastable[0];   
    end
    
  end 
   


  //mimic_sample_counter : 
  always @(posedge measure_clk or negedge reset_measure_clk_n)
  begin
  
    if (reset_measure_clk_n == 1'b0) 
    begin
        mimic_sample_count <= 0;        // (others => '0'); 
    end  
    else

    begin
      if (mimic_done_in_dbg == 1'b1)
      begin
          mimic_sample_count <= mimic_sample_count + 1'b1;       

          if (mimic_sample_count == PLL_STEPS_PER_CYCLE-1)
          begin
              mimic_sample_count <= 0; //(others => '0');
          end
                
      end 
      
    end 
  
  end 

  

  //RAMWrAddressCounter : 
  always @(posedge measure_clk or negedge reset_measure_clk_n)
  begin
  
      if (reset_measure_clk_n == 1'b0)
      begin
          ram_write_address <= 0;      //(others => '0');   
          clear_ram_wr_enable <= 1'b0;
      end
      else

      begin
  
          if (s_clr_ram_wr_address_count == 1'b1) // then --Active high synchronous reset
          begin
              ram_write_address <= 0;      //(others => '0');
              clear_ram_wr_enable <= 1'b1;         
          end

          else       
          begin
              clear_ram_wr_enable <= 1'b0;   
          
              if (mimic_done_in_dbg == 1'b1)
              begin
                  if (ram_write_address != NUM_DEBUG_SAMPLES_TO_STORE-1)  
                  begin 
                      ram_write_address <= ram_write_address + 1'b1;             
                  end
                  
                  else
                  begin
                      clear_ram_wr_enable <= 1'b1;        
                  end 
              end
          
          end 
      
      end
  
  end 
  
  //ClearRAMWrAddressCounter : 
  always @(posedge measure_clk or negedge reset_measure_clk_n)
  begin
  
      if (reset_measure_clk_n == 1'b0)
      begin
          s_clr_ram_wr_address_count <= 1'b0;  
      end       

      else
      begin
          if (mimic_recapture_debug_data_metastable[1] == 1'b1)
          begin
              s_clr_ram_wr_address_count <= 1'b1;
          end
       
          else if (mimic_sample_count == 0)       
          begin
              s_clr_ram_wr_address_count <= 1'b0;
          end
      
      end 
  
  end 
  
  endmodule

//

`ifdef ALT_MEM_PHY_DEFINES
`else
`include "alt_mem_phy_defines.v"
`endif

//
module ddr2_phy_alt_mem_phy_reset_pipe (
                                input  wire clock,
                                input  wire pre_clear,
                                output wire reset_out
                              );

parameter PIPE_DEPTH = 4;

    // Declare pipeline registers.
    reg [PIPE_DEPTH - 1 : 0]  ams_pipe;
    integer                   i;

//    begin : RESET_PIPE
        always @(posedge clock or negedge pre_clear)
        begin

            if (pre_clear == 1'b0)
            begin
                ams_pipe <= 0;
            end

            else
            begin
               for (i=0; i< PIPE_DEPTH; i = i + 1)
               begin
                   if (i==0)
                       ams_pipe[i] <= 1'b1;
                   else
                       ams_pipe[i] <= ams_pipe[i-1];
               end
            end // if-else

        end // always
//    end

    assign reset_out = ams_pipe[PIPE_DEPTH-1];


endmodule
