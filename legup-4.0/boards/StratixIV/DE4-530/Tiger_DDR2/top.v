//`define USE_DDR2_DIMM2 // When switch : 1.Run Analysis & Synthesis 2.Run Tcl 3.Full Compile

module DE4_530 (

	//////// CLOCK //////////
	GCLKIN,
	GCLKOUT_FPGA,
	OSC_50_Bank2,
	OSC_50_Bank3,
	OSC_50_Bank4,
	OSC_50_Bank5,
	OSC_50_Bank6,
	OSC_50_Bank7,
	PLL_CLKIN_p,

	//////// LED x 8 //////////
	LED,

	//////// BUTTON x 4 //////////
	BUTTON,
	CPU_RESET_n,
	EXT_IO,

`ifndef USE_DDR2_DIMM2
	//////// DDR2 SODIMM //////////
	M1_DDR2_addr,
	M1_DDR2_ba,
	M1_DDR2_cas_n,
	M1_DDR2_cke,
	M1_DDR2_clk,
	M1_DDR2_clk_n,
	M1_DDR2_cs_n,
	M1_DDR2_dm,
	M1_DDR2_dq,
	M1_DDR2_dqs,
	M1_DDR2_dqsn,
	M1_DDR2_odt,
	M1_DDR2_ras_n,
	M1_DDR2_SA,
	M1_DDR2_SCL,
	M1_DDR2_SDA,
	M1_DDR2_we_n,
	M1_DDR2_oct_rdn,
	M1_DDR2_oct_rup
	

`else
	//////// DDR2 SODIMM //////////

	M2_DDR2_addr,
	M2_DDR2_ba,
	M2_DDR2_cas_n,
	M2_DDR2_cke,
	M2_DDR2_clk,
	M2_DDR2_clk_n,
	M2_DDR2_cs_n,
	M2_DDR2_dm,
	M2_DDR2_dq,
	M2_DDR2_dqs,
	M2_DDR2_dqsn,
	M2_DDR2_odt,
	M2_DDR2_ras_n,
	M2_DDR2_SA,
	M2_DDR2_SCL,
	M2_DDR2_SDA,
	M2_DDR2_we_n, 
	M2_DDR2_oct_rdn,
	M2_DDR2_oct_rup		
	
`endif	//USE_DDR2_DIMM2
);

//////////// CLOCK //////////
input		       		GCLKIN;
output		      		GCLKOUT_FPGA;
input		       		OSC_50_Bank2;
input		       		OSC_50_Bank3;
input		       		OSC_50_Bank4;
input		       		OSC_50_Bank5;
input		       		OSC_50_Bank6;
input		       		OSC_50_Bank7;
input		       		PLL_CLKIN_p;

//////////// LED x 8 //////////
output		  [7:0]		LED;

//////////// BUTTON x 4 //////////
input		  [3:0]		BUTTON;
input		       		CPU_RESET_n;
inout		       		EXT_IO;

`ifndef USE_DDR2_DIMM2
//////////// DDR2 SODIMM //////////
output		 [15:0]		M1_DDR2_addr;
output		  [2:0]		M1_DDR2_ba;
output		       		M1_DDR2_cas_n;
output		  [1:0]		M1_DDR2_cke;
inout		  [1:0]		M1_DDR2_clk;
inout		  [1:0]		M1_DDR2_clk_n;
output		  [1:0]		M1_DDR2_cs_n;
output		  [7:0]		M1_DDR2_dm;
inout		 [63:0]		M1_DDR2_dq;
inout		  [7:0]		M1_DDR2_dqs;
inout		  [7:0]		M1_DDR2_dqsn;
output		  [1:0]		M1_DDR2_odt;
output		       		M1_DDR2_ras_n;
output		  [1:0]		M1_DDR2_SA;
output		       		M1_DDR2_SCL;
inout		    		M1_DDR2_SDA;
output		       		M1_DDR2_we_n;
input                   M1_DDR2_oct_rdn;
input                   M1_DDR2_oct_rup;

`else
//////////// DDR2 SODIMM //////////
output		 [15:0]		M2_DDR2_addr;
output		  [2:0]		M2_DDR2_ba;
output		       		M2_DDR2_cas_n;
output		  [1:0]		M2_DDR2_cke;
inout		  [1:0]		M2_DDR2_clk;
inout		  [1:0]		M2_DDR2_clk_n;
output		  [1:0]		M2_DDR2_cs_n;
output		  [7:0]		M2_DDR2_dm;
inout		 [63:0]		M2_DDR2_dq;
inout		  [7:0]		M2_DDR2_dqs;
inout		  [7:0]		M2_DDR2_dqsn;
output		  [1:0]		M2_DDR2_odt;
output		       		M2_DDR2_ras_n;
output		  [1:0]		M2_DDR2_SA;
output		       		M2_DDR2_SCL;
inout		   	   		M2_DDR2_SDA;
output		       		M2_DDR2_we_n;
input                   M2_DDR2_oct_rdn;
input                   M2_DDR2_oct_rup;

`endif //`ifndef USE_DDR2_DIMM2

wire CLOCK_50i;
	
	// 50 MHz pll giving out -3ns phase shift for SDRAM clock
//	pll50MHz pll50(.inclk0(CLOCK_50), .c0(CLOCK_50i), .c1(DRAM_CLK));

	legup_system legup_system_qsys(
		// clock and reset
		.reset_reset_n(1'b1),
		
`ifndef USE_DDR2_DIMM2		  
        .clk_clk                                                (OSC_50_Bank3), 
		  
        .ddr2_memory_mem_a                                      (M1_DDR2_addr),
        .ddr2_memory_mem_ba                                     (M1_DDR2_ba),
        .ddr2_memory_mem_ck                                     (M1_DDR2_clk),
        .ddr2_memory_mem_ck_n                                   (M1_DDR2_clk_n),
        .ddr2_memory_mem_cke                                    (M1_DDR2_cke),
        .ddr2_memory_mem_cs_n                                   (M1_DDR2_cs_n),
        .ddr2_memory_mem_dm                                     (M1_DDR2_dm),
        .ddr2_memory_mem_ras_n                                  (M1_DDR2_ras_n), 
        .ddr2_memory_mem_cas_n                                  (M1_DDR2_cas_n),
        .ddr2_memory_mem_we_n                                   (M1_DDR2_we_n),
        .ddr2_memory_mem_dq                                     (M1_DDR2_dq),
        .ddr2_memory_mem_dqs                                    (M1_DDR2_dqs),
        .ddr2_memory_mem_dqs_n                                  (M1_DDR2_dqsn),
        .ddr2_memory_mem_odt                                    (M1_DDR2_odt),
        .ddr2_oct_rdn                                           (M1_DDR2_oct_rdn),
        .ddr2_oct_rup                                           (M1_DDR2_oct_rup),
`else
        .clk_clk                                                (OSC_50_Bank4),
		  
        .ddr2_memory_mem_a                                      (M2_DDR2_addr),
        .ddr2_memory_mem_ba                                     (M2_DDR2_ba),
        .ddr2_memory_mem_ck                                     (M2_DDR2_clk), 
        .ddr2_memory_mem_ck_n                                   (M2_DDR2_clk_n),
        .ddr2_memory_mem_cke                                    (M2_DDR2_cke),
        .ddr2_memory_mem_cs_n                                   (M2_DDR2_cs_n),
        .ddr2_memory_mem_dm                                     (M2_DDR2_dm), 
        .ddr2_memory_mem_ras_n                                  (M2_DDR2_ras_n),
        .ddr2_memory_mem_cas_n                                  (M2_DDR2_cas_n),
        .ddr2_memory_mem_we_n                                   (M2_DDR2_we_n), 
        .ddr2_memory_mem_dq                                     (M2_DDR2_dq), 
        .ddr2_memory_mem_dqs                                    (M2_DDR2_dqs),
        .ddr2_memory_mem_dqs_n                                  (M2_DDR2_dqsn),
        .ddr2_memory_mem_odt                                    (M2_DDR2_odt), 
        .ddr2_oct_rdn                                           (M2_DDR2_oct_rdn), 
        .ddr2_oct_rup                                           (M2_DDR2_oct_rup),
`endif
        .ddr2_mem_if_ddr2_emif_status_local_init_done            (ddr2_init_done),
        .ddr2_mem_if_ddr2_emif_status_local_cal_success          (ddr2_cal_success),
        .ddr2_mem_if_ddr2_emif_status_local_cal_fail             (ddr2_cal_fail)
	);

endmodule
