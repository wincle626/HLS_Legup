// tiger_top_0.v

// This file was auto-generated as part of a SOPC Builder generate operation.
// If you edit it your changes will probably be lost.

module tiger_top_0 (
		input  wire        clk,                                 // global_signals_clock.clk
		input  wire        reset,                               //                     .reset
		output wire        avm_dataMaster_read,                 //           dataMaster.read
		output wire        avm_dataMaster_write,                //                     .write
		output wire [31:0] avm_dataMaster_address,              //                     .address
		output wire [31:0] avm_dataMaster_writedata,            //                     .writedata
		output wire [3:0]  avm_dataMaster_byteenable,           //                     .byteenable
		input  wire [31:0] avm_dataMaster_readdata,             //                     .readdata
		input  wire        avm_dataMaster_waitrequest,          //                     .waitrequest
		input  wire        avm_dataMaster_readdatavalid,        //                     .readdatavalid
		input  wire [5:0]  avm_dataMaster_irqnumber,            //                     .irqnumber
		input  wire        avm_dataMaster_irq,                  //       dataMaster_irq.irq
		output wire        avm_instructionMaster_read,          //    instructionMaster.read
		output wire [31:0] avm_instructionMaster_address,       //                     .address
		input  wire [31:0] avm_instructionMaster_readdata,      //                     .readdata
		input  wire        avm_instructionMaster_waitrequest,   //                     .waitrequest
		input  wire        avm_instructionMaster_readdatavalid, //                     .readdatavalid
		input  wire        avs_debugSlave_write,                //           debugSlave.write
		input  wire        avs_debugSlave_writedata,            //                     .writedata
		output wire        avs_debugSlave_irq                   //       debugSlave_irq.irq
	);

	tiger_top tiger_top_0 (
		.clk                                 (clk),                                 // global_signals_clock.clk
		.reset                               (reset),                               //                     .reset
		.avm_dataMaster_read                 (avm_dataMaster_read),                 //           dataMaster.read
		.avm_dataMaster_write                (avm_dataMaster_write),                //                     .write
		.avm_dataMaster_address              (avm_dataMaster_address),              //                     .address
		.avm_dataMaster_writedata            (avm_dataMaster_writedata),            //                     .writedata
		.avm_dataMaster_byteenable           (avm_dataMaster_byteenable),           //                     .byteenable
		.avm_dataMaster_readdata             (avm_dataMaster_readdata),             //                     .readdata
		.avm_dataMaster_waitrequest          (avm_dataMaster_waitrequest),          //                     .waitrequest
		.avm_dataMaster_readdatavalid        (avm_dataMaster_readdatavalid),        //                     .readdatavalid
		.avm_dataMaster_irqnumber            (avm_dataMaster_irqnumber),            //                     .irqnumber
		.avm_dataMaster_irq                  (avm_dataMaster_irq),                  //       dataMaster_irq.irq
		.avm_instructionMaster_read          (avm_instructionMaster_read),          //    instructionMaster.read
		.avm_instructionMaster_address       (avm_instructionMaster_address),       //                     .address
		.avm_instructionMaster_readdata      (avm_instructionMaster_readdata),      //                     .readdata
		.avm_instructionMaster_waitrequest   (avm_instructionMaster_waitrequest),   //                     .waitrequest
		.avm_instructionMaster_readdatavalid (avm_instructionMaster_readdatavalid), //                     .readdatavalid
		.avs_debugSlave_write                (avs_debugSlave_write),                //           debugSlave.write
		.avs_debugSlave_writedata            (avs_debugSlave_writedata),            //                     .writedata
		.avs_debugSlave_irq                  (avs_debugSlave_irq)                   //       debugSlave_irq.irq
	);

endmodule
