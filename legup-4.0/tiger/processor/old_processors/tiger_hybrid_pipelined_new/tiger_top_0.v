// tiger_top_0.v

// This file was auto-generated as part of a SOPC Builder generate operation.
// If you edit it your changes will probably be lost.

module tiger_top_0 (
		input  wire         clk,                                      //             clock.clk
		input  wire         reset,                                    //             reset.reset
		output wire [31:0]  avm_CACHE_address,                        //             CACHE.address
		output wire         avm_CACHE_read,                           //                  .read
		output wire         avm_CACHE_write,                          //                  .write
		output wire [127:0] avm_CACHE_writedata,                      //                  .writedata
		input  wire [127:0] avm_CACHE_readdata,                       //                  .readdata
		input  wire         avm_CACHE_waitrequest,                    //                  .waitrequest
		input  wire [7:0]   asi_PROC_data,                            //              PROC.data
		output wire [31:0]  avm_procMaster_address,                   //        procMaster.address
		output wire         avm_procMaster_read,                      //                  .read
		output wire         avm_procMaster_write,                     //                  .write
		output wire [31:0]  avm_procMaster_writedata,                 //                  .writedata
		output wire [3:0]   avm_procMaster_byteenable,                //                  .byteenable
		input  wire [31:0]  avm_procMaster_readdata,                  //                  .readdata
		input  wire         avm_procMaster_waitrequest,               //                  .waitrequest
		input  wire         avm_procMaster_readdatavalid,             //                  .readdatavalid
		output wire         avm_instructionMaster_read,               // instructionMaster.read
		output wire [31:0]  avm_instructionMaster_address,            //                  .address
		input  wire [31:0]  avm_instructionMaster_readdata,           //                  .readdata
		output wire         avm_instructionMaster_beginbursttransfer, //                  .beginbursttransfer
		output wire [2:0]   avm_instructionMaster_burstcount,         //                  .burstcount
		input  wire         avm_instructionMaster_waitrequest,        //                  .waitrequest
		input  wire         avm_instructionMaster_readdatavalid       //                  .readdatavalid
	);

	tiger_top tiger_top_0 (
		.clk                                      (clk),                                      //             clock.clk
		.reset                                    (reset),                                    //             reset.reset
		.avm_CACHE_address                        (avm_CACHE_address),                        //             CACHE.address
		.avm_CACHE_read                           (avm_CACHE_read),                           //                  .read
		.avm_CACHE_write                          (avm_CACHE_write),                          //                  .write
		.avm_CACHE_writedata                      (avm_CACHE_writedata),                      //                  .writedata
		.avm_CACHE_readdata                       (avm_CACHE_readdata),                       //                  .readdata
		.avm_CACHE_waitrequest                    (avm_CACHE_waitrequest),                    //                  .waitrequest
		.asi_PROC_data                            (asi_PROC_data),                            //              PROC.data
		.avm_procMaster_address                   (avm_procMaster_address),                   //        procMaster.address
		.avm_procMaster_read                      (avm_procMaster_read),                      //                  .read
		.avm_procMaster_write                     (avm_procMaster_write),                     //                  .write
		.avm_procMaster_writedata                 (avm_procMaster_writedata),                 //                  .writedata
		.avm_procMaster_byteenable                (avm_procMaster_byteenable),                //                  .byteenable
		.avm_procMaster_readdata                  (avm_procMaster_readdata),                  //                  .readdata
		.avm_procMaster_waitrequest               (avm_procMaster_waitrequest),               //                  .waitrequest
		.avm_procMaster_readdatavalid             (avm_procMaster_readdatavalid),             //                  .readdatavalid
		.avm_instructionMaster_read               (avm_instructionMaster_read),               // instructionMaster.read
		.avm_instructionMaster_address            (avm_instructionMaster_address),            //                  .address
		.avm_instructionMaster_readdata           (avm_instructionMaster_readdata),           //                  .readdata
		.avm_instructionMaster_beginbursttransfer (avm_instructionMaster_beginbursttransfer), //                  .beginbursttransfer
		.avm_instructionMaster_burstcount         (avm_instructionMaster_burstcount),         //                  .burstcount
		.avm_instructionMaster_waitrequest        (avm_instructionMaster_waitrequest),        //                  .waitrequest
		.avm_instructionMaster_readdatavalid      (avm_instructionMaster_readdatavalid)       //                  .readdatavalid
	);

endmodule
