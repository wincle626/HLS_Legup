// data_cache_0.v

// This file was auto-generated as part of a SOPC Builder generate operation.
// If you edit it your changes will probably be lost.

module data_cache_0 (
		input  wire         csi_clockreset_clk,                 //       clockreset.clk
		input  wire         csi_clockreset_reset_n,             // clockreset_reset.reset_n
		input  wire [71:0]  asi_TigertoCache_data,              //     TigertoCache.data
		output wire [39:0]  aso_CachetoTiger_data,              //     CachetoTiger.data
		input  wire [2:0]   avs_ACCEL_address,                  //            ACCEL.address
		input  wire         avs_ACCEL_begintransfer,            //                 .begintransfer
		input  wire         avs_ACCEL_read,                     //                 .read
		input  wire         avs_ACCEL_write,                    //                 .write
		input  wire [127:0] avs_ACCEL_writedata,                //                 .writedata
		output wire [127:0] avs_ACCEL_readdata,                 //                 .readdata
		output wire         avs_ACCEL_waitrequest,              //                 .waitrequest
		output wire         avm_AccelMaster_read,               //      AccelMaster.read
		output wire         avm_AccelMaster_write,              //                 .write
		output wire [31:0]  avm_AccelMaster_address,            //                 .address
		output wire [31:0]  avm_AccelMaster_writedata,          //                 .writedata
		output wire [3:0]   avm_AccelMaster_byteenable,         //                 .byteenable
		input  wire [31:0]  avm_AccelMaster_readdata,           //                 .readdata
		output wire         avm_AccelMaster_beginbursttransfer, //                 .beginbursttransfer
		output wire [2:0]   avm_AccelMaster_burstcount,         //                 .burstcount
		input  wire         avm_AccelMaster_waitrequest,        //                 .waitrequest
		input  wire         avm_AccelMaster_readdatavalid,      //                 .readdatavalid
		output wire         avm_dataMaster_read,                //       dataMaster.read
		output wire         avm_dataMaster_write,               //                 .write
		output wire [31:0]  avm_dataMaster_address,             //                 .address
		output wire [31:0]  avm_dataMaster_writedata,           //                 .writedata
		output wire [3:0]   avm_dataMaster_byteenable,          //                 .byteenable
		input  wire [31:0]  avm_dataMaster_readdata,            //                 .readdata
		output wire         avm_dataMaster_beginbursttransfer,  //                 .beginbursttransfer
		output wire [2:0]   avm_dataMaster_burstcount,          //                 .burstcount
		input  wire         avm_dataMaster_waitrequest,         //                 .waitrequest
		input  wire         avm_dataMaster_readdatavalid        //                 .readdatavalid
	);

	data_cache #(
		.stateIDLE         (0),
		.stateREAD         (1),
		.stateFETCH        (2),
		.stateWRITE        (3),
		.stateAVALON_READ  (4),
		.stateAVALON_WRITE (5),
		.stateFLUSH        (6),
		.stateHOLD         (7),
		.blockSize         (4),
		.blockSizeBits     (128),
		.cacheSize         (9),
		.burstCount        (4)
	) data_cache_0 (
		.csi_clockreset_clk                 (csi_clockreset_clk),                 //       clockreset.clk
		.csi_clockreset_reset_n             (csi_clockreset_reset_n),             // clockreset_reset.reset_n
		.asi_TigertoCache_data              (asi_TigertoCache_data),              //     TigertoCache.data
		.aso_CachetoTiger_data              (aso_CachetoTiger_data),              //     CachetoTiger.data
		.avs_ACCEL_address                  (avs_ACCEL_address),                  //            ACCEL.address
		.avs_ACCEL_begintransfer            (avs_ACCEL_begintransfer),            //                 .begintransfer
		.avs_ACCEL_read                     (avs_ACCEL_read),                     //                 .read
		.avs_ACCEL_write                    (avs_ACCEL_write),                    //                 .write
		.avs_ACCEL_writedata                (avs_ACCEL_writedata),                //                 .writedata
		.avs_ACCEL_readdata                 (avs_ACCEL_readdata),                 //                 .readdata
		.avs_ACCEL_waitrequest              (avs_ACCEL_waitrequest),              //                 .waitrequest
		.avm_AccelMaster_read               (avm_AccelMaster_read),               //      AccelMaster.read
		.avm_AccelMaster_write              (avm_AccelMaster_write),              //                 .write
		.avm_AccelMaster_address            (avm_AccelMaster_address),            //                 .address
		.avm_AccelMaster_writedata          (avm_AccelMaster_writedata),          //                 .writedata
		.avm_AccelMaster_byteenable         (avm_AccelMaster_byteenable),         //                 .byteenable
		.avm_AccelMaster_readdata           (avm_AccelMaster_readdata),           //                 .readdata
		.avm_AccelMaster_beginbursttransfer (avm_AccelMaster_beginbursttransfer), //                 .beginbursttransfer
		.avm_AccelMaster_burstcount         (avm_AccelMaster_burstcount),         //                 .burstcount
		.avm_AccelMaster_waitrequest        (avm_AccelMaster_waitrequest),        //                 .waitrequest
		.avm_AccelMaster_readdatavalid      (avm_AccelMaster_readdatavalid),      //                 .readdatavalid
		.avm_dataMaster_read                (avm_dataMaster_read),                //       dataMaster.read
		.avm_dataMaster_write               (avm_dataMaster_write),               //                 .write
		.avm_dataMaster_address             (avm_dataMaster_address),             //                 .address
		.avm_dataMaster_writedata           (avm_dataMaster_writedata),           //                 .writedata
		.avm_dataMaster_byteenable          (avm_dataMaster_byteenable),          //                 .byteenable
		.avm_dataMaster_readdata            (avm_dataMaster_readdata),            //                 .readdata
		.avm_dataMaster_beginbursttransfer  (avm_dataMaster_beginbursttransfer),  //                 .beginbursttransfer
		.avm_dataMaster_burstcount          (avm_dataMaster_burstcount),          //                 .burstcount
		.avm_dataMaster_waitrequest         (avm_dataMaster_waitrequest),         //                 .waitrequest
		.avm_dataMaster_readdatavalid       (avm_dataMaster_readdatavalid)        //                 .readdatavalid
	);

endmodule
