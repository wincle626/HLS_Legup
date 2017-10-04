//Legal Notice: (C)2010 Altera Corporation. All rights reserved.  Your
//use of Altera Corporation's design tools, logic functions and other
//software and tools, and its AMPP partner logic functions, and any
//output files any of the foregoing (including device programming or
//simulation files), and any associated documentation or information are
//expressly subject to the terms and conditions of the Altera Program
//License Subscription Agreement or other applicable license agreement,
//including, without limitation, that your use is for the sole purpose
//of programming logic devices manufactured by Altera and sold by Altera
//or its authorized distributors.  Please refer to the applicable
//agreement for further details.

///////////////////////////////////////////////////////////////////////////////
// Title         : DDR controller Write Data FIFO
//
// File          : alt_ddrx_wdata_fifo.v
//
// Abstract      : Store write data
///////////////////////////////////////////////////////////////////////////////

`timescale 1 ps / 1 ps
module alt_ddrx_wdata_fifo 
    #(parameter     WDATA_BEATS_WIDTH     = 9,
                    LOCAL_DATA_WIDTH      = 32, 
                    LOCAL_SIZE_WIDTH      = 6,
                    DWIDTH_RATIO          = 2,
                    FAMILY                = "Stratix",
					USE_BYTEENABLE        = 1
     )(
        // input
        ctl_clk, 								
        ctl_reset_n,  
        write_req_to_wfifo,
        wdata_to_wfifo,
        be_to_wfifo, 
        wdata_fifo_read,
        
        //output
        wdata_fifo_full,
        wdata_fifo_wdata,
        wdata_fifo_be,
        beats_in_wfifo
    );
    localparam   LOCAL_BE_WIDTH         = LOCAL_DATA_WIDTH/8;
	localparam   FIFO_NUMWORDS_WIDTH    = 8;
	localparam   FIFO_NUMWORDS          = 2**FIFO_NUMWORDS_WIDTH;
	localparam   FIFO_NUMWORDS_ALMOST_FULL = FIFO_NUMWORDS-16;
    
    input                		            ctl_clk; 	    // controller clock
    input                		            ctl_reset_n; 	// controller reset_n, synchronous to ctl_clk
    input                                   write_req_to_wfifo;
    input                                   wdata_fifo_read;
    input   [LOCAL_DATA_WIDTH-1 : 0]        wdata_to_wfifo;
    input   [LOCAL_BE_WIDTH-1 : 0]          be_to_wfifo;
    output  [LOCAL_DATA_WIDTH-1 : 0]        wdata_fifo_wdata;
    output  [LOCAL_BE_WIDTH-1 : 0]          wdata_fifo_be;
    output                                  wdata_fifo_full;
    output  [WDATA_BEATS_WIDTH-1 : 0]       beats_in_wfifo;
    
    wire                		            ctl_clk; 	
    wire                		            ctl_reset_n;
    wire                                    reset;
    wire                                    write_req_to_wfifo;
    wire                                    wdata_fifo_read;
    wire    [LOCAL_DATA_WIDTH-1 : 0]        wdata_to_wfifo;
    wire    [LOCAL_BE_WIDTH-1 : 0]          be_to_wfifo;
    wire                                    wdata_fifo_full;
    wire    [LOCAL_DATA_WIDTH-1 : 0]        wdata_fifo_wdata;
    wire    [LOCAL_BE_WIDTH-1 : 0]          wdata_fifo_be;
    reg     [WDATA_BEATS_WIDTH-1 : 0]       beats_in_wfifo;
    
    assign reset = !ctl_reset_n;            // scfifo has an active high async reset
    
    //We fix the fifo depth to 256 in order to match the depth of the M9k memories that has the maximum data width (256 depth x 36 width),                         
    //by doing this, we can minimize the usage of M9k.
    //Currently, we need at lease 18 M9k (256 x 36) 
    //Calculation : Maximum data width that we support, 72 (with ecc) * 8 (quarter rate) = 576, byteen bit = 576 / 8 = 72, LOCAL_WFIFO_Q_WIDTH = 576 + 72 = 648
    //Number of M9k we need = 648 / 36 = 18.
    
    scfifo #(
            .intended_device_family  (FAMILY),
            .lpm_width               (LOCAL_DATA_WIDTH),          // one BE per byte, not per DQS
            .lpm_numwords            (FIFO_NUMWORDS),              
            .lpm_widthu              (FIFO_NUMWORDS_WIDTH),      
            .almost_full_value       (FIFO_NUMWORDS_ALMOST_FULL),                       
            .lpm_type                ("scfifo"),
            .lpm_showahead           ("OFF"),                        // Always OFF at the moment   
            .overflow_checking       ("OFF"),
            .underflow_checking      ("OFF"),
            .use_eab                 ("ON"),
            .add_ram_output_register ("ON")                          // Always ON at the moment    
    ) wdata_fifo (
            .rdreq                   (wdata_fifo_read),                          
            .aclr                    (reset),
            .clock                   (ctl_clk),
            .wrreq                   (write_req_to_wfifo),
            .data                    (wdata_to_wfifo),
            .full                    (),
            .q                       (wdata_fifo_wdata),
            .sclr                    (1'b0),
            .usedw                   (),
            .empty                   (),           
            .almost_full             (wdata_fifo_full),
            .almost_empty            ()
   );

	generate
		if (USE_BYTEENABLE == 1) begin
			scfifo #(
					.intended_device_family  (FAMILY),
					.lpm_width               (LOCAL_BE_WIDTH),          // one BE per byte, not per DQS
					.lpm_numwords            (FIFO_NUMWORDS),              
					.lpm_widthu              (FIFO_NUMWORDS_WIDTH),      
					.lpm_type                ("scfifo"),
					.lpm_showahead           ("OFF"),                        // Always OFF at the moment   
					.overflow_checking       ("OFF"),
					.underflow_checking      ("OFF"),
					.use_eab                 ("ON"),
					.add_ram_output_register ("ON")                          // Always ON at the moment    
			) wdata_be_fifo (
					.rdreq                   (wdata_fifo_read),                          
					.aclr                    (reset),
					.clock                   (ctl_clk),
					.wrreq                   (write_req_to_wfifo),
					.data                    (be_to_wfifo),
					.full                    (),
					.q                       (wdata_fifo_be),
					.sclr                    (1'b0),
					.usedw                   (),
					.empty                   (),           
					.almost_full             (),
					.almost_empty            ()
		   );
		end
		else
			assign wdata_fifo_be = {LOCAL_BE_WIDTH{1'b1}};
	endgenerate
   
   // Tell the state machine how many data entry is in the write data fifo
   always @(posedge ctl_clk or negedge ctl_reset_n) begin     
        if (~ctl_reset_n) begin                               
           beats_in_wfifo <= 0; 
        end
        else if(write_req_to_wfifo) begin
             if(wdata_fifo_read) begin
                 beats_in_wfifo <= beats_in_wfifo;
             end
             else begin
                 beats_in_wfifo <= beats_in_wfifo + 1'b1;
             end
        end
        else if(wdata_fifo_read) begin
            beats_in_wfifo <= beats_in_wfifo - 1'b1;
        end
   end
        
        
endmodule
