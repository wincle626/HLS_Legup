// tigers_jtag_uart_1.v

// This file was auto-generated as part of a SOPC Builder generate operation.
// If you edit it your changes will probably be lost.

module tigers_jtag_uart_1 (
		input  wire        clk,                        //       global_signals_clock.clk
		input  wire        reset_n,                    // global_signals_clock_reset.reset_n
		input  wire        avs_controlSlave_read,      //               controlSlave.read
		input  wire        avs_controlSlave_write,     //                           .write
		output wire [31:0] avs_controlSlave_readdata,  //                           .readdata
		input  wire [31:0] avs_controlSlave_writedata, //                           .writedata
		input  wire        avs_controlSlave_address    //                           .address
	);

	vJTAGUart tigers_jtag_uart_1 (
		.clk                        (clk),                        //       global_signals_clock.clk
		.reset_n                    (reset_n),                    // global_signals_clock_reset.reset_n
		.avs_controlSlave_read      (avs_controlSlave_read),      //               controlSlave.read
		.avs_controlSlave_write     (avs_controlSlave_write),     //                           .write
		.avs_controlSlave_readdata  (avs_controlSlave_readdata),  //                           .readdata
		.avs_controlSlave_writedata (avs_controlSlave_writedata), //                           .writedata
		.avs_controlSlave_address   (avs_controlSlave_address)    //                           .address
	);

endmodule
