

module top (
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pins
	CLOCK_50,
	CLOCK2_50,
	CLOCK3_50,
	CLOCK4_50,

	// Pushbuttons
	KEY,

	////////////////////////////////////
	// HPS Pins
	////////////////////////////////////

	// DDR3 SDRAM
	HPS_DDR3_ADDR,
	HPS_DDR3_BA,
	HPS_DDR3_CAS_N,
	HPS_DDR3_CKE,
	HPS_DDR3_CK_N,
	HPS_DDR3_CK_P,
	HPS_DDR3_CS_N,
	HPS_DDR3_DM,
	HPS_DDR3_DQ,
	HPS_DDR3_DQS_N,
	HPS_DDR3_DQS_P,
	HPS_DDR3_ODT,
	HPS_DDR3_RAS_N,
	HPS_DDR3_RESET_N,
	HPS_DDR3_RZQ,
	HPS_DDR3_WE_N,

	// Pushbutton
	HPS_KEY,

	// LED
	HPS_LED
);

//=======================================================
//  PARAMETER declarations
//=======================================================


//=======================================================
//  PORT declarations
//=======================================================

////////////////////////////////////
// FPGA Pins
////////////////////////////////////

// Clock pins
input					CLOCK_50;
input					CLOCK2_50;
input					CLOCK3_50;
input					CLOCK4_50;

// Pushbuttons
input		    [ 3: 0]	KEY;


////////////////////////////////////
// HPS Pins
////////////////////////////////////

// DDR3 SDRAM
output		    [14: 0]	HPS_DDR3_ADDR;
output		    [ 2: 0] HPS_DDR3_BA;
output					HPS_DDR3_CAS_N;
output					HPS_DDR3_CKE;
output					HPS_DDR3_CK_N;
output					HPS_DDR3_CK_P;
output					HPS_DDR3_CS_N;
output		    [ 3: 0]	HPS_DDR3_DM;
inout			[31: 0]	HPS_DDR3_DQ;
inout			[ 3: 0]	HPS_DDR3_DQS_N;
inout			[ 3: 0]	HPS_DDR3_DQS_P;
output					HPS_DDR3_ODT;
output					HPS_DDR3_RAS_N;
output					HPS_DDR3_RESET_N;
input					HPS_DDR3_RZQ;
output					HPS_DDR3_WE_N;

// Pushbutton
inout					HPS_KEY;

// LED
inout					HPS_LED;


//=======================================================
//  REG/WIRE declarations
//=======================================================


//=======================================================
//  Structural coding
//=======================================================

legup_system The_System (
	////////////////////////////////////
	// FPGA Side
	////////////////////////////////////

	// Global signals
	.system_pll_refclk_clk 	            (CLOCK_50),
	//.system_pll_reset_reset             (1'b0),
	.system_pll_reset_reset             (~KEY[0]),


	////////////////////////////////////
	// HPS Side
	////////////////////////////////////

	// DDR3 SDRAM
	.memory_mem_a	        		    (HPS_DDR3_ADDR),
	.memory_mem_ba	    	        	(HPS_DDR3_BA),
	.memory_mem_ck	    	        	(HPS_DDR3_CK_P),
	.memory_mem_ck_n    	        	(HPS_DDR3_CK_N),
	.memory_mem_cke	    	            (HPS_DDR3_CKE),
	.memory_mem_cs_n    		        (HPS_DDR3_CS_N),
	.memory_mem_ras_n   		        (HPS_DDR3_RAS_N),
	.memory_mem_cas_n   		        (HPS_DDR3_CAS_N),
	.memory_mem_we_n    		        (HPS_DDR3_WE_N),
	.memory_mem_reset_n 	            (HPS_DDR3_RESET_N),
	.memory_mem_dq	    		        (HPS_DDR3_DQ),
	.memory_mem_dqs	    	            (HPS_DDR3_DQS_P),
	.memory_mem_dqs_n   		        (HPS_DDR3_DQS_N),
	.memory_mem_odt	    	            (HPS_DDR3_ODT),
	.memory_mem_dm	    		        (HPS_DDR3_DM),
	.memory_oct_rzqin   		        (HPS_DDR3_RZQ),

	// Pushbutton
	.hps_io_hps_io_gpio_inst_GPIO54 	(HPS_KEY),

	// LED
	.hps_io_hps_io_gpio_inst_GPIO53 	(HPS_LED)
);


endmodule
