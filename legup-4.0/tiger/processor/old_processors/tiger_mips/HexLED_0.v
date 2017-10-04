// HexLED_0.v

// This file was auto-generated as part of a SOPC Builder generate operation.
// If you edit it your changes will probably be lost.

module HexLED_0 (
		input  wire        iCLOCK,   //  global_signals_clock.clk
		input  wire        iRESET_N, //                      .reset_n
		input  wire        iWR,      //        avalon_slave_0.write
		input  wire [31:0] iDATA,    //                      .writedata
		output wire [7:0]  HEX0,     // avalon_slave_0_export.export
		output wire [7:0]  HEX1,     //                      .export
		output wire [7:0]  HEX2,     //                      .export
		output wire [7:0]  HEX3,     //                      .export
		output wire [7:0]  HEX4,     //                      .export
		output wire [7:0]  HEX5,     //                      .export
		output wire [7:0]  HEX6,     //                      .export
		output wire [7:0]  HEX7      //                      .export
	);

	HexLED hexled_0 (
		.iCLOCK   (iCLOCK),   //  global_signals_clock.clk
		.iRESET_N (iRESET_N), //                      .reset_n
		.iWR      (iWR),      //        avalon_slave_0.write
		.iDATA    (iDATA),    //                      .writedata
		.HEX0     (HEX0),     // avalon_slave_0_export.export
		.HEX1     (HEX1),     //                      .export
		.HEX2     (HEX2),     //                      .export
		.HEX3     (HEX3),     //                      .export
		.HEX4     (HEX4),     //                      .export
		.HEX5     (HEX5),     //                      .export
		.HEX6     (HEX6),     //                      .export
		.HEX7     (HEX7)      //                      .export
	);

endmodule
