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
// This file instantiates the OCT block.
// ******************************************************************************************************************************** 

`timescale 1 ps / 1 ps

(* altera_attribute = "-name IP_TOOL_NAME altera_mem_if_oct; -name IP_TOOL_VERSION 13.1; -name FITTER_ADJUST_HC_SHORT_PATH_GUARDBAND 100; -name ALLOW_SYNCH_CTRL_USAGE OFF; -name AUTO_CLOCK_ENABLE_RECOGNITION OFF; -name AUTO_SHIFT_REGISTER_RECOGNITION OFF" *)


module altera_mem_if_oct_stratixiv (
	oct_rdn,
	oct_rup,
	parallelterminationcontrol,
	seriesterminationcontrol
);


parameter OCT_TERM_CONTROL_WIDTH = 0;


// These should be connected to reference resistance pins on the board, via OCT control block if instantiated by user
input oct_rdn;
input oct_rup;

// for OCT master, termination control signals will be available to top level
output [OCT_TERM_CONTROL_WIDTH-1:0] parallelterminationcontrol;
output [OCT_TERM_CONTROL_WIDTH-1:0] seriesterminationcontrol;



	`ifndef ALTERA_RESERVED_QIS
	// synopsys translate_off
	`endif
	tri0  oct_rdn;
	tri0  oct_rup;
	`ifndef ALTERA_RESERVED_QIS
	// synopsys translate_on
	`endif

	wire  [0:0]   wire_sd1a_serializerenableout;
	wire  [0:0]   wire_sd1a_terminationcontrol;

	stratixiv_termination   sd1a_0
	( 
	.incrdn(),
	.incrup(),
	.rdn(oct_rdn),
	.rup(oct_rup),
	.scanout(),
	.serializerenableout(wire_sd1a_serializerenableout[0:0]),
	.shiftregisterprobe(),
	.terminationcontrol(wire_sd1a_terminationcontrol[0:0]),
	.terminationcontrolprobe()
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.otherserializerenable({9{1'b0}}),
	.scanen(1'b0),
	.serializerenable(1'b0),
	.terminationclear(1'b0),
	.terminationclock(1'b0),
	.terminationcontrolin(1'b0),
	.terminationenable(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	// synopsys translate_off
	,
	.devclrn(1'b1),
	.devpor(1'b1)
	// synopsys translate_on
	);
	defparam sd1a_0.enable_parallel_termination  = "TRUE";

	stratixiv_termination_logic   sd2a_0
	( 
	.parallelterminationcontrol(parallelterminationcontrol),
	.serialloadenable(wire_sd1a_serializerenableout),
	.seriesterminationcontrol(seriesterminationcontrol),
	.terminationdata(wire_sd1a_terminationcontrol)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.parallelloadenable(1'b0),
	.terminationclock(1'b0)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	// synopsys translate_off
	,
	.devclrn(1'b1),
	.devpor(1'b1)
	// synopsys translate_on
	);



endmodule

