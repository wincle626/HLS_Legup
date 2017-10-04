// This top level module chooses between the original Altera-ST JTAG Interface
// component in ACDS version 8.1 and before, and the new one with the PLI 
// Simulation mode turned on, which adds a wrapper over the original component.

`timescale 1 ns / 1 ns

module altera_avalon_st_jtag_interface (
				 clk,
				 reset_n,
				 source_ready,
				 source_data,
				 source_valid,
				 sink_data,
				 sink_valid,
				 sink_ready,
				 resetrequest,
				 debug_reset,
				 mgmt_valid,
				 mgmt_channel,
				 mgmt_data
				 );
   input        clk;
   input        reset_n;
   output [7:0] source_data;
   input        source_ready;
   output       source_valid;
   input [7:0]  sink_data;
   input        sink_valid;
   output       sink_ready;
   output       resetrequest;
   output       debug_reset;
   output       mgmt_valid;
   output       mgmt_data;

   parameter PURPOSE = 0; // for discovery of services behind this JTAG Phy - 0
                          //   for JTAG Phy, 1 for Packets to Master
   parameter UPSTREAM_FIFO_SIZE = 0;
   parameter DOWNSTREAM_FIFO_SIZE = 0;
   parameter MGMT_CHANNEL_WIDTH = -1;
   parameter USE_PLI = 0; // set to 1 enable PLI Simulation Mode 
   parameter PLI_PORT = 50000; // PLI Simulation Port
   
   output [(MGMT_CHANNEL_WIDTH>0?MGMT_CHANNEL_WIDTH:1)-1:0] mgmt_channel;

   wire       clk;
   wire       resetrequest; 
   wire [7:0] source_data;
   wire       source_ready;
   wire       source_valid;
   wire [7:0] sink_data;
   wire       sink_valid;
   wire       sink_ready;

   generate 
     if (USE_PLI == 0)
       begin : normal
         altera_jtag_dc_streaming #(
           .PURPOSE(PURPOSE),
           .UPSTREAM_FIFO_SIZE(UPSTREAM_FIFO_SIZE),
           .DOWNSTREAM_FIFO_SIZE(DOWNSTREAM_FIFO_SIZE),
           .MGMT_CHANNEL_WIDTH(MGMT_CHANNEL_WIDTH)
         ) jtag_dc_streaming (
           .clk(clk),
           .reset_n(reset_n),
           .source_data(source_data),
           .source_valid(source_valid),
           .sink_data(sink_data),
           .sink_valid(sink_valid),
           .sink_ready(sink_ready),
           .resetrequest(resetrequest),
           .debug_reset(debug_reset),
           .mgmt_valid(mgmt_valid),
           .mgmt_channel(mgmt_channel),
           .mgmt_data(mgmt_data)
         );
       end
     else
       begin : pli_mode
         altera_pli_streaming #(.PURPOSE(PURPOSE), .PLI_PORT(PLI_PORT)) pli_streaming (
           .clk(clk),
           .reset_n(reset_n),
           .source_data(source_data),
           .source_valid(source_valid),
           .source_ready(source_ready),
           .sink_data(sink_data),
           .sink_valid(sink_valid),
           .sink_ready(sink_ready),
           .resetrequest(resetrequest)
         );
       end
   endgenerate
endmodule
