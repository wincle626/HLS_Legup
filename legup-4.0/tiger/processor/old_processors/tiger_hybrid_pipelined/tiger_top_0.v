// tiger_top_0.v

// This file was auto-generated as part of a SOPC Builder generate operation.
// If you edit it your changes will probably be lost.

module tiger_top_0 (
		input  wire        clk,                                      //             clock.clk
		input  wire        reset,                                    //             reset.reset
		output wire [71:0] aso_TigertoCache_data,                    //      TigertoCache.data
		input  wire [39:0] asi_CachetoTiger_data,                    //      CachetoTiger.data
		output wire        avm_instructionMaster_read,               // instructionMaster.read
		output wire [31:0] avm_instructionMaster_address,            //                  .address
		input  wire [31:0] avm_instructionMaster_readdata,           //                  .readdata
		output wire        avm_instructionMaster_beginbursttransfer, //                  .beginbursttransfer
		output wire [2:0]  avm_instructionMaster_burstcount,         //                  .burstcount
		input  wire        avm_instructionMaster_waitrequest,        //                  .waitrequest
		input  wire        avm_instructionMaster_readdatavalid       //                  .readdatavalid
	);

	tiger_top tiger_top_0 (
		.clk                                      (clk),                                      //             clock.clk
		.reset                                    (reset),                                    //             reset.reset
		.aso_TigertoCache_data                    (aso_TigertoCache_data),                    //      TigertoCache.data
		.asi_CachetoTiger_data                    (asi_CachetoTiger_data),                    //      CachetoTiger.data
		.avm_instructionMaster_read               (avm_instructionMaster_read),               // instructionMaster.read
		.avm_instructionMaster_address            (avm_instructionMaster_address),            //                  .address
		.avm_instructionMaster_readdata           (avm_instructionMaster_readdata),           //                  .readdata
		.avm_instructionMaster_beginbursttransfer (avm_instructionMaster_beginbursttransfer), //                  .beginbursttransfer
		.avm_instructionMaster_burstcount         (avm_instructionMaster_burstcount),         //                  .burstcount
		.avm_instructionMaster_waitrequest        (avm_instructionMaster_waitrequest),        //                  .waitrequest
		.avm_instructionMaster_readdatavalid      (avm_instructionMaster_readdatavalid)       //                  .readdatavalid
	);

endmodule
