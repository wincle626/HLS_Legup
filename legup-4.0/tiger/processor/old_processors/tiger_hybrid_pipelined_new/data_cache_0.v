// data_cache_0.v

// This file was auto-generated as part of a SOPC Builder generate operation.
// If you edit it your changes will probably be lost.

module data_cache_0 (
		input  wire         csi_clockreset_clk,                 //  clockreset.clk
		input  wire         csi_clockreset_reset_n,             //            .reset_n
		output wire [7:0]   aso_PROC_data,                      //        PROC.data
		input  wire         avs_CACHE0_begintransfer,           //      CACHE0.begintransfer
		input  wire         avs_CACHE0_read,                    //            .read
		input  wire         avs_CACHE0_write,                   //            .write
		input  wire [127:0] avs_CACHE0_writedata,               //            .writedata
		output wire [127:0] avs_CACHE0_readdata,                //            .readdata
		output wire         avs_CACHE0_waitrequest,             //            .waitrequest
		output wire         avm_dataMaster0_read,               // dataMaster0.read
		output wire         avm_dataMaster0_write,              //            .write
		output wire [31:0]  avm_dataMaster0_address,            //            .address
		output wire [31:0]  avm_dataMaster0_writedata,          //            .writedata
		output wire [3:0]   avm_dataMaster0_byteenable,         //            .byteenable
		input  wire [31:0]  avm_dataMaster0_readdata,           //            .readdata
		output wire         avm_dataMaster0_beginbursttransfer, //            .beginbursttransfer
		output wire [2:0]   avm_dataMaster0_burstcount,         //            .burstcount
		input  wire         avm_dataMaster0_waitrequest,        //            .waitrequest
		input  wire         avm_dataMaster0_readdatavalid       //            .readdatavalid
	);

	data_cache data_cache_0 (
		.csi_clockreset_clk                 (csi_clockreset_clk),                 //  clockreset.clk
		.csi_clockreset_reset_n             (csi_clockreset_reset_n),             //            .reset_n
		.aso_PROC_data                      (aso_PROC_data),                      //        PROC.data
		.avs_CACHE0_begintransfer           (avs_CACHE0_begintransfer),           //      CACHE0.begintransfer
		.avs_CACHE0_read                    (avs_CACHE0_read),                    //            .read
		.avs_CACHE0_write                   (avs_CACHE0_write),                   //            .write
		.avs_CACHE0_writedata               (avs_CACHE0_writedata),               //            .writedata
		.avs_CACHE0_readdata                (avs_CACHE0_readdata),                //            .readdata
		.avs_CACHE0_waitrequest             (avs_CACHE0_waitrequest),             //            .waitrequest
		.avm_dataMaster0_read               (avm_dataMaster0_read),               // dataMaster0.read
		.avm_dataMaster0_write              (avm_dataMaster0_write),              //            .write
		.avm_dataMaster0_address            (avm_dataMaster0_address),            //            .address
		.avm_dataMaster0_writedata          (avm_dataMaster0_writedata),          //            .writedata
		.avm_dataMaster0_byteenable         (avm_dataMaster0_byteenable),         //            .byteenable
		.avm_dataMaster0_readdata           (avm_dataMaster0_readdata),           //            .readdata
		.avm_dataMaster0_beginbursttransfer (avm_dataMaster0_beginbursttransfer), //            .beginbursttransfer
		.avm_dataMaster0_burstcount         (avm_dataMaster0_burstcount),         //            .burstcount
		.avm_dataMaster0_waitrequest        (avm_dataMaster0_waitrequest),        //            .waitrequest
		.avm_dataMaster0_readdatavalid      (avm_dataMaster0_readdatavalid)       //            .readdatavalid
	);

endmodule
