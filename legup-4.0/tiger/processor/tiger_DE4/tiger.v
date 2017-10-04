//megafunction wizard: %Altera SOPC Builder%
//GENERATION: STANDARD
//VERSION: WM1.0


//Legal Notice: (C)2013 Altera Corporation. All rights reserved.  Your
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

// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module data_cache_0_CACHE0_arbitrator (
                                        // inputs:
                                         clk,
                                         data_cache_0_CACHE0_readdata,
                                         data_cache_0_CACHE0_waitrequest,
                                         reset_n,
                                         tiger_top_0_CACHE_address_to_slave,
                                         tiger_top_0_CACHE_read,
                                         tiger_top_0_CACHE_write,
                                         tiger_top_0_CACHE_writedata,

                                        // outputs:
                                         d1_data_cache_0_CACHE0_end_xfer,
                                         data_cache_0_CACHE0_begintransfer,
                                         data_cache_0_CACHE0_read,
                                         data_cache_0_CACHE0_readdata_from_sa,
                                         data_cache_0_CACHE0_waitrequest_from_sa,
                                         data_cache_0_CACHE0_write,
                                         data_cache_0_CACHE0_writedata,
                                         tiger_top_0_CACHE_granted_data_cache_0_CACHE0,
                                         tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0,
                                         tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0,
                                         tiger_top_0_CACHE_requests_data_cache_0_CACHE0
                                      )
;

  output           d1_data_cache_0_CACHE0_end_xfer;
  output           data_cache_0_CACHE0_begintransfer;
  output           data_cache_0_CACHE0_read;
  output  [127: 0] data_cache_0_CACHE0_readdata_from_sa;
  output           data_cache_0_CACHE0_waitrequest_from_sa;
  output           data_cache_0_CACHE0_write;
  output  [127: 0] data_cache_0_CACHE0_writedata;
  output           tiger_top_0_CACHE_granted_data_cache_0_CACHE0;
  output           tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0;
  output           tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0;
  output           tiger_top_0_CACHE_requests_data_cache_0_CACHE0;
  input            clk;
  input   [127: 0] data_cache_0_CACHE0_readdata;
  input            data_cache_0_CACHE0_waitrequest;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_CACHE_address_to_slave;
  input            tiger_top_0_CACHE_read;
  input            tiger_top_0_CACHE_write;
  input   [127: 0] tiger_top_0_CACHE_writedata;

  reg              d1_data_cache_0_CACHE0_end_xfer;
  reg              d1_reasons_to_wait;
  wire             data_cache_0_CACHE0_allgrants;
  wire             data_cache_0_CACHE0_allow_new_arb_cycle;
  wire             data_cache_0_CACHE0_any_bursting_master_saved_grant;
  wire             data_cache_0_CACHE0_any_continuerequest;
  wire             data_cache_0_CACHE0_arb_counter_enable;
  reg              data_cache_0_CACHE0_arb_share_counter;
  wire             data_cache_0_CACHE0_arb_share_counter_next_value;
  wire             data_cache_0_CACHE0_arb_share_set_values;
  wire             data_cache_0_CACHE0_beginbursttransfer_internal;
  wire             data_cache_0_CACHE0_begins_xfer;
  wire             data_cache_0_CACHE0_begintransfer;
  wire             data_cache_0_CACHE0_end_xfer;
  wire             data_cache_0_CACHE0_firsttransfer;
  wire             data_cache_0_CACHE0_grant_vector;
  wire             data_cache_0_CACHE0_in_a_read_cycle;
  wire             data_cache_0_CACHE0_in_a_write_cycle;
  wire             data_cache_0_CACHE0_master_qreq_vector;
  wire             data_cache_0_CACHE0_non_bursting_master_requests;
  wire             data_cache_0_CACHE0_read;
  wire    [127: 0] data_cache_0_CACHE0_readdata_from_sa;
  reg              data_cache_0_CACHE0_reg_firsttransfer;
  reg              data_cache_0_CACHE0_slavearbiterlockenable;
  wire             data_cache_0_CACHE0_slavearbiterlockenable2;
  wire             data_cache_0_CACHE0_unreg_firsttransfer;
  wire             data_cache_0_CACHE0_waitrequest_from_sa;
  wire             data_cache_0_CACHE0_waits_for_read;
  wire             data_cache_0_CACHE0_waits_for_write;
  wire             data_cache_0_CACHE0_write;
  wire    [127: 0] data_cache_0_CACHE0_writedata;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_data_cache_0_CACHE0;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire    [ 31: 0] shifted_address_to_data_cache_0_CACHE0_from_tiger_top_0_CACHE;
  wire             tiger_top_0_CACHE_arbiterlock;
  wire             tiger_top_0_CACHE_arbiterlock2;
  wire             tiger_top_0_CACHE_continuerequest;
  wire             tiger_top_0_CACHE_granted_data_cache_0_CACHE0;
  wire             tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0;
  wire             tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0;
  wire             tiger_top_0_CACHE_requests_data_cache_0_CACHE0;
  wire             tiger_top_0_CACHE_saved_grant_data_cache_0_CACHE0;
  wire             wait_for_data_cache_0_CACHE0_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~data_cache_0_CACHE0_end_xfer;
    end


  assign data_cache_0_CACHE0_begins_xfer = ~d1_reasons_to_wait & ((tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0));
  //assign data_cache_0_CACHE0_readdata_from_sa = data_cache_0_CACHE0_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign data_cache_0_CACHE0_readdata_from_sa = data_cache_0_CACHE0_readdata;

  assign tiger_top_0_CACHE_requests_data_cache_0_CACHE0 = ({tiger_top_0_CACHE_address_to_slave[31 : 4] , 4'b0} == 32'h0) & (tiger_top_0_CACHE_read | tiger_top_0_CACHE_write);
  //assign data_cache_0_CACHE0_waitrequest_from_sa = data_cache_0_CACHE0_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign data_cache_0_CACHE0_waitrequest_from_sa = data_cache_0_CACHE0_waitrequest;

  //data_cache_0_CACHE0_arb_share_counter set values, which is an e_mux
  assign data_cache_0_CACHE0_arb_share_set_values = 1;

  //data_cache_0_CACHE0_non_bursting_master_requests mux, which is an e_mux
  assign data_cache_0_CACHE0_non_bursting_master_requests = tiger_top_0_CACHE_requests_data_cache_0_CACHE0;

  //data_cache_0_CACHE0_any_bursting_master_saved_grant mux, which is an e_mux
  assign data_cache_0_CACHE0_any_bursting_master_saved_grant = 0;

  //data_cache_0_CACHE0_arb_share_counter_next_value assignment, which is an e_assign
  assign data_cache_0_CACHE0_arb_share_counter_next_value = data_cache_0_CACHE0_firsttransfer ? (data_cache_0_CACHE0_arb_share_set_values - 1) : |data_cache_0_CACHE0_arb_share_counter ? (data_cache_0_CACHE0_arb_share_counter - 1) : 0;

  //data_cache_0_CACHE0_allgrants all slave grants, which is an e_mux
  assign data_cache_0_CACHE0_allgrants = |data_cache_0_CACHE0_grant_vector;

  //data_cache_0_CACHE0_end_xfer assignment, which is an e_assign
  assign data_cache_0_CACHE0_end_xfer = ~(data_cache_0_CACHE0_waits_for_read | data_cache_0_CACHE0_waits_for_write);

  //end_xfer_arb_share_counter_term_data_cache_0_CACHE0 arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_data_cache_0_CACHE0 = data_cache_0_CACHE0_end_xfer & (~data_cache_0_CACHE0_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //data_cache_0_CACHE0_arb_share_counter arbitration counter enable, which is an e_assign
  assign data_cache_0_CACHE0_arb_counter_enable = (end_xfer_arb_share_counter_term_data_cache_0_CACHE0 & data_cache_0_CACHE0_allgrants) | (end_xfer_arb_share_counter_term_data_cache_0_CACHE0 & ~data_cache_0_CACHE0_non_bursting_master_requests);

  //data_cache_0_CACHE0_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_CACHE0_arb_share_counter <= 0;
      else if (data_cache_0_CACHE0_arb_counter_enable)
          data_cache_0_CACHE0_arb_share_counter <= data_cache_0_CACHE0_arb_share_counter_next_value;
    end


  //data_cache_0_CACHE0_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_CACHE0_slavearbiterlockenable <= 0;
      else if ((|data_cache_0_CACHE0_master_qreq_vector & end_xfer_arb_share_counter_term_data_cache_0_CACHE0) | (end_xfer_arb_share_counter_term_data_cache_0_CACHE0 & ~data_cache_0_CACHE0_non_bursting_master_requests))
          data_cache_0_CACHE0_slavearbiterlockenable <= |data_cache_0_CACHE0_arb_share_counter_next_value;
    end


  //tiger_top_0/CACHE data_cache_0/CACHE0 arbiterlock, which is an e_assign
  assign tiger_top_0_CACHE_arbiterlock = data_cache_0_CACHE0_slavearbiterlockenable & tiger_top_0_CACHE_continuerequest;

  //data_cache_0_CACHE0_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign data_cache_0_CACHE0_slavearbiterlockenable2 = |data_cache_0_CACHE0_arb_share_counter_next_value;

  //tiger_top_0/CACHE data_cache_0/CACHE0 arbiterlock2, which is an e_assign
  assign tiger_top_0_CACHE_arbiterlock2 = data_cache_0_CACHE0_slavearbiterlockenable2 & tiger_top_0_CACHE_continuerequest;

  //data_cache_0_CACHE0_any_continuerequest at least one master continues requesting, which is an e_assign
  assign data_cache_0_CACHE0_any_continuerequest = 1;

  //tiger_top_0_CACHE_continuerequest continued request, which is an e_assign
  assign tiger_top_0_CACHE_continuerequest = 1;

  assign tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0 = tiger_top_0_CACHE_requests_data_cache_0_CACHE0;
  //data_cache_0_CACHE0_writedata mux, which is an e_mux
  assign data_cache_0_CACHE0_writedata = tiger_top_0_CACHE_writedata;

  //master is always granted when requested
  assign tiger_top_0_CACHE_granted_data_cache_0_CACHE0 = tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0;

  //tiger_top_0/CACHE saved-grant data_cache_0/CACHE0, which is an e_assign
  assign tiger_top_0_CACHE_saved_grant_data_cache_0_CACHE0 = tiger_top_0_CACHE_requests_data_cache_0_CACHE0;

  //allow new arb cycle for data_cache_0/CACHE0, which is an e_assign
  assign data_cache_0_CACHE0_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign data_cache_0_CACHE0_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign data_cache_0_CACHE0_master_qreq_vector = 1;

  assign data_cache_0_CACHE0_begintransfer = data_cache_0_CACHE0_begins_xfer;
  //data_cache_0_CACHE0_firsttransfer first transaction, which is an e_assign
  assign data_cache_0_CACHE0_firsttransfer = data_cache_0_CACHE0_begins_xfer ? data_cache_0_CACHE0_unreg_firsttransfer : data_cache_0_CACHE0_reg_firsttransfer;

  //data_cache_0_CACHE0_unreg_firsttransfer first transaction, which is an e_assign
  assign data_cache_0_CACHE0_unreg_firsttransfer = ~(data_cache_0_CACHE0_slavearbiterlockenable & data_cache_0_CACHE0_any_continuerequest);

  //data_cache_0_CACHE0_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_CACHE0_reg_firsttransfer <= 1'b1;
      else if (data_cache_0_CACHE0_begins_xfer)
          data_cache_0_CACHE0_reg_firsttransfer <= data_cache_0_CACHE0_unreg_firsttransfer;
    end


  //data_cache_0_CACHE0_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign data_cache_0_CACHE0_beginbursttransfer_internal = data_cache_0_CACHE0_begins_xfer;

  //data_cache_0_CACHE0_read assignment, which is an e_mux
  assign data_cache_0_CACHE0_read = tiger_top_0_CACHE_granted_data_cache_0_CACHE0 & tiger_top_0_CACHE_read;

  //data_cache_0_CACHE0_write assignment, which is an e_mux
  assign data_cache_0_CACHE0_write = tiger_top_0_CACHE_granted_data_cache_0_CACHE0 & tiger_top_0_CACHE_write;

  assign shifted_address_to_data_cache_0_CACHE0_from_tiger_top_0_CACHE = tiger_top_0_CACHE_address_to_slave;
  //d1_data_cache_0_CACHE0_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_data_cache_0_CACHE0_end_xfer <= 1;
      else 
        d1_data_cache_0_CACHE0_end_xfer <= data_cache_0_CACHE0_end_xfer;
    end


  //data_cache_0_CACHE0_waits_for_read in a cycle, which is an e_mux
  assign data_cache_0_CACHE0_waits_for_read = data_cache_0_CACHE0_in_a_read_cycle & data_cache_0_CACHE0_waitrequest_from_sa;

  //data_cache_0_CACHE0_in_a_read_cycle assignment, which is an e_assign
  assign data_cache_0_CACHE0_in_a_read_cycle = tiger_top_0_CACHE_granted_data_cache_0_CACHE0 & tiger_top_0_CACHE_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = data_cache_0_CACHE0_in_a_read_cycle;

  //data_cache_0_CACHE0_waits_for_write in a cycle, which is an e_mux
  assign data_cache_0_CACHE0_waits_for_write = data_cache_0_CACHE0_in_a_write_cycle & data_cache_0_CACHE0_waitrequest_from_sa;

  //data_cache_0_CACHE0_in_a_write_cycle assignment, which is an e_assign
  assign data_cache_0_CACHE0_in_a_write_cycle = tiger_top_0_CACHE_granted_data_cache_0_CACHE0 & tiger_top_0_CACHE_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = data_cache_0_CACHE0_in_a_write_cycle;

  assign wait_for_data_cache_0_CACHE0_counter = 0;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //data_cache_0/CACHE0 enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module data_cache_0_PROC_arbitrator (
                                      // inputs:
                                       clk,
                                       data_cache_0_PROC_data,
                                       reset_n,

                                      // outputs:
                                       data_cache_0_PROC_reset_n
                                    )
;

  output           data_cache_0_PROC_reset_n;
  input            clk;
  input   [  7: 0] data_cache_0_PROC_data;
  input            reset_n;

  wire             data_cache_0_PROC_reset_n;
  //data_cache_0_PROC_reset_n assignment, which is an e_assign
  assign data_cache_0_PROC_reset_n = reset_n;


endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module data_cache_0_dataMaster0_arbitrator (
                                             // inputs:
                                              clk,
                                              d1_pipeline_bridge_MEMORY_s1_end_xfer,
                                              data_cache_0_dataMaster0_address,
                                              data_cache_0_dataMaster0_burstcount,
                                              data_cache_0_dataMaster0_byteenable,
                                              data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_dataMaster0_read,
                                              data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                              data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_dataMaster0_write,
                                              data_cache_0_dataMaster0_writedata,
                                              pipeline_bridge_MEMORY_s1_readdata_from_sa,
                                              pipeline_bridge_MEMORY_s1_waitrequest_from_sa,
                                              reset_n,

                                             // outputs:
                                              data_cache_0_dataMaster0_address_to_slave,
                                              data_cache_0_dataMaster0_latency_counter,
                                              data_cache_0_dataMaster0_readdata,
                                              data_cache_0_dataMaster0_readdatavalid,
                                              data_cache_0_dataMaster0_waitrequest
                                           )
;

  output  [ 31: 0] data_cache_0_dataMaster0_address_to_slave;
  output           data_cache_0_dataMaster0_latency_counter;
  output  [255: 0] data_cache_0_dataMaster0_readdata;
  output           data_cache_0_dataMaster0_readdatavalid;
  output           data_cache_0_dataMaster0_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_MEMORY_s1_end_xfer;
  input   [ 31: 0] data_cache_0_dataMaster0_address;
  input   [  5: 0] data_cache_0_dataMaster0_burstcount;
  input   [ 31: 0] data_cache_0_dataMaster0_byteenable;
  input            data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_read;
  input            data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  input            data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_write;
  input   [255: 0] data_cache_0_dataMaster0_writedata;
  input   [255: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  input            pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  input            reset_n;

  reg              active_and_waiting_last_time;
  reg     [ 31: 0] data_cache_0_dataMaster0_address_last_time;
  wire    [ 31: 0] data_cache_0_dataMaster0_address_to_slave;
  reg     [  5: 0] data_cache_0_dataMaster0_burstcount_last_time;
  reg     [ 31: 0] data_cache_0_dataMaster0_byteenable_last_time;
  wire             data_cache_0_dataMaster0_is_granted_some_slave;
  reg              data_cache_0_dataMaster0_latency_counter;
  reg              data_cache_0_dataMaster0_read_but_no_slave_selected;
  reg              data_cache_0_dataMaster0_read_last_time;
  wire    [255: 0] data_cache_0_dataMaster0_readdata;
  wire             data_cache_0_dataMaster0_readdatavalid;
  wire             data_cache_0_dataMaster0_run;
  wire             data_cache_0_dataMaster0_waitrequest;
  reg              data_cache_0_dataMaster0_write_last_time;
  reg     [255: 0] data_cache_0_dataMaster0_writedata_last_time;
  wire             latency_load_value;
  wire             p1_data_cache_0_dataMaster0_latency_counter;
  wire             pre_flush_data_cache_0_dataMaster0_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 | ~data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1) & (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 | ~data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1) & ((~data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write)))) & ((~data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write))));

  //cascaded wait assignment, which is an e_assign
  assign data_cache_0_dataMaster0_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign data_cache_0_dataMaster0_address_to_slave = {1'b0,
    data_cache_0_dataMaster0_address[30 : 0]};

  //data_cache_0_dataMaster0_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_read_but_no_slave_selected <= 0;
      else 
        data_cache_0_dataMaster0_read_but_no_slave_selected <= data_cache_0_dataMaster0_read & data_cache_0_dataMaster0_run & ~data_cache_0_dataMaster0_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign data_cache_0_dataMaster0_is_granted_some_slave = data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_data_cache_0_dataMaster0_readdatavalid = data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign data_cache_0_dataMaster0_readdatavalid = data_cache_0_dataMaster0_read_but_no_slave_selected |
    pre_flush_data_cache_0_dataMaster0_readdatavalid;

  //data_cache_0/dataMaster0 readdata mux, which is an e_mux
  assign data_cache_0_dataMaster0_readdata = pipeline_bridge_MEMORY_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign data_cache_0_dataMaster0_waitrequest = ~data_cache_0_dataMaster0_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_latency_counter <= 0;
      else 
        data_cache_0_dataMaster0_latency_counter <= p1_data_cache_0_dataMaster0_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_data_cache_0_dataMaster0_latency_counter = ((data_cache_0_dataMaster0_run & data_cache_0_dataMaster0_read))? latency_load_value :
    (data_cache_0_dataMaster0_latency_counter)? data_cache_0_dataMaster0_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //data_cache_0_dataMaster0_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_address_last_time <= 0;
      else 
        data_cache_0_dataMaster0_address_last_time <= data_cache_0_dataMaster0_address;
    end


  //data_cache_0/dataMaster0 waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= data_cache_0_dataMaster0_waitrequest & (data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write);
    end


  //data_cache_0_dataMaster0_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster0_address != data_cache_0_dataMaster0_address_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster0_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster0_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_burstcount_last_time <= 0;
      else 
        data_cache_0_dataMaster0_burstcount_last_time <= data_cache_0_dataMaster0_burstcount;
    end


  //data_cache_0_dataMaster0_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster0_burstcount != data_cache_0_dataMaster0_burstcount_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster0_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster0_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_byteenable_last_time <= 0;
      else 
        data_cache_0_dataMaster0_byteenable_last_time <= data_cache_0_dataMaster0_byteenable;
    end


  //data_cache_0_dataMaster0_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster0_byteenable != data_cache_0_dataMaster0_byteenable_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster0_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster0_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_read_last_time <= 0;
      else 
        data_cache_0_dataMaster0_read_last_time <= data_cache_0_dataMaster0_read;
    end


  //data_cache_0_dataMaster0_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster0_read != data_cache_0_dataMaster0_read_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster0_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster0_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_write_last_time <= 0;
      else 
        data_cache_0_dataMaster0_write_last_time <= data_cache_0_dataMaster0_write;
    end


  //data_cache_0_dataMaster0_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster0_write != data_cache_0_dataMaster0_write_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster0_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster0_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster0_writedata_last_time <= 0;
      else 
        data_cache_0_dataMaster0_writedata_last_time <= data_cache_0_dataMaster0_writedata;
    end


  //data_cache_0_dataMaster0_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster0_writedata != data_cache_0_dataMaster0_writedata_last_time) & data_cache_0_dataMaster0_write)
        begin
          $write("%0d ns: data_cache_0_dataMaster0_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_jtag_to_ava_master_bridge_master_to_ddr2_s1_module (
                                                                         // inputs:
                                                                          clear_fifo,
                                                                          clk,
                                                                          data_in,
                                                                          read,
                                                                          reset_n,
                                                                          sync_reset,
                                                                          write,

                                                                         // outputs:
                                                                          data_out,
                                                                          empty,
                                                                          fifo_contains_ones_n,
                                                                          full
                                                                       )
;

  output           data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input            data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire             data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_10;
  reg              full_11;
  reg              full_12;
  reg              full_13;
  reg              full_14;
  reg              full_15;
  reg              full_16;
  reg              full_17;
  reg              full_18;
  reg              full_19;
  reg              full_2;
  reg              full_20;
  reg              full_21;
  reg              full_22;
  reg              full_23;
  reg              full_24;
  reg              full_25;
  reg              full_26;
  reg              full_27;
  reg              full_28;
  reg              full_29;
  reg              full_3;
  reg              full_30;
  reg              full_31;
  wire             full_32;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  6: 0] how_many_ones;
  wire    [  6: 0] one_count_minus_one;
  wire    [  6: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p10_full_10;
  wire             p10_stage_10;
  wire             p11_full_11;
  wire             p11_stage_11;
  wire             p12_full_12;
  wire             p12_stage_12;
  wire             p13_full_13;
  wire             p13_stage_13;
  wire             p14_full_14;
  wire             p14_stage_14;
  wire             p15_full_15;
  wire             p15_stage_15;
  wire             p16_full_16;
  wire             p16_stage_16;
  wire             p17_full_17;
  wire             p17_stage_17;
  wire             p18_full_18;
  wire             p18_stage_18;
  wire             p19_full_19;
  wire             p19_stage_19;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p20_full_20;
  wire             p20_stage_20;
  wire             p21_full_21;
  wire             p21_stage_21;
  wire             p22_full_22;
  wire             p22_stage_22;
  wire             p23_full_23;
  wire             p23_stage_23;
  wire             p24_full_24;
  wire             p24_stage_24;
  wire             p25_full_25;
  wire             p25_stage_25;
  wire             p26_full_26;
  wire             p26_stage_26;
  wire             p27_full_27;
  wire             p27_stage_27;
  wire             p28_full_28;
  wire             p28_stage_28;
  wire             p29_full_29;
  wire             p29_stage_29;
  wire             p2_full_2;
  wire             p2_stage_2;
  wire             p30_full_30;
  wire             p30_stage_30;
  wire             p31_full_31;
  wire             p31_stage_31;
  wire             p3_full_3;
  wire             p3_stage_3;
  wire             p4_full_4;
  wire             p4_stage_4;
  wire             p5_full_5;
  wire             p5_stage_5;
  wire             p6_full_6;
  wire             p6_stage_6;
  wire             p7_full_7;
  wire             p7_stage_7;
  wire             p8_full_8;
  wire             p8_stage_8;
  wire             p9_full_9;
  wire             p9_stage_9;
  reg              stage_0;
  reg              stage_1;
  reg              stage_10;
  reg              stage_11;
  reg              stage_12;
  reg              stage_13;
  reg              stage_14;
  reg              stage_15;
  reg              stage_16;
  reg              stage_17;
  reg              stage_18;
  reg              stage_19;
  reg              stage_2;
  reg              stage_20;
  reg              stage_21;
  reg              stage_22;
  reg              stage_23;
  reg              stage_24;
  reg              stage_25;
  reg              stage_26;
  reg              stage_27;
  reg              stage_28;
  reg              stage_29;
  reg              stage_3;
  reg              stage_30;
  reg              stage_31;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  reg              stage_7;
  reg              stage_8;
  reg              stage_9;
  wire    [  6: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_31;
  assign empty = !full_0;
  assign full_32 = 0;
  //data_31, which is an e_mux
  assign p31_stage_31 = ((full_32 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_31 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_31))
          if (sync_reset & full_31 & !((full_32 == 0) & read & write))
              stage_31 <= 0;
          else 
            stage_31 <= p31_stage_31;
    end


  //control_31, which is an e_mux
  assign p31_full_31 = ((read & !write) == 0)? full_30 :
    0;

  //control_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_31 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_31 <= 0;
          else 
            full_31 <= p31_full_31;
    end


  //data_30, which is an e_mux
  assign p30_stage_30 = ((full_31 & ~clear_fifo) == 0)? data_in :
    stage_31;

  //data_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_30 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_30))
          if (sync_reset & full_30 & !((full_31 == 0) & read & write))
              stage_30 <= 0;
          else 
            stage_30 <= p30_stage_30;
    end


  //control_30, which is an e_mux
  assign p30_full_30 = ((read & !write) == 0)? full_29 :
    full_31;

  //control_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_30 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_30 <= 0;
          else 
            full_30 <= p30_full_30;
    end


  //data_29, which is an e_mux
  assign p29_stage_29 = ((full_30 & ~clear_fifo) == 0)? data_in :
    stage_30;

  //data_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_29 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_29))
          if (sync_reset & full_29 & !((full_30 == 0) & read & write))
              stage_29 <= 0;
          else 
            stage_29 <= p29_stage_29;
    end


  //control_29, which is an e_mux
  assign p29_full_29 = ((read & !write) == 0)? full_28 :
    full_30;

  //control_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_29 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_29 <= 0;
          else 
            full_29 <= p29_full_29;
    end


  //data_28, which is an e_mux
  assign p28_stage_28 = ((full_29 & ~clear_fifo) == 0)? data_in :
    stage_29;

  //data_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_28 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_28))
          if (sync_reset & full_28 & !((full_29 == 0) & read & write))
              stage_28 <= 0;
          else 
            stage_28 <= p28_stage_28;
    end


  //control_28, which is an e_mux
  assign p28_full_28 = ((read & !write) == 0)? full_27 :
    full_29;

  //control_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_28 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_28 <= 0;
          else 
            full_28 <= p28_full_28;
    end


  //data_27, which is an e_mux
  assign p27_stage_27 = ((full_28 & ~clear_fifo) == 0)? data_in :
    stage_28;

  //data_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_27 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_27))
          if (sync_reset & full_27 & !((full_28 == 0) & read & write))
              stage_27 <= 0;
          else 
            stage_27 <= p27_stage_27;
    end


  //control_27, which is an e_mux
  assign p27_full_27 = ((read & !write) == 0)? full_26 :
    full_28;

  //control_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_27 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_27 <= 0;
          else 
            full_27 <= p27_full_27;
    end


  //data_26, which is an e_mux
  assign p26_stage_26 = ((full_27 & ~clear_fifo) == 0)? data_in :
    stage_27;

  //data_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_26 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_26))
          if (sync_reset & full_26 & !((full_27 == 0) & read & write))
              stage_26 <= 0;
          else 
            stage_26 <= p26_stage_26;
    end


  //control_26, which is an e_mux
  assign p26_full_26 = ((read & !write) == 0)? full_25 :
    full_27;

  //control_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_26 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_26 <= 0;
          else 
            full_26 <= p26_full_26;
    end


  //data_25, which is an e_mux
  assign p25_stage_25 = ((full_26 & ~clear_fifo) == 0)? data_in :
    stage_26;

  //data_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_25 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_25))
          if (sync_reset & full_25 & !((full_26 == 0) & read & write))
              stage_25 <= 0;
          else 
            stage_25 <= p25_stage_25;
    end


  //control_25, which is an e_mux
  assign p25_full_25 = ((read & !write) == 0)? full_24 :
    full_26;

  //control_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_25 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_25 <= 0;
          else 
            full_25 <= p25_full_25;
    end


  //data_24, which is an e_mux
  assign p24_stage_24 = ((full_25 & ~clear_fifo) == 0)? data_in :
    stage_25;

  //data_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_24 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_24))
          if (sync_reset & full_24 & !((full_25 == 0) & read & write))
              stage_24 <= 0;
          else 
            stage_24 <= p24_stage_24;
    end


  //control_24, which is an e_mux
  assign p24_full_24 = ((read & !write) == 0)? full_23 :
    full_25;

  //control_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_24 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_24 <= 0;
          else 
            full_24 <= p24_full_24;
    end


  //data_23, which is an e_mux
  assign p23_stage_23 = ((full_24 & ~clear_fifo) == 0)? data_in :
    stage_24;

  //data_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_23 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_23))
          if (sync_reset & full_23 & !((full_24 == 0) & read & write))
              stage_23 <= 0;
          else 
            stage_23 <= p23_stage_23;
    end


  //control_23, which is an e_mux
  assign p23_full_23 = ((read & !write) == 0)? full_22 :
    full_24;

  //control_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_23 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_23 <= 0;
          else 
            full_23 <= p23_full_23;
    end


  //data_22, which is an e_mux
  assign p22_stage_22 = ((full_23 & ~clear_fifo) == 0)? data_in :
    stage_23;

  //data_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_22 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_22))
          if (sync_reset & full_22 & !((full_23 == 0) & read & write))
              stage_22 <= 0;
          else 
            stage_22 <= p22_stage_22;
    end


  //control_22, which is an e_mux
  assign p22_full_22 = ((read & !write) == 0)? full_21 :
    full_23;

  //control_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_22 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_22 <= 0;
          else 
            full_22 <= p22_full_22;
    end


  //data_21, which is an e_mux
  assign p21_stage_21 = ((full_22 & ~clear_fifo) == 0)? data_in :
    stage_22;

  //data_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_21 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_21))
          if (sync_reset & full_21 & !((full_22 == 0) & read & write))
              stage_21 <= 0;
          else 
            stage_21 <= p21_stage_21;
    end


  //control_21, which is an e_mux
  assign p21_full_21 = ((read & !write) == 0)? full_20 :
    full_22;

  //control_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_21 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_21 <= 0;
          else 
            full_21 <= p21_full_21;
    end


  //data_20, which is an e_mux
  assign p20_stage_20 = ((full_21 & ~clear_fifo) == 0)? data_in :
    stage_21;

  //data_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_20 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_20))
          if (sync_reset & full_20 & !((full_21 == 0) & read & write))
              stage_20 <= 0;
          else 
            stage_20 <= p20_stage_20;
    end


  //control_20, which is an e_mux
  assign p20_full_20 = ((read & !write) == 0)? full_19 :
    full_21;

  //control_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_20 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_20 <= 0;
          else 
            full_20 <= p20_full_20;
    end


  //data_19, which is an e_mux
  assign p19_stage_19 = ((full_20 & ~clear_fifo) == 0)? data_in :
    stage_20;

  //data_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_19 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_19))
          if (sync_reset & full_19 & !((full_20 == 0) & read & write))
              stage_19 <= 0;
          else 
            stage_19 <= p19_stage_19;
    end


  //control_19, which is an e_mux
  assign p19_full_19 = ((read & !write) == 0)? full_18 :
    full_20;

  //control_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_19 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_19 <= 0;
          else 
            full_19 <= p19_full_19;
    end


  //data_18, which is an e_mux
  assign p18_stage_18 = ((full_19 & ~clear_fifo) == 0)? data_in :
    stage_19;

  //data_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_18 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_18))
          if (sync_reset & full_18 & !((full_19 == 0) & read & write))
              stage_18 <= 0;
          else 
            stage_18 <= p18_stage_18;
    end


  //control_18, which is an e_mux
  assign p18_full_18 = ((read & !write) == 0)? full_17 :
    full_19;

  //control_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_18 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_18 <= 0;
          else 
            full_18 <= p18_full_18;
    end


  //data_17, which is an e_mux
  assign p17_stage_17 = ((full_18 & ~clear_fifo) == 0)? data_in :
    stage_18;

  //data_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_17 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_17))
          if (sync_reset & full_17 & !((full_18 == 0) & read & write))
              stage_17 <= 0;
          else 
            stage_17 <= p17_stage_17;
    end


  //control_17, which is an e_mux
  assign p17_full_17 = ((read & !write) == 0)? full_16 :
    full_18;

  //control_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_17 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_17 <= 0;
          else 
            full_17 <= p17_full_17;
    end


  //data_16, which is an e_mux
  assign p16_stage_16 = ((full_17 & ~clear_fifo) == 0)? data_in :
    stage_17;

  //data_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_16 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_16))
          if (sync_reset & full_16 & !((full_17 == 0) & read & write))
              stage_16 <= 0;
          else 
            stage_16 <= p16_stage_16;
    end


  //control_16, which is an e_mux
  assign p16_full_16 = ((read & !write) == 0)? full_15 :
    full_17;

  //control_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_16 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_16 <= 0;
          else 
            full_16 <= p16_full_16;
    end


  //data_15, which is an e_mux
  assign p15_stage_15 = ((full_16 & ~clear_fifo) == 0)? data_in :
    stage_16;

  //data_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_15 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_15))
          if (sync_reset & full_15 & !((full_16 == 0) & read & write))
              stage_15 <= 0;
          else 
            stage_15 <= p15_stage_15;
    end


  //control_15, which is an e_mux
  assign p15_full_15 = ((read & !write) == 0)? full_14 :
    full_16;

  //control_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_15 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_15 <= 0;
          else 
            full_15 <= p15_full_15;
    end


  //data_14, which is an e_mux
  assign p14_stage_14 = ((full_15 & ~clear_fifo) == 0)? data_in :
    stage_15;

  //data_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_14 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_14))
          if (sync_reset & full_14 & !((full_15 == 0) & read & write))
              stage_14 <= 0;
          else 
            stage_14 <= p14_stage_14;
    end


  //control_14, which is an e_mux
  assign p14_full_14 = ((read & !write) == 0)? full_13 :
    full_15;

  //control_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_14 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_14 <= 0;
          else 
            full_14 <= p14_full_14;
    end


  //data_13, which is an e_mux
  assign p13_stage_13 = ((full_14 & ~clear_fifo) == 0)? data_in :
    stage_14;

  //data_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_13 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_13))
          if (sync_reset & full_13 & !((full_14 == 0) & read & write))
              stage_13 <= 0;
          else 
            stage_13 <= p13_stage_13;
    end


  //control_13, which is an e_mux
  assign p13_full_13 = ((read & !write) == 0)? full_12 :
    full_14;

  //control_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_13 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_13 <= 0;
          else 
            full_13 <= p13_full_13;
    end


  //data_12, which is an e_mux
  assign p12_stage_12 = ((full_13 & ~clear_fifo) == 0)? data_in :
    stage_13;

  //data_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_12 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_12))
          if (sync_reset & full_12 & !((full_13 == 0) & read & write))
              stage_12 <= 0;
          else 
            stage_12 <= p12_stage_12;
    end


  //control_12, which is an e_mux
  assign p12_full_12 = ((read & !write) == 0)? full_11 :
    full_13;

  //control_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_12 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_12 <= 0;
          else 
            full_12 <= p12_full_12;
    end


  //data_11, which is an e_mux
  assign p11_stage_11 = ((full_12 & ~clear_fifo) == 0)? data_in :
    stage_12;

  //data_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_11 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_11))
          if (sync_reset & full_11 & !((full_12 == 0) & read & write))
              stage_11 <= 0;
          else 
            stage_11 <= p11_stage_11;
    end


  //control_11, which is an e_mux
  assign p11_full_11 = ((read & !write) == 0)? full_10 :
    full_12;

  //control_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_11 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_11 <= 0;
          else 
            full_11 <= p11_full_11;
    end


  //data_10, which is an e_mux
  assign p10_stage_10 = ((full_11 & ~clear_fifo) == 0)? data_in :
    stage_11;

  //data_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_10 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_10))
          if (sync_reset & full_10 & !((full_11 == 0) & read & write))
              stage_10 <= 0;
          else 
            stage_10 <= p10_stage_10;
    end


  //control_10, which is an e_mux
  assign p10_full_10 = ((read & !write) == 0)? full_9 :
    full_11;

  //control_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_10 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_10 <= 0;
          else 
            full_10 <= p10_full_10;
    end


  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    stage_10;

  //data_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_9 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_9))
          if (sync_reset & full_9 & !((full_10 == 0) & read & write))
              stage_9 <= 0;
          else 
            stage_9 <= p9_stage_9;
    end


  //control_9, which is an e_mux
  assign p9_full_9 = ((read & !write) == 0)? full_8 :
    full_10;

  //control_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_9 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_9 <= 0;
          else 
            full_9 <= p9_full_9;
    end


  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    stage_9;

  //data_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_8 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_8))
          if (sync_reset & full_8 & !((full_9 == 0) & read & write))
              stage_8 <= 0;
          else 
            stage_8 <= p8_stage_8;
    end


  //control_8, which is an e_mux
  assign p8_full_8 = ((read & !write) == 0)? full_7 :
    full_9;

  //control_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_8 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_8 <= 0;
          else 
            full_8 <= p8_full_8;
    end


  //data_7, which is an e_mux
  assign p7_stage_7 = ((full_8 & ~clear_fifo) == 0)? data_in :
    stage_8;

  //data_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_7 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_7))
          if (sync_reset & full_7 & !((full_8 == 0) & read & write))
              stage_7 <= 0;
          else 
            stage_7 <= p7_stage_7;
    end


  //control_7, which is an e_mux
  assign p7_full_7 = ((read & !write) == 0)? full_6 :
    full_8;

  //control_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_7 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_7 <= 0;
          else 
            full_7 <= p7_full_7;
    end


  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    stage_7;

  //data_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_6 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_6))
          if (sync_reset & full_6 & !((full_7 == 0) & read & write))
              stage_6 <= 0;
          else 
            stage_6 <= p6_stage_6;
    end


  //control_6, which is an e_mux
  assign p6_full_6 = ((read & !write) == 0)? full_5 :
    full_7;

  //control_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_6 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_6 <= 0;
          else 
            full_6 <= p6_full_6;
    end


  //data_5, which is an e_mux
  assign p5_stage_5 = ((full_6 & ~clear_fifo) == 0)? data_in :
    stage_6;

  //data_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_5 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_5))
          if (sync_reset & full_5 & !((full_6 == 0) & read & write))
              stage_5 <= 0;
          else 
            stage_5 <= p5_stage_5;
    end


  //control_5, which is an e_mux
  assign p5_full_5 = ((read & !write) == 0)? full_4 :
    full_6;

  //control_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_5 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_5 <= 0;
          else 
            full_5 <= p5_full_5;
    end


  //data_4, which is an e_mux
  assign p4_stage_4 = ((full_5 & ~clear_fifo) == 0)? data_in :
    stage_5;

  //data_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_4 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_4))
          if (sync_reset & full_4 & !((full_5 == 0) & read & write))
              stage_4 <= 0;
          else 
            stage_4 <= p4_stage_4;
    end


  //control_4, which is an e_mux
  assign p4_full_4 = ((read & !write) == 0)? full_3 :
    full_5;

  //control_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_4 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_4 <= 0;
          else 
            full_4 <= p4_full_4;
    end


  //data_3, which is an e_mux
  assign p3_stage_3 = ((full_4 & ~clear_fifo) == 0)? data_in :
    stage_4;

  //data_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_3 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_3))
          if (sync_reset & full_3 & !((full_4 == 0) & read & write))
              stage_3 <= 0;
          else 
            stage_3 <= p3_stage_3;
    end


  //control_3, which is an e_mux
  assign p3_full_3 = ((read & !write) == 0)? full_2 :
    full_4;

  //control_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_3 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_3 <= 0;
          else 
            full_3 <= p3_full_3;
    end


  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    stage_3;

  //data_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_2 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_2))
          if (sync_reset & full_2 & !((full_3 == 0) & read & write))
              stage_2 <= 0;
          else 
            stage_2 <= p2_stage_2;
    end


  //control_2, which is an e_mux
  assign p2_full_2 = ((read & !write) == 0)? full_1 :
    full_3;

  //control_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_2 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_2 <= 0;
          else 
            full_2 <= p2_full_2;
    end


  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    stage_2;

  //data_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_1 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_1))
          if (sync_reset & full_1 & !((full_2 == 0) & read & write))
              stage_1 <= 0;
          else 
            stage_1 <= p1_stage_1;
    end


  //control_1, which is an e_mux
  assign p1_full_1 = ((read & !write) == 0)? full_0 :
    full_2;

  //control_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_1 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_1 <= 0;
          else 
            full_1 <= p1_full_1;
    end


  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    stage_1;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    full_1;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module burstcount_fifo_for_ddr2_s1_module (
                                            // inputs:
                                             clear_fifo,
                                             clk,
                                             data_in,
                                             read,
                                             reset_n,
                                             sync_reset,
                                             write,

                                            // outputs:
                                             data_out,
                                             empty,
                                             fifo_contains_ones_n,
                                             full
                                          )
;

  output  [  6: 0] data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input   [  6: 0] data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire    [  6: 0] data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_10;
  reg              full_11;
  reg              full_12;
  reg              full_13;
  reg              full_14;
  reg              full_15;
  reg              full_16;
  reg              full_17;
  reg              full_18;
  reg              full_19;
  reg              full_2;
  reg              full_20;
  reg              full_21;
  reg              full_22;
  reg              full_23;
  reg              full_24;
  reg              full_25;
  reg              full_26;
  reg              full_27;
  reg              full_28;
  reg              full_29;
  reg              full_3;
  reg              full_30;
  reg              full_31;
  wire             full_32;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  6: 0] how_many_ones;
  wire    [  6: 0] one_count_minus_one;
  wire    [  6: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  6: 0] p0_stage_0;
  wire             p10_full_10;
  wire    [  6: 0] p10_stage_10;
  wire             p11_full_11;
  wire    [  6: 0] p11_stage_11;
  wire             p12_full_12;
  wire    [  6: 0] p12_stage_12;
  wire             p13_full_13;
  wire    [  6: 0] p13_stage_13;
  wire             p14_full_14;
  wire    [  6: 0] p14_stage_14;
  wire             p15_full_15;
  wire    [  6: 0] p15_stage_15;
  wire             p16_full_16;
  wire    [  6: 0] p16_stage_16;
  wire             p17_full_17;
  wire    [  6: 0] p17_stage_17;
  wire             p18_full_18;
  wire    [  6: 0] p18_stage_18;
  wire             p19_full_19;
  wire    [  6: 0] p19_stage_19;
  wire             p1_full_1;
  wire    [  6: 0] p1_stage_1;
  wire             p20_full_20;
  wire    [  6: 0] p20_stage_20;
  wire             p21_full_21;
  wire    [  6: 0] p21_stage_21;
  wire             p22_full_22;
  wire    [  6: 0] p22_stage_22;
  wire             p23_full_23;
  wire    [  6: 0] p23_stage_23;
  wire             p24_full_24;
  wire    [  6: 0] p24_stage_24;
  wire             p25_full_25;
  wire    [  6: 0] p25_stage_25;
  wire             p26_full_26;
  wire    [  6: 0] p26_stage_26;
  wire             p27_full_27;
  wire    [  6: 0] p27_stage_27;
  wire             p28_full_28;
  wire    [  6: 0] p28_stage_28;
  wire             p29_full_29;
  wire    [  6: 0] p29_stage_29;
  wire             p2_full_2;
  wire    [  6: 0] p2_stage_2;
  wire             p30_full_30;
  wire    [  6: 0] p30_stage_30;
  wire             p31_full_31;
  wire    [  6: 0] p31_stage_31;
  wire             p3_full_3;
  wire    [  6: 0] p3_stage_3;
  wire             p4_full_4;
  wire    [  6: 0] p4_stage_4;
  wire             p5_full_5;
  wire    [  6: 0] p5_stage_5;
  wire             p6_full_6;
  wire    [  6: 0] p6_stage_6;
  wire             p7_full_7;
  wire    [  6: 0] p7_stage_7;
  wire             p8_full_8;
  wire    [  6: 0] p8_stage_8;
  wire             p9_full_9;
  wire    [  6: 0] p9_stage_9;
  reg     [  6: 0] stage_0;
  reg     [  6: 0] stage_1;
  reg     [  6: 0] stage_10;
  reg     [  6: 0] stage_11;
  reg     [  6: 0] stage_12;
  reg     [  6: 0] stage_13;
  reg     [  6: 0] stage_14;
  reg     [  6: 0] stage_15;
  reg     [  6: 0] stage_16;
  reg     [  6: 0] stage_17;
  reg     [  6: 0] stage_18;
  reg     [  6: 0] stage_19;
  reg     [  6: 0] stage_2;
  reg     [  6: 0] stage_20;
  reg     [  6: 0] stage_21;
  reg     [  6: 0] stage_22;
  reg     [  6: 0] stage_23;
  reg     [  6: 0] stage_24;
  reg     [  6: 0] stage_25;
  reg     [  6: 0] stage_26;
  reg     [  6: 0] stage_27;
  reg     [  6: 0] stage_28;
  reg     [  6: 0] stage_29;
  reg     [  6: 0] stage_3;
  reg     [  6: 0] stage_30;
  reg     [  6: 0] stage_31;
  reg     [  6: 0] stage_4;
  reg     [  6: 0] stage_5;
  reg     [  6: 0] stage_6;
  reg     [  6: 0] stage_7;
  reg     [  6: 0] stage_8;
  reg     [  6: 0] stage_9;
  wire    [  6: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_31;
  assign empty = !full_0;
  assign full_32 = 0;
  //data_31, which is an e_mux
  assign p31_stage_31 = ((full_32 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_31 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_31))
          if (sync_reset & full_31 & !((full_32 == 0) & read & write))
              stage_31 <= 0;
          else 
            stage_31 <= p31_stage_31;
    end


  //control_31, which is an e_mux
  assign p31_full_31 = ((read & !write) == 0)? full_30 :
    0;

  //control_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_31 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_31 <= 0;
          else 
            full_31 <= p31_full_31;
    end


  //data_30, which is an e_mux
  assign p30_stage_30 = ((full_31 & ~clear_fifo) == 0)? data_in :
    stage_31;

  //data_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_30 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_30))
          if (sync_reset & full_30 & !((full_31 == 0) & read & write))
              stage_30 <= 0;
          else 
            stage_30 <= p30_stage_30;
    end


  //control_30, which is an e_mux
  assign p30_full_30 = ((read & !write) == 0)? full_29 :
    full_31;

  //control_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_30 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_30 <= 0;
          else 
            full_30 <= p30_full_30;
    end


  //data_29, which is an e_mux
  assign p29_stage_29 = ((full_30 & ~clear_fifo) == 0)? data_in :
    stage_30;

  //data_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_29 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_29))
          if (sync_reset & full_29 & !((full_30 == 0) & read & write))
              stage_29 <= 0;
          else 
            stage_29 <= p29_stage_29;
    end


  //control_29, which is an e_mux
  assign p29_full_29 = ((read & !write) == 0)? full_28 :
    full_30;

  //control_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_29 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_29 <= 0;
          else 
            full_29 <= p29_full_29;
    end


  //data_28, which is an e_mux
  assign p28_stage_28 = ((full_29 & ~clear_fifo) == 0)? data_in :
    stage_29;

  //data_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_28 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_28))
          if (sync_reset & full_28 & !((full_29 == 0) & read & write))
              stage_28 <= 0;
          else 
            stage_28 <= p28_stage_28;
    end


  //control_28, which is an e_mux
  assign p28_full_28 = ((read & !write) == 0)? full_27 :
    full_29;

  //control_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_28 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_28 <= 0;
          else 
            full_28 <= p28_full_28;
    end


  //data_27, which is an e_mux
  assign p27_stage_27 = ((full_28 & ~clear_fifo) == 0)? data_in :
    stage_28;

  //data_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_27 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_27))
          if (sync_reset & full_27 & !((full_28 == 0) & read & write))
              stage_27 <= 0;
          else 
            stage_27 <= p27_stage_27;
    end


  //control_27, which is an e_mux
  assign p27_full_27 = ((read & !write) == 0)? full_26 :
    full_28;

  //control_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_27 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_27 <= 0;
          else 
            full_27 <= p27_full_27;
    end


  //data_26, which is an e_mux
  assign p26_stage_26 = ((full_27 & ~clear_fifo) == 0)? data_in :
    stage_27;

  //data_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_26 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_26))
          if (sync_reset & full_26 & !((full_27 == 0) & read & write))
              stage_26 <= 0;
          else 
            stage_26 <= p26_stage_26;
    end


  //control_26, which is an e_mux
  assign p26_full_26 = ((read & !write) == 0)? full_25 :
    full_27;

  //control_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_26 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_26 <= 0;
          else 
            full_26 <= p26_full_26;
    end


  //data_25, which is an e_mux
  assign p25_stage_25 = ((full_26 & ~clear_fifo) == 0)? data_in :
    stage_26;

  //data_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_25 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_25))
          if (sync_reset & full_25 & !((full_26 == 0) & read & write))
              stage_25 <= 0;
          else 
            stage_25 <= p25_stage_25;
    end


  //control_25, which is an e_mux
  assign p25_full_25 = ((read & !write) == 0)? full_24 :
    full_26;

  //control_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_25 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_25 <= 0;
          else 
            full_25 <= p25_full_25;
    end


  //data_24, which is an e_mux
  assign p24_stage_24 = ((full_25 & ~clear_fifo) == 0)? data_in :
    stage_25;

  //data_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_24 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_24))
          if (sync_reset & full_24 & !((full_25 == 0) & read & write))
              stage_24 <= 0;
          else 
            stage_24 <= p24_stage_24;
    end


  //control_24, which is an e_mux
  assign p24_full_24 = ((read & !write) == 0)? full_23 :
    full_25;

  //control_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_24 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_24 <= 0;
          else 
            full_24 <= p24_full_24;
    end


  //data_23, which is an e_mux
  assign p23_stage_23 = ((full_24 & ~clear_fifo) == 0)? data_in :
    stage_24;

  //data_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_23 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_23))
          if (sync_reset & full_23 & !((full_24 == 0) & read & write))
              stage_23 <= 0;
          else 
            stage_23 <= p23_stage_23;
    end


  //control_23, which is an e_mux
  assign p23_full_23 = ((read & !write) == 0)? full_22 :
    full_24;

  //control_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_23 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_23 <= 0;
          else 
            full_23 <= p23_full_23;
    end


  //data_22, which is an e_mux
  assign p22_stage_22 = ((full_23 & ~clear_fifo) == 0)? data_in :
    stage_23;

  //data_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_22 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_22))
          if (sync_reset & full_22 & !((full_23 == 0) & read & write))
              stage_22 <= 0;
          else 
            stage_22 <= p22_stage_22;
    end


  //control_22, which is an e_mux
  assign p22_full_22 = ((read & !write) == 0)? full_21 :
    full_23;

  //control_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_22 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_22 <= 0;
          else 
            full_22 <= p22_full_22;
    end


  //data_21, which is an e_mux
  assign p21_stage_21 = ((full_22 & ~clear_fifo) == 0)? data_in :
    stage_22;

  //data_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_21 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_21))
          if (sync_reset & full_21 & !((full_22 == 0) & read & write))
              stage_21 <= 0;
          else 
            stage_21 <= p21_stage_21;
    end


  //control_21, which is an e_mux
  assign p21_full_21 = ((read & !write) == 0)? full_20 :
    full_22;

  //control_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_21 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_21 <= 0;
          else 
            full_21 <= p21_full_21;
    end


  //data_20, which is an e_mux
  assign p20_stage_20 = ((full_21 & ~clear_fifo) == 0)? data_in :
    stage_21;

  //data_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_20 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_20))
          if (sync_reset & full_20 & !((full_21 == 0) & read & write))
              stage_20 <= 0;
          else 
            stage_20 <= p20_stage_20;
    end


  //control_20, which is an e_mux
  assign p20_full_20 = ((read & !write) == 0)? full_19 :
    full_21;

  //control_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_20 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_20 <= 0;
          else 
            full_20 <= p20_full_20;
    end


  //data_19, which is an e_mux
  assign p19_stage_19 = ((full_20 & ~clear_fifo) == 0)? data_in :
    stage_20;

  //data_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_19 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_19))
          if (sync_reset & full_19 & !((full_20 == 0) & read & write))
              stage_19 <= 0;
          else 
            stage_19 <= p19_stage_19;
    end


  //control_19, which is an e_mux
  assign p19_full_19 = ((read & !write) == 0)? full_18 :
    full_20;

  //control_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_19 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_19 <= 0;
          else 
            full_19 <= p19_full_19;
    end


  //data_18, which is an e_mux
  assign p18_stage_18 = ((full_19 & ~clear_fifo) == 0)? data_in :
    stage_19;

  //data_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_18 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_18))
          if (sync_reset & full_18 & !((full_19 == 0) & read & write))
              stage_18 <= 0;
          else 
            stage_18 <= p18_stage_18;
    end


  //control_18, which is an e_mux
  assign p18_full_18 = ((read & !write) == 0)? full_17 :
    full_19;

  //control_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_18 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_18 <= 0;
          else 
            full_18 <= p18_full_18;
    end


  //data_17, which is an e_mux
  assign p17_stage_17 = ((full_18 & ~clear_fifo) == 0)? data_in :
    stage_18;

  //data_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_17 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_17))
          if (sync_reset & full_17 & !((full_18 == 0) & read & write))
              stage_17 <= 0;
          else 
            stage_17 <= p17_stage_17;
    end


  //control_17, which is an e_mux
  assign p17_full_17 = ((read & !write) == 0)? full_16 :
    full_18;

  //control_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_17 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_17 <= 0;
          else 
            full_17 <= p17_full_17;
    end


  //data_16, which is an e_mux
  assign p16_stage_16 = ((full_17 & ~clear_fifo) == 0)? data_in :
    stage_17;

  //data_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_16 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_16))
          if (sync_reset & full_16 & !((full_17 == 0) & read & write))
              stage_16 <= 0;
          else 
            stage_16 <= p16_stage_16;
    end


  //control_16, which is an e_mux
  assign p16_full_16 = ((read & !write) == 0)? full_15 :
    full_17;

  //control_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_16 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_16 <= 0;
          else 
            full_16 <= p16_full_16;
    end


  //data_15, which is an e_mux
  assign p15_stage_15 = ((full_16 & ~clear_fifo) == 0)? data_in :
    stage_16;

  //data_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_15 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_15))
          if (sync_reset & full_15 & !((full_16 == 0) & read & write))
              stage_15 <= 0;
          else 
            stage_15 <= p15_stage_15;
    end


  //control_15, which is an e_mux
  assign p15_full_15 = ((read & !write) == 0)? full_14 :
    full_16;

  //control_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_15 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_15 <= 0;
          else 
            full_15 <= p15_full_15;
    end


  //data_14, which is an e_mux
  assign p14_stage_14 = ((full_15 & ~clear_fifo) == 0)? data_in :
    stage_15;

  //data_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_14 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_14))
          if (sync_reset & full_14 & !((full_15 == 0) & read & write))
              stage_14 <= 0;
          else 
            stage_14 <= p14_stage_14;
    end


  //control_14, which is an e_mux
  assign p14_full_14 = ((read & !write) == 0)? full_13 :
    full_15;

  //control_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_14 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_14 <= 0;
          else 
            full_14 <= p14_full_14;
    end


  //data_13, which is an e_mux
  assign p13_stage_13 = ((full_14 & ~clear_fifo) == 0)? data_in :
    stage_14;

  //data_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_13 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_13))
          if (sync_reset & full_13 & !((full_14 == 0) & read & write))
              stage_13 <= 0;
          else 
            stage_13 <= p13_stage_13;
    end


  //control_13, which is an e_mux
  assign p13_full_13 = ((read & !write) == 0)? full_12 :
    full_14;

  //control_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_13 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_13 <= 0;
          else 
            full_13 <= p13_full_13;
    end


  //data_12, which is an e_mux
  assign p12_stage_12 = ((full_13 & ~clear_fifo) == 0)? data_in :
    stage_13;

  //data_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_12 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_12))
          if (sync_reset & full_12 & !((full_13 == 0) & read & write))
              stage_12 <= 0;
          else 
            stage_12 <= p12_stage_12;
    end


  //control_12, which is an e_mux
  assign p12_full_12 = ((read & !write) == 0)? full_11 :
    full_13;

  //control_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_12 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_12 <= 0;
          else 
            full_12 <= p12_full_12;
    end


  //data_11, which is an e_mux
  assign p11_stage_11 = ((full_12 & ~clear_fifo) == 0)? data_in :
    stage_12;

  //data_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_11 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_11))
          if (sync_reset & full_11 & !((full_12 == 0) & read & write))
              stage_11 <= 0;
          else 
            stage_11 <= p11_stage_11;
    end


  //control_11, which is an e_mux
  assign p11_full_11 = ((read & !write) == 0)? full_10 :
    full_12;

  //control_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_11 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_11 <= 0;
          else 
            full_11 <= p11_full_11;
    end


  //data_10, which is an e_mux
  assign p10_stage_10 = ((full_11 & ~clear_fifo) == 0)? data_in :
    stage_11;

  //data_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_10 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_10))
          if (sync_reset & full_10 & !((full_11 == 0) & read & write))
              stage_10 <= 0;
          else 
            stage_10 <= p10_stage_10;
    end


  //control_10, which is an e_mux
  assign p10_full_10 = ((read & !write) == 0)? full_9 :
    full_11;

  //control_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_10 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_10 <= 0;
          else 
            full_10 <= p10_full_10;
    end


  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    stage_10;

  //data_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_9 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_9))
          if (sync_reset & full_9 & !((full_10 == 0) & read & write))
              stage_9 <= 0;
          else 
            stage_9 <= p9_stage_9;
    end


  //control_9, which is an e_mux
  assign p9_full_9 = ((read & !write) == 0)? full_8 :
    full_10;

  //control_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_9 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_9 <= 0;
          else 
            full_9 <= p9_full_9;
    end


  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    stage_9;

  //data_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_8 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_8))
          if (sync_reset & full_8 & !((full_9 == 0) & read & write))
              stage_8 <= 0;
          else 
            stage_8 <= p8_stage_8;
    end


  //control_8, which is an e_mux
  assign p8_full_8 = ((read & !write) == 0)? full_7 :
    full_9;

  //control_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_8 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_8 <= 0;
          else 
            full_8 <= p8_full_8;
    end


  //data_7, which is an e_mux
  assign p7_stage_7 = ((full_8 & ~clear_fifo) == 0)? data_in :
    stage_8;

  //data_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_7 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_7))
          if (sync_reset & full_7 & !((full_8 == 0) & read & write))
              stage_7 <= 0;
          else 
            stage_7 <= p7_stage_7;
    end


  //control_7, which is an e_mux
  assign p7_full_7 = ((read & !write) == 0)? full_6 :
    full_8;

  //control_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_7 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_7 <= 0;
          else 
            full_7 <= p7_full_7;
    end


  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    stage_7;

  //data_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_6 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_6))
          if (sync_reset & full_6 & !((full_7 == 0) & read & write))
              stage_6 <= 0;
          else 
            stage_6 <= p6_stage_6;
    end


  //control_6, which is an e_mux
  assign p6_full_6 = ((read & !write) == 0)? full_5 :
    full_7;

  //control_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_6 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_6 <= 0;
          else 
            full_6 <= p6_full_6;
    end


  //data_5, which is an e_mux
  assign p5_stage_5 = ((full_6 & ~clear_fifo) == 0)? data_in :
    stage_6;

  //data_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_5 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_5))
          if (sync_reset & full_5 & !((full_6 == 0) & read & write))
              stage_5 <= 0;
          else 
            stage_5 <= p5_stage_5;
    end


  //control_5, which is an e_mux
  assign p5_full_5 = ((read & !write) == 0)? full_4 :
    full_6;

  //control_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_5 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_5 <= 0;
          else 
            full_5 <= p5_full_5;
    end


  //data_4, which is an e_mux
  assign p4_stage_4 = ((full_5 & ~clear_fifo) == 0)? data_in :
    stage_5;

  //data_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_4 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_4))
          if (sync_reset & full_4 & !((full_5 == 0) & read & write))
              stage_4 <= 0;
          else 
            stage_4 <= p4_stage_4;
    end


  //control_4, which is an e_mux
  assign p4_full_4 = ((read & !write) == 0)? full_3 :
    full_5;

  //control_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_4 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_4 <= 0;
          else 
            full_4 <= p4_full_4;
    end


  //data_3, which is an e_mux
  assign p3_stage_3 = ((full_4 & ~clear_fifo) == 0)? data_in :
    stage_4;

  //data_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_3 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_3))
          if (sync_reset & full_3 & !((full_4 == 0) & read & write))
              stage_3 <= 0;
          else 
            stage_3 <= p3_stage_3;
    end


  //control_3, which is an e_mux
  assign p3_full_3 = ((read & !write) == 0)? full_2 :
    full_4;

  //control_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_3 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_3 <= 0;
          else 
            full_3 <= p3_full_3;
    end


  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    stage_3;

  //data_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_2 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_2))
          if (sync_reset & full_2 & !((full_3 == 0) & read & write))
              stage_2 <= 0;
          else 
            stage_2 <= p2_stage_2;
    end


  //control_2, which is an e_mux
  assign p2_full_2 = ((read & !write) == 0)? full_1 :
    full_3;

  //control_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_2 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_2 <= 0;
          else 
            full_2 <= p2_full_2;
    end


  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    stage_2;

  //data_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_1 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_1))
          if (sync_reset & full_1 & !((full_2 == 0) & read & write))
              stage_1 <= 0;
          else 
            stage_1 <= p1_stage_1;
    end


  //control_1, which is an e_mux
  assign p1_full_1 = ((read & !write) == 0)? full_0 :
    full_2;

  //control_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_1 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_1 <= 0;
          else 
            full_1 <= p1_full_1;
    end


  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    stage_1;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    full_1;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_ddr2_s1_module (
                                                                  // inputs:
                                                                   clear_fifo,
                                                                   clk,
                                                                   data_in,
                                                                   read,
                                                                   reset_n,
                                                                   sync_reset,
                                                                   write,

                                                                  // outputs:
                                                                   data_out,
                                                                   empty,
                                                                   fifo_contains_ones_n,
                                                                   full
                                                                )
;

  output           data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input            data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire             data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_10;
  reg              full_11;
  reg              full_12;
  reg              full_13;
  reg              full_14;
  reg              full_15;
  reg              full_16;
  reg              full_17;
  reg              full_18;
  reg              full_19;
  reg              full_2;
  reg              full_20;
  reg              full_21;
  reg              full_22;
  reg              full_23;
  reg              full_24;
  reg              full_25;
  reg              full_26;
  reg              full_27;
  reg              full_28;
  reg              full_29;
  reg              full_3;
  reg              full_30;
  reg              full_31;
  wire             full_32;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  6: 0] how_many_ones;
  wire    [  6: 0] one_count_minus_one;
  wire    [  6: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p10_full_10;
  wire             p10_stage_10;
  wire             p11_full_11;
  wire             p11_stage_11;
  wire             p12_full_12;
  wire             p12_stage_12;
  wire             p13_full_13;
  wire             p13_stage_13;
  wire             p14_full_14;
  wire             p14_stage_14;
  wire             p15_full_15;
  wire             p15_stage_15;
  wire             p16_full_16;
  wire             p16_stage_16;
  wire             p17_full_17;
  wire             p17_stage_17;
  wire             p18_full_18;
  wire             p18_stage_18;
  wire             p19_full_19;
  wire             p19_stage_19;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p20_full_20;
  wire             p20_stage_20;
  wire             p21_full_21;
  wire             p21_stage_21;
  wire             p22_full_22;
  wire             p22_stage_22;
  wire             p23_full_23;
  wire             p23_stage_23;
  wire             p24_full_24;
  wire             p24_stage_24;
  wire             p25_full_25;
  wire             p25_stage_25;
  wire             p26_full_26;
  wire             p26_stage_26;
  wire             p27_full_27;
  wire             p27_stage_27;
  wire             p28_full_28;
  wire             p28_stage_28;
  wire             p29_full_29;
  wire             p29_stage_29;
  wire             p2_full_2;
  wire             p2_stage_2;
  wire             p30_full_30;
  wire             p30_stage_30;
  wire             p31_full_31;
  wire             p31_stage_31;
  wire             p3_full_3;
  wire             p3_stage_3;
  wire             p4_full_4;
  wire             p4_stage_4;
  wire             p5_full_5;
  wire             p5_stage_5;
  wire             p6_full_6;
  wire             p6_stage_6;
  wire             p7_full_7;
  wire             p7_stage_7;
  wire             p8_full_8;
  wire             p8_stage_8;
  wire             p9_full_9;
  wire             p9_stage_9;
  reg              stage_0;
  reg              stage_1;
  reg              stage_10;
  reg              stage_11;
  reg              stage_12;
  reg              stage_13;
  reg              stage_14;
  reg              stage_15;
  reg              stage_16;
  reg              stage_17;
  reg              stage_18;
  reg              stage_19;
  reg              stage_2;
  reg              stage_20;
  reg              stage_21;
  reg              stage_22;
  reg              stage_23;
  reg              stage_24;
  reg              stage_25;
  reg              stage_26;
  reg              stage_27;
  reg              stage_28;
  reg              stage_29;
  reg              stage_3;
  reg              stage_30;
  reg              stage_31;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  reg              stage_7;
  reg              stage_8;
  reg              stage_9;
  wire    [  6: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_31;
  assign empty = !full_0;
  assign full_32 = 0;
  //data_31, which is an e_mux
  assign p31_stage_31 = ((full_32 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_31 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_31))
          if (sync_reset & full_31 & !((full_32 == 0) & read & write))
              stage_31 <= 0;
          else 
            stage_31 <= p31_stage_31;
    end


  //control_31, which is an e_mux
  assign p31_full_31 = ((read & !write) == 0)? full_30 :
    0;

  //control_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_31 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_31 <= 0;
          else 
            full_31 <= p31_full_31;
    end


  //data_30, which is an e_mux
  assign p30_stage_30 = ((full_31 & ~clear_fifo) == 0)? data_in :
    stage_31;

  //data_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_30 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_30))
          if (sync_reset & full_30 & !((full_31 == 0) & read & write))
              stage_30 <= 0;
          else 
            stage_30 <= p30_stage_30;
    end


  //control_30, which is an e_mux
  assign p30_full_30 = ((read & !write) == 0)? full_29 :
    full_31;

  //control_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_30 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_30 <= 0;
          else 
            full_30 <= p30_full_30;
    end


  //data_29, which is an e_mux
  assign p29_stage_29 = ((full_30 & ~clear_fifo) == 0)? data_in :
    stage_30;

  //data_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_29 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_29))
          if (sync_reset & full_29 & !((full_30 == 0) & read & write))
              stage_29 <= 0;
          else 
            stage_29 <= p29_stage_29;
    end


  //control_29, which is an e_mux
  assign p29_full_29 = ((read & !write) == 0)? full_28 :
    full_30;

  //control_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_29 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_29 <= 0;
          else 
            full_29 <= p29_full_29;
    end


  //data_28, which is an e_mux
  assign p28_stage_28 = ((full_29 & ~clear_fifo) == 0)? data_in :
    stage_29;

  //data_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_28 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_28))
          if (sync_reset & full_28 & !((full_29 == 0) & read & write))
              stage_28 <= 0;
          else 
            stage_28 <= p28_stage_28;
    end


  //control_28, which is an e_mux
  assign p28_full_28 = ((read & !write) == 0)? full_27 :
    full_29;

  //control_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_28 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_28 <= 0;
          else 
            full_28 <= p28_full_28;
    end


  //data_27, which is an e_mux
  assign p27_stage_27 = ((full_28 & ~clear_fifo) == 0)? data_in :
    stage_28;

  //data_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_27 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_27))
          if (sync_reset & full_27 & !((full_28 == 0) & read & write))
              stage_27 <= 0;
          else 
            stage_27 <= p27_stage_27;
    end


  //control_27, which is an e_mux
  assign p27_full_27 = ((read & !write) == 0)? full_26 :
    full_28;

  //control_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_27 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_27 <= 0;
          else 
            full_27 <= p27_full_27;
    end


  //data_26, which is an e_mux
  assign p26_stage_26 = ((full_27 & ~clear_fifo) == 0)? data_in :
    stage_27;

  //data_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_26 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_26))
          if (sync_reset & full_26 & !((full_27 == 0) & read & write))
              stage_26 <= 0;
          else 
            stage_26 <= p26_stage_26;
    end


  //control_26, which is an e_mux
  assign p26_full_26 = ((read & !write) == 0)? full_25 :
    full_27;

  //control_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_26 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_26 <= 0;
          else 
            full_26 <= p26_full_26;
    end


  //data_25, which is an e_mux
  assign p25_stage_25 = ((full_26 & ~clear_fifo) == 0)? data_in :
    stage_26;

  //data_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_25 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_25))
          if (sync_reset & full_25 & !((full_26 == 0) & read & write))
              stage_25 <= 0;
          else 
            stage_25 <= p25_stage_25;
    end


  //control_25, which is an e_mux
  assign p25_full_25 = ((read & !write) == 0)? full_24 :
    full_26;

  //control_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_25 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_25 <= 0;
          else 
            full_25 <= p25_full_25;
    end


  //data_24, which is an e_mux
  assign p24_stage_24 = ((full_25 & ~clear_fifo) == 0)? data_in :
    stage_25;

  //data_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_24 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_24))
          if (sync_reset & full_24 & !((full_25 == 0) & read & write))
              stage_24 <= 0;
          else 
            stage_24 <= p24_stage_24;
    end


  //control_24, which is an e_mux
  assign p24_full_24 = ((read & !write) == 0)? full_23 :
    full_25;

  //control_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_24 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_24 <= 0;
          else 
            full_24 <= p24_full_24;
    end


  //data_23, which is an e_mux
  assign p23_stage_23 = ((full_24 & ~clear_fifo) == 0)? data_in :
    stage_24;

  //data_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_23 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_23))
          if (sync_reset & full_23 & !((full_24 == 0) & read & write))
              stage_23 <= 0;
          else 
            stage_23 <= p23_stage_23;
    end


  //control_23, which is an e_mux
  assign p23_full_23 = ((read & !write) == 0)? full_22 :
    full_24;

  //control_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_23 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_23 <= 0;
          else 
            full_23 <= p23_full_23;
    end


  //data_22, which is an e_mux
  assign p22_stage_22 = ((full_23 & ~clear_fifo) == 0)? data_in :
    stage_23;

  //data_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_22 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_22))
          if (sync_reset & full_22 & !((full_23 == 0) & read & write))
              stage_22 <= 0;
          else 
            stage_22 <= p22_stage_22;
    end


  //control_22, which is an e_mux
  assign p22_full_22 = ((read & !write) == 0)? full_21 :
    full_23;

  //control_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_22 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_22 <= 0;
          else 
            full_22 <= p22_full_22;
    end


  //data_21, which is an e_mux
  assign p21_stage_21 = ((full_22 & ~clear_fifo) == 0)? data_in :
    stage_22;

  //data_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_21 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_21))
          if (sync_reset & full_21 & !((full_22 == 0) & read & write))
              stage_21 <= 0;
          else 
            stage_21 <= p21_stage_21;
    end


  //control_21, which is an e_mux
  assign p21_full_21 = ((read & !write) == 0)? full_20 :
    full_22;

  //control_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_21 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_21 <= 0;
          else 
            full_21 <= p21_full_21;
    end


  //data_20, which is an e_mux
  assign p20_stage_20 = ((full_21 & ~clear_fifo) == 0)? data_in :
    stage_21;

  //data_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_20 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_20))
          if (sync_reset & full_20 & !((full_21 == 0) & read & write))
              stage_20 <= 0;
          else 
            stage_20 <= p20_stage_20;
    end


  //control_20, which is an e_mux
  assign p20_full_20 = ((read & !write) == 0)? full_19 :
    full_21;

  //control_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_20 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_20 <= 0;
          else 
            full_20 <= p20_full_20;
    end


  //data_19, which is an e_mux
  assign p19_stage_19 = ((full_20 & ~clear_fifo) == 0)? data_in :
    stage_20;

  //data_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_19 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_19))
          if (sync_reset & full_19 & !((full_20 == 0) & read & write))
              stage_19 <= 0;
          else 
            stage_19 <= p19_stage_19;
    end


  //control_19, which is an e_mux
  assign p19_full_19 = ((read & !write) == 0)? full_18 :
    full_20;

  //control_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_19 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_19 <= 0;
          else 
            full_19 <= p19_full_19;
    end


  //data_18, which is an e_mux
  assign p18_stage_18 = ((full_19 & ~clear_fifo) == 0)? data_in :
    stage_19;

  //data_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_18 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_18))
          if (sync_reset & full_18 & !((full_19 == 0) & read & write))
              stage_18 <= 0;
          else 
            stage_18 <= p18_stage_18;
    end


  //control_18, which is an e_mux
  assign p18_full_18 = ((read & !write) == 0)? full_17 :
    full_19;

  //control_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_18 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_18 <= 0;
          else 
            full_18 <= p18_full_18;
    end


  //data_17, which is an e_mux
  assign p17_stage_17 = ((full_18 & ~clear_fifo) == 0)? data_in :
    stage_18;

  //data_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_17 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_17))
          if (sync_reset & full_17 & !((full_18 == 0) & read & write))
              stage_17 <= 0;
          else 
            stage_17 <= p17_stage_17;
    end


  //control_17, which is an e_mux
  assign p17_full_17 = ((read & !write) == 0)? full_16 :
    full_18;

  //control_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_17 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_17 <= 0;
          else 
            full_17 <= p17_full_17;
    end


  //data_16, which is an e_mux
  assign p16_stage_16 = ((full_17 & ~clear_fifo) == 0)? data_in :
    stage_17;

  //data_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_16 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_16))
          if (sync_reset & full_16 & !((full_17 == 0) & read & write))
              stage_16 <= 0;
          else 
            stage_16 <= p16_stage_16;
    end


  //control_16, which is an e_mux
  assign p16_full_16 = ((read & !write) == 0)? full_15 :
    full_17;

  //control_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_16 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_16 <= 0;
          else 
            full_16 <= p16_full_16;
    end


  //data_15, which is an e_mux
  assign p15_stage_15 = ((full_16 & ~clear_fifo) == 0)? data_in :
    stage_16;

  //data_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_15 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_15))
          if (sync_reset & full_15 & !((full_16 == 0) & read & write))
              stage_15 <= 0;
          else 
            stage_15 <= p15_stage_15;
    end


  //control_15, which is an e_mux
  assign p15_full_15 = ((read & !write) == 0)? full_14 :
    full_16;

  //control_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_15 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_15 <= 0;
          else 
            full_15 <= p15_full_15;
    end


  //data_14, which is an e_mux
  assign p14_stage_14 = ((full_15 & ~clear_fifo) == 0)? data_in :
    stage_15;

  //data_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_14 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_14))
          if (sync_reset & full_14 & !((full_15 == 0) & read & write))
              stage_14 <= 0;
          else 
            stage_14 <= p14_stage_14;
    end


  //control_14, which is an e_mux
  assign p14_full_14 = ((read & !write) == 0)? full_13 :
    full_15;

  //control_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_14 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_14 <= 0;
          else 
            full_14 <= p14_full_14;
    end


  //data_13, which is an e_mux
  assign p13_stage_13 = ((full_14 & ~clear_fifo) == 0)? data_in :
    stage_14;

  //data_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_13 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_13))
          if (sync_reset & full_13 & !((full_14 == 0) & read & write))
              stage_13 <= 0;
          else 
            stage_13 <= p13_stage_13;
    end


  //control_13, which is an e_mux
  assign p13_full_13 = ((read & !write) == 0)? full_12 :
    full_14;

  //control_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_13 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_13 <= 0;
          else 
            full_13 <= p13_full_13;
    end


  //data_12, which is an e_mux
  assign p12_stage_12 = ((full_13 & ~clear_fifo) == 0)? data_in :
    stage_13;

  //data_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_12 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_12))
          if (sync_reset & full_12 & !((full_13 == 0) & read & write))
              stage_12 <= 0;
          else 
            stage_12 <= p12_stage_12;
    end


  //control_12, which is an e_mux
  assign p12_full_12 = ((read & !write) == 0)? full_11 :
    full_13;

  //control_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_12 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_12 <= 0;
          else 
            full_12 <= p12_full_12;
    end


  //data_11, which is an e_mux
  assign p11_stage_11 = ((full_12 & ~clear_fifo) == 0)? data_in :
    stage_12;

  //data_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_11 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_11))
          if (sync_reset & full_11 & !((full_12 == 0) & read & write))
              stage_11 <= 0;
          else 
            stage_11 <= p11_stage_11;
    end


  //control_11, which is an e_mux
  assign p11_full_11 = ((read & !write) == 0)? full_10 :
    full_12;

  //control_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_11 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_11 <= 0;
          else 
            full_11 <= p11_full_11;
    end


  //data_10, which is an e_mux
  assign p10_stage_10 = ((full_11 & ~clear_fifo) == 0)? data_in :
    stage_11;

  //data_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_10 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_10))
          if (sync_reset & full_10 & !((full_11 == 0) & read & write))
              stage_10 <= 0;
          else 
            stage_10 <= p10_stage_10;
    end


  //control_10, which is an e_mux
  assign p10_full_10 = ((read & !write) == 0)? full_9 :
    full_11;

  //control_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_10 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_10 <= 0;
          else 
            full_10 <= p10_full_10;
    end


  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    stage_10;

  //data_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_9 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_9))
          if (sync_reset & full_9 & !((full_10 == 0) & read & write))
              stage_9 <= 0;
          else 
            stage_9 <= p9_stage_9;
    end


  //control_9, which is an e_mux
  assign p9_full_9 = ((read & !write) == 0)? full_8 :
    full_10;

  //control_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_9 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_9 <= 0;
          else 
            full_9 <= p9_full_9;
    end


  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    stage_9;

  //data_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_8 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_8))
          if (sync_reset & full_8 & !((full_9 == 0) & read & write))
              stage_8 <= 0;
          else 
            stage_8 <= p8_stage_8;
    end


  //control_8, which is an e_mux
  assign p8_full_8 = ((read & !write) == 0)? full_7 :
    full_9;

  //control_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_8 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_8 <= 0;
          else 
            full_8 <= p8_full_8;
    end


  //data_7, which is an e_mux
  assign p7_stage_7 = ((full_8 & ~clear_fifo) == 0)? data_in :
    stage_8;

  //data_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_7 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_7))
          if (sync_reset & full_7 & !((full_8 == 0) & read & write))
              stage_7 <= 0;
          else 
            stage_7 <= p7_stage_7;
    end


  //control_7, which is an e_mux
  assign p7_full_7 = ((read & !write) == 0)? full_6 :
    full_8;

  //control_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_7 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_7 <= 0;
          else 
            full_7 <= p7_full_7;
    end


  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    stage_7;

  //data_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_6 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_6))
          if (sync_reset & full_6 & !((full_7 == 0) & read & write))
              stage_6 <= 0;
          else 
            stage_6 <= p6_stage_6;
    end


  //control_6, which is an e_mux
  assign p6_full_6 = ((read & !write) == 0)? full_5 :
    full_7;

  //control_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_6 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_6 <= 0;
          else 
            full_6 <= p6_full_6;
    end


  //data_5, which is an e_mux
  assign p5_stage_5 = ((full_6 & ~clear_fifo) == 0)? data_in :
    stage_6;

  //data_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_5 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_5))
          if (sync_reset & full_5 & !((full_6 == 0) & read & write))
              stage_5 <= 0;
          else 
            stage_5 <= p5_stage_5;
    end


  //control_5, which is an e_mux
  assign p5_full_5 = ((read & !write) == 0)? full_4 :
    full_6;

  //control_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_5 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_5 <= 0;
          else 
            full_5 <= p5_full_5;
    end


  //data_4, which is an e_mux
  assign p4_stage_4 = ((full_5 & ~clear_fifo) == 0)? data_in :
    stage_5;

  //data_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_4 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_4))
          if (sync_reset & full_4 & !((full_5 == 0) & read & write))
              stage_4 <= 0;
          else 
            stage_4 <= p4_stage_4;
    end


  //control_4, which is an e_mux
  assign p4_full_4 = ((read & !write) == 0)? full_3 :
    full_5;

  //control_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_4 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_4 <= 0;
          else 
            full_4 <= p4_full_4;
    end


  //data_3, which is an e_mux
  assign p3_stage_3 = ((full_4 & ~clear_fifo) == 0)? data_in :
    stage_4;

  //data_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_3 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_3))
          if (sync_reset & full_3 & !((full_4 == 0) & read & write))
              stage_3 <= 0;
          else 
            stage_3 <= p3_stage_3;
    end


  //control_3, which is an e_mux
  assign p3_full_3 = ((read & !write) == 0)? full_2 :
    full_4;

  //control_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_3 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_3 <= 0;
          else 
            full_3 <= p3_full_3;
    end


  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    stage_3;

  //data_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_2 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_2))
          if (sync_reset & full_2 & !((full_3 == 0) & read & write))
              stage_2 <= 0;
          else 
            stage_2 <= p2_stage_2;
    end


  //control_2, which is an e_mux
  assign p2_full_2 = ((read & !write) == 0)? full_1 :
    full_3;

  //control_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_2 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_2 <= 0;
          else 
            full_2 <= p2_full_2;
    end


  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    stage_2;

  //data_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_1 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_1))
          if (sync_reset & full_1 & !((full_2 == 0) & read & write))
              stage_1 <= 0;
          else 
            stage_1 <= p1_stage_1;
    end


  //control_1, which is an e_mux
  assign p1_full_1 = ((read & !write) == 0)? full_0 :
    full_2;

  //control_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_1 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_1 <= 0;
          else 
            full_1 <= p1_full_1;
    end


  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    stage_1;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    full_1;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module ddr2_s1_arbitrator (
                            // inputs:
                             clk,
                             ddr2_s1_readdata,
                             ddr2_s1_readdatavalid,
                             ddr2_s1_resetrequest_n,
                             ddr2_s1_waitrequest_n,
                             jtag_to_ava_master_bridge_latency_counter,
                             jtag_to_ava_master_bridge_master_address_to_slave,
                             jtag_to_ava_master_bridge_master_byteenable,
                             jtag_to_ava_master_bridge_master_read,
                             jtag_to_ava_master_bridge_master_write,
                             jtag_to_ava_master_bridge_master_writedata,
                             pipeline_bridge_MEMORY_m1_address_to_slave,
                             pipeline_bridge_MEMORY_m1_burstcount,
                             pipeline_bridge_MEMORY_m1_byteenable,
                             pipeline_bridge_MEMORY_m1_chipselect,
                             pipeline_bridge_MEMORY_m1_latency_counter,
                             pipeline_bridge_MEMORY_m1_read,
                             pipeline_bridge_MEMORY_m1_write,
                             pipeline_bridge_MEMORY_m1_writedata,
                             reset_n,

                            // outputs:
                             d1_ddr2_s1_end_xfer,
                             ddr2_s1_address,
                             ddr2_s1_beginbursttransfer,
                             ddr2_s1_burstcount,
                             ddr2_s1_byteenable,
                             ddr2_s1_read,
                             ddr2_s1_readdata_from_sa,
                             ddr2_s1_resetrequest_n_from_sa,
                             ddr2_s1_waitrequest_n_from_sa,
                             ddr2_s1_write,
                             ddr2_s1_writedata,
                             jtag_to_ava_master_bridge_granted_ddr2_s1,
                             jtag_to_ava_master_bridge_qualified_request_ddr2_s1,
                             jtag_to_ava_master_bridge_read_data_valid_ddr2_s1,
                             jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register,
                             jtag_to_ava_master_bridge_requests_ddr2_s1,
                             pipeline_bridge_MEMORY_m1_granted_ddr2_s1,
                             pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1,
                             pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1,
                             pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register,
                             pipeline_bridge_MEMORY_m1_requests_ddr2_s1
                          )
;

  output           d1_ddr2_s1_end_xfer;
  output  [ 24: 0] ddr2_s1_address;
  output           ddr2_s1_beginbursttransfer;
  output  [  6: 0] ddr2_s1_burstcount;
  output  [ 31: 0] ddr2_s1_byteenable;
  output           ddr2_s1_read;
  output  [255: 0] ddr2_s1_readdata_from_sa;
  output           ddr2_s1_resetrequest_n_from_sa;
  output           ddr2_s1_waitrequest_n_from_sa;
  output           ddr2_s1_write;
  output  [255: 0] ddr2_s1_writedata;
  output           jtag_to_ava_master_bridge_granted_ddr2_s1;
  output           jtag_to_ava_master_bridge_qualified_request_ddr2_s1;
  output           jtag_to_ava_master_bridge_read_data_valid_ddr2_s1;
  output           jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register;
  output           jtag_to_ava_master_bridge_requests_ddr2_s1;
  output           pipeline_bridge_MEMORY_m1_granted_ddr2_s1;
  output           pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register;
  output           pipeline_bridge_MEMORY_m1_requests_ddr2_s1;
  input            clk;
  input   [255: 0] ddr2_s1_readdata;
  input            ddr2_s1_readdatavalid;
  input            ddr2_s1_resetrequest_n;
  input            ddr2_s1_waitrequest_n;
  input            jtag_to_ava_master_bridge_latency_counter;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  input   [  3: 0] jtag_to_ava_master_bridge_master_byteenable;
  input            jtag_to_ava_master_bridge_master_read;
  input            jtag_to_ava_master_bridge_master_write;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_writedata;
  input   [ 30: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  input   [  6: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [ 31: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input            pipeline_bridge_MEMORY_m1_latency_counter;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_write;
  input   [255: 0] pipeline_bridge_MEMORY_m1_writedata;
  input            reset_n;

  reg              d1_ddr2_s1_end_xfer;
  reg              d1_reasons_to_wait;
  wire    [ 24: 0] ddr2_s1_address;
  wire             ddr2_s1_allgrants;
  wire             ddr2_s1_allow_new_arb_cycle;
  wire             ddr2_s1_any_bursting_master_saved_grant;
  wire             ddr2_s1_any_continuerequest;
  reg     [  1: 0] ddr2_s1_arb_addend;
  wire             ddr2_s1_arb_counter_enable;
  reg     [  6: 0] ddr2_s1_arb_share_counter;
  wire    [  6: 0] ddr2_s1_arb_share_counter_next_value;
  wire    [  6: 0] ddr2_s1_arb_share_set_values;
  wire    [  1: 0] ddr2_s1_arb_winner;
  wire             ddr2_s1_arbitration_holdoff_internal;
  reg     [  5: 0] ddr2_s1_bbt_burstcounter;
  wire             ddr2_s1_beginbursttransfer;
  wire             ddr2_s1_beginbursttransfer_internal;
  wire             ddr2_s1_begins_xfer;
  wire    [  6: 0] ddr2_s1_burstcount;
  wire             ddr2_s1_burstcount_fifo_empty;
  wire    [ 31: 0] ddr2_s1_byteenable;
  wire    [  3: 0] ddr2_s1_chosen_master_double_vector;
  wire    [  1: 0] ddr2_s1_chosen_master_rot_left;
  reg     [  6: 0] ddr2_s1_current_burst;
  wire    [  6: 0] ddr2_s1_current_burst_minus_one;
  wire             ddr2_s1_end_xfer;
  wire             ddr2_s1_firsttransfer;
  wire    [  1: 0] ddr2_s1_grant_vector;
  wire             ddr2_s1_in_a_read_cycle;
  wire             ddr2_s1_in_a_write_cycle;
  reg              ddr2_s1_load_fifo;
  wire    [  1: 0] ddr2_s1_master_qreq_vector;
  wire             ddr2_s1_move_on_to_next_transaction;
  wire    [  5: 0] ddr2_s1_next_bbt_burstcount;
  wire    [  6: 0] ddr2_s1_next_burst_count;
  wire             ddr2_s1_non_bursting_master_requests;
  wire             ddr2_s1_read;
  wire    [255: 0] ddr2_s1_readdata_from_sa;
  wire             ddr2_s1_readdatavalid_from_sa;
  reg              ddr2_s1_reg_firsttransfer;
  wire             ddr2_s1_resetrequest_n_from_sa;
  reg     [  1: 0] ddr2_s1_saved_chosen_master_vector;
  wire    [  6: 0] ddr2_s1_selected_burstcount;
  reg              ddr2_s1_slavearbiterlockenable;
  wire             ddr2_s1_slavearbiterlockenable2;
  wire             ddr2_s1_this_cycle_is_the_last_burst;
  wire    [  6: 0] ddr2_s1_transaction_burst_count;
  wire             ddr2_s1_unreg_firsttransfer;
  wire             ddr2_s1_waitrequest_n_from_sa;
  wire             ddr2_s1_waits_for_read;
  wire             ddr2_s1_waits_for_write;
  wire             ddr2_s1_write;
  wire    [255: 0] ddr2_s1_writedata;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_ddr2_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire    [ 31: 0] jtag_to_ava_master_bridge_byteenable_ddr2_s1;
  wire             jtag_to_ava_master_bridge_granted_ddr2_s1;
  wire             jtag_to_ava_master_bridge_master_arbiterlock;
  wire             jtag_to_ava_master_bridge_master_arbiterlock2;
  wire             jtag_to_ava_master_bridge_master_continuerequest;
  wire    [255: 0] jtag_to_ava_master_bridge_master_writedata_replicated;
  wire             jtag_to_ava_master_bridge_qualified_request_ddr2_s1;
  wire             jtag_to_ava_master_bridge_rdv_fifo_empty_ddr2_s1;
  wire             jtag_to_ava_master_bridge_rdv_fifo_output_from_ddr2_s1;
  wire             jtag_to_ava_master_bridge_read_data_valid_ddr2_s1;
  wire             jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register;
  wire             jtag_to_ava_master_bridge_requests_ddr2_s1;
  wire             jtag_to_ava_master_bridge_saved_grant_ddr2_s1;
  reg              last_cycle_jtag_to_ava_master_bridge_master_granted_slave_ddr2_s1;
  reg              last_cycle_pipeline_bridge_MEMORY_m1_granted_slave_ddr2_s1;
  wire             p0_ddr2_s1_load_fifo;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock2;
  wire             pipeline_bridge_MEMORY_m1_continuerequest;
  wire             pipeline_bridge_MEMORY_m1_granted_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_empty_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register;
  wire             pipeline_bridge_MEMORY_m1_requests_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_saved_grant_ddr2_s1;
  wire    [ 31: 0] shifted_address_to_ddr2_s1_from_jtag_to_ava_master_bridge_master;
  wire    [ 30: 0] shifted_address_to_ddr2_s1_from_pipeline_bridge_MEMORY_m1;
  wire             wait_for_ddr2_s1_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~ddr2_s1_end_xfer;
    end


  assign ddr2_s1_begins_xfer = ~d1_reasons_to_wait & ((jtag_to_ava_master_bridge_qualified_request_ddr2_s1 | pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1));
  //assign ddr2_s1_readdata_from_sa = ddr2_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign ddr2_s1_readdata_from_sa = ddr2_s1_readdata;

  assign jtag_to_ava_master_bridge_requests_ddr2_s1 = ({jtag_to_ava_master_bridge_master_address_to_slave[31 : 30] , 30'b0} == 32'h40000000) & (jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write);
  //assign ddr2_s1_waitrequest_n_from_sa = ddr2_s1_waitrequest_n so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign ddr2_s1_waitrequest_n_from_sa = ddr2_s1_waitrequest_n;

  //assign ddr2_s1_readdatavalid_from_sa = ddr2_s1_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign ddr2_s1_readdatavalid_from_sa = ddr2_s1_readdatavalid;

  //ddr2_s1_arb_share_counter set values, which is an e_mux
  assign ddr2_s1_arb_share_set_values = (pipeline_bridge_MEMORY_m1_granted_ddr2_s1)? ((((pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect)) ? pipeline_bridge_MEMORY_m1_burstcount : 1)) :
    (pipeline_bridge_MEMORY_m1_granted_ddr2_s1)? ((((pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect)) ? pipeline_bridge_MEMORY_m1_burstcount : 1)) :
    1;

  //ddr2_s1_non_bursting_master_requests mux, which is an e_mux
  assign ddr2_s1_non_bursting_master_requests = jtag_to_ava_master_bridge_requests_ddr2_s1 |
    jtag_to_ava_master_bridge_requests_ddr2_s1;

  //ddr2_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign ddr2_s1_any_bursting_master_saved_grant = 0 |
    pipeline_bridge_MEMORY_m1_saved_grant_ddr2_s1 |
    pipeline_bridge_MEMORY_m1_saved_grant_ddr2_s1;

  //ddr2_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign ddr2_s1_arb_share_counter_next_value = ddr2_s1_firsttransfer ? (ddr2_s1_arb_share_set_values - 1) : |ddr2_s1_arb_share_counter ? (ddr2_s1_arb_share_counter - 1) : 0;

  //ddr2_s1_allgrants all slave grants, which is an e_mux
  assign ddr2_s1_allgrants = (|ddr2_s1_grant_vector) |
    (|ddr2_s1_grant_vector) |
    (|ddr2_s1_grant_vector) |
    (|ddr2_s1_grant_vector);

  //ddr2_s1_end_xfer assignment, which is an e_assign
  assign ddr2_s1_end_xfer = ~(ddr2_s1_waits_for_read | ddr2_s1_waits_for_write);

  //end_xfer_arb_share_counter_term_ddr2_s1 arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_ddr2_s1 = ddr2_s1_end_xfer & (~ddr2_s1_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //ddr2_s1_arb_share_counter arbitration counter enable, which is an e_assign
  assign ddr2_s1_arb_counter_enable = (end_xfer_arb_share_counter_term_ddr2_s1 & ddr2_s1_allgrants) | (end_xfer_arb_share_counter_term_ddr2_s1 & ~ddr2_s1_non_bursting_master_requests);

  //ddr2_s1_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_arb_share_counter <= 0;
      else if (ddr2_s1_arb_counter_enable)
          ddr2_s1_arb_share_counter <= ddr2_s1_arb_share_counter_next_value;
    end


  //ddr2_s1_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_slavearbiterlockenable <= 0;
      else if ((|ddr2_s1_master_qreq_vector & end_xfer_arb_share_counter_term_ddr2_s1) | (end_xfer_arb_share_counter_term_ddr2_s1 & ~ddr2_s1_non_bursting_master_requests))
          ddr2_s1_slavearbiterlockenable <= |ddr2_s1_arb_share_counter_next_value;
    end


  //jtag_to_ava_master_bridge/master ddr2/s1 arbiterlock, which is an e_assign
  assign jtag_to_ava_master_bridge_master_arbiterlock = ddr2_s1_slavearbiterlockenable & jtag_to_ava_master_bridge_master_continuerequest;

  //ddr2_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign ddr2_s1_slavearbiterlockenable2 = |ddr2_s1_arb_share_counter_next_value;

  //jtag_to_ava_master_bridge/master ddr2/s1 arbiterlock2, which is an e_assign
  assign jtag_to_ava_master_bridge_master_arbiterlock2 = ddr2_s1_slavearbiterlockenable2 & jtag_to_ava_master_bridge_master_continuerequest;

  //pipeline_bridge_MEMORY/m1 ddr2/s1 arbiterlock, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock = ddr2_s1_slavearbiterlockenable & pipeline_bridge_MEMORY_m1_continuerequest;

  //pipeline_bridge_MEMORY/m1 ddr2/s1 arbiterlock2, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock2 = ddr2_s1_slavearbiterlockenable2 & pipeline_bridge_MEMORY_m1_continuerequest;

  //pipeline_bridge_MEMORY/m1 granted ddr2/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_pipeline_bridge_MEMORY_m1_granted_slave_ddr2_s1 <= 0;
      else 
        last_cycle_pipeline_bridge_MEMORY_m1_granted_slave_ddr2_s1 <= pipeline_bridge_MEMORY_m1_saved_grant_ddr2_s1 ? 1 : (ddr2_s1_arbitration_holdoff_internal | ~pipeline_bridge_MEMORY_m1_requests_ddr2_s1) ? 0 : last_cycle_pipeline_bridge_MEMORY_m1_granted_slave_ddr2_s1;
    end


  //pipeline_bridge_MEMORY_m1_continuerequest continued request, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_continuerequest = last_cycle_pipeline_bridge_MEMORY_m1_granted_slave_ddr2_s1 & 1;

  //ddr2_s1_any_continuerequest at least one master continues requesting, which is an e_mux
  assign ddr2_s1_any_continuerequest = pipeline_bridge_MEMORY_m1_continuerequest |
    jtag_to_ava_master_bridge_master_continuerequest;

  assign jtag_to_ava_master_bridge_qualified_request_ddr2_s1 = jtag_to_ava_master_bridge_requests_ddr2_s1 & ~((jtag_to_ava_master_bridge_master_read & ((jtag_to_ava_master_bridge_latency_counter != 0) | (1 < jtag_to_ava_master_bridge_latency_counter))) | pipeline_bridge_MEMORY_m1_arbiterlock);
  //unique name for ddr2_s1_move_on_to_next_transaction, which is an e_assign
  assign ddr2_s1_move_on_to_next_transaction = ddr2_s1_this_cycle_is_the_last_burst & ddr2_s1_load_fifo;

  //rdv_fifo_for_jtag_to_ava_master_bridge_master_to_ddr2_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_jtag_to_ava_master_bridge_master_to_ddr2_s1_module rdv_fifo_for_jtag_to_ava_master_bridge_master_to_ddr2_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (jtag_to_ava_master_bridge_granted_ddr2_s1),
      .data_out             (jtag_to_ava_master_bridge_rdv_fifo_output_from_ddr2_s1),
      .empty                (),
      .fifo_contains_ones_n (jtag_to_ava_master_bridge_rdv_fifo_empty_ddr2_s1),
      .full                 (),
      .read                 (ddr2_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~ddr2_s1_waits_for_read)
    );

  assign jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register = ~jtag_to_ava_master_bridge_rdv_fifo_empty_ddr2_s1;
  //local readdatavalid jtag_to_ava_master_bridge_read_data_valid_ddr2_s1, which is an e_mux
  assign jtag_to_ava_master_bridge_read_data_valid_ddr2_s1 = (ddr2_s1_readdatavalid_from_sa & jtag_to_ava_master_bridge_rdv_fifo_output_from_ddr2_s1) & ~ jtag_to_ava_master_bridge_rdv_fifo_empty_ddr2_s1;

  //replicate narrow data for wide slave
  assign jtag_to_ava_master_bridge_master_writedata_replicated = {jtag_to_ava_master_bridge_master_writedata,
    jtag_to_ava_master_bridge_master_writedata,
    jtag_to_ava_master_bridge_master_writedata,
    jtag_to_ava_master_bridge_master_writedata,
    jtag_to_ava_master_bridge_master_writedata,
    jtag_to_ava_master_bridge_master_writedata,
    jtag_to_ava_master_bridge_master_writedata,
    jtag_to_ava_master_bridge_master_writedata};

  //ddr2_s1_writedata mux, which is an e_mux
  assign ddr2_s1_writedata = (jtag_to_ava_master_bridge_granted_ddr2_s1)? jtag_to_ava_master_bridge_master_writedata_replicated :
    pipeline_bridge_MEMORY_m1_writedata;

  assign pipeline_bridge_MEMORY_m1_requests_ddr2_s1 = ({pipeline_bridge_MEMORY_m1_address_to_slave[30] , 30'b0} == 31'h40000000) & pipeline_bridge_MEMORY_m1_chipselect;
  //jtag_to_ava_master_bridge/master granted ddr2/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_jtag_to_ava_master_bridge_master_granted_slave_ddr2_s1 <= 0;
      else 
        last_cycle_jtag_to_ava_master_bridge_master_granted_slave_ddr2_s1 <= jtag_to_ava_master_bridge_saved_grant_ddr2_s1 ? 1 : (ddr2_s1_arbitration_holdoff_internal | 0) ? 0 : last_cycle_jtag_to_ava_master_bridge_master_granted_slave_ddr2_s1;
    end


  //jtag_to_ava_master_bridge_master_continuerequest continued request, which is an e_mux
  assign jtag_to_ava_master_bridge_master_continuerequest = last_cycle_jtag_to_ava_master_bridge_master_granted_slave_ddr2_s1 & jtag_to_ava_master_bridge_requests_ddr2_s1;

  assign pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1 = pipeline_bridge_MEMORY_m1_requests_ddr2_s1 & ~(((pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & ((pipeline_bridge_MEMORY_m1_latency_counter != 0) | (1 < pipeline_bridge_MEMORY_m1_latency_counter))) | jtag_to_ava_master_bridge_master_arbiterlock);
  //the currently selected burstcount for ddr2_s1, which is an e_mux
  assign ddr2_s1_selected_burstcount = (pipeline_bridge_MEMORY_m1_granted_ddr2_s1)? pipeline_bridge_MEMORY_m1_burstcount :
    1;

  //burstcount_fifo_for_ddr2_s1, which is an e_fifo_with_registered_outputs
  burstcount_fifo_for_ddr2_s1_module burstcount_fifo_for_ddr2_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (ddr2_s1_selected_burstcount),
      .data_out             (ddr2_s1_transaction_burst_count),
      .empty                (ddr2_s1_burstcount_fifo_empty),
      .fifo_contains_ones_n (),
      .full                 (),
      .read                 (ddr2_s1_this_cycle_is_the_last_burst),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~ddr2_s1_waits_for_read & ddr2_s1_load_fifo & ~(ddr2_s1_this_cycle_is_the_last_burst & ddr2_s1_burstcount_fifo_empty))
    );

  //ddr2_s1 current burst minus one, which is an e_assign
  assign ddr2_s1_current_burst_minus_one = ddr2_s1_current_burst - 1;

  //what to load in current_burst, for ddr2_s1, which is an e_mux
  assign ddr2_s1_next_burst_count = (((in_a_read_cycle & ~ddr2_s1_waits_for_read) & ~ddr2_s1_load_fifo))? ddr2_s1_selected_burstcount :
    ((in_a_read_cycle & ~ddr2_s1_waits_for_read & ddr2_s1_this_cycle_is_the_last_burst & ddr2_s1_burstcount_fifo_empty))? ddr2_s1_selected_burstcount :
    (ddr2_s1_this_cycle_is_the_last_burst)? ddr2_s1_transaction_burst_count :
    ddr2_s1_current_burst_minus_one;

  //the current burst count for ddr2_s1, to be decremented, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_current_burst <= 0;
      else if (ddr2_s1_readdatavalid_from_sa | (~ddr2_s1_load_fifo & (in_a_read_cycle & ~ddr2_s1_waits_for_read)))
          ddr2_s1_current_burst <= ddr2_s1_next_burst_count;
    end


  //a 1 or burstcount fifo empty, to initialize the counter, which is an e_mux
  assign p0_ddr2_s1_load_fifo = (~ddr2_s1_load_fifo)? 1 :
    (((in_a_read_cycle & ~ddr2_s1_waits_for_read) & ddr2_s1_load_fifo))? 1 :
    ~ddr2_s1_burstcount_fifo_empty;

  //whether to load directly to the counter or to the fifo, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_load_fifo <= 0;
      else if ((in_a_read_cycle & ~ddr2_s1_waits_for_read) & ~ddr2_s1_load_fifo | ddr2_s1_this_cycle_is_the_last_burst)
          ddr2_s1_load_fifo <= p0_ddr2_s1_load_fifo;
    end


  //the last cycle in the burst for ddr2_s1, which is an e_assign
  assign ddr2_s1_this_cycle_is_the_last_burst = ~(|ddr2_s1_current_burst_minus_one) & ddr2_s1_readdatavalid_from_sa;

  //rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_ddr2_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_ddr2_s1_module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_ddr2_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (pipeline_bridge_MEMORY_m1_granted_ddr2_s1),
      .data_out             (pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_ddr2_s1),
      .empty                (),
      .fifo_contains_ones_n (pipeline_bridge_MEMORY_m1_rdv_fifo_empty_ddr2_s1),
      .full                 (),
      .read                 (ddr2_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~ddr2_s1_waits_for_read)
    );

  assign pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register = ~pipeline_bridge_MEMORY_m1_rdv_fifo_empty_ddr2_s1;
  //local readdatavalid pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1 = (ddr2_s1_readdatavalid_from_sa & pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_ddr2_s1) & ~ pipeline_bridge_MEMORY_m1_rdv_fifo_empty_ddr2_s1;

  //allow new arb cycle for ddr2/s1, which is an e_assign
  assign ddr2_s1_allow_new_arb_cycle = ~jtag_to_ava_master_bridge_master_arbiterlock & ~pipeline_bridge_MEMORY_m1_arbiterlock;

  //pipeline_bridge_MEMORY/m1 assignment into master qualified-requests vector for ddr2/s1, which is an e_assign
  assign ddr2_s1_master_qreq_vector[0] = pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1;

  //pipeline_bridge_MEMORY/m1 grant ddr2/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_granted_ddr2_s1 = ddr2_s1_grant_vector[0];

  //pipeline_bridge_MEMORY/m1 saved-grant ddr2/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_saved_grant_ddr2_s1 = ddr2_s1_arb_winner[0];

  //jtag_to_ava_master_bridge/master assignment into master qualified-requests vector for ddr2/s1, which is an e_assign
  assign ddr2_s1_master_qreq_vector[1] = jtag_to_ava_master_bridge_qualified_request_ddr2_s1;

  //jtag_to_ava_master_bridge/master grant ddr2/s1, which is an e_assign
  assign jtag_to_ava_master_bridge_granted_ddr2_s1 = ddr2_s1_grant_vector[1];

  //jtag_to_ava_master_bridge/master saved-grant ddr2/s1, which is an e_assign
  assign jtag_to_ava_master_bridge_saved_grant_ddr2_s1 = ddr2_s1_arb_winner[1] && jtag_to_ava_master_bridge_requests_ddr2_s1;

  //ddr2/s1 chosen-master double-vector, which is an e_assign
  assign ddr2_s1_chosen_master_double_vector = {ddr2_s1_master_qreq_vector, ddr2_s1_master_qreq_vector} & ({~ddr2_s1_master_qreq_vector, ~ddr2_s1_master_qreq_vector} + ddr2_s1_arb_addend);

  //stable onehot encoding of arb winner
  assign ddr2_s1_arb_winner = (ddr2_s1_allow_new_arb_cycle & | ddr2_s1_grant_vector) ? ddr2_s1_grant_vector : ddr2_s1_saved_chosen_master_vector;

  //saved ddr2_s1_grant_vector, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_saved_chosen_master_vector <= 0;
      else if (ddr2_s1_allow_new_arb_cycle)
          ddr2_s1_saved_chosen_master_vector <= |ddr2_s1_grant_vector ? ddr2_s1_grant_vector : ddr2_s1_saved_chosen_master_vector;
    end


  //onehot encoding of chosen master
  assign ddr2_s1_grant_vector = {(ddr2_s1_chosen_master_double_vector[1] | ddr2_s1_chosen_master_double_vector[3]),
    (ddr2_s1_chosen_master_double_vector[0] | ddr2_s1_chosen_master_double_vector[2])};

  //ddr2/s1 chosen master rotated left, which is an e_assign
  assign ddr2_s1_chosen_master_rot_left = (ddr2_s1_arb_winner << 1) ? (ddr2_s1_arb_winner << 1) : 1;

  //ddr2/s1's addend for next-master-grant
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_arb_addend <= 1;
      else if (|ddr2_s1_grant_vector)
          ddr2_s1_arb_addend <= ddr2_s1_end_xfer? ddr2_s1_chosen_master_rot_left : ddr2_s1_grant_vector;
    end


  //assign ddr2_s1_resetrequest_n_from_sa = ddr2_s1_resetrequest_n so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign ddr2_s1_resetrequest_n_from_sa = ddr2_s1_resetrequest_n;

  //ddr2_s1_firsttransfer first transaction, which is an e_assign
  assign ddr2_s1_firsttransfer = ddr2_s1_begins_xfer ? ddr2_s1_unreg_firsttransfer : ddr2_s1_reg_firsttransfer;

  //ddr2_s1_unreg_firsttransfer first transaction, which is an e_assign
  assign ddr2_s1_unreg_firsttransfer = ~(ddr2_s1_slavearbiterlockenable & ddr2_s1_any_continuerequest);

  //ddr2_s1_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_reg_firsttransfer <= 1'b1;
      else if (ddr2_s1_begins_xfer)
          ddr2_s1_reg_firsttransfer <= ddr2_s1_unreg_firsttransfer;
    end


  //ddr2_s1_next_bbt_burstcount next_bbt_burstcount, which is an e_mux
  assign ddr2_s1_next_bbt_burstcount = ((((ddr2_s1_write) && (ddr2_s1_bbt_burstcounter == 0))))? (ddr2_s1_burstcount - 1) :
    ((((ddr2_s1_read) && (ddr2_s1_bbt_burstcounter == 0))))? 0 :
    (ddr2_s1_bbt_burstcounter - 1);

  //ddr2_s1_bbt_burstcounter bbt_burstcounter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          ddr2_s1_bbt_burstcounter <= 0;
      else if (ddr2_s1_begins_xfer)
          ddr2_s1_bbt_burstcounter <= ddr2_s1_next_bbt_burstcount;
    end


  //ddr2_s1_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign ddr2_s1_beginbursttransfer_internal = ddr2_s1_begins_xfer & (ddr2_s1_bbt_burstcounter == 0);

  //ddr2/s1 begin burst transfer to slave, which is an e_assign
  assign ddr2_s1_beginbursttransfer = ddr2_s1_beginbursttransfer_internal;

  //ddr2_s1_arbitration_holdoff_internal arbitration_holdoff, which is an e_assign
  assign ddr2_s1_arbitration_holdoff_internal = ddr2_s1_begins_xfer & ddr2_s1_firsttransfer;

  //ddr2_s1_read assignment, which is an e_mux
  assign ddr2_s1_read = (jtag_to_ava_master_bridge_granted_ddr2_s1 & jtag_to_ava_master_bridge_master_read) | (pipeline_bridge_MEMORY_m1_granted_ddr2_s1 & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect));

  //ddr2_s1_write assignment, which is an e_mux
  assign ddr2_s1_write = (jtag_to_ava_master_bridge_granted_ddr2_s1 & jtag_to_ava_master_bridge_master_write) | (pipeline_bridge_MEMORY_m1_granted_ddr2_s1 & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect));

  assign shifted_address_to_ddr2_s1_from_jtag_to_ava_master_bridge_master = jtag_to_ava_master_bridge_master_address_to_slave;
  //ddr2_s1_address mux, which is an e_mux
  assign ddr2_s1_address = (jtag_to_ava_master_bridge_granted_ddr2_s1)? (shifted_address_to_ddr2_s1_from_jtag_to_ava_master_bridge_master >> 5) :
    (shifted_address_to_ddr2_s1_from_pipeline_bridge_MEMORY_m1 >> 5);

  assign shifted_address_to_ddr2_s1_from_pipeline_bridge_MEMORY_m1 = pipeline_bridge_MEMORY_m1_address_to_slave;
  //d1_ddr2_s1_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_ddr2_s1_end_xfer <= 1;
      else 
        d1_ddr2_s1_end_xfer <= ddr2_s1_end_xfer;
    end


  //ddr2_s1_waits_for_read in a cycle, which is an e_mux
  assign ddr2_s1_waits_for_read = ddr2_s1_in_a_read_cycle & ~ddr2_s1_waitrequest_n_from_sa;

  //ddr2_s1_in_a_read_cycle assignment, which is an e_assign
  assign ddr2_s1_in_a_read_cycle = (jtag_to_ava_master_bridge_granted_ddr2_s1 & jtag_to_ava_master_bridge_master_read) | (pipeline_bridge_MEMORY_m1_granted_ddr2_s1 & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect));

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = ddr2_s1_in_a_read_cycle;

  //ddr2_s1_waits_for_write in a cycle, which is an e_mux
  assign ddr2_s1_waits_for_write = ddr2_s1_in_a_write_cycle & ~ddr2_s1_waitrequest_n_from_sa;

  //ddr2_s1_in_a_write_cycle assignment, which is an e_assign
  assign ddr2_s1_in_a_write_cycle = (jtag_to_ava_master_bridge_granted_ddr2_s1 & jtag_to_ava_master_bridge_master_write) | (pipeline_bridge_MEMORY_m1_granted_ddr2_s1 & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect));

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = ddr2_s1_in_a_write_cycle;

  assign wait_for_ddr2_s1_counter = 0;
  //ddr2_s1_byteenable byte enable port mux, which is an e_mux
  assign ddr2_s1_byteenable = (jtag_to_ava_master_bridge_granted_ddr2_s1)? jtag_to_ava_master_bridge_byteenable_ddr2_s1 :
    (pipeline_bridge_MEMORY_m1_granted_ddr2_s1)? pipeline_bridge_MEMORY_m1_byteenable :
    -1;

  //byte_enable_mux for jtag_to_ava_master_bridge/master and ddr2/s1, which is an e_mux
  assign jtag_to_ava_master_bridge_byteenable_ddr2_s1 = (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2] == 0)? jtag_to_ava_master_bridge_master_byteenable :
    (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2] == 1)? {jtag_to_ava_master_bridge_master_byteenable, {4'b0}} :
    (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2] == 2)? {jtag_to_ava_master_bridge_master_byteenable, {8'b0}} :
    (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2] == 3)? {jtag_to_ava_master_bridge_master_byteenable, {12'b0}} :
    (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2] == 4)? {jtag_to_ava_master_bridge_master_byteenable, {16'b0}} :
    (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2] == 5)? {jtag_to_ava_master_bridge_master_byteenable, {20'b0}} :
    (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2] == 6)? {jtag_to_ava_master_bridge_master_byteenable, {24'b0}} :
    {jtag_to_ava_master_bridge_master_byteenable, {28'b0}};

  //burstcount mux, which is an e_mux
  assign ddr2_s1_burstcount = (pipeline_bridge_MEMORY_m1_granted_ddr2_s1)? pipeline_bridge_MEMORY_m1_burstcount :
    1;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //ddr2/s1 enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end


  //pipeline_bridge_MEMORY/m1 non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (pipeline_bridge_MEMORY_m1_requests_ddr2_s1 && (pipeline_bridge_MEMORY_m1_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: pipeline_bridge_MEMORY/m1 drove 0 on its 'burstcount' port while accessing slave ddr2/s1", $time);
          $stop;
        end
    end


  //grant signals are active simultaneously, which is an e_process
  always @(posedge clk)
    begin
      if (jtag_to_ava_master_bridge_granted_ddr2_s1 + pipeline_bridge_MEMORY_m1_granted_ddr2_s1 > 1)
        begin
          $write("%0d ns: > 1 of grant signals are active simultaneously", $time);
          $stop;
        end
    end


  //saved_grant signals are active simultaneously, which is an e_process
  always @(posedge clk)
    begin
      if (jtag_to_ava_master_bridge_saved_grant_ddr2_s1 + pipeline_bridge_MEMORY_m1_saved_grant_ddr2_s1 > 1)
        begin
          $write("%0d ns: > 1 of saved_grant signals are active simultaneously", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_reset_clk_domain_synch_module (
                                             // inputs:
                                              clk,
                                              data_in,
                                              reset_n,

                                             // outputs:
                                              data_out
                                           )
;

  output           data_out;
  input            clk;
  input            data_in;
  input            reset_n;

  reg              data_in_d1 /* synthesis ALTERA_ATTRIBUTE = "{-from \"*\"} CUT=ON ; PRESERVE_REGISTER=ON ; SUPPRESS_DA_RULE_INTERNAL=R101"  */;
  reg              data_out /* synthesis ALTERA_ATTRIBUTE = "PRESERVE_REGISTER=ON ; SUPPRESS_DA_RULE_INTERNAL=R101"  */;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_in_d1 <= 0;
      else 
        data_in_d1 <= data_in;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_out <= 0;
      else 
        data_out <= data_in_d1;
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_module (
                                                                             // inputs:
                                                                              clear_fifo,
                                                                              clk,
                                                                              data_in,
                                                                              read,
                                                                              reset_n,
                                                                              sync_reset,
                                                                              write,

                                                                             // outputs:
                                                                              data_out,
                                                                              empty,
                                                                              fifo_contains_ones_n,
                                                                              full
                                                                           )
;

  output  [  2: 0] data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input   [  2: 0] data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire    [  2: 0] data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_10;
  reg              full_11;
  reg              full_12;
  reg              full_13;
  reg              full_14;
  reg              full_15;
  reg              full_16;
  reg              full_17;
  reg              full_18;
  reg              full_19;
  reg              full_2;
  reg              full_20;
  reg              full_21;
  reg              full_22;
  reg              full_23;
  reg              full_24;
  reg              full_25;
  reg              full_26;
  reg              full_27;
  reg              full_28;
  reg              full_29;
  reg              full_3;
  reg              full_30;
  reg              full_31;
  wire             full_32;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  6: 0] how_many_ones;
  wire    [  6: 0] one_count_minus_one;
  wire    [  6: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  2: 0] p0_stage_0;
  wire             p10_full_10;
  wire    [  2: 0] p10_stage_10;
  wire             p11_full_11;
  wire    [  2: 0] p11_stage_11;
  wire             p12_full_12;
  wire    [  2: 0] p12_stage_12;
  wire             p13_full_13;
  wire    [  2: 0] p13_stage_13;
  wire             p14_full_14;
  wire    [  2: 0] p14_stage_14;
  wire             p15_full_15;
  wire    [  2: 0] p15_stage_15;
  wire             p16_full_16;
  wire    [  2: 0] p16_stage_16;
  wire             p17_full_17;
  wire    [  2: 0] p17_stage_17;
  wire             p18_full_18;
  wire    [  2: 0] p18_stage_18;
  wire             p19_full_19;
  wire    [  2: 0] p19_stage_19;
  wire             p1_full_1;
  wire    [  2: 0] p1_stage_1;
  wire             p20_full_20;
  wire    [  2: 0] p20_stage_20;
  wire             p21_full_21;
  wire    [  2: 0] p21_stage_21;
  wire             p22_full_22;
  wire    [  2: 0] p22_stage_22;
  wire             p23_full_23;
  wire    [  2: 0] p23_stage_23;
  wire             p24_full_24;
  wire    [  2: 0] p24_stage_24;
  wire             p25_full_25;
  wire    [  2: 0] p25_stage_25;
  wire             p26_full_26;
  wire    [  2: 0] p26_stage_26;
  wire             p27_full_27;
  wire    [  2: 0] p27_stage_27;
  wire             p28_full_28;
  wire    [  2: 0] p28_stage_28;
  wire             p29_full_29;
  wire    [  2: 0] p29_stage_29;
  wire             p2_full_2;
  wire    [  2: 0] p2_stage_2;
  wire             p30_full_30;
  wire    [  2: 0] p30_stage_30;
  wire             p31_full_31;
  wire    [  2: 0] p31_stage_31;
  wire             p3_full_3;
  wire    [  2: 0] p3_stage_3;
  wire             p4_full_4;
  wire    [  2: 0] p4_stage_4;
  wire             p5_full_5;
  wire    [  2: 0] p5_stage_5;
  wire             p6_full_6;
  wire    [  2: 0] p6_stage_6;
  wire             p7_full_7;
  wire    [  2: 0] p7_stage_7;
  wire             p8_full_8;
  wire    [  2: 0] p8_stage_8;
  wire             p9_full_9;
  wire    [  2: 0] p9_stage_9;
  reg     [  2: 0] stage_0;
  reg     [  2: 0] stage_1;
  reg     [  2: 0] stage_10;
  reg     [  2: 0] stage_11;
  reg     [  2: 0] stage_12;
  reg     [  2: 0] stage_13;
  reg     [  2: 0] stage_14;
  reg     [  2: 0] stage_15;
  reg     [  2: 0] stage_16;
  reg     [  2: 0] stage_17;
  reg     [  2: 0] stage_18;
  reg     [  2: 0] stage_19;
  reg     [  2: 0] stage_2;
  reg     [  2: 0] stage_20;
  reg     [  2: 0] stage_21;
  reg     [  2: 0] stage_22;
  reg     [  2: 0] stage_23;
  reg     [  2: 0] stage_24;
  reg     [  2: 0] stage_25;
  reg     [  2: 0] stage_26;
  reg     [  2: 0] stage_27;
  reg     [  2: 0] stage_28;
  reg     [  2: 0] stage_29;
  reg     [  2: 0] stage_3;
  reg     [  2: 0] stage_30;
  reg     [  2: 0] stage_31;
  reg     [  2: 0] stage_4;
  reg     [  2: 0] stage_5;
  reg     [  2: 0] stage_6;
  reg     [  2: 0] stage_7;
  reg     [  2: 0] stage_8;
  reg     [  2: 0] stage_9;
  wire    [  6: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_31;
  assign empty = !full_0;
  assign full_32 = 0;
  //data_31, which is an e_mux
  assign p31_stage_31 = ((full_32 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_31 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_31))
          if (sync_reset & full_31 & !((full_32 == 0) & read & write))
              stage_31 <= 0;
          else 
            stage_31 <= p31_stage_31;
    end


  //control_31, which is an e_mux
  assign p31_full_31 = ((read & !write) == 0)? full_30 :
    0;

  //control_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_31 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_31 <= 0;
          else 
            full_31 <= p31_full_31;
    end


  //data_30, which is an e_mux
  assign p30_stage_30 = ((full_31 & ~clear_fifo) == 0)? data_in :
    stage_31;

  //data_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_30 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_30))
          if (sync_reset & full_30 & !((full_31 == 0) & read & write))
              stage_30 <= 0;
          else 
            stage_30 <= p30_stage_30;
    end


  //control_30, which is an e_mux
  assign p30_full_30 = ((read & !write) == 0)? full_29 :
    full_31;

  //control_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_30 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_30 <= 0;
          else 
            full_30 <= p30_full_30;
    end


  //data_29, which is an e_mux
  assign p29_stage_29 = ((full_30 & ~clear_fifo) == 0)? data_in :
    stage_30;

  //data_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_29 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_29))
          if (sync_reset & full_29 & !((full_30 == 0) & read & write))
              stage_29 <= 0;
          else 
            stage_29 <= p29_stage_29;
    end


  //control_29, which is an e_mux
  assign p29_full_29 = ((read & !write) == 0)? full_28 :
    full_30;

  //control_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_29 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_29 <= 0;
          else 
            full_29 <= p29_full_29;
    end


  //data_28, which is an e_mux
  assign p28_stage_28 = ((full_29 & ~clear_fifo) == 0)? data_in :
    stage_29;

  //data_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_28 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_28))
          if (sync_reset & full_28 & !((full_29 == 0) & read & write))
              stage_28 <= 0;
          else 
            stage_28 <= p28_stage_28;
    end


  //control_28, which is an e_mux
  assign p28_full_28 = ((read & !write) == 0)? full_27 :
    full_29;

  //control_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_28 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_28 <= 0;
          else 
            full_28 <= p28_full_28;
    end


  //data_27, which is an e_mux
  assign p27_stage_27 = ((full_28 & ~clear_fifo) == 0)? data_in :
    stage_28;

  //data_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_27 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_27))
          if (sync_reset & full_27 & !((full_28 == 0) & read & write))
              stage_27 <= 0;
          else 
            stage_27 <= p27_stage_27;
    end


  //control_27, which is an e_mux
  assign p27_full_27 = ((read & !write) == 0)? full_26 :
    full_28;

  //control_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_27 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_27 <= 0;
          else 
            full_27 <= p27_full_27;
    end


  //data_26, which is an e_mux
  assign p26_stage_26 = ((full_27 & ~clear_fifo) == 0)? data_in :
    stage_27;

  //data_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_26 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_26))
          if (sync_reset & full_26 & !((full_27 == 0) & read & write))
              stage_26 <= 0;
          else 
            stage_26 <= p26_stage_26;
    end


  //control_26, which is an e_mux
  assign p26_full_26 = ((read & !write) == 0)? full_25 :
    full_27;

  //control_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_26 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_26 <= 0;
          else 
            full_26 <= p26_full_26;
    end


  //data_25, which is an e_mux
  assign p25_stage_25 = ((full_26 & ~clear_fifo) == 0)? data_in :
    stage_26;

  //data_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_25 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_25))
          if (sync_reset & full_25 & !((full_26 == 0) & read & write))
              stage_25 <= 0;
          else 
            stage_25 <= p25_stage_25;
    end


  //control_25, which is an e_mux
  assign p25_full_25 = ((read & !write) == 0)? full_24 :
    full_26;

  //control_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_25 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_25 <= 0;
          else 
            full_25 <= p25_full_25;
    end


  //data_24, which is an e_mux
  assign p24_stage_24 = ((full_25 & ~clear_fifo) == 0)? data_in :
    stage_25;

  //data_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_24 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_24))
          if (sync_reset & full_24 & !((full_25 == 0) & read & write))
              stage_24 <= 0;
          else 
            stage_24 <= p24_stage_24;
    end


  //control_24, which is an e_mux
  assign p24_full_24 = ((read & !write) == 0)? full_23 :
    full_25;

  //control_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_24 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_24 <= 0;
          else 
            full_24 <= p24_full_24;
    end


  //data_23, which is an e_mux
  assign p23_stage_23 = ((full_24 & ~clear_fifo) == 0)? data_in :
    stage_24;

  //data_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_23 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_23))
          if (sync_reset & full_23 & !((full_24 == 0) & read & write))
              stage_23 <= 0;
          else 
            stage_23 <= p23_stage_23;
    end


  //control_23, which is an e_mux
  assign p23_full_23 = ((read & !write) == 0)? full_22 :
    full_24;

  //control_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_23 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_23 <= 0;
          else 
            full_23 <= p23_full_23;
    end


  //data_22, which is an e_mux
  assign p22_stage_22 = ((full_23 & ~clear_fifo) == 0)? data_in :
    stage_23;

  //data_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_22 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_22))
          if (sync_reset & full_22 & !((full_23 == 0) & read & write))
              stage_22 <= 0;
          else 
            stage_22 <= p22_stage_22;
    end


  //control_22, which is an e_mux
  assign p22_full_22 = ((read & !write) == 0)? full_21 :
    full_23;

  //control_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_22 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_22 <= 0;
          else 
            full_22 <= p22_full_22;
    end


  //data_21, which is an e_mux
  assign p21_stage_21 = ((full_22 & ~clear_fifo) == 0)? data_in :
    stage_22;

  //data_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_21 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_21))
          if (sync_reset & full_21 & !((full_22 == 0) & read & write))
              stage_21 <= 0;
          else 
            stage_21 <= p21_stage_21;
    end


  //control_21, which is an e_mux
  assign p21_full_21 = ((read & !write) == 0)? full_20 :
    full_22;

  //control_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_21 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_21 <= 0;
          else 
            full_21 <= p21_full_21;
    end


  //data_20, which is an e_mux
  assign p20_stage_20 = ((full_21 & ~clear_fifo) == 0)? data_in :
    stage_21;

  //data_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_20 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_20))
          if (sync_reset & full_20 & !((full_21 == 0) & read & write))
              stage_20 <= 0;
          else 
            stage_20 <= p20_stage_20;
    end


  //control_20, which is an e_mux
  assign p20_full_20 = ((read & !write) == 0)? full_19 :
    full_21;

  //control_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_20 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_20 <= 0;
          else 
            full_20 <= p20_full_20;
    end


  //data_19, which is an e_mux
  assign p19_stage_19 = ((full_20 & ~clear_fifo) == 0)? data_in :
    stage_20;

  //data_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_19 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_19))
          if (sync_reset & full_19 & !((full_20 == 0) & read & write))
              stage_19 <= 0;
          else 
            stage_19 <= p19_stage_19;
    end


  //control_19, which is an e_mux
  assign p19_full_19 = ((read & !write) == 0)? full_18 :
    full_20;

  //control_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_19 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_19 <= 0;
          else 
            full_19 <= p19_full_19;
    end


  //data_18, which is an e_mux
  assign p18_stage_18 = ((full_19 & ~clear_fifo) == 0)? data_in :
    stage_19;

  //data_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_18 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_18))
          if (sync_reset & full_18 & !((full_19 == 0) & read & write))
              stage_18 <= 0;
          else 
            stage_18 <= p18_stage_18;
    end


  //control_18, which is an e_mux
  assign p18_full_18 = ((read & !write) == 0)? full_17 :
    full_19;

  //control_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_18 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_18 <= 0;
          else 
            full_18 <= p18_full_18;
    end


  //data_17, which is an e_mux
  assign p17_stage_17 = ((full_18 & ~clear_fifo) == 0)? data_in :
    stage_18;

  //data_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_17 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_17))
          if (sync_reset & full_17 & !((full_18 == 0) & read & write))
              stage_17 <= 0;
          else 
            stage_17 <= p17_stage_17;
    end


  //control_17, which is an e_mux
  assign p17_full_17 = ((read & !write) == 0)? full_16 :
    full_18;

  //control_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_17 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_17 <= 0;
          else 
            full_17 <= p17_full_17;
    end


  //data_16, which is an e_mux
  assign p16_stage_16 = ((full_17 & ~clear_fifo) == 0)? data_in :
    stage_17;

  //data_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_16 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_16))
          if (sync_reset & full_16 & !((full_17 == 0) & read & write))
              stage_16 <= 0;
          else 
            stage_16 <= p16_stage_16;
    end


  //control_16, which is an e_mux
  assign p16_full_16 = ((read & !write) == 0)? full_15 :
    full_17;

  //control_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_16 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_16 <= 0;
          else 
            full_16 <= p16_full_16;
    end


  //data_15, which is an e_mux
  assign p15_stage_15 = ((full_16 & ~clear_fifo) == 0)? data_in :
    stage_16;

  //data_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_15 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_15))
          if (sync_reset & full_15 & !((full_16 == 0) & read & write))
              stage_15 <= 0;
          else 
            stage_15 <= p15_stage_15;
    end


  //control_15, which is an e_mux
  assign p15_full_15 = ((read & !write) == 0)? full_14 :
    full_16;

  //control_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_15 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_15 <= 0;
          else 
            full_15 <= p15_full_15;
    end


  //data_14, which is an e_mux
  assign p14_stage_14 = ((full_15 & ~clear_fifo) == 0)? data_in :
    stage_15;

  //data_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_14 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_14))
          if (sync_reset & full_14 & !((full_15 == 0) & read & write))
              stage_14 <= 0;
          else 
            stage_14 <= p14_stage_14;
    end


  //control_14, which is an e_mux
  assign p14_full_14 = ((read & !write) == 0)? full_13 :
    full_15;

  //control_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_14 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_14 <= 0;
          else 
            full_14 <= p14_full_14;
    end


  //data_13, which is an e_mux
  assign p13_stage_13 = ((full_14 & ~clear_fifo) == 0)? data_in :
    stage_14;

  //data_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_13 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_13))
          if (sync_reset & full_13 & !((full_14 == 0) & read & write))
              stage_13 <= 0;
          else 
            stage_13 <= p13_stage_13;
    end


  //control_13, which is an e_mux
  assign p13_full_13 = ((read & !write) == 0)? full_12 :
    full_14;

  //control_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_13 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_13 <= 0;
          else 
            full_13 <= p13_full_13;
    end


  //data_12, which is an e_mux
  assign p12_stage_12 = ((full_13 & ~clear_fifo) == 0)? data_in :
    stage_13;

  //data_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_12 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_12))
          if (sync_reset & full_12 & !((full_13 == 0) & read & write))
              stage_12 <= 0;
          else 
            stage_12 <= p12_stage_12;
    end


  //control_12, which is an e_mux
  assign p12_full_12 = ((read & !write) == 0)? full_11 :
    full_13;

  //control_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_12 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_12 <= 0;
          else 
            full_12 <= p12_full_12;
    end


  //data_11, which is an e_mux
  assign p11_stage_11 = ((full_12 & ~clear_fifo) == 0)? data_in :
    stage_12;

  //data_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_11 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_11))
          if (sync_reset & full_11 & !((full_12 == 0) & read & write))
              stage_11 <= 0;
          else 
            stage_11 <= p11_stage_11;
    end


  //control_11, which is an e_mux
  assign p11_full_11 = ((read & !write) == 0)? full_10 :
    full_12;

  //control_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_11 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_11 <= 0;
          else 
            full_11 <= p11_full_11;
    end


  //data_10, which is an e_mux
  assign p10_stage_10 = ((full_11 & ~clear_fifo) == 0)? data_in :
    stage_11;

  //data_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_10 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_10))
          if (sync_reset & full_10 & !((full_11 == 0) & read & write))
              stage_10 <= 0;
          else 
            stage_10 <= p10_stage_10;
    end


  //control_10, which is an e_mux
  assign p10_full_10 = ((read & !write) == 0)? full_9 :
    full_11;

  //control_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_10 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_10 <= 0;
          else 
            full_10 <= p10_full_10;
    end


  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    stage_10;

  //data_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_9 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_9))
          if (sync_reset & full_9 & !((full_10 == 0) & read & write))
              stage_9 <= 0;
          else 
            stage_9 <= p9_stage_9;
    end


  //control_9, which is an e_mux
  assign p9_full_9 = ((read & !write) == 0)? full_8 :
    full_10;

  //control_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_9 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_9 <= 0;
          else 
            full_9 <= p9_full_9;
    end


  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    stage_9;

  //data_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_8 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_8))
          if (sync_reset & full_8 & !((full_9 == 0) & read & write))
              stage_8 <= 0;
          else 
            stage_8 <= p8_stage_8;
    end


  //control_8, which is an e_mux
  assign p8_full_8 = ((read & !write) == 0)? full_7 :
    full_9;

  //control_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_8 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_8 <= 0;
          else 
            full_8 <= p8_full_8;
    end


  //data_7, which is an e_mux
  assign p7_stage_7 = ((full_8 & ~clear_fifo) == 0)? data_in :
    stage_8;

  //data_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_7 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_7))
          if (sync_reset & full_7 & !((full_8 == 0) & read & write))
              stage_7 <= 0;
          else 
            stage_7 <= p7_stage_7;
    end


  //control_7, which is an e_mux
  assign p7_full_7 = ((read & !write) == 0)? full_6 :
    full_8;

  //control_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_7 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_7 <= 0;
          else 
            full_7 <= p7_full_7;
    end


  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    stage_7;

  //data_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_6 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_6))
          if (sync_reset & full_6 & !((full_7 == 0) & read & write))
              stage_6 <= 0;
          else 
            stage_6 <= p6_stage_6;
    end


  //control_6, which is an e_mux
  assign p6_full_6 = ((read & !write) == 0)? full_5 :
    full_7;

  //control_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_6 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_6 <= 0;
          else 
            full_6 <= p6_full_6;
    end


  //data_5, which is an e_mux
  assign p5_stage_5 = ((full_6 & ~clear_fifo) == 0)? data_in :
    stage_6;

  //data_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_5 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_5))
          if (sync_reset & full_5 & !((full_6 == 0) & read & write))
              stage_5 <= 0;
          else 
            stage_5 <= p5_stage_5;
    end


  //control_5, which is an e_mux
  assign p5_full_5 = ((read & !write) == 0)? full_4 :
    full_6;

  //control_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_5 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_5 <= 0;
          else 
            full_5 <= p5_full_5;
    end


  //data_4, which is an e_mux
  assign p4_stage_4 = ((full_5 & ~clear_fifo) == 0)? data_in :
    stage_5;

  //data_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_4 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_4))
          if (sync_reset & full_4 & !((full_5 == 0) & read & write))
              stage_4 <= 0;
          else 
            stage_4 <= p4_stage_4;
    end


  //control_4, which is an e_mux
  assign p4_full_4 = ((read & !write) == 0)? full_3 :
    full_5;

  //control_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_4 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_4 <= 0;
          else 
            full_4 <= p4_full_4;
    end


  //data_3, which is an e_mux
  assign p3_stage_3 = ((full_4 & ~clear_fifo) == 0)? data_in :
    stage_4;

  //data_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_3 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_3))
          if (sync_reset & full_3 & !((full_4 == 0) & read & write))
              stage_3 <= 0;
          else 
            stage_3 <= p3_stage_3;
    end


  //control_3, which is an e_mux
  assign p3_full_3 = ((read & !write) == 0)? full_2 :
    full_4;

  //control_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_3 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_3 <= 0;
          else 
            full_3 <= p3_full_3;
    end


  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    stage_3;

  //data_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_2 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_2))
          if (sync_reset & full_2 & !((full_3 == 0) & read & write))
              stage_2 <= 0;
          else 
            stage_2 <= p2_stage_2;
    end


  //control_2, which is an e_mux
  assign p2_full_2 = ((read & !write) == 0)? full_1 :
    full_3;

  //control_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_2 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_2 <= 0;
          else 
            full_2 <= p2_full_2;
    end


  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    stage_2;

  //data_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_1 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_1))
          if (sync_reset & full_1 & !((full_2 == 0) & read & write))
              stage_1 <= 0;
          else 
            stage_1 <= p1_stage_1;
    end


  //control_1, which is an e_mux
  assign p1_full_1 = ((read & !write) == 0)? full_0 :
    full_2;

  //control_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_1 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_1 <= 0;
          else 
            full_1 <= p1_full_1;
    end


  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    stage_1;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    full_1;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module jtag_to_ava_master_bridge_master_arbitrator (
                                                     // inputs:
                                                      clk,
                                                      d1_ddr2_s1_end_xfer,
                                                      d1_tiger_top_0_leapSlave_end_xfer,
                                                      ddr2_s1_readdata_from_sa,
                                                      ddr2_s1_waitrequest_n_from_sa,
                                                      jtag_to_ava_master_bridge_granted_ddr2_s1,
                                                      jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave,
                                                      jtag_to_ava_master_bridge_master_address,
                                                      jtag_to_ava_master_bridge_master_byteenable,
                                                      jtag_to_ava_master_bridge_master_read,
                                                      jtag_to_ava_master_bridge_master_write,
                                                      jtag_to_ava_master_bridge_master_writedata,
                                                      jtag_to_ava_master_bridge_qualified_request_ddr2_s1,
                                                      jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave,
                                                      jtag_to_ava_master_bridge_read_data_valid_ddr2_s1,
                                                      jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register,
                                                      jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave,
                                                      jtag_to_ava_master_bridge_requests_ddr2_s1,
                                                      jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave,
                                                      reset_n,
                                                      tiger_top_0_leapSlave_readdata_from_sa,

                                                     // outputs:
                                                      jtag_to_ava_master_bridge_latency_counter,
                                                      jtag_to_ava_master_bridge_master_address_to_slave,
                                                      jtag_to_ava_master_bridge_master_readdata,
                                                      jtag_to_ava_master_bridge_master_readdatavalid,
                                                      jtag_to_ava_master_bridge_master_reset,
                                                      jtag_to_ava_master_bridge_master_waitrequest
                                                   )
;

  output           jtag_to_ava_master_bridge_latency_counter;
  output  [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  output  [ 31: 0] jtag_to_ava_master_bridge_master_readdata;
  output           jtag_to_ava_master_bridge_master_readdatavalid;
  output           jtag_to_ava_master_bridge_master_reset;
  output           jtag_to_ava_master_bridge_master_waitrequest;
  input            clk;
  input            d1_ddr2_s1_end_xfer;
  input            d1_tiger_top_0_leapSlave_end_xfer;
  input   [255: 0] ddr2_s1_readdata_from_sa;
  input            ddr2_s1_waitrequest_n_from_sa;
  input            jtag_to_ava_master_bridge_granted_ddr2_s1;
  input            jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_address;
  input   [  3: 0] jtag_to_ava_master_bridge_master_byteenable;
  input            jtag_to_ava_master_bridge_master_read;
  input            jtag_to_ava_master_bridge_master_write;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_writedata;
  input            jtag_to_ava_master_bridge_qualified_request_ddr2_s1;
  input            jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave;
  input            jtag_to_ava_master_bridge_read_data_valid_ddr2_s1;
  input            jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register;
  input            jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;
  input            jtag_to_ava_master_bridge_requests_ddr2_s1;
  input            jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_leapSlave_readdata_from_sa;

  reg              active_and_waiting_last_time;
  wire    [ 31: 0] ddr2_s1_readdata_from_sa_part_selected_by_negative_dbs;
  wire             empty_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo;
  wire             full_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo;
  reg              jtag_to_ava_master_bridge_latency_counter;
  reg     [ 31: 0] jtag_to_ava_master_bridge_master_address_last_time;
  wire    [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  reg     [  3: 0] jtag_to_ava_master_bridge_master_byteenable_last_time;
  wire             jtag_to_ava_master_bridge_master_is_granted_some_slave;
  reg              jtag_to_ava_master_bridge_master_read_but_no_slave_selected;
  reg              jtag_to_ava_master_bridge_master_read_last_time;
  wire    [ 31: 0] jtag_to_ava_master_bridge_master_readdata;
  wire             jtag_to_ava_master_bridge_master_readdatavalid;
  wire             jtag_to_ava_master_bridge_master_reset;
  wire             jtag_to_ava_master_bridge_master_run;
  wire             jtag_to_ava_master_bridge_master_waitrequest;
  reg              jtag_to_ava_master_bridge_master_write_last_time;
  reg     [ 31: 0] jtag_to_ava_master_bridge_master_writedata_last_time;
  wire             latency_load_value;
  wire             p1_jtag_to_ava_master_bridge_latency_counter;
  wire             pre_flush_jtag_to_ava_master_bridge_master_readdatavalid;
  wire             r_0;
  wire             read_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo;
  wire    [  2: 0] selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output;
  wire    [  2: 0] selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1;
  wire             write_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (jtag_to_ava_master_bridge_qualified_request_ddr2_s1 | ~jtag_to_ava_master_bridge_requests_ddr2_s1) & (jtag_to_ava_master_bridge_granted_ddr2_s1 | ~jtag_to_ava_master_bridge_qualified_request_ddr2_s1) & ((~jtag_to_ava_master_bridge_qualified_request_ddr2_s1 | ~(jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write) | (1 & ddr2_s1_waitrequest_n_from_sa & (jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write)))) & ((~jtag_to_ava_master_bridge_qualified_request_ddr2_s1 | ~(jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write) | (1 & ddr2_s1_waitrequest_n_from_sa & (jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write)))) & 1 & (jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave | ~jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave) & ((~jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave | ~jtag_to_ava_master_bridge_master_read | (1 & ~d1_tiger_top_0_leapSlave_end_xfer & jtag_to_ava_master_bridge_master_read))) & ((~jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave | ~jtag_to_ava_master_bridge_master_write | (1 & jtag_to_ava_master_bridge_master_write)));

  //cascaded wait assignment, which is an e_assign
  assign jtag_to_ava_master_bridge_master_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign jtag_to_ava_master_bridge_master_address_to_slave = jtag_to_ava_master_bridge_master_address[31 : 0];

  //jtag_to_ava_master_bridge_master_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_master_read_but_no_slave_selected <= 0;
      else 
        jtag_to_ava_master_bridge_master_read_but_no_slave_selected <= jtag_to_ava_master_bridge_master_read & jtag_to_ava_master_bridge_master_run & ~jtag_to_ava_master_bridge_master_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign jtag_to_ava_master_bridge_master_is_granted_some_slave = jtag_to_ava_master_bridge_granted_ddr2_s1 |
    jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_jtag_to_ava_master_bridge_master_readdatavalid = jtag_to_ava_master_bridge_read_data_valid_ddr2_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign jtag_to_ava_master_bridge_master_readdatavalid = jtag_to_ava_master_bridge_master_read_but_no_slave_selected |
    pre_flush_jtag_to_ava_master_bridge_master_readdatavalid |
    jtag_to_ava_master_bridge_master_read_but_no_slave_selected |
    pre_flush_jtag_to_ava_master_bridge_master_readdatavalid |
    jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;

  //Negative Dynamic Bus-sizing mux.
  //this mux selects the correct eighth of the 
  //wide data coming from the slave ddr2/s1 
  assign ddr2_s1_readdata_from_sa_part_selected_by_negative_dbs = ((selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 == 0))? ddr2_s1_readdata_from_sa[31 : 0] :
    ((selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 == 1))? ddr2_s1_readdata_from_sa[63 : 32] :
    ((selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 == 2))? ddr2_s1_readdata_from_sa[95 : 64] :
    ((selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 == 3))? ddr2_s1_readdata_from_sa[127 : 96] :
    ((selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 == 4))? ddr2_s1_readdata_from_sa[159 : 128] :
    ((selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 == 5))? ddr2_s1_readdata_from_sa[191 : 160] :
    ((selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 == 6))? ddr2_s1_readdata_from_sa[223 : 192] :
    ddr2_s1_readdata_from_sa[255 : 224];

  //read_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo fifo read, which is an e_mux
  assign read_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo = jtag_to_ava_master_bridge_read_data_valid_ddr2_s1;

  //write_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo fifo write, which is an e_mux
  assign write_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo = jtag_to_ava_master_bridge_master_read & jtag_to_ava_master_bridge_master_run & jtag_to_ava_master_bridge_requests_ddr2_s1;

  assign selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output_ddr2_s1 = selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output;
  //selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo, which is an e_fifo_with_registered_outputs
  selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_module selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (jtag_to_ava_master_bridge_master_address_to_slave[4 : 2]),
      .data_out             (selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo_output),
      .empty                (empty_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo),
      .fifo_contains_ones_n (),
      .full                 (full_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo),
      .read                 (read_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (write_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo)
    );

  //jtag_to_ava_master_bridge/master readdata mux, which is an e_mux
  assign jtag_to_ava_master_bridge_master_readdata = ({32 {~jtag_to_ava_master_bridge_read_data_valid_ddr2_s1}} | ddr2_s1_readdata_from_sa_part_selected_by_negative_dbs) &
    ({32 {~(jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave & jtag_to_ava_master_bridge_master_read)}} | tiger_top_0_leapSlave_readdata_from_sa);

  //actual waitrequest port, which is an e_assign
  assign jtag_to_ava_master_bridge_master_waitrequest = ~jtag_to_ava_master_bridge_master_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_latency_counter <= 0;
      else 
        jtag_to_ava_master_bridge_latency_counter <= p1_jtag_to_ava_master_bridge_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_jtag_to_ava_master_bridge_latency_counter = ((jtag_to_ava_master_bridge_master_run & jtag_to_ava_master_bridge_master_read))? latency_load_value :
    (jtag_to_ava_master_bridge_latency_counter)? jtag_to_ava_master_bridge_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;

  //~jtag_to_ava_master_bridge_master_reset assignment, which is an e_assign
  assign jtag_to_ava_master_bridge_master_reset = ~reset_n;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //jtag_to_ava_master_bridge_master_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_master_address_last_time <= 0;
      else 
        jtag_to_ava_master_bridge_master_address_last_time <= jtag_to_ava_master_bridge_master_address;
    end


  //jtag_to_ava_master_bridge/master waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= jtag_to_ava_master_bridge_master_waitrequest & (jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write);
    end


  //jtag_to_ava_master_bridge_master_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (jtag_to_ava_master_bridge_master_address != jtag_to_ava_master_bridge_master_address_last_time))
        begin
          $write("%0d ns: jtag_to_ava_master_bridge_master_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //jtag_to_ava_master_bridge_master_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_master_byteenable_last_time <= 0;
      else 
        jtag_to_ava_master_bridge_master_byteenable_last_time <= jtag_to_ava_master_bridge_master_byteenable;
    end


  //jtag_to_ava_master_bridge_master_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (jtag_to_ava_master_bridge_master_byteenable != jtag_to_ava_master_bridge_master_byteenable_last_time))
        begin
          $write("%0d ns: jtag_to_ava_master_bridge_master_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //jtag_to_ava_master_bridge_master_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_master_read_last_time <= 0;
      else 
        jtag_to_ava_master_bridge_master_read_last_time <= jtag_to_ava_master_bridge_master_read;
    end


  //jtag_to_ava_master_bridge_master_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (jtag_to_ava_master_bridge_master_read != jtag_to_ava_master_bridge_master_read_last_time))
        begin
          $write("%0d ns: jtag_to_ava_master_bridge_master_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //jtag_to_ava_master_bridge_master_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_master_write_last_time <= 0;
      else 
        jtag_to_ava_master_bridge_master_write_last_time <= jtag_to_ava_master_bridge_master_write;
    end


  //jtag_to_ava_master_bridge_master_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (jtag_to_ava_master_bridge_master_write != jtag_to_ava_master_bridge_master_write_last_time))
        begin
          $write("%0d ns: jtag_to_ava_master_bridge_master_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //jtag_to_ava_master_bridge_master_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_master_writedata_last_time <= 0;
      else 
        jtag_to_ava_master_bridge_master_writedata_last_time <= jtag_to_ava_master_bridge_master_writedata;
    end


  //jtag_to_ava_master_bridge_master_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (jtag_to_ava_master_bridge_master_writedata != jtag_to_ava_master_bridge_master_writedata_last_time) & jtag_to_ava_master_bridge_master_write)
        begin
          $write("%0d ns: jtag_to_ava_master_bridge_master_writedata did not heed wait!!!", $time);
          $stop;
        end
    end


  //selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo read when empty, which is an e_process
  always @(posedge clk)
    begin
      if (empty_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo & read_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo)
        begin
          $write("%0d ns: jtag_to_ava_master_bridge/master negative rdv fifo selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo: read AND empty.\n", $time);
          $stop;
        end
    end


  //selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo write when full, which is an e_process
  always @(posedge clk)
    begin
      if (full_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo & write_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo & ~read_selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo)
        begin
          $write("%0d ns: jtag_to_ava_master_bridge/master negative rdv fifo selecto_nrdv_jtag_to_ava_master_bridge_master_3_ddr2_s1_fifo: write AND full.\n", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module burstcount_fifo_for_pipeline_bridge_MEMORY_s1_module (
                                                              // inputs:
                                                               clear_fifo,
                                                               clk,
                                                               data_in,
                                                               read,
                                                               reset_n,
                                                               sync_reset,
                                                               write,

                                                              // outputs:
                                                               data_out,
                                                               empty,
                                                               fifo_contains_ones_n,
                                                               full
                                                            )
;

  output  [  5: 0] data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input   [  5: 0] data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire    [  5: 0] data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_10;
  reg              full_11;
  reg              full_12;
  reg              full_13;
  reg              full_14;
  reg              full_15;
  reg              full_16;
  reg              full_17;
  reg              full_18;
  reg              full_19;
  reg              full_2;
  reg              full_20;
  reg              full_21;
  reg              full_22;
  reg              full_23;
  reg              full_24;
  reg              full_25;
  reg              full_26;
  reg              full_27;
  reg              full_28;
  reg              full_29;
  reg              full_3;
  reg              full_30;
  reg              full_31;
  reg              full_32;
  reg              full_33;
  reg              full_34;
  wire             full_35;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  6: 0] how_many_ones;
  wire    [  6: 0] one_count_minus_one;
  wire    [  6: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  5: 0] p0_stage_0;
  wire             p10_full_10;
  wire    [  5: 0] p10_stage_10;
  wire             p11_full_11;
  wire    [  5: 0] p11_stage_11;
  wire             p12_full_12;
  wire    [  5: 0] p12_stage_12;
  wire             p13_full_13;
  wire    [  5: 0] p13_stage_13;
  wire             p14_full_14;
  wire    [  5: 0] p14_stage_14;
  wire             p15_full_15;
  wire    [  5: 0] p15_stage_15;
  wire             p16_full_16;
  wire    [  5: 0] p16_stage_16;
  wire             p17_full_17;
  wire    [  5: 0] p17_stage_17;
  wire             p18_full_18;
  wire    [  5: 0] p18_stage_18;
  wire             p19_full_19;
  wire    [  5: 0] p19_stage_19;
  wire             p1_full_1;
  wire    [  5: 0] p1_stage_1;
  wire             p20_full_20;
  wire    [  5: 0] p20_stage_20;
  wire             p21_full_21;
  wire    [  5: 0] p21_stage_21;
  wire             p22_full_22;
  wire    [  5: 0] p22_stage_22;
  wire             p23_full_23;
  wire    [  5: 0] p23_stage_23;
  wire             p24_full_24;
  wire    [  5: 0] p24_stage_24;
  wire             p25_full_25;
  wire    [  5: 0] p25_stage_25;
  wire             p26_full_26;
  wire    [  5: 0] p26_stage_26;
  wire             p27_full_27;
  wire    [  5: 0] p27_stage_27;
  wire             p28_full_28;
  wire    [  5: 0] p28_stage_28;
  wire             p29_full_29;
  wire    [  5: 0] p29_stage_29;
  wire             p2_full_2;
  wire    [  5: 0] p2_stage_2;
  wire             p30_full_30;
  wire    [  5: 0] p30_stage_30;
  wire             p31_full_31;
  wire    [  5: 0] p31_stage_31;
  wire             p32_full_32;
  wire    [  5: 0] p32_stage_32;
  wire             p33_full_33;
  wire    [  5: 0] p33_stage_33;
  wire             p34_full_34;
  wire    [  5: 0] p34_stage_34;
  wire             p3_full_3;
  wire    [  5: 0] p3_stage_3;
  wire             p4_full_4;
  wire    [  5: 0] p4_stage_4;
  wire             p5_full_5;
  wire    [  5: 0] p5_stage_5;
  wire             p6_full_6;
  wire    [  5: 0] p6_stage_6;
  wire             p7_full_7;
  wire    [  5: 0] p7_stage_7;
  wire             p8_full_8;
  wire    [  5: 0] p8_stage_8;
  wire             p9_full_9;
  wire    [  5: 0] p9_stage_9;
  reg     [  5: 0] stage_0;
  reg     [  5: 0] stage_1;
  reg     [  5: 0] stage_10;
  reg     [  5: 0] stage_11;
  reg     [  5: 0] stage_12;
  reg     [  5: 0] stage_13;
  reg     [  5: 0] stage_14;
  reg     [  5: 0] stage_15;
  reg     [  5: 0] stage_16;
  reg     [  5: 0] stage_17;
  reg     [  5: 0] stage_18;
  reg     [  5: 0] stage_19;
  reg     [  5: 0] stage_2;
  reg     [  5: 0] stage_20;
  reg     [  5: 0] stage_21;
  reg     [  5: 0] stage_22;
  reg     [  5: 0] stage_23;
  reg     [  5: 0] stage_24;
  reg     [  5: 0] stage_25;
  reg     [  5: 0] stage_26;
  reg     [  5: 0] stage_27;
  reg     [  5: 0] stage_28;
  reg     [  5: 0] stage_29;
  reg     [  5: 0] stage_3;
  reg     [  5: 0] stage_30;
  reg     [  5: 0] stage_31;
  reg     [  5: 0] stage_32;
  reg     [  5: 0] stage_33;
  reg     [  5: 0] stage_34;
  reg     [  5: 0] stage_4;
  reg     [  5: 0] stage_5;
  reg     [  5: 0] stage_6;
  reg     [  5: 0] stage_7;
  reg     [  5: 0] stage_8;
  reg     [  5: 0] stage_9;
  wire    [  6: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_34;
  assign empty = !full_0;
  assign full_35 = 0;
  //data_34, which is an e_mux
  assign p34_stage_34 = ((full_35 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_34, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_34 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_34))
          if (sync_reset & full_34 & !((full_35 == 0) & read & write))
              stage_34 <= 0;
          else 
            stage_34 <= p34_stage_34;
    end


  //control_34, which is an e_mux
  assign p34_full_34 = ((read & !write) == 0)? full_33 :
    0;

  //control_reg_34, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_34 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_34 <= 0;
          else 
            full_34 <= p34_full_34;
    end


  //data_33, which is an e_mux
  assign p33_stage_33 = ((full_34 & ~clear_fifo) == 0)? data_in :
    stage_34;

  //data_reg_33, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_33 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_33))
          if (sync_reset & full_33 & !((full_34 == 0) & read & write))
              stage_33 <= 0;
          else 
            stage_33 <= p33_stage_33;
    end


  //control_33, which is an e_mux
  assign p33_full_33 = ((read & !write) == 0)? full_32 :
    full_34;

  //control_reg_33, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_33 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_33 <= 0;
          else 
            full_33 <= p33_full_33;
    end


  //data_32, which is an e_mux
  assign p32_stage_32 = ((full_33 & ~clear_fifo) == 0)? data_in :
    stage_33;

  //data_reg_32, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_32 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_32))
          if (sync_reset & full_32 & !((full_33 == 0) & read & write))
              stage_32 <= 0;
          else 
            stage_32 <= p32_stage_32;
    end


  //control_32, which is an e_mux
  assign p32_full_32 = ((read & !write) == 0)? full_31 :
    full_33;

  //control_reg_32, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_32 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_32 <= 0;
          else 
            full_32 <= p32_full_32;
    end


  //data_31, which is an e_mux
  assign p31_stage_31 = ((full_32 & ~clear_fifo) == 0)? data_in :
    stage_32;

  //data_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_31 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_31))
          if (sync_reset & full_31 & !((full_32 == 0) & read & write))
              stage_31 <= 0;
          else 
            stage_31 <= p31_stage_31;
    end


  //control_31, which is an e_mux
  assign p31_full_31 = ((read & !write) == 0)? full_30 :
    full_32;

  //control_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_31 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_31 <= 0;
          else 
            full_31 <= p31_full_31;
    end


  //data_30, which is an e_mux
  assign p30_stage_30 = ((full_31 & ~clear_fifo) == 0)? data_in :
    stage_31;

  //data_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_30 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_30))
          if (sync_reset & full_30 & !((full_31 == 0) & read & write))
              stage_30 <= 0;
          else 
            stage_30 <= p30_stage_30;
    end


  //control_30, which is an e_mux
  assign p30_full_30 = ((read & !write) == 0)? full_29 :
    full_31;

  //control_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_30 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_30 <= 0;
          else 
            full_30 <= p30_full_30;
    end


  //data_29, which is an e_mux
  assign p29_stage_29 = ((full_30 & ~clear_fifo) == 0)? data_in :
    stage_30;

  //data_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_29 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_29))
          if (sync_reset & full_29 & !((full_30 == 0) & read & write))
              stage_29 <= 0;
          else 
            stage_29 <= p29_stage_29;
    end


  //control_29, which is an e_mux
  assign p29_full_29 = ((read & !write) == 0)? full_28 :
    full_30;

  //control_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_29 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_29 <= 0;
          else 
            full_29 <= p29_full_29;
    end


  //data_28, which is an e_mux
  assign p28_stage_28 = ((full_29 & ~clear_fifo) == 0)? data_in :
    stage_29;

  //data_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_28 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_28))
          if (sync_reset & full_28 & !((full_29 == 0) & read & write))
              stage_28 <= 0;
          else 
            stage_28 <= p28_stage_28;
    end


  //control_28, which is an e_mux
  assign p28_full_28 = ((read & !write) == 0)? full_27 :
    full_29;

  //control_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_28 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_28 <= 0;
          else 
            full_28 <= p28_full_28;
    end


  //data_27, which is an e_mux
  assign p27_stage_27 = ((full_28 & ~clear_fifo) == 0)? data_in :
    stage_28;

  //data_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_27 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_27))
          if (sync_reset & full_27 & !((full_28 == 0) & read & write))
              stage_27 <= 0;
          else 
            stage_27 <= p27_stage_27;
    end


  //control_27, which is an e_mux
  assign p27_full_27 = ((read & !write) == 0)? full_26 :
    full_28;

  //control_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_27 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_27 <= 0;
          else 
            full_27 <= p27_full_27;
    end


  //data_26, which is an e_mux
  assign p26_stage_26 = ((full_27 & ~clear_fifo) == 0)? data_in :
    stage_27;

  //data_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_26 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_26))
          if (sync_reset & full_26 & !((full_27 == 0) & read & write))
              stage_26 <= 0;
          else 
            stage_26 <= p26_stage_26;
    end


  //control_26, which is an e_mux
  assign p26_full_26 = ((read & !write) == 0)? full_25 :
    full_27;

  //control_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_26 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_26 <= 0;
          else 
            full_26 <= p26_full_26;
    end


  //data_25, which is an e_mux
  assign p25_stage_25 = ((full_26 & ~clear_fifo) == 0)? data_in :
    stage_26;

  //data_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_25 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_25))
          if (sync_reset & full_25 & !((full_26 == 0) & read & write))
              stage_25 <= 0;
          else 
            stage_25 <= p25_stage_25;
    end


  //control_25, which is an e_mux
  assign p25_full_25 = ((read & !write) == 0)? full_24 :
    full_26;

  //control_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_25 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_25 <= 0;
          else 
            full_25 <= p25_full_25;
    end


  //data_24, which is an e_mux
  assign p24_stage_24 = ((full_25 & ~clear_fifo) == 0)? data_in :
    stage_25;

  //data_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_24 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_24))
          if (sync_reset & full_24 & !((full_25 == 0) & read & write))
              stage_24 <= 0;
          else 
            stage_24 <= p24_stage_24;
    end


  //control_24, which is an e_mux
  assign p24_full_24 = ((read & !write) == 0)? full_23 :
    full_25;

  //control_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_24 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_24 <= 0;
          else 
            full_24 <= p24_full_24;
    end


  //data_23, which is an e_mux
  assign p23_stage_23 = ((full_24 & ~clear_fifo) == 0)? data_in :
    stage_24;

  //data_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_23 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_23))
          if (sync_reset & full_23 & !((full_24 == 0) & read & write))
              stage_23 <= 0;
          else 
            stage_23 <= p23_stage_23;
    end


  //control_23, which is an e_mux
  assign p23_full_23 = ((read & !write) == 0)? full_22 :
    full_24;

  //control_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_23 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_23 <= 0;
          else 
            full_23 <= p23_full_23;
    end


  //data_22, which is an e_mux
  assign p22_stage_22 = ((full_23 & ~clear_fifo) == 0)? data_in :
    stage_23;

  //data_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_22 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_22))
          if (sync_reset & full_22 & !((full_23 == 0) & read & write))
              stage_22 <= 0;
          else 
            stage_22 <= p22_stage_22;
    end


  //control_22, which is an e_mux
  assign p22_full_22 = ((read & !write) == 0)? full_21 :
    full_23;

  //control_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_22 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_22 <= 0;
          else 
            full_22 <= p22_full_22;
    end


  //data_21, which is an e_mux
  assign p21_stage_21 = ((full_22 & ~clear_fifo) == 0)? data_in :
    stage_22;

  //data_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_21 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_21))
          if (sync_reset & full_21 & !((full_22 == 0) & read & write))
              stage_21 <= 0;
          else 
            stage_21 <= p21_stage_21;
    end


  //control_21, which is an e_mux
  assign p21_full_21 = ((read & !write) == 0)? full_20 :
    full_22;

  //control_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_21 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_21 <= 0;
          else 
            full_21 <= p21_full_21;
    end


  //data_20, which is an e_mux
  assign p20_stage_20 = ((full_21 & ~clear_fifo) == 0)? data_in :
    stage_21;

  //data_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_20 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_20))
          if (sync_reset & full_20 & !((full_21 == 0) & read & write))
              stage_20 <= 0;
          else 
            stage_20 <= p20_stage_20;
    end


  //control_20, which is an e_mux
  assign p20_full_20 = ((read & !write) == 0)? full_19 :
    full_21;

  //control_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_20 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_20 <= 0;
          else 
            full_20 <= p20_full_20;
    end


  //data_19, which is an e_mux
  assign p19_stage_19 = ((full_20 & ~clear_fifo) == 0)? data_in :
    stage_20;

  //data_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_19 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_19))
          if (sync_reset & full_19 & !((full_20 == 0) & read & write))
              stage_19 <= 0;
          else 
            stage_19 <= p19_stage_19;
    end


  //control_19, which is an e_mux
  assign p19_full_19 = ((read & !write) == 0)? full_18 :
    full_20;

  //control_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_19 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_19 <= 0;
          else 
            full_19 <= p19_full_19;
    end


  //data_18, which is an e_mux
  assign p18_stage_18 = ((full_19 & ~clear_fifo) == 0)? data_in :
    stage_19;

  //data_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_18 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_18))
          if (sync_reset & full_18 & !((full_19 == 0) & read & write))
              stage_18 <= 0;
          else 
            stage_18 <= p18_stage_18;
    end


  //control_18, which is an e_mux
  assign p18_full_18 = ((read & !write) == 0)? full_17 :
    full_19;

  //control_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_18 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_18 <= 0;
          else 
            full_18 <= p18_full_18;
    end


  //data_17, which is an e_mux
  assign p17_stage_17 = ((full_18 & ~clear_fifo) == 0)? data_in :
    stage_18;

  //data_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_17 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_17))
          if (sync_reset & full_17 & !((full_18 == 0) & read & write))
              stage_17 <= 0;
          else 
            stage_17 <= p17_stage_17;
    end


  //control_17, which is an e_mux
  assign p17_full_17 = ((read & !write) == 0)? full_16 :
    full_18;

  //control_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_17 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_17 <= 0;
          else 
            full_17 <= p17_full_17;
    end


  //data_16, which is an e_mux
  assign p16_stage_16 = ((full_17 & ~clear_fifo) == 0)? data_in :
    stage_17;

  //data_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_16 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_16))
          if (sync_reset & full_16 & !((full_17 == 0) & read & write))
              stage_16 <= 0;
          else 
            stage_16 <= p16_stage_16;
    end


  //control_16, which is an e_mux
  assign p16_full_16 = ((read & !write) == 0)? full_15 :
    full_17;

  //control_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_16 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_16 <= 0;
          else 
            full_16 <= p16_full_16;
    end


  //data_15, which is an e_mux
  assign p15_stage_15 = ((full_16 & ~clear_fifo) == 0)? data_in :
    stage_16;

  //data_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_15 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_15))
          if (sync_reset & full_15 & !((full_16 == 0) & read & write))
              stage_15 <= 0;
          else 
            stage_15 <= p15_stage_15;
    end


  //control_15, which is an e_mux
  assign p15_full_15 = ((read & !write) == 0)? full_14 :
    full_16;

  //control_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_15 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_15 <= 0;
          else 
            full_15 <= p15_full_15;
    end


  //data_14, which is an e_mux
  assign p14_stage_14 = ((full_15 & ~clear_fifo) == 0)? data_in :
    stage_15;

  //data_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_14 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_14))
          if (sync_reset & full_14 & !((full_15 == 0) & read & write))
              stage_14 <= 0;
          else 
            stage_14 <= p14_stage_14;
    end


  //control_14, which is an e_mux
  assign p14_full_14 = ((read & !write) == 0)? full_13 :
    full_15;

  //control_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_14 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_14 <= 0;
          else 
            full_14 <= p14_full_14;
    end


  //data_13, which is an e_mux
  assign p13_stage_13 = ((full_14 & ~clear_fifo) == 0)? data_in :
    stage_14;

  //data_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_13 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_13))
          if (sync_reset & full_13 & !((full_14 == 0) & read & write))
              stage_13 <= 0;
          else 
            stage_13 <= p13_stage_13;
    end


  //control_13, which is an e_mux
  assign p13_full_13 = ((read & !write) == 0)? full_12 :
    full_14;

  //control_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_13 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_13 <= 0;
          else 
            full_13 <= p13_full_13;
    end


  //data_12, which is an e_mux
  assign p12_stage_12 = ((full_13 & ~clear_fifo) == 0)? data_in :
    stage_13;

  //data_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_12 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_12))
          if (sync_reset & full_12 & !((full_13 == 0) & read & write))
              stage_12 <= 0;
          else 
            stage_12 <= p12_stage_12;
    end


  //control_12, which is an e_mux
  assign p12_full_12 = ((read & !write) == 0)? full_11 :
    full_13;

  //control_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_12 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_12 <= 0;
          else 
            full_12 <= p12_full_12;
    end


  //data_11, which is an e_mux
  assign p11_stage_11 = ((full_12 & ~clear_fifo) == 0)? data_in :
    stage_12;

  //data_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_11 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_11))
          if (sync_reset & full_11 & !((full_12 == 0) & read & write))
              stage_11 <= 0;
          else 
            stage_11 <= p11_stage_11;
    end


  //control_11, which is an e_mux
  assign p11_full_11 = ((read & !write) == 0)? full_10 :
    full_12;

  //control_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_11 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_11 <= 0;
          else 
            full_11 <= p11_full_11;
    end


  //data_10, which is an e_mux
  assign p10_stage_10 = ((full_11 & ~clear_fifo) == 0)? data_in :
    stage_11;

  //data_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_10 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_10))
          if (sync_reset & full_10 & !((full_11 == 0) & read & write))
              stage_10 <= 0;
          else 
            stage_10 <= p10_stage_10;
    end


  //control_10, which is an e_mux
  assign p10_full_10 = ((read & !write) == 0)? full_9 :
    full_11;

  //control_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_10 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_10 <= 0;
          else 
            full_10 <= p10_full_10;
    end


  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    stage_10;

  //data_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_9 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_9))
          if (sync_reset & full_9 & !((full_10 == 0) & read & write))
              stage_9 <= 0;
          else 
            stage_9 <= p9_stage_9;
    end


  //control_9, which is an e_mux
  assign p9_full_9 = ((read & !write) == 0)? full_8 :
    full_10;

  //control_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_9 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_9 <= 0;
          else 
            full_9 <= p9_full_9;
    end


  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    stage_9;

  //data_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_8 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_8))
          if (sync_reset & full_8 & !((full_9 == 0) & read & write))
              stage_8 <= 0;
          else 
            stage_8 <= p8_stage_8;
    end


  //control_8, which is an e_mux
  assign p8_full_8 = ((read & !write) == 0)? full_7 :
    full_9;

  //control_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_8 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_8 <= 0;
          else 
            full_8 <= p8_full_8;
    end


  //data_7, which is an e_mux
  assign p7_stage_7 = ((full_8 & ~clear_fifo) == 0)? data_in :
    stage_8;

  //data_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_7 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_7))
          if (sync_reset & full_7 & !((full_8 == 0) & read & write))
              stage_7 <= 0;
          else 
            stage_7 <= p7_stage_7;
    end


  //control_7, which is an e_mux
  assign p7_full_7 = ((read & !write) == 0)? full_6 :
    full_8;

  //control_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_7 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_7 <= 0;
          else 
            full_7 <= p7_full_7;
    end


  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    stage_7;

  //data_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_6 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_6))
          if (sync_reset & full_6 & !((full_7 == 0) & read & write))
              stage_6 <= 0;
          else 
            stage_6 <= p6_stage_6;
    end


  //control_6, which is an e_mux
  assign p6_full_6 = ((read & !write) == 0)? full_5 :
    full_7;

  //control_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_6 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_6 <= 0;
          else 
            full_6 <= p6_full_6;
    end


  //data_5, which is an e_mux
  assign p5_stage_5 = ((full_6 & ~clear_fifo) == 0)? data_in :
    stage_6;

  //data_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_5 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_5))
          if (sync_reset & full_5 & !((full_6 == 0) & read & write))
              stage_5 <= 0;
          else 
            stage_5 <= p5_stage_5;
    end


  //control_5, which is an e_mux
  assign p5_full_5 = ((read & !write) == 0)? full_4 :
    full_6;

  //control_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_5 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_5 <= 0;
          else 
            full_5 <= p5_full_5;
    end


  //data_4, which is an e_mux
  assign p4_stage_4 = ((full_5 & ~clear_fifo) == 0)? data_in :
    stage_5;

  //data_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_4 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_4))
          if (sync_reset & full_4 & !((full_5 == 0) & read & write))
              stage_4 <= 0;
          else 
            stage_4 <= p4_stage_4;
    end


  //control_4, which is an e_mux
  assign p4_full_4 = ((read & !write) == 0)? full_3 :
    full_5;

  //control_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_4 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_4 <= 0;
          else 
            full_4 <= p4_full_4;
    end


  //data_3, which is an e_mux
  assign p3_stage_3 = ((full_4 & ~clear_fifo) == 0)? data_in :
    stage_4;

  //data_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_3 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_3))
          if (sync_reset & full_3 & !((full_4 == 0) & read & write))
              stage_3 <= 0;
          else 
            stage_3 <= p3_stage_3;
    end


  //control_3, which is an e_mux
  assign p3_full_3 = ((read & !write) == 0)? full_2 :
    full_4;

  //control_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_3 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_3 <= 0;
          else 
            full_3 <= p3_full_3;
    end


  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    stage_3;

  //data_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_2 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_2))
          if (sync_reset & full_2 & !((full_3 == 0) & read & write))
              stage_2 <= 0;
          else 
            stage_2 <= p2_stage_2;
    end


  //control_2, which is an e_mux
  assign p2_full_2 = ((read & !write) == 0)? full_1 :
    full_3;

  //control_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_2 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_2 <= 0;
          else 
            full_2 <= p2_full_2;
    end


  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    stage_2;

  //data_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_1 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_1))
          if (sync_reset & full_1 & !((full_2 == 0) & read & write))
              stage_1 <= 0;
          else 
            stage_1 <= p1_stage_1;
    end


  //control_1, which is an e_mux
  assign p1_full_1 = ((read & !write) == 0)? full_0 :
    full_2;

  //control_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_1 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_1 <= 0;
          else 
            full_1 <= p1_full_1;
    end


  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    stage_1;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    full_1;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_data_cache_0_dataMaster0_to_pipeline_bridge_MEMORY_s1_module (
                                                                                   // inputs:
                                                                                    clear_fifo,
                                                                                    clk,
                                                                                    data_in,
                                                                                    read,
                                                                                    reset_n,
                                                                                    sync_reset,
                                                                                    write,

                                                                                   // outputs:
                                                                                    data_out,
                                                                                    empty,
                                                                                    fifo_contains_ones_n,
                                                                                    full
                                                                                 )
;

  output           data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input            data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire             data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_10;
  reg              full_11;
  reg              full_12;
  reg              full_13;
  reg              full_14;
  reg              full_15;
  reg              full_16;
  reg              full_17;
  reg              full_18;
  reg              full_19;
  reg              full_2;
  reg              full_20;
  reg              full_21;
  reg              full_22;
  reg              full_23;
  reg              full_24;
  reg              full_25;
  reg              full_26;
  reg              full_27;
  reg              full_28;
  reg              full_29;
  reg              full_3;
  reg              full_30;
  reg              full_31;
  reg              full_32;
  reg              full_33;
  reg              full_34;
  wire             full_35;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  6: 0] how_many_ones;
  wire    [  6: 0] one_count_minus_one;
  wire    [  6: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p10_full_10;
  wire             p10_stage_10;
  wire             p11_full_11;
  wire             p11_stage_11;
  wire             p12_full_12;
  wire             p12_stage_12;
  wire             p13_full_13;
  wire             p13_stage_13;
  wire             p14_full_14;
  wire             p14_stage_14;
  wire             p15_full_15;
  wire             p15_stage_15;
  wire             p16_full_16;
  wire             p16_stage_16;
  wire             p17_full_17;
  wire             p17_stage_17;
  wire             p18_full_18;
  wire             p18_stage_18;
  wire             p19_full_19;
  wire             p19_stage_19;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p20_full_20;
  wire             p20_stage_20;
  wire             p21_full_21;
  wire             p21_stage_21;
  wire             p22_full_22;
  wire             p22_stage_22;
  wire             p23_full_23;
  wire             p23_stage_23;
  wire             p24_full_24;
  wire             p24_stage_24;
  wire             p25_full_25;
  wire             p25_stage_25;
  wire             p26_full_26;
  wire             p26_stage_26;
  wire             p27_full_27;
  wire             p27_stage_27;
  wire             p28_full_28;
  wire             p28_stage_28;
  wire             p29_full_29;
  wire             p29_stage_29;
  wire             p2_full_2;
  wire             p2_stage_2;
  wire             p30_full_30;
  wire             p30_stage_30;
  wire             p31_full_31;
  wire             p31_stage_31;
  wire             p32_full_32;
  wire             p32_stage_32;
  wire             p33_full_33;
  wire             p33_stage_33;
  wire             p34_full_34;
  wire             p34_stage_34;
  wire             p3_full_3;
  wire             p3_stage_3;
  wire             p4_full_4;
  wire             p4_stage_4;
  wire             p5_full_5;
  wire             p5_stage_5;
  wire             p6_full_6;
  wire             p6_stage_6;
  wire             p7_full_7;
  wire             p7_stage_7;
  wire             p8_full_8;
  wire             p8_stage_8;
  wire             p9_full_9;
  wire             p9_stage_9;
  reg              stage_0;
  reg              stage_1;
  reg              stage_10;
  reg              stage_11;
  reg              stage_12;
  reg              stage_13;
  reg              stage_14;
  reg              stage_15;
  reg              stage_16;
  reg              stage_17;
  reg              stage_18;
  reg              stage_19;
  reg              stage_2;
  reg              stage_20;
  reg              stage_21;
  reg              stage_22;
  reg              stage_23;
  reg              stage_24;
  reg              stage_25;
  reg              stage_26;
  reg              stage_27;
  reg              stage_28;
  reg              stage_29;
  reg              stage_3;
  reg              stage_30;
  reg              stage_31;
  reg              stage_32;
  reg              stage_33;
  reg              stage_34;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  reg              stage_7;
  reg              stage_8;
  reg              stage_9;
  wire    [  6: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_34;
  assign empty = !full_0;
  assign full_35 = 0;
  //data_34, which is an e_mux
  assign p34_stage_34 = ((full_35 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_34, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_34 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_34))
          if (sync_reset & full_34 & !((full_35 == 0) & read & write))
              stage_34 <= 0;
          else 
            stage_34 <= p34_stage_34;
    end


  //control_34, which is an e_mux
  assign p34_full_34 = ((read & !write) == 0)? full_33 :
    0;

  //control_reg_34, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_34 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_34 <= 0;
          else 
            full_34 <= p34_full_34;
    end


  //data_33, which is an e_mux
  assign p33_stage_33 = ((full_34 & ~clear_fifo) == 0)? data_in :
    stage_34;

  //data_reg_33, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_33 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_33))
          if (sync_reset & full_33 & !((full_34 == 0) & read & write))
              stage_33 <= 0;
          else 
            stage_33 <= p33_stage_33;
    end


  //control_33, which is an e_mux
  assign p33_full_33 = ((read & !write) == 0)? full_32 :
    full_34;

  //control_reg_33, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_33 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_33 <= 0;
          else 
            full_33 <= p33_full_33;
    end


  //data_32, which is an e_mux
  assign p32_stage_32 = ((full_33 & ~clear_fifo) == 0)? data_in :
    stage_33;

  //data_reg_32, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_32 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_32))
          if (sync_reset & full_32 & !((full_33 == 0) & read & write))
              stage_32 <= 0;
          else 
            stage_32 <= p32_stage_32;
    end


  //control_32, which is an e_mux
  assign p32_full_32 = ((read & !write) == 0)? full_31 :
    full_33;

  //control_reg_32, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_32 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_32 <= 0;
          else 
            full_32 <= p32_full_32;
    end


  //data_31, which is an e_mux
  assign p31_stage_31 = ((full_32 & ~clear_fifo) == 0)? data_in :
    stage_32;

  //data_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_31 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_31))
          if (sync_reset & full_31 & !((full_32 == 0) & read & write))
              stage_31 <= 0;
          else 
            stage_31 <= p31_stage_31;
    end


  //control_31, which is an e_mux
  assign p31_full_31 = ((read & !write) == 0)? full_30 :
    full_32;

  //control_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_31 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_31 <= 0;
          else 
            full_31 <= p31_full_31;
    end


  //data_30, which is an e_mux
  assign p30_stage_30 = ((full_31 & ~clear_fifo) == 0)? data_in :
    stage_31;

  //data_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_30 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_30))
          if (sync_reset & full_30 & !((full_31 == 0) & read & write))
              stage_30 <= 0;
          else 
            stage_30 <= p30_stage_30;
    end


  //control_30, which is an e_mux
  assign p30_full_30 = ((read & !write) == 0)? full_29 :
    full_31;

  //control_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_30 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_30 <= 0;
          else 
            full_30 <= p30_full_30;
    end


  //data_29, which is an e_mux
  assign p29_stage_29 = ((full_30 & ~clear_fifo) == 0)? data_in :
    stage_30;

  //data_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_29 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_29))
          if (sync_reset & full_29 & !((full_30 == 0) & read & write))
              stage_29 <= 0;
          else 
            stage_29 <= p29_stage_29;
    end


  //control_29, which is an e_mux
  assign p29_full_29 = ((read & !write) == 0)? full_28 :
    full_30;

  //control_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_29 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_29 <= 0;
          else 
            full_29 <= p29_full_29;
    end


  //data_28, which is an e_mux
  assign p28_stage_28 = ((full_29 & ~clear_fifo) == 0)? data_in :
    stage_29;

  //data_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_28 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_28))
          if (sync_reset & full_28 & !((full_29 == 0) & read & write))
              stage_28 <= 0;
          else 
            stage_28 <= p28_stage_28;
    end


  //control_28, which is an e_mux
  assign p28_full_28 = ((read & !write) == 0)? full_27 :
    full_29;

  //control_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_28 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_28 <= 0;
          else 
            full_28 <= p28_full_28;
    end


  //data_27, which is an e_mux
  assign p27_stage_27 = ((full_28 & ~clear_fifo) == 0)? data_in :
    stage_28;

  //data_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_27 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_27))
          if (sync_reset & full_27 & !((full_28 == 0) & read & write))
              stage_27 <= 0;
          else 
            stage_27 <= p27_stage_27;
    end


  //control_27, which is an e_mux
  assign p27_full_27 = ((read & !write) == 0)? full_26 :
    full_28;

  //control_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_27 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_27 <= 0;
          else 
            full_27 <= p27_full_27;
    end


  //data_26, which is an e_mux
  assign p26_stage_26 = ((full_27 & ~clear_fifo) == 0)? data_in :
    stage_27;

  //data_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_26 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_26))
          if (sync_reset & full_26 & !((full_27 == 0) & read & write))
              stage_26 <= 0;
          else 
            stage_26 <= p26_stage_26;
    end


  //control_26, which is an e_mux
  assign p26_full_26 = ((read & !write) == 0)? full_25 :
    full_27;

  //control_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_26 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_26 <= 0;
          else 
            full_26 <= p26_full_26;
    end


  //data_25, which is an e_mux
  assign p25_stage_25 = ((full_26 & ~clear_fifo) == 0)? data_in :
    stage_26;

  //data_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_25 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_25))
          if (sync_reset & full_25 & !((full_26 == 0) & read & write))
              stage_25 <= 0;
          else 
            stage_25 <= p25_stage_25;
    end


  //control_25, which is an e_mux
  assign p25_full_25 = ((read & !write) == 0)? full_24 :
    full_26;

  //control_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_25 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_25 <= 0;
          else 
            full_25 <= p25_full_25;
    end


  //data_24, which is an e_mux
  assign p24_stage_24 = ((full_25 & ~clear_fifo) == 0)? data_in :
    stage_25;

  //data_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_24 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_24))
          if (sync_reset & full_24 & !((full_25 == 0) & read & write))
              stage_24 <= 0;
          else 
            stage_24 <= p24_stage_24;
    end


  //control_24, which is an e_mux
  assign p24_full_24 = ((read & !write) == 0)? full_23 :
    full_25;

  //control_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_24 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_24 <= 0;
          else 
            full_24 <= p24_full_24;
    end


  //data_23, which is an e_mux
  assign p23_stage_23 = ((full_24 & ~clear_fifo) == 0)? data_in :
    stage_24;

  //data_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_23 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_23))
          if (sync_reset & full_23 & !((full_24 == 0) & read & write))
              stage_23 <= 0;
          else 
            stage_23 <= p23_stage_23;
    end


  //control_23, which is an e_mux
  assign p23_full_23 = ((read & !write) == 0)? full_22 :
    full_24;

  //control_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_23 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_23 <= 0;
          else 
            full_23 <= p23_full_23;
    end


  //data_22, which is an e_mux
  assign p22_stage_22 = ((full_23 & ~clear_fifo) == 0)? data_in :
    stage_23;

  //data_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_22 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_22))
          if (sync_reset & full_22 & !((full_23 == 0) & read & write))
              stage_22 <= 0;
          else 
            stage_22 <= p22_stage_22;
    end


  //control_22, which is an e_mux
  assign p22_full_22 = ((read & !write) == 0)? full_21 :
    full_23;

  //control_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_22 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_22 <= 0;
          else 
            full_22 <= p22_full_22;
    end


  //data_21, which is an e_mux
  assign p21_stage_21 = ((full_22 & ~clear_fifo) == 0)? data_in :
    stage_22;

  //data_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_21 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_21))
          if (sync_reset & full_21 & !((full_22 == 0) & read & write))
              stage_21 <= 0;
          else 
            stage_21 <= p21_stage_21;
    end


  //control_21, which is an e_mux
  assign p21_full_21 = ((read & !write) == 0)? full_20 :
    full_22;

  //control_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_21 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_21 <= 0;
          else 
            full_21 <= p21_full_21;
    end


  //data_20, which is an e_mux
  assign p20_stage_20 = ((full_21 & ~clear_fifo) == 0)? data_in :
    stage_21;

  //data_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_20 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_20))
          if (sync_reset & full_20 & !((full_21 == 0) & read & write))
              stage_20 <= 0;
          else 
            stage_20 <= p20_stage_20;
    end


  //control_20, which is an e_mux
  assign p20_full_20 = ((read & !write) == 0)? full_19 :
    full_21;

  //control_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_20 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_20 <= 0;
          else 
            full_20 <= p20_full_20;
    end


  //data_19, which is an e_mux
  assign p19_stage_19 = ((full_20 & ~clear_fifo) == 0)? data_in :
    stage_20;

  //data_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_19 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_19))
          if (sync_reset & full_19 & !((full_20 == 0) & read & write))
              stage_19 <= 0;
          else 
            stage_19 <= p19_stage_19;
    end


  //control_19, which is an e_mux
  assign p19_full_19 = ((read & !write) == 0)? full_18 :
    full_20;

  //control_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_19 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_19 <= 0;
          else 
            full_19 <= p19_full_19;
    end


  //data_18, which is an e_mux
  assign p18_stage_18 = ((full_19 & ~clear_fifo) == 0)? data_in :
    stage_19;

  //data_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_18 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_18))
          if (sync_reset & full_18 & !((full_19 == 0) & read & write))
              stage_18 <= 0;
          else 
            stage_18 <= p18_stage_18;
    end


  //control_18, which is an e_mux
  assign p18_full_18 = ((read & !write) == 0)? full_17 :
    full_19;

  //control_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_18 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_18 <= 0;
          else 
            full_18 <= p18_full_18;
    end


  //data_17, which is an e_mux
  assign p17_stage_17 = ((full_18 & ~clear_fifo) == 0)? data_in :
    stage_18;

  //data_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_17 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_17))
          if (sync_reset & full_17 & !((full_18 == 0) & read & write))
              stage_17 <= 0;
          else 
            stage_17 <= p17_stage_17;
    end


  //control_17, which is an e_mux
  assign p17_full_17 = ((read & !write) == 0)? full_16 :
    full_18;

  //control_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_17 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_17 <= 0;
          else 
            full_17 <= p17_full_17;
    end


  //data_16, which is an e_mux
  assign p16_stage_16 = ((full_17 & ~clear_fifo) == 0)? data_in :
    stage_17;

  //data_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_16 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_16))
          if (sync_reset & full_16 & !((full_17 == 0) & read & write))
              stage_16 <= 0;
          else 
            stage_16 <= p16_stage_16;
    end


  //control_16, which is an e_mux
  assign p16_full_16 = ((read & !write) == 0)? full_15 :
    full_17;

  //control_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_16 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_16 <= 0;
          else 
            full_16 <= p16_full_16;
    end


  //data_15, which is an e_mux
  assign p15_stage_15 = ((full_16 & ~clear_fifo) == 0)? data_in :
    stage_16;

  //data_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_15 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_15))
          if (sync_reset & full_15 & !((full_16 == 0) & read & write))
              stage_15 <= 0;
          else 
            stage_15 <= p15_stage_15;
    end


  //control_15, which is an e_mux
  assign p15_full_15 = ((read & !write) == 0)? full_14 :
    full_16;

  //control_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_15 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_15 <= 0;
          else 
            full_15 <= p15_full_15;
    end


  //data_14, which is an e_mux
  assign p14_stage_14 = ((full_15 & ~clear_fifo) == 0)? data_in :
    stage_15;

  //data_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_14 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_14))
          if (sync_reset & full_14 & !((full_15 == 0) & read & write))
              stage_14 <= 0;
          else 
            stage_14 <= p14_stage_14;
    end


  //control_14, which is an e_mux
  assign p14_full_14 = ((read & !write) == 0)? full_13 :
    full_15;

  //control_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_14 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_14 <= 0;
          else 
            full_14 <= p14_full_14;
    end


  //data_13, which is an e_mux
  assign p13_stage_13 = ((full_14 & ~clear_fifo) == 0)? data_in :
    stage_14;

  //data_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_13 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_13))
          if (sync_reset & full_13 & !((full_14 == 0) & read & write))
              stage_13 <= 0;
          else 
            stage_13 <= p13_stage_13;
    end


  //control_13, which is an e_mux
  assign p13_full_13 = ((read & !write) == 0)? full_12 :
    full_14;

  //control_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_13 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_13 <= 0;
          else 
            full_13 <= p13_full_13;
    end


  //data_12, which is an e_mux
  assign p12_stage_12 = ((full_13 & ~clear_fifo) == 0)? data_in :
    stage_13;

  //data_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_12 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_12))
          if (sync_reset & full_12 & !((full_13 == 0) & read & write))
              stage_12 <= 0;
          else 
            stage_12 <= p12_stage_12;
    end


  //control_12, which is an e_mux
  assign p12_full_12 = ((read & !write) == 0)? full_11 :
    full_13;

  //control_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_12 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_12 <= 0;
          else 
            full_12 <= p12_full_12;
    end


  //data_11, which is an e_mux
  assign p11_stage_11 = ((full_12 & ~clear_fifo) == 0)? data_in :
    stage_12;

  //data_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_11 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_11))
          if (sync_reset & full_11 & !((full_12 == 0) & read & write))
              stage_11 <= 0;
          else 
            stage_11 <= p11_stage_11;
    end


  //control_11, which is an e_mux
  assign p11_full_11 = ((read & !write) == 0)? full_10 :
    full_12;

  //control_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_11 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_11 <= 0;
          else 
            full_11 <= p11_full_11;
    end


  //data_10, which is an e_mux
  assign p10_stage_10 = ((full_11 & ~clear_fifo) == 0)? data_in :
    stage_11;

  //data_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_10 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_10))
          if (sync_reset & full_10 & !((full_11 == 0) & read & write))
              stage_10 <= 0;
          else 
            stage_10 <= p10_stage_10;
    end


  //control_10, which is an e_mux
  assign p10_full_10 = ((read & !write) == 0)? full_9 :
    full_11;

  //control_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_10 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_10 <= 0;
          else 
            full_10 <= p10_full_10;
    end


  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    stage_10;

  //data_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_9 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_9))
          if (sync_reset & full_9 & !((full_10 == 0) & read & write))
              stage_9 <= 0;
          else 
            stage_9 <= p9_stage_9;
    end


  //control_9, which is an e_mux
  assign p9_full_9 = ((read & !write) == 0)? full_8 :
    full_10;

  //control_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_9 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_9 <= 0;
          else 
            full_9 <= p9_full_9;
    end


  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    stage_9;

  //data_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_8 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_8))
          if (sync_reset & full_8 & !((full_9 == 0) & read & write))
              stage_8 <= 0;
          else 
            stage_8 <= p8_stage_8;
    end


  //control_8, which is an e_mux
  assign p8_full_8 = ((read & !write) == 0)? full_7 :
    full_9;

  //control_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_8 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_8 <= 0;
          else 
            full_8 <= p8_full_8;
    end


  //data_7, which is an e_mux
  assign p7_stage_7 = ((full_8 & ~clear_fifo) == 0)? data_in :
    stage_8;

  //data_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_7 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_7))
          if (sync_reset & full_7 & !((full_8 == 0) & read & write))
              stage_7 <= 0;
          else 
            stage_7 <= p7_stage_7;
    end


  //control_7, which is an e_mux
  assign p7_full_7 = ((read & !write) == 0)? full_6 :
    full_8;

  //control_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_7 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_7 <= 0;
          else 
            full_7 <= p7_full_7;
    end


  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    stage_7;

  //data_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_6 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_6))
          if (sync_reset & full_6 & !((full_7 == 0) & read & write))
              stage_6 <= 0;
          else 
            stage_6 <= p6_stage_6;
    end


  //control_6, which is an e_mux
  assign p6_full_6 = ((read & !write) == 0)? full_5 :
    full_7;

  //control_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_6 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_6 <= 0;
          else 
            full_6 <= p6_full_6;
    end


  //data_5, which is an e_mux
  assign p5_stage_5 = ((full_6 & ~clear_fifo) == 0)? data_in :
    stage_6;

  //data_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_5 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_5))
          if (sync_reset & full_5 & !((full_6 == 0) & read & write))
              stage_5 <= 0;
          else 
            stage_5 <= p5_stage_5;
    end


  //control_5, which is an e_mux
  assign p5_full_5 = ((read & !write) == 0)? full_4 :
    full_6;

  //control_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_5 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_5 <= 0;
          else 
            full_5 <= p5_full_5;
    end


  //data_4, which is an e_mux
  assign p4_stage_4 = ((full_5 & ~clear_fifo) == 0)? data_in :
    stage_5;

  //data_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_4 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_4))
          if (sync_reset & full_4 & !((full_5 == 0) & read & write))
              stage_4 <= 0;
          else 
            stage_4 <= p4_stage_4;
    end


  //control_4, which is an e_mux
  assign p4_full_4 = ((read & !write) == 0)? full_3 :
    full_5;

  //control_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_4 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_4 <= 0;
          else 
            full_4 <= p4_full_4;
    end


  //data_3, which is an e_mux
  assign p3_stage_3 = ((full_4 & ~clear_fifo) == 0)? data_in :
    stage_4;

  //data_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_3 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_3))
          if (sync_reset & full_3 & !((full_4 == 0) & read & write))
              stage_3 <= 0;
          else 
            stage_3 <= p3_stage_3;
    end


  //control_3, which is an e_mux
  assign p3_full_3 = ((read & !write) == 0)? full_2 :
    full_4;

  //control_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_3 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_3 <= 0;
          else 
            full_3 <= p3_full_3;
    end


  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    stage_3;

  //data_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_2 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_2))
          if (sync_reset & full_2 & !((full_3 == 0) & read & write))
              stage_2 <= 0;
          else 
            stage_2 <= p2_stage_2;
    end


  //control_2, which is an e_mux
  assign p2_full_2 = ((read & !write) == 0)? full_1 :
    full_3;

  //control_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_2 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_2 <= 0;
          else 
            full_2 <= p2_full_2;
    end


  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    stage_2;

  //data_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_1 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_1))
          if (sync_reset & full_1 & !((full_2 == 0) & read & write))
              stage_1 <= 0;
          else 
            stage_1 <= p1_stage_1;
    end


  //control_1, which is an e_mux
  assign p1_full_1 = ((read & !write) == 0)? full_0 :
    full_2;

  //control_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_1 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_1 <= 0;
          else 
            full_1 <= p1_full_1;
    end


  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    stage_1;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    full_1;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_tiger_top_0_instructionMaster_to_pipeline_bridge_MEMORY_s1_module (
                                                                                        // inputs:
                                                                                         clear_fifo,
                                                                                         clk,
                                                                                         data_in,
                                                                                         read,
                                                                                         reset_n,
                                                                                         sync_reset,
                                                                                         write,

                                                                                        // outputs:
                                                                                         data_out,
                                                                                         empty,
                                                                                         fifo_contains_ones_n,
                                                                                         full
                                                                                      )
;

  output           data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input            data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire             data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_10;
  reg              full_11;
  reg              full_12;
  reg              full_13;
  reg              full_14;
  reg              full_15;
  reg              full_16;
  reg              full_17;
  reg              full_18;
  reg              full_19;
  reg              full_2;
  reg              full_20;
  reg              full_21;
  reg              full_22;
  reg              full_23;
  reg              full_24;
  reg              full_25;
  reg              full_26;
  reg              full_27;
  reg              full_28;
  reg              full_29;
  reg              full_3;
  reg              full_30;
  reg              full_31;
  reg              full_32;
  reg              full_33;
  reg              full_34;
  wire             full_35;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  6: 0] how_many_ones;
  wire    [  6: 0] one_count_minus_one;
  wire    [  6: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p10_full_10;
  wire             p10_stage_10;
  wire             p11_full_11;
  wire             p11_stage_11;
  wire             p12_full_12;
  wire             p12_stage_12;
  wire             p13_full_13;
  wire             p13_stage_13;
  wire             p14_full_14;
  wire             p14_stage_14;
  wire             p15_full_15;
  wire             p15_stage_15;
  wire             p16_full_16;
  wire             p16_stage_16;
  wire             p17_full_17;
  wire             p17_stage_17;
  wire             p18_full_18;
  wire             p18_stage_18;
  wire             p19_full_19;
  wire             p19_stage_19;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p20_full_20;
  wire             p20_stage_20;
  wire             p21_full_21;
  wire             p21_stage_21;
  wire             p22_full_22;
  wire             p22_stage_22;
  wire             p23_full_23;
  wire             p23_stage_23;
  wire             p24_full_24;
  wire             p24_stage_24;
  wire             p25_full_25;
  wire             p25_stage_25;
  wire             p26_full_26;
  wire             p26_stage_26;
  wire             p27_full_27;
  wire             p27_stage_27;
  wire             p28_full_28;
  wire             p28_stage_28;
  wire             p29_full_29;
  wire             p29_stage_29;
  wire             p2_full_2;
  wire             p2_stage_2;
  wire             p30_full_30;
  wire             p30_stage_30;
  wire             p31_full_31;
  wire             p31_stage_31;
  wire             p32_full_32;
  wire             p32_stage_32;
  wire             p33_full_33;
  wire             p33_stage_33;
  wire             p34_full_34;
  wire             p34_stage_34;
  wire             p3_full_3;
  wire             p3_stage_3;
  wire             p4_full_4;
  wire             p4_stage_4;
  wire             p5_full_5;
  wire             p5_stage_5;
  wire             p6_full_6;
  wire             p6_stage_6;
  wire             p7_full_7;
  wire             p7_stage_7;
  wire             p8_full_8;
  wire             p8_stage_8;
  wire             p9_full_9;
  wire             p9_stage_9;
  reg              stage_0;
  reg              stage_1;
  reg              stage_10;
  reg              stage_11;
  reg              stage_12;
  reg              stage_13;
  reg              stage_14;
  reg              stage_15;
  reg              stage_16;
  reg              stage_17;
  reg              stage_18;
  reg              stage_19;
  reg              stage_2;
  reg              stage_20;
  reg              stage_21;
  reg              stage_22;
  reg              stage_23;
  reg              stage_24;
  reg              stage_25;
  reg              stage_26;
  reg              stage_27;
  reg              stage_28;
  reg              stage_29;
  reg              stage_3;
  reg              stage_30;
  reg              stage_31;
  reg              stage_32;
  reg              stage_33;
  reg              stage_34;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  reg              stage_7;
  reg              stage_8;
  reg              stage_9;
  wire    [  6: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_34;
  assign empty = !full_0;
  assign full_35 = 0;
  //data_34, which is an e_mux
  assign p34_stage_34 = ((full_35 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_34, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_34 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_34))
          if (sync_reset & full_34 & !((full_35 == 0) & read & write))
              stage_34 <= 0;
          else 
            stage_34 <= p34_stage_34;
    end


  //control_34, which is an e_mux
  assign p34_full_34 = ((read & !write) == 0)? full_33 :
    0;

  //control_reg_34, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_34 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_34 <= 0;
          else 
            full_34 <= p34_full_34;
    end


  //data_33, which is an e_mux
  assign p33_stage_33 = ((full_34 & ~clear_fifo) == 0)? data_in :
    stage_34;

  //data_reg_33, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_33 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_33))
          if (sync_reset & full_33 & !((full_34 == 0) & read & write))
              stage_33 <= 0;
          else 
            stage_33 <= p33_stage_33;
    end


  //control_33, which is an e_mux
  assign p33_full_33 = ((read & !write) == 0)? full_32 :
    full_34;

  //control_reg_33, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_33 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_33 <= 0;
          else 
            full_33 <= p33_full_33;
    end


  //data_32, which is an e_mux
  assign p32_stage_32 = ((full_33 & ~clear_fifo) == 0)? data_in :
    stage_33;

  //data_reg_32, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_32 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_32))
          if (sync_reset & full_32 & !((full_33 == 0) & read & write))
              stage_32 <= 0;
          else 
            stage_32 <= p32_stage_32;
    end


  //control_32, which is an e_mux
  assign p32_full_32 = ((read & !write) == 0)? full_31 :
    full_33;

  //control_reg_32, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_32 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_32 <= 0;
          else 
            full_32 <= p32_full_32;
    end


  //data_31, which is an e_mux
  assign p31_stage_31 = ((full_32 & ~clear_fifo) == 0)? data_in :
    stage_32;

  //data_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_31 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_31))
          if (sync_reset & full_31 & !((full_32 == 0) & read & write))
              stage_31 <= 0;
          else 
            stage_31 <= p31_stage_31;
    end


  //control_31, which is an e_mux
  assign p31_full_31 = ((read & !write) == 0)? full_30 :
    full_32;

  //control_reg_31, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_31 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_31 <= 0;
          else 
            full_31 <= p31_full_31;
    end


  //data_30, which is an e_mux
  assign p30_stage_30 = ((full_31 & ~clear_fifo) == 0)? data_in :
    stage_31;

  //data_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_30 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_30))
          if (sync_reset & full_30 & !((full_31 == 0) & read & write))
              stage_30 <= 0;
          else 
            stage_30 <= p30_stage_30;
    end


  //control_30, which is an e_mux
  assign p30_full_30 = ((read & !write) == 0)? full_29 :
    full_31;

  //control_reg_30, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_30 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_30 <= 0;
          else 
            full_30 <= p30_full_30;
    end


  //data_29, which is an e_mux
  assign p29_stage_29 = ((full_30 & ~clear_fifo) == 0)? data_in :
    stage_30;

  //data_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_29 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_29))
          if (sync_reset & full_29 & !((full_30 == 0) & read & write))
              stage_29 <= 0;
          else 
            stage_29 <= p29_stage_29;
    end


  //control_29, which is an e_mux
  assign p29_full_29 = ((read & !write) == 0)? full_28 :
    full_30;

  //control_reg_29, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_29 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_29 <= 0;
          else 
            full_29 <= p29_full_29;
    end


  //data_28, which is an e_mux
  assign p28_stage_28 = ((full_29 & ~clear_fifo) == 0)? data_in :
    stage_29;

  //data_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_28 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_28))
          if (sync_reset & full_28 & !((full_29 == 0) & read & write))
              stage_28 <= 0;
          else 
            stage_28 <= p28_stage_28;
    end


  //control_28, which is an e_mux
  assign p28_full_28 = ((read & !write) == 0)? full_27 :
    full_29;

  //control_reg_28, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_28 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_28 <= 0;
          else 
            full_28 <= p28_full_28;
    end


  //data_27, which is an e_mux
  assign p27_stage_27 = ((full_28 & ~clear_fifo) == 0)? data_in :
    stage_28;

  //data_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_27 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_27))
          if (sync_reset & full_27 & !((full_28 == 0) & read & write))
              stage_27 <= 0;
          else 
            stage_27 <= p27_stage_27;
    end


  //control_27, which is an e_mux
  assign p27_full_27 = ((read & !write) == 0)? full_26 :
    full_28;

  //control_reg_27, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_27 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_27 <= 0;
          else 
            full_27 <= p27_full_27;
    end


  //data_26, which is an e_mux
  assign p26_stage_26 = ((full_27 & ~clear_fifo) == 0)? data_in :
    stage_27;

  //data_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_26 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_26))
          if (sync_reset & full_26 & !((full_27 == 0) & read & write))
              stage_26 <= 0;
          else 
            stage_26 <= p26_stage_26;
    end


  //control_26, which is an e_mux
  assign p26_full_26 = ((read & !write) == 0)? full_25 :
    full_27;

  //control_reg_26, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_26 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_26 <= 0;
          else 
            full_26 <= p26_full_26;
    end


  //data_25, which is an e_mux
  assign p25_stage_25 = ((full_26 & ~clear_fifo) == 0)? data_in :
    stage_26;

  //data_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_25 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_25))
          if (sync_reset & full_25 & !((full_26 == 0) & read & write))
              stage_25 <= 0;
          else 
            stage_25 <= p25_stage_25;
    end


  //control_25, which is an e_mux
  assign p25_full_25 = ((read & !write) == 0)? full_24 :
    full_26;

  //control_reg_25, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_25 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_25 <= 0;
          else 
            full_25 <= p25_full_25;
    end


  //data_24, which is an e_mux
  assign p24_stage_24 = ((full_25 & ~clear_fifo) == 0)? data_in :
    stage_25;

  //data_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_24 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_24))
          if (sync_reset & full_24 & !((full_25 == 0) & read & write))
              stage_24 <= 0;
          else 
            stage_24 <= p24_stage_24;
    end


  //control_24, which is an e_mux
  assign p24_full_24 = ((read & !write) == 0)? full_23 :
    full_25;

  //control_reg_24, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_24 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_24 <= 0;
          else 
            full_24 <= p24_full_24;
    end


  //data_23, which is an e_mux
  assign p23_stage_23 = ((full_24 & ~clear_fifo) == 0)? data_in :
    stage_24;

  //data_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_23 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_23))
          if (sync_reset & full_23 & !((full_24 == 0) & read & write))
              stage_23 <= 0;
          else 
            stage_23 <= p23_stage_23;
    end


  //control_23, which is an e_mux
  assign p23_full_23 = ((read & !write) == 0)? full_22 :
    full_24;

  //control_reg_23, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_23 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_23 <= 0;
          else 
            full_23 <= p23_full_23;
    end


  //data_22, which is an e_mux
  assign p22_stage_22 = ((full_23 & ~clear_fifo) == 0)? data_in :
    stage_23;

  //data_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_22 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_22))
          if (sync_reset & full_22 & !((full_23 == 0) & read & write))
              stage_22 <= 0;
          else 
            stage_22 <= p22_stage_22;
    end


  //control_22, which is an e_mux
  assign p22_full_22 = ((read & !write) == 0)? full_21 :
    full_23;

  //control_reg_22, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_22 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_22 <= 0;
          else 
            full_22 <= p22_full_22;
    end


  //data_21, which is an e_mux
  assign p21_stage_21 = ((full_22 & ~clear_fifo) == 0)? data_in :
    stage_22;

  //data_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_21 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_21))
          if (sync_reset & full_21 & !((full_22 == 0) & read & write))
              stage_21 <= 0;
          else 
            stage_21 <= p21_stage_21;
    end


  //control_21, which is an e_mux
  assign p21_full_21 = ((read & !write) == 0)? full_20 :
    full_22;

  //control_reg_21, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_21 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_21 <= 0;
          else 
            full_21 <= p21_full_21;
    end


  //data_20, which is an e_mux
  assign p20_stage_20 = ((full_21 & ~clear_fifo) == 0)? data_in :
    stage_21;

  //data_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_20 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_20))
          if (sync_reset & full_20 & !((full_21 == 0) & read & write))
              stage_20 <= 0;
          else 
            stage_20 <= p20_stage_20;
    end


  //control_20, which is an e_mux
  assign p20_full_20 = ((read & !write) == 0)? full_19 :
    full_21;

  //control_reg_20, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_20 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_20 <= 0;
          else 
            full_20 <= p20_full_20;
    end


  //data_19, which is an e_mux
  assign p19_stage_19 = ((full_20 & ~clear_fifo) == 0)? data_in :
    stage_20;

  //data_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_19 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_19))
          if (sync_reset & full_19 & !((full_20 == 0) & read & write))
              stage_19 <= 0;
          else 
            stage_19 <= p19_stage_19;
    end


  //control_19, which is an e_mux
  assign p19_full_19 = ((read & !write) == 0)? full_18 :
    full_20;

  //control_reg_19, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_19 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_19 <= 0;
          else 
            full_19 <= p19_full_19;
    end


  //data_18, which is an e_mux
  assign p18_stage_18 = ((full_19 & ~clear_fifo) == 0)? data_in :
    stage_19;

  //data_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_18 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_18))
          if (sync_reset & full_18 & !((full_19 == 0) & read & write))
              stage_18 <= 0;
          else 
            stage_18 <= p18_stage_18;
    end


  //control_18, which is an e_mux
  assign p18_full_18 = ((read & !write) == 0)? full_17 :
    full_19;

  //control_reg_18, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_18 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_18 <= 0;
          else 
            full_18 <= p18_full_18;
    end


  //data_17, which is an e_mux
  assign p17_stage_17 = ((full_18 & ~clear_fifo) == 0)? data_in :
    stage_18;

  //data_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_17 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_17))
          if (sync_reset & full_17 & !((full_18 == 0) & read & write))
              stage_17 <= 0;
          else 
            stage_17 <= p17_stage_17;
    end


  //control_17, which is an e_mux
  assign p17_full_17 = ((read & !write) == 0)? full_16 :
    full_18;

  //control_reg_17, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_17 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_17 <= 0;
          else 
            full_17 <= p17_full_17;
    end


  //data_16, which is an e_mux
  assign p16_stage_16 = ((full_17 & ~clear_fifo) == 0)? data_in :
    stage_17;

  //data_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_16 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_16))
          if (sync_reset & full_16 & !((full_17 == 0) & read & write))
              stage_16 <= 0;
          else 
            stage_16 <= p16_stage_16;
    end


  //control_16, which is an e_mux
  assign p16_full_16 = ((read & !write) == 0)? full_15 :
    full_17;

  //control_reg_16, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_16 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_16 <= 0;
          else 
            full_16 <= p16_full_16;
    end


  //data_15, which is an e_mux
  assign p15_stage_15 = ((full_16 & ~clear_fifo) == 0)? data_in :
    stage_16;

  //data_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_15 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_15))
          if (sync_reset & full_15 & !((full_16 == 0) & read & write))
              stage_15 <= 0;
          else 
            stage_15 <= p15_stage_15;
    end


  //control_15, which is an e_mux
  assign p15_full_15 = ((read & !write) == 0)? full_14 :
    full_16;

  //control_reg_15, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_15 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_15 <= 0;
          else 
            full_15 <= p15_full_15;
    end


  //data_14, which is an e_mux
  assign p14_stage_14 = ((full_15 & ~clear_fifo) == 0)? data_in :
    stage_15;

  //data_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_14 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_14))
          if (sync_reset & full_14 & !((full_15 == 0) & read & write))
              stage_14 <= 0;
          else 
            stage_14 <= p14_stage_14;
    end


  //control_14, which is an e_mux
  assign p14_full_14 = ((read & !write) == 0)? full_13 :
    full_15;

  //control_reg_14, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_14 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_14 <= 0;
          else 
            full_14 <= p14_full_14;
    end


  //data_13, which is an e_mux
  assign p13_stage_13 = ((full_14 & ~clear_fifo) == 0)? data_in :
    stage_14;

  //data_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_13 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_13))
          if (sync_reset & full_13 & !((full_14 == 0) & read & write))
              stage_13 <= 0;
          else 
            stage_13 <= p13_stage_13;
    end


  //control_13, which is an e_mux
  assign p13_full_13 = ((read & !write) == 0)? full_12 :
    full_14;

  //control_reg_13, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_13 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_13 <= 0;
          else 
            full_13 <= p13_full_13;
    end


  //data_12, which is an e_mux
  assign p12_stage_12 = ((full_13 & ~clear_fifo) == 0)? data_in :
    stage_13;

  //data_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_12 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_12))
          if (sync_reset & full_12 & !((full_13 == 0) & read & write))
              stage_12 <= 0;
          else 
            stage_12 <= p12_stage_12;
    end


  //control_12, which is an e_mux
  assign p12_full_12 = ((read & !write) == 0)? full_11 :
    full_13;

  //control_reg_12, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_12 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_12 <= 0;
          else 
            full_12 <= p12_full_12;
    end


  //data_11, which is an e_mux
  assign p11_stage_11 = ((full_12 & ~clear_fifo) == 0)? data_in :
    stage_12;

  //data_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_11 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_11))
          if (sync_reset & full_11 & !((full_12 == 0) & read & write))
              stage_11 <= 0;
          else 
            stage_11 <= p11_stage_11;
    end


  //control_11, which is an e_mux
  assign p11_full_11 = ((read & !write) == 0)? full_10 :
    full_12;

  //control_reg_11, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_11 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_11 <= 0;
          else 
            full_11 <= p11_full_11;
    end


  //data_10, which is an e_mux
  assign p10_stage_10 = ((full_11 & ~clear_fifo) == 0)? data_in :
    stage_11;

  //data_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_10 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_10))
          if (sync_reset & full_10 & !((full_11 == 0) & read & write))
              stage_10 <= 0;
          else 
            stage_10 <= p10_stage_10;
    end


  //control_10, which is an e_mux
  assign p10_full_10 = ((read & !write) == 0)? full_9 :
    full_11;

  //control_reg_10, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_10 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_10 <= 0;
          else 
            full_10 <= p10_full_10;
    end


  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    stage_10;

  //data_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_9 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_9))
          if (sync_reset & full_9 & !((full_10 == 0) & read & write))
              stage_9 <= 0;
          else 
            stage_9 <= p9_stage_9;
    end


  //control_9, which is an e_mux
  assign p9_full_9 = ((read & !write) == 0)? full_8 :
    full_10;

  //control_reg_9, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_9 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_9 <= 0;
          else 
            full_9 <= p9_full_9;
    end


  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    stage_9;

  //data_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_8 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_8))
          if (sync_reset & full_8 & !((full_9 == 0) & read & write))
              stage_8 <= 0;
          else 
            stage_8 <= p8_stage_8;
    end


  //control_8, which is an e_mux
  assign p8_full_8 = ((read & !write) == 0)? full_7 :
    full_9;

  //control_reg_8, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_8 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_8 <= 0;
          else 
            full_8 <= p8_full_8;
    end


  //data_7, which is an e_mux
  assign p7_stage_7 = ((full_8 & ~clear_fifo) == 0)? data_in :
    stage_8;

  //data_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_7 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_7))
          if (sync_reset & full_7 & !((full_8 == 0) & read & write))
              stage_7 <= 0;
          else 
            stage_7 <= p7_stage_7;
    end


  //control_7, which is an e_mux
  assign p7_full_7 = ((read & !write) == 0)? full_6 :
    full_8;

  //control_reg_7, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_7 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_7 <= 0;
          else 
            full_7 <= p7_full_7;
    end


  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    stage_7;

  //data_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_6 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_6))
          if (sync_reset & full_6 & !((full_7 == 0) & read & write))
              stage_6 <= 0;
          else 
            stage_6 <= p6_stage_6;
    end


  //control_6, which is an e_mux
  assign p6_full_6 = ((read & !write) == 0)? full_5 :
    full_7;

  //control_reg_6, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_6 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_6 <= 0;
          else 
            full_6 <= p6_full_6;
    end


  //data_5, which is an e_mux
  assign p5_stage_5 = ((full_6 & ~clear_fifo) == 0)? data_in :
    stage_6;

  //data_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_5 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_5))
          if (sync_reset & full_5 & !((full_6 == 0) & read & write))
              stage_5 <= 0;
          else 
            stage_5 <= p5_stage_5;
    end


  //control_5, which is an e_mux
  assign p5_full_5 = ((read & !write) == 0)? full_4 :
    full_6;

  //control_reg_5, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_5 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_5 <= 0;
          else 
            full_5 <= p5_full_5;
    end


  //data_4, which is an e_mux
  assign p4_stage_4 = ((full_5 & ~clear_fifo) == 0)? data_in :
    stage_5;

  //data_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_4 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_4))
          if (sync_reset & full_4 & !((full_5 == 0) & read & write))
              stage_4 <= 0;
          else 
            stage_4 <= p4_stage_4;
    end


  //control_4, which is an e_mux
  assign p4_full_4 = ((read & !write) == 0)? full_3 :
    full_5;

  //control_reg_4, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_4 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_4 <= 0;
          else 
            full_4 <= p4_full_4;
    end


  //data_3, which is an e_mux
  assign p3_stage_3 = ((full_4 & ~clear_fifo) == 0)? data_in :
    stage_4;

  //data_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_3 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_3))
          if (sync_reset & full_3 & !((full_4 == 0) & read & write))
              stage_3 <= 0;
          else 
            stage_3 <= p3_stage_3;
    end


  //control_3, which is an e_mux
  assign p3_full_3 = ((read & !write) == 0)? full_2 :
    full_4;

  //control_reg_3, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_3 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_3 <= 0;
          else 
            full_3 <= p3_full_3;
    end


  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    stage_3;

  //data_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_2 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_2))
          if (sync_reset & full_2 & !((full_3 == 0) & read & write))
              stage_2 <= 0;
          else 
            stage_2 <= p2_stage_2;
    end


  //control_2, which is an e_mux
  assign p2_full_2 = ((read & !write) == 0)? full_1 :
    full_3;

  //control_reg_2, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_2 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_2 <= 0;
          else 
            full_2 <= p2_full_2;
    end


  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    stage_2;

  //data_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_1 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_1))
          if (sync_reset & full_1 & !((full_2 == 0) & read & write))
              stage_1 <= 0;
          else 
            stage_1 <= p1_stage_1;
    end


  //control_1, which is an e_mux
  assign p1_full_1 = ((read & !write) == 0)? full_0 :
    full_2;

  //control_reg_1, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_1 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo)
              full_1 <= 0;
          else 
            full_1 <= p1_full_1;
    end


  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    stage_1;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    full_1;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_MEMORY_s1_arbitrator (
                                              // inputs:
                                               clk,
                                               data_cache_0_dataMaster0_address_to_slave,
                                               data_cache_0_dataMaster0_burstcount,
                                               data_cache_0_dataMaster0_byteenable,
                                               data_cache_0_dataMaster0_latency_counter,
                                               data_cache_0_dataMaster0_read,
                                               data_cache_0_dataMaster0_write,
                                               data_cache_0_dataMaster0_writedata,
                                               pipeline_bridge_MEMORY_s1_endofpacket,
                                               pipeline_bridge_MEMORY_s1_readdata,
                                               pipeline_bridge_MEMORY_s1_readdatavalid,
                                               pipeline_bridge_MEMORY_s1_waitrequest,
                                               reset_n,
                                               tiger_top_0_instructionMaster_address_to_slave,
                                               tiger_top_0_instructionMaster_burstcount,
                                               tiger_top_0_instructionMaster_latency_counter,
                                               tiger_top_0_instructionMaster_read,

                                              // outputs:
                                               d1_pipeline_bridge_MEMORY_s1_end_xfer,
                                               data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                               data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1,
                                               pipeline_bridge_MEMORY_s1_address,
                                               pipeline_bridge_MEMORY_s1_arbiterlock,
                                               pipeline_bridge_MEMORY_s1_arbiterlock2,
                                               pipeline_bridge_MEMORY_s1_burstcount,
                                               pipeline_bridge_MEMORY_s1_byteenable,
                                               pipeline_bridge_MEMORY_s1_chipselect,
                                               pipeline_bridge_MEMORY_s1_debugaccess,
                                               pipeline_bridge_MEMORY_s1_endofpacket_from_sa,
                                               pipeline_bridge_MEMORY_s1_nativeaddress,
                                               pipeline_bridge_MEMORY_s1_read,
                                               pipeline_bridge_MEMORY_s1_readdata_from_sa,
                                               pipeline_bridge_MEMORY_s1_reset_n,
                                               pipeline_bridge_MEMORY_s1_waitrequest_from_sa,
                                               pipeline_bridge_MEMORY_s1_write,
                                               pipeline_bridge_MEMORY_s1_writedata,
                                               tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1,
                                               tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1,
                                               tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1,
                                               tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                               tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1
                                            )
;

  output           d1_pipeline_bridge_MEMORY_s1_end_xfer;
  output           data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  output           data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1;
  output  [ 25: 0] pipeline_bridge_MEMORY_s1_address;
  output           pipeline_bridge_MEMORY_s1_arbiterlock;
  output           pipeline_bridge_MEMORY_s1_arbiterlock2;
  output  [  6: 0] pipeline_bridge_MEMORY_s1_burstcount;
  output  [ 31: 0] pipeline_bridge_MEMORY_s1_byteenable;
  output           pipeline_bridge_MEMORY_s1_chipselect;
  output           pipeline_bridge_MEMORY_s1_debugaccess;
  output           pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  output  [ 25: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  output           pipeline_bridge_MEMORY_s1_read;
  output  [255: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  output           pipeline_bridge_MEMORY_s1_reset_n;
  output           pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  output           pipeline_bridge_MEMORY_s1_write;
  output  [255: 0] pipeline_bridge_MEMORY_s1_writedata;
  output           tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;
  output           tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  output           tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  output           tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  output           tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1;
  input            clk;
  input   [ 31: 0] data_cache_0_dataMaster0_address_to_slave;
  input   [  5: 0] data_cache_0_dataMaster0_burstcount;
  input   [ 31: 0] data_cache_0_dataMaster0_byteenable;
  input            data_cache_0_dataMaster0_latency_counter;
  input            data_cache_0_dataMaster0_read;
  input            data_cache_0_dataMaster0_write;
  input   [255: 0] data_cache_0_dataMaster0_writedata;
  input            pipeline_bridge_MEMORY_s1_endofpacket;
  input   [255: 0] pipeline_bridge_MEMORY_s1_readdata;
  input            pipeline_bridge_MEMORY_s1_readdatavalid;
  input            pipeline_bridge_MEMORY_s1_waitrequest;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_instructionMaster_address_to_slave;
  input   [  5: 0] tiger_top_0_instructionMaster_burstcount;
  input            tiger_top_0_instructionMaster_latency_counter;
  input            tiger_top_0_instructionMaster_read;

  reg              d1_pipeline_bridge_MEMORY_s1_end_xfer;
  reg              d1_reasons_to_wait;
  wire             data_cache_0_dataMaster0_arbiterlock;
  wire             data_cache_0_dataMaster0_arbiterlock2;
  wire             data_cache_0_dataMaster0_continuerequest;
  wire             data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire             data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_saved_grant_pipeline_bridge_MEMORY_s1;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  reg              last_cycle_data_cache_0_dataMaster0_granted_slave_pipeline_bridge_MEMORY_s1;
  reg              last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1;
  wire             p0_pipeline_bridge_MEMORY_s1_load_fifo;
  wire    [ 25: 0] pipeline_bridge_MEMORY_s1_address;
  wire             pipeline_bridge_MEMORY_s1_allgrants;
  wire             pipeline_bridge_MEMORY_s1_allow_new_arb_cycle;
  wire             pipeline_bridge_MEMORY_s1_any_bursting_master_saved_grant;
  wire             pipeline_bridge_MEMORY_s1_any_continuerequest;
  reg     [  1: 0] pipeline_bridge_MEMORY_s1_arb_addend;
  wire             pipeline_bridge_MEMORY_s1_arb_counter_enable;
  reg     [  5: 0] pipeline_bridge_MEMORY_s1_arb_share_counter;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_arb_share_counter_next_value;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_arb_share_set_values;
  wire    [  1: 0] pipeline_bridge_MEMORY_s1_arb_winner;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock2;
  wire             pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal;
  reg     [  5: 0] pipeline_bridge_MEMORY_s1_bbt_burstcounter;
  wire             pipeline_bridge_MEMORY_s1_beginbursttransfer_internal;
  wire             pipeline_bridge_MEMORY_s1_begins_xfer;
  wire    [  6: 0] pipeline_bridge_MEMORY_s1_burstcount;
  wire             pipeline_bridge_MEMORY_s1_burstcount_fifo_empty;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_byteenable;
  wire             pipeline_bridge_MEMORY_s1_chipselect;
  wire    [  3: 0] pipeline_bridge_MEMORY_s1_chosen_master_double_vector;
  wire    [  1: 0] pipeline_bridge_MEMORY_s1_chosen_master_rot_left;
  reg     [  5: 0] pipeline_bridge_MEMORY_s1_current_burst;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_current_burst_minus_one;
  wire             pipeline_bridge_MEMORY_s1_debugaccess;
  wire             pipeline_bridge_MEMORY_s1_end_xfer;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire             pipeline_bridge_MEMORY_s1_firsttransfer;
  wire    [  1: 0] pipeline_bridge_MEMORY_s1_grant_vector;
  wire             pipeline_bridge_MEMORY_s1_in_a_read_cycle;
  wire             pipeline_bridge_MEMORY_s1_in_a_write_cycle;
  reg              pipeline_bridge_MEMORY_s1_load_fifo;
  wire    [  1: 0] pipeline_bridge_MEMORY_s1_master_qreq_vector;
  wire             pipeline_bridge_MEMORY_s1_move_on_to_next_transaction;
  wire    [ 25: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_next_bbt_burstcount;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_next_burst_count;
  wire             pipeline_bridge_MEMORY_s1_non_bursting_master_requests;
  wire             pipeline_bridge_MEMORY_s1_read;
  wire    [255: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  wire             pipeline_bridge_MEMORY_s1_readdatavalid_from_sa;
  reg              pipeline_bridge_MEMORY_s1_reg_firsttransfer;
  wire             pipeline_bridge_MEMORY_s1_reset_n;
  reg     [  1: 0] pipeline_bridge_MEMORY_s1_saved_chosen_master_vector;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_selected_burstcount;
  reg              pipeline_bridge_MEMORY_s1_slavearbiterlockenable;
  wire             pipeline_bridge_MEMORY_s1_slavearbiterlockenable2;
  wire             pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_transaction_burst_count;
  wire             pipeline_bridge_MEMORY_s1_unreg_firsttransfer;
  wire             pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  wire             pipeline_bridge_MEMORY_s1_waits_for_read;
  wire             pipeline_bridge_MEMORY_s1_waits_for_write;
  wire             pipeline_bridge_MEMORY_s1_write;
  wire    [255: 0] pipeline_bridge_MEMORY_s1_writedata;
  wire    [ 31: 0] shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_dataMaster0;
  wire    [ 31: 0] shifted_address_to_pipeline_bridge_MEMORY_s1_from_tiger_top_0_instructionMaster;
  wire             tiger_top_0_instructionMaster_arbiterlock;
  wire             tiger_top_0_instructionMaster_arbiterlock2;
  wire             tiger_top_0_instructionMaster_continuerequest;
  wire             tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire             tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1;
  wire             wait_for_pipeline_bridge_MEMORY_s1_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~pipeline_bridge_MEMORY_s1_end_xfer;
    end


  assign pipeline_bridge_MEMORY_s1_begins_xfer = ~d1_reasons_to_wait & ((data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 | tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1));
  //assign pipeline_bridge_MEMORY_s1_readdata_from_sa = pipeline_bridge_MEMORY_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_readdata_from_sa = pipeline_bridge_MEMORY_s1_readdata;

  assign data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1 = ({data_cache_0_dataMaster0_address_to_slave[31] , 31'b0} == 32'h0) & (data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write);
  //assign pipeline_bridge_MEMORY_s1_waitrequest_from_sa = pipeline_bridge_MEMORY_s1_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_waitrequest_from_sa = pipeline_bridge_MEMORY_s1_waitrequest;

  //assign pipeline_bridge_MEMORY_s1_readdatavalid_from_sa = pipeline_bridge_MEMORY_s1_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_readdatavalid_from_sa = pipeline_bridge_MEMORY_s1_readdatavalid;

  //pipeline_bridge_MEMORY_s1_arb_share_counter set values, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_arb_share_set_values = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_dataMaster0_write) ? data_cache_0_dataMaster0_burstcount : 1)) :
    (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_dataMaster0_write) ? data_cache_0_dataMaster0_burstcount : 1)) :
    (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_dataMaster0_write) ? data_cache_0_dataMaster0_burstcount : 1)) :
    1;

  //pipeline_bridge_MEMORY_s1_non_bursting_master_requests mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_non_bursting_master_requests = 0;

  //pipeline_bridge_MEMORY_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_any_bursting_master_saved_grant = data_cache_0_dataMaster0_saved_grant_pipeline_bridge_MEMORY_s1 |
    tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster0_saved_grant_pipeline_bridge_MEMORY_s1 |
    tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster0_saved_grant_pipeline_bridge_MEMORY_s1 |
    tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1;

  //pipeline_bridge_MEMORY_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_arb_share_counter_next_value = pipeline_bridge_MEMORY_s1_firsttransfer ? (pipeline_bridge_MEMORY_s1_arb_share_set_values - 1) : |pipeline_bridge_MEMORY_s1_arb_share_counter ? (pipeline_bridge_MEMORY_s1_arb_share_counter - 1) : 0;

  //pipeline_bridge_MEMORY_s1_allgrants all slave grants, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_allgrants = (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector);

  //pipeline_bridge_MEMORY_s1_end_xfer assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_end_xfer = ~(pipeline_bridge_MEMORY_s1_waits_for_read | pipeline_bridge_MEMORY_s1_waits_for_write);

  //end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1 arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_end_xfer & (~pipeline_bridge_MEMORY_s1_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //pipeline_bridge_MEMORY_s1_arb_share_counter arbitration counter enable, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_arb_counter_enable = (end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1 & pipeline_bridge_MEMORY_s1_allgrants) | (end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1 & ~pipeline_bridge_MEMORY_s1_non_bursting_master_requests);

  //pipeline_bridge_MEMORY_s1_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_arb_share_counter <= 0;
      else if (pipeline_bridge_MEMORY_s1_arb_counter_enable)
          pipeline_bridge_MEMORY_s1_arb_share_counter <= pipeline_bridge_MEMORY_s1_arb_share_counter_next_value;
    end


  //pipeline_bridge_MEMORY_s1_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_slavearbiterlockenable <= 0;
      else if ((|pipeline_bridge_MEMORY_s1_master_qreq_vector & end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1) | (end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1 & ~pipeline_bridge_MEMORY_s1_non_bursting_master_requests))
          pipeline_bridge_MEMORY_s1_slavearbiterlockenable <= |pipeline_bridge_MEMORY_s1_arb_share_counter_next_value;
    end


  //data_cache_0/dataMaster0 pipeline_bridge_MEMORY/s1 arbiterlock, which is an e_assign
  assign data_cache_0_dataMaster0_arbiterlock = pipeline_bridge_MEMORY_s1_slavearbiterlockenable & data_cache_0_dataMaster0_continuerequest;

  //pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 = |pipeline_bridge_MEMORY_s1_arb_share_counter_next_value;

  //data_cache_0/dataMaster0 pipeline_bridge_MEMORY/s1 arbiterlock2, which is an e_assign
  assign data_cache_0_dataMaster0_arbiterlock2 = pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 & data_cache_0_dataMaster0_continuerequest;

  //tiger_top_0/instructionMaster pipeline_bridge_MEMORY/s1 arbiterlock, which is an e_assign
  assign tiger_top_0_instructionMaster_arbiterlock = pipeline_bridge_MEMORY_s1_slavearbiterlockenable & tiger_top_0_instructionMaster_continuerequest;

  //tiger_top_0/instructionMaster pipeline_bridge_MEMORY/s1 arbiterlock2, which is an e_assign
  assign tiger_top_0_instructionMaster_arbiterlock2 = pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 & tiger_top_0_instructionMaster_continuerequest;

  //tiger_top_0/instructionMaster granted pipeline_bridge_MEMORY/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1 <= 0;
      else 
        last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1 <= tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 ? 1 : (pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal | 0) ? 0 : last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1;
    end


  //tiger_top_0_instructionMaster_continuerequest continued request, which is an e_mux
  assign tiger_top_0_instructionMaster_continuerequest = last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1 & 1;

  //pipeline_bridge_MEMORY_s1_any_continuerequest at least one master continues requesting, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_any_continuerequest = tiger_top_0_instructionMaster_continuerequest |
    data_cache_0_dataMaster0_continuerequest;

  assign data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 = data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1 & ~((data_cache_0_dataMaster0_read & ((data_cache_0_dataMaster0_latency_counter != 0) | (1 < data_cache_0_dataMaster0_latency_counter))) | tiger_top_0_instructionMaster_arbiterlock);
  //unique name for pipeline_bridge_MEMORY_s1_move_on_to_next_transaction, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_move_on_to_next_transaction = pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst & pipeline_bridge_MEMORY_s1_load_fifo;

  //the currently selected burstcount for pipeline_bridge_MEMORY_s1, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_selected_burstcount = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_dataMaster0_burstcount :
    (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1)? tiger_top_0_instructionMaster_burstcount :
    1;

  //burstcount_fifo_for_pipeline_bridge_MEMORY_s1, which is an e_fifo_with_registered_outputs
  burstcount_fifo_for_pipeline_bridge_MEMORY_s1_module burstcount_fifo_for_pipeline_bridge_MEMORY_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (pipeline_bridge_MEMORY_s1_selected_burstcount),
      .data_out             (pipeline_bridge_MEMORY_s1_transaction_burst_count),
      .empty                (pipeline_bridge_MEMORY_s1_burstcount_fifo_empty),
      .fifo_contains_ones_n (),
      .full                 (),
      .read                 (pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read & pipeline_bridge_MEMORY_s1_load_fifo & ~(pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst & pipeline_bridge_MEMORY_s1_burstcount_fifo_empty))
    );

  //pipeline_bridge_MEMORY_s1 current burst minus one, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_current_burst_minus_one = pipeline_bridge_MEMORY_s1_current_burst - 1;

  //what to load in current_burst, for pipeline_bridge_MEMORY_s1, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_next_burst_count = (((in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read) & ~pipeline_bridge_MEMORY_s1_load_fifo))? pipeline_bridge_MEMORY_s1_selected_burstcount :
    ((in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read & pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst & pipeline_bridge_MEMORY_s1_burstcount_fifo_empty))? pipeline_bridge_MEMORY_s1_selected_burstcount :
    (pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst)? pipeline_bridge_MEMORY_s1_transaction_burst_count :
    pipeline_bridge_MEMORY_s1_current_burst_minus_one;

  //the current burst count for pipeline_bridge_MEMORY_s1, to be decremented, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_current_burst <= 0;
      else if (pipeline_bridge_MEMORY_s1_readdatavalid_from_sa | (~pipeline_bridge_MEMORY_s1_load_fifo & (in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read)))
          pipeline_bridge_MEMORY_s1_current_burst <= pipeline_bridge_MEMORY_s1_next_burst_count;
    end


  //a 1 or burstcount fifo empty, to initialize the counter, which is an e_mux
  assign p0_pipeline_bridge_MEMORY_s1_load_fifo = (~pipeline_bridge_MEMORY_s1_load_fifo)? 1 :
    (((in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read) & pipeline_bridge_MEMORY_s1_load_fifo))? 1 :
    ~pipeline_bridge_MEMORY_s1_burstcount_fifo_empty;

  //whether to load directly to the counter or to the fifo, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_load_fifo <= 0;
      else if ((in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read) & ~pipeline_bridge_MEMORY_s1_load_fifo | pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst)
          pipeline_bridge_MEMORY_s1_load_fifo <= p0_pipeline_bridge_MEMORY_s1_load_fifo;
    end


  //the last cycle in the burst for pipeline_bridge_MEMORY_s1, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst = ~(|pipeline_bridge_MEMORY_s1_current_burst_minus_one) & pipeline_bridge_MEMORY_s1_readdatavalid_from_sa;

  //rdv_fifo_for_data_cache_0_dataMaster0_to_pipeline_bridge_MEMORY_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_data_cache_0_dataMaster0_to_pipeline_bridge_MEMORY_s1_module rdv_fifo_for_data_cache_0_dataMaster0_to_pipeline_bridge_MEMORY_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1),
      .data_out             (data_cache_0_dataMaster0_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1),
      .empty                (),
      .fifo_contains_ones_n (data_cache_0_dataMaster0_rdv_fifo_empty_pipeline_bridge_MEMORY_s1),
      .full                 (),
      .read                 (pipeline_bridge_MEMORY_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read)
    );

  assign data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register = ~data_cache_0_dataMaster0_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  //local readdatavalid data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1, which is an e_mux
  assign data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1 = (pipeline_bridge_MEMORY_s1_readdatavalid_from_sa & data_cache_0_dataMaster0_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1) & ~ data_cache_0_dataMaster0_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;

  //pipeline_bridge_MEMORY_s1_writedata mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_writedata = data_cache_0_dataMaster0_writedata;

  //assign pipeline_bridge_MEMORY_s1_endofpacket_from_sa = pipeline_bridge_MEMORY_s1_endofpacket so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_endofpacket_from_sa = pipeline_bridge_MEMORY_s1_endofpacket;

  assign tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1 = (({tiger_top_0_instructionMaster_address_to_slave[31] , 31'b0} == 32'h0) & (tiger_top_0_instructionMaster_read)) & tiger_top_0_instructionMaster_read;
  //data_cache_0/dataMaster0 granted pipeline_bridge_MEMORY/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_data_cache_0_dataMaster0_granted_slave_pipeline_bridge_MEMORY_s1 <= 0;
      else 
        last_cycle_data_cache_0_dataMaster0_granted_slave_pipeline_bridge_MEMORY_s1 <= data_cache_0_dataMaster0_saved_grant_pipeline_bridge_MEMORY_s1 ? 1 : (pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal | 0) ? 0 : last_cycle_data_cache_0_dataMaster0_granted_slave_pipeline_bridge_MEMORY_s1;
    end


  //data_cache_0_dataMaster0_continuerequest continued request, which is an e_mux
  assign data_cache_0_dataMaster0_continuerequest = last_cycle_data_cache_0_dataMaster0_granted_slave_pipeline_bridge_MEMORY_s1 & 1;

  assign tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1 = tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1 & ~((tiger_top_0_instructionMaster_read & ((tiger_top_0_instructionMaster_latency_counter != 0) | (1 < tiger_top_0_instructionMaster_latency_counter))) | data_cache_0_dataMaster0_arbiterlock);
  //rdv_fifo_for_tiger_top_0_instructionMaster_to_pipeline_bridge_MEMORY_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_tiger_top_0_instructionMaster_to_pipeline_bridge_MEMORY_s1_module rdv_fifo_for_tiger_top_0_instructionMaster_to_pipeline_bridge_MEMORY_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1),
      .data_out             (tiger_top_0_instructionMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1),
      .empty                (),
      .fifo_contains_ones_n (tiger_top_0_instructionMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1),
      .full                 (),
      .read                 (pipeline_bridge_MEMORY_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read)
    );

  assign tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register = ~tiger_top_0_instructionMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  //local readdatavalid tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1, which is an e_mux
  assign tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1 = (pipeline_bridge_MEMORY_s1_readdatavalid_from_sa & tiger_top_0_instructionMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1) & ~ tiger_top_0_instructionMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;

  //allow new arb cycle for pipeline_bridge_MEMORY/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_allow_new_arb_cycle = ~data_cache_0_dataMaster0_arbiterlock & ~tiger_top_0_instructionMaster_arbiterlock;

  //tiger_top_0/instructionMaster assignment into master qualified-requests vector for pipeline_bridge_MEMORY/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_master_qreq_vector[0] = tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1;

  //tiger_top_0/instructionMaster grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_grant_vector[0];

  //tiger_top_0/instructionMaster saved-grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_arb_winner[0];

  //data_cache_0/dataMaster0 assignment into master qualified-requests vector for pipeline_bridge_MEMORY/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_master_qreq_vector[1] = data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1;

  //data_cache_0/dataMaster0 grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_grant_vector[1];

  //data_cache_0/dataMaster0 saved-grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign data_cache_0_dataMaster0_saved_grant_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_arb_winner[1];

  //pipeline_bridge_MEMORY/s1 chosen-master double-vector, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_chosen_master_double_vector = {pipeline_bridge_MEMORY_s1_master_qreq_vector, pipeline_bridge_MEMORY_s1_master_qreq_vector} & ({~pipeline_bridge_MEMORY_s1_master_qreq_vector, ~pipeline_bridge_MEMORY_s1_master_qreq_vector} + pipeline_bridge_MEMORY_s1_arb_addend);

  //stable onehot encoding of arb winner
  assign pipeline_bridge_MEMORY_s1_arb_winner = (pipeline_bridge_MEMORY_s1_allow_new_arb_cycle & | pipeline_bridge_MEMORY_s1_grant_vector) ? pipeline_bridge_MEMORY_s1_grant_vector : pipeline_bridge_MEMORY_s1_saved_chosen_master_vector;

  //saved pipeline_bridge_MEMORY_s1_grant_vector, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_saved_chosen_master_vector <= 0;
      else if (pipeline_bridge_MEMORY_s1_allow_new_arb_cycle)
          pipeline_bridge_MEMORY_s1_saved_chosen_master_vector <= |pipeline_bridge_MEMORY_s1_grant_vector ? pipeline_bridge_MEMORY_s1_grant_vector : pipeline_bridge_MEMORY_s1_saved_chosen_master_vector;
    end


  //onehot encoding of chosen master
  assign pipeline_bridge_MEMORY_s1_grant_vector = {(pipeline_bridge_MEMORY_s1_chosen_master_double_vector[1] | pipeline_bridge_MEMORY_s1_chosen_master_double_vector[3]),
    (pipeline_bridge_MEMORY_s1_chosen_master_double_vector[0] | pipeline_bridge_MEMORY_s1_chosen_master_double_vector[2])};

  //pipeline_bridge_MEMORY/s1 chosen master rotated left, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_chosen_master_rot_left = (pipeline_bridge_MEMORY_s1_arb_winner << 1) ? (pipeline_bridge_MEMORY_s1_arb_winner << 1) : 1;

  //pipeline_bridge_MEMORY/s1's addend for next-master-grant
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_arb_addend <= 1;
      else if (|pipeline_bridge_MEMORY_s1_grant_vector)
          pipeline_bridge_MEMORY_s1_arb_addend <= pipeline_bridge_MEMORY_s1_end_xfer? pipeline_bridge_MEMORY_s1_chosen_master_rot_left : pipeline_bridge_MEMORY_s1_grant_vector;
    end


  //pipeline_bridge_MEMORY_s1_reset_n assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_reset_n = reset_n;

  assign pipeline_bridge_MEMORY_s1_chipselect = data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 | tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;
  //pipeline_bridge_MEMORY_s1_firsttransfer first transaction, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_firsttransfer = pipeline_bridge_MEMORY_s1_begins_xfer ? pipeline_bridge_MEMORY_s1_unreg_firsttransfer : pipeline_bridge_MEMORY_s1_reg_firsttransfer;

  //pipeline_bridge_MEMORY_s1_unreg_firsttransfer first transaction, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_unreg_firsttransfer = ~(pipeline_bridge_MEMORY_s1_slavearbiterlockenable & pipeline_bridge_MEMORY_s1_any_continuerequest);

  //pipeline_bridge_MEMORY_s1_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_reg_firsttransfer <= 1'b1;
      else if (pipeline_bridge_MEMORY_s1_begins_xfer)
          pipeline_bridge_MEMORY_s1_reg_firsttransfer <= pipeline_bridge_MEMORY_s1_unreg_firsttransfer;
    end


  //pipeline_bridge_MEMORY_s1_next_bbt_burstcount next_bbt_burstcount, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_next_bbt_burstcount = ((((pipeline_bridge_MEMORY_s1_write) && (pipeline_bridge_MEMORY_s1_bbt_burstcounter == 0))))? (pipeline_bridge_MEMORY_s1_burstcount - 1) :
    ((((pipeline_bridge_MEMORY_s1_read) && (pipeline_bridge_MEMORY_s1_bbt_burstcounter == 0))))? 0 :
    (pipeline_bridge_MEMORY_s1_bbt_burstcounter - 1);

  //pipeline_bridge_MEMORY_s1_bbt_burstcounter bbt_burstcounter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_s1_bbt_burstcounter <= 0;
      else if (pipeline_bridge_MEMORY_s1_begins_xfer)
          pipeline_bridge_MEMORY_s1_bbt_burstcounter <= pipeline_bridge_MEMORY_s1_next_bbt_burstcount;
    end


  //pipeline_bridge_MEMORY_s1_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_beginbursttransfer_internal = pipeline_bridge_MEMORY_s1_begins_xfer & (pipeline_bridge_MEMORY_s1_bbt_burstcounter == 0);

  //pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal arbitration_holdoff, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal = pipeline_bridge_MEMORY_s1_begins_xfer & pipeline_bridge_MEMORY_s1_firsttransfer;

  //pipeline_bridge_MEMORY_s1_read assignment, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_read = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster0_read) | (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 & tiger_top_0_instructionMaster_read);

  //pipeline_bridge_MEMORY_s1_write assignment, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_write = data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster0_write;

  assign shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_dataMaster0 = data_cache_0_dataMaster0_address_to_slave;
  //pipeline_bridge_MEMORY_s1_address mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_address = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? (shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_dataMaster0 >> 5) :
    (shifted_address_to_pipeline_bridge_MEMORY_s1_from_tiger_top_0_instructionMaster >> 5);

  assign shifted_address_to_pipeline_bridge_MEMORY_s1_from_tiger_top_0_instructionMaster = tiger_top_0_instructionMaster_address_to_slave;
  //slaveid pipeline_bridge_MEMORY_s1_nativeaddress nativeaddress mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_nativeaddress = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? (data_cache_0_dataMaster0_address_to_slave >> 5) :
    (tiger_top_0_instructionMaster_address_to_slave >> 5);

  //d1_pipeline_bridge_MEMORY_s1_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_pipeline_bridge_MEMORY_s1_end_xfer <= 1;
      else 
        d1_pipeline_bridge_MEMORY_s1_end_xfer <= pipeline_bridge_MEMORY_s1_end_xfer;
    end


  //pipeline_bridge_MEMORY_s1_waits_for_read in a cycle, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_waits_for_read = pipeline_bridge_MEMORY_s1_in_a_read_cycle & pipeline_bridge_MEMORY_s1_waitrequest_from_sa;

  //pipeline_bridge_MEMORY_s1_in_a_read_cycle assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_in_a_read_cycle = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster0_read) | (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 & tiger_top_0_instructionMaster_read);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = pipeline_bridge_MEMORY_s1_in_a_read_cycle;

  //pipeline_bridge_MEMORY_s1_waits_for_write in a cycle, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_waits_for_write = pipeline_bridge_MEMORY_s1_in_a_write_cycle & pipeline_bridge_MEMORY_s1_waitrequest_from_sa;

  //pipeline_bridge_MEMORY_s1_in_a_write_cycle assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_in_a_write_cycle = data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster0_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = pipeline_bridge_MEMORY_s1_in_a_write_cycle;

  assign wait_for_pipeline_bridge_MEMORY_s1_counter = 0;
  //pipeline_bridge_MEMORY_s1_byteenable byte enable port mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_byteenable = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_dataMaster0_byteenable :
    -1;

  //burstcount mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_burstcount = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_dataMaster0_burstcount :
    (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1)? tiger_top_0_instructionMaster_burstcount :
    1;

  //pipeline_bridge_MEMORY/s1 arbiterlock assigned from _handle_arbiterlock, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_arbiterlock = (data_cache_0_dataMaster0_arbiterlock)? data_cache_0_dataMaster0_arbiterlock :
    tiger_top_0_instructionMaster_arbiterlock;

  //pipeline_bridge_MEMORY/s1 arbiterlock2 assigned from _handle_arbiterlock2, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_arbiterlock2 = (data_cache_0_dataMaster0_arbiterlock2)? data_cache_0_dataMaster0_arbiterlock2 :
    tiger_top_0_instructionMaster_arbiterlock2;

  //debugaccess mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_debugaccess = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //pipeline_bridge_MEMORY/s1 enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end


  //data_cache_0/dataMaster0 non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1 && (data_cache_0_dataMaster0_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: data_cache_0/dataMaster0 drove 0 on its 'burstcount' port while accessing slave pipeline_bridge_MEMORY/s1", $time);
          $stop;
        end
    end


  //tiger_top_0/instructionMaster non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1 && (tiger_top_0_instructionMaster_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_top_0/instructionMaster drove 0 on its 'burstcount' port while accessing slave pipeline_bridge_MEMORY/s1", $time);
          $stop;
        end
    end


  //grant signals are active simultaneously, which is an e_process
  always @(posedge clk)
    begin
      if (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 + tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 > 1)
        begin
          $write("%0d ns: > 1 of grant signals are active simultaneously", $time);
          $stop;
        end
    end


  //saved_grant signals are active simultaneously, which is an e_process
  always @(posedge clk)
    begin
      if (data_cache_0_dataMaster0_saved_grant_pipeline_bridge_MEMORY_s1 + tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 > 1)
        begin
          $write("%0d ns: > 1 of saved_grant signals are active simultaneously", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_MEMORY_m1_arbitrator (
                                              // inputs:
                                               clk,
                                               d1_ddr2_s1_end_xfer,
                                               ddr2_s1_readdata_from_sa,
                                               ddr2_s1_waitrequest_n_from_sa,
                                               pipeline_bridge_MEMORY_m1_address,
                                               pipeline_bridge_MEMORY_m1_burstcount,
                                               pipeline_bridge_MEMORY_m1_byteenable,
                                               pipeline_bridge_MEMORY_m1_chipselect,
                                               pipeline_bridge_MEMORY_m1_granted_ddr2_s1,
                                               pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1,
                                               pipeline_bridge_MEMORY_m1_read,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register,
                                               pipeline_bridge_MEMORY_m1_requests_ddr2_s1,
                                               pipeline_bridge_MEMORY_m1_write,
                                               pipeline_bridge_MEMORY_m1_writedata,
                                               reset_n,

                                              // outputs:
                                               pipeline_bridge_MEMORY_m1_address_to_slave,
                                               pipeline_bridge_MEMORY_m1_latency_counter,
                                               pipeline_bridge_MEMORY_m1_readdata,
                                               pipeline_bridge_MEMORY_m1_readdatavalid,
                                               pipeline_bridge_MEMORY_m1_waitrequest
                                            )
;

  output  [ 30: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  output           pipeline_bridge_MEMORY_m1_latency_counter;
  output  [255: 0] pipeline_bridge_MEMORY_m1_readdata;
  output           pipeline_bridge_MEMORY_m1_readdatavalid;
  output           pipeline_bridge_MEMORY_m1_waitrequest;
  input            clk;
  input            d1_ddr2_s1_end_xfer;
  input   [255: 0] ddr2_s1_readdata_from_sa;
  input            ddr2_s1_waitrequest_n_from_sa;
  input   [ 30: 0] pipeline_bridge_MEMORY_m1_address;
  input   [  6: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [ 31: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input            pipeline_bridge_MEMORY_m1_granted_ddr2_s1;
  input            pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register;
  input            pipeline_bridge_MEMORY_m1_requests_ddr2_s1;
  input            pipeline_bridge_MEMORY_m1_write;
  input   [255: 0] pipeline_bridge_MEMORY_m1_writedata;
  input            reset_n;

  reg              active_and_waiting_last_time;
  wire             latency_load_value;
  wire             p1_pipeline_bridge_MEMORY_m1_latency_counter;
  reg     [ 30: 0] pipeline_bridge_MEMORY_m1_address_last_time;
  wire    [ 30: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  reg     [  6: 0] pipeline_bridge_MEMORY_m1_burstcount_last_time;
  reg     [ 31: 0] pipeline_bridge_MEMORY_m1_byteenable_last_time;
  reg              pipeline_bridge_MEMORY_m1_chipselect_last_time;
  wire             pipeline_bridge_MEMORY_m1_is_granted_some_slave;
  reg              pipeline_bridge_MEMORY_m1_latency_counter;
  reg              pipeline_bridge_MEMORY_m1_read_but_no_slave_selected;
  reg              pipeline_bridge_MEMORY_m1_read_last_time;
  wire    [255: 0] pipeline_bridge_MEMORY_m1_readdata;
  wire             pipeline_bridge_MEMORY_m1_readdatavalid;
  wire             pipeline_bridge_MEMORY_m1_run;
  wire             pipeline_bridge_MEMORY_m1_waitrequest;
  reg              pipeline_bridge_MEMORY_m1_write_last_time;
  reg     [255: 0] pipeline_bridge_MEMORY_m1_writedata_last_time;
  wire             pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1 | ~pipeline_bridge_MEMORY_m1_requests_ddr2_s1) & (pipeline_bridge_MEMORY_m1_granted_ddr2_s1 | ~pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1) & ((~pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1 | ~pipeline_bridge_MEMORY_m1_chipselect | (1 & ddr2_s1_waitrequest_n_from_sa & pipeline_bridge_MEMORY_m1_chipselect))) & ((~pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1 | ~pipeline_bridge_MEMORY_m1_chipselect | (1 & ddr2_s1_waitrequest_n_from_sa & pipeline_bridge_MEMORY_m1_chipselect)));

  //cascaded wait assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign pipeline_bridge_MEMORY_m1_address_to_slave = {1'b1,
    pipeline_bridge_MEMORY_m1_address[29 : 0]};

  //pipeline_bridge_MEMORY_m1_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_read_but_no_slave_selected <= 0;
      else 
        pipeline_bridge_MEMORY_m1_read_but_no_slave_selected <= (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & pipeline_bridge_MEMORY_m1_run & ~pipeline_bridge_MEMORY_m1_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_is_granted_some_slave = pipeline_bridge_MEMORY_m1_granted_ddr2_s1;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid = pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_readdatavalid = pipeline_bridge_MEMORY_m1_read_but_no_slave_selected |
    pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid;

  //pipeline_bridge_MEMORY/m1 readdata mux, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_readdata = ddr2_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_waitrequest = ~pipeline_bridge_MEMORY_m1_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_latency_counter <= 0;
      else 
        pipeline_bridge_MEMORY_m1_latency_counter <= p1_pipeline_bridge_MEMORY_m1_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_pipeline_bridge_MEMORY_m1_latency_counter = ((pipeline_bridge_MEMORY_m1_run & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect)))? latency_load_value :
    (pipeline_bridge_MEMORY_m1_latency_counter)? pipeline_bridge_MEMORY_m1_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //pipeline_bridge_MEMORY_m1_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_address_last_time <= 0;
      else 
        pipeline_bridge_MEMORY_m1_address_last_time <= pipeline_bridge_MEMORY_m1_address;
    end


  //pipeline_bridge_MEMORY/m1 waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= pipeline_bridge_MEMORY_m1_waitrequest & pipeline_bridge_MEMORY_m1_chipselect;
    end


  //pipeline_bridge_MEMORY_m1_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_MEMORY_m1_address != pipeline_bridge_MEMORY_m1_address_last_time))
        begin
          $write("%0d ns: pipeline_bridge_MEMORY_m1_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_MEMORY_m1_chipselect check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_chipselect_last_time <= 0;
      else 
        pipeline_bridge_MEMORY_m1_chipselect_last_time <= pipeline_bridge_MEMORY_m1_chipselect;
    end


  //pipeline_bridge_MEMORY_m1_chipselect matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_MEMORY_m1_chipselect != pipeline_bridge_MEMORY_m1_chipselect_last_time))
        begin
          $write("%0d ns: pipeline_bridge_MEMORY_m1_chipselect did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_MEMORY_m1_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_burstcount_last_time <= 0;
      else 
        pipeline_bridge_MEMORY_m1_burstcount_last_time <= pipeline_bridge_MEMORY_m1_burstcount;
    end


  //pipeline_bridge_MEMORY_m1_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_MEMORY_m1_burstcount != pipeline_bridge_MEMORY_m1_burstcount_last_time))
        begin
          $write("%0d ns: pipeline_bridge_MEMORY_m1_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_MEMORY_m1_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_byteenable_last_time <= 0;
      else 
        pipeline_bridge_MEMORY_m1_byteenable_last_time <= pipeline_bridge_MEMORY_m1_byteenable;
    end


  //pipeline_bridge_MEMORY_m1_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_MEMORY_m1_byteenable != pipeline_bridge_MEMORY_m1_byteenable_last_time))
        begin
          $write("%0d ns: pipeline_bridge_MEMORY_m1_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_MEMORY_m1_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_read_last_time <= 0;
      else 
        pipeline_bridge_MEMORY_m1_read_last_time <= pipeline_bridge_MEMORY_m1_read;
    end


  //pipeline_bridge_MEMORY_m1_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_MEMORY_m1_read != pipeline_bridge_MEMORY_m1_read_last_time))
        begin
          $write("%0d ns: pipeline_bridge_MEMORY_m1_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_MEMORY_m1_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_write_last_time <= 0;
      else 
        pipeline_bridge_MEMORY_m1_write_last_time <= pipeline_bridge_MEMORY_m1_write;
    end


  //pipeline_bridge_MEMORY_m1_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_MEMORY_m1_write != pipeline_bridge_MEMORY_m1_write_last_time))
        begin
          $write("%0d ns: pipeline_bridge_MEMORY_m1_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_MEMORY_m1_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_writedata_last_time <= 0;
      else 
        pipeline_bridge_MEMORY_m1_writedata_last_time <= pipeline_bridge_MEMORY_m1_writedata;
    end


  //pipeline_bridge_MEMORY_m1_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_MEMORY_m1_writedata != pipeline_bridge_MEMORY_m1_writedata_last_time) & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect))
        begin
          $write("%0d ns: pipeline_bridge_MEMORY_m1_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_MEMORY_bridge_arbitrator 
;



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_tiger_top_0_procMaster_to_pipeline_bridge_PERIPHERALS_s1_module (
                                                                                      // inputs:
                                                                                       clear_fifo,
                                                                                       clk,
                                                                                       data_in,
                                                                                       read,
                                                                                       reset_n,
                                                                                       sync_reset,
                                                                                       write,

                                                                                      // outputs:
                                                                                       data_out,
                                                                                       empty,
                                                                                       fifo_contains_ones_n,
                                                                                       full
                                                                                    )
;

  output           data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input            data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire             data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  wire             full_1;
  reg     [  1: 0] how_many_ones;
  wire    [  1: 0] one_count_minus_one;
  wire    [  1: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  reg              stage_0;
  wire    [  1: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_0;
  assign empty = !full_0;
  assign full_1 = 0;
  //data_0, which is an e_mux
  assign p0_stage_0 = ((full_1 & ~clear_fifo) == 0)? data_in :
    data_in;

  //data_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          stage_0 <= 0;
      else if (clear_fifo | sync_reset | read | (write & !full_0))
          if (sync_reset & full_0 & !((full_1 == 0) & read & write))
              stage_0 <= 0;
          else 
            stage_0 <= p0_stage_0;
    end


  //control_0, which is an e_mux
  assign p0_full_0 = ((read & !write) == 0)? 1 :
    0;

  //control_reg_0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_0 <= 0;
      else if (clear_fifo | (read ^ write) | (write & !full_0))
          if (clear_fifo & ~write)
              full_0 <= 0;
          else 
            full_0 <= p0_full_0;
    end


  assign one_count_plus_one = how_many_ones + 1;
  assign one_count_minus_one = how_many_ones - 1;
  //updated_one_count, which is an e_mux
  assign updated_one_count = ((((clear_fifo | sync_reset) & !write)))? 0 :
    ((((clear_fifo | sync_reset) & write)))? |data_in :
    ((read & (|data_in) & write & (|stage_0)))? how_many_ones :
    ((write & (|data_in)))? one_count_plus_one :
    ((read & (|stage_0)))? one_count_minus_one :
    how_many_ones;

  //counts how many ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          how_many_ones <= 0;
      else if (clear_fifo | sync_reset | read | write)
          how_many_ones <= updated_one_count;
    end


  //this fifo contains ones in the data pipeline, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_contains_ones_n <= 1;
      else if (clear_fifo | sync_reset | read | write)
          fifo_contains_ones_n <= ~(|updated_one_count);
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_PERIPHERALS_s1_arbitrator (
                                                   // inputs:
                                                    clk,
                                                    pipeline_bridge_PERIPHERALS_s1_endofpacket,
                                                    pipeline_bridge_PERIPHERALS_s1_readdata,
                                                    pipeline_bridge_PERIPHERALS_s1_readdatavalid,
                                                    pipeline_bridge_PERIPHERALS_s1_waitrequest,
                                                    reset_n,
                                                    tiger_top_0_procMaster_address_to_slave,
                                                    tiger_top_0_procMaster_byteenable,
                                                    tiger_top_0_procMaster_latency_counter,
                                                    tiger_top_0_procMaster_read,
                                                    tiger_top_0_procMaster_write,
                                                    tiger_top_0_procMaster_writedata,

                                                   // outputs:
                                                    d1_pipeline_bridge_PERIPHERALS_s1_end_xfer,
                                                    pipeline_bridge_PERIPHERALS_s1_address,
                                                    pipeline_bridge_PERIPHERALS_s1_arbiterlock,
                                                    pipeline_bridge_PERIPHERALS_s1_arbiterlock2,
                                                    pipeline_bridge_PERIPHERALS_s1_burstcount,
                                                    pipeline_bridge_PERIPHERALS_s1_byteenable,
                                                    pipeline_bridge_PERIPHERALS_s1_chipselect,
                                                    pipeline_bridge_PERIPHERALS_s1_debugaccess,
                                                    pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa,
                                                    pipeline_bridge_PERIPHERALS_s1_nativeaddress,
                                                    pipeline_bridge_PERIPHERALS_s1_read,
                                                    pipeline_bridge_PERIPHERALS_s1_readdata_from_sa,
                                                    pipeline_bridge_PERIPHERALS_s1_reset_n,
                                                    pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa,
                                                    pipeline_bridge_PERIPHERALS_s1_write,
                                                    pipeline_bridge_PERIPHERALS_s1_writedata,
                                                    tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1,
                                                    tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1,
                                                    tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1,
                                                    tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register,
                                                    tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1
                                                 )
;

  output           d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  output  [ 10: 0] pipeline_bridge_PERIPHERALS_s1_address;
  output           pipeline_bridge_PERIPHERALS_s1_arbiterlock;
  output           pipeline_bridge_PERIPHERALS_s1_arbiterlock2;
  output           pipeline_bridge_PERIPHERALS_s1_burstcount;
  output  [  3: 0] pipeline_bridge_PERIPHERALS_s1_byteenable;
  output           pipeline_bridge_PERIPHERALS_s1_chipselect;
  output           pipeline_bridge_PERIPHERALS_s1_debugaccess;
  output           pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  output  [ 10: 0] pipeline_bridge_PERIPHERALS_s1_nativeaddress;
  output           pipeline_bridge_PERIPHERALS_s1_read;
  output  [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;
  output           pipeline_bridge_PERIPHERALS_s1_reset_n;
  output           pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;
  output           pipeline_bridge_PERIPHERALS_s1_write;
  output  [ 31: 0] pipeline_bridge_PERIPHERALS_s1_writedata;
  output           tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1;
  output           tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  output           tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  output           tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  output           tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1;
  input            clk;
  input            pipeline_bridge_PERIPHERALS_s1_endofpacket;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata;
  input            pipeline_bridge_PERIPHERALS_s1_readdatavalid;
  input            pipeline_bridge_PERIPHERALS_s1_waitrequest;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_procMaster_address_to_slave;
  input   [  3: 0] tiger_top_0_procMaster_byteenable;
  input            tiger_top_0_procMaster_latency_counter;
  input            tiger_top_0_procMaster_read;
  input            tiger_top_0_procMaster_write;
  input   [ 31: 0] tiger_top_0_procMaster_writedata;

  reg              d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  reg              d1_reasons_to_wait;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire    [ 10: 0] pipeline_bridge_PERIPHERALS_s1_address;
  wire             pipeline_bridge_PERIPHERALS_s1_allgrants;
  wire             pipeline_bridge_PERIPHERALS_s1_allow_new_arb_cycle;
  wire             pipeline_bridge_PERIPHERALS_s1_any_bursting_master_saved_grant;
  wire             pipeline_bridge_PERIPHERALS_s1_any_continuerequest;
  wire             pipeline_bridge_PERIPHERALS_s1_arb_counter_enable;
  reg              pipeline_bridge_PERIPHERALS_s1_arb_share_counter;
  wire             pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value;
  wire             pipeline_bridge_PERIPHERALS_s1_arb_share_set_values;
  wire             pipeline_bridge_PERIPHERALS_s1_arbiterlock;
  wire             pipeline_bridge_PERIPHERALS_s1_arbiterlock2;
  wire             pipeline_bridge_PERIPHERALS_s1_arbitration_holdoff_internal;
  wire             pipeline_bridge_PERIPHERALS_s1_beginbursttransfer_internal;
  wire             pipeline_bridge_PERIPHERALS_s1_begins_xfer;
  wire             pipeline_bridge_PERIPHERALS_s1_burstcount;
  wire    [  3: 0] pipeline_bridge_PERIPHERALS_s1_byteenable;
  wire             pipeline_bridge_PERIPHERALS_s1_chipselect;
  wire             pipeline_bridge_PERIPHERALS_s1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_s1_end_xfer;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  wire             pipeline_bridge_PERIPHERALS_s1_firsttransfer;
  wire             pipeline_bridge_PERIPHERALS_s1_grant_vector;
  wire             pipeline_bridge_PERIPHERALS_s1_in_a_read_cycle;
  wire             pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle;
  wire             pipeline_bridge_PERIPHERALS_s1_master_qreq_vector;
  wire             pipeline_bridge_PERIPHERALS_s1_move_on_to_next_transaction;
  wire    [ 10: 0] pipeline_bridge_PERIPHERALS_s1_nativeaddress;
  wire             pipeline_bridge_PERIPHERALS_s1_non_bursting_master_requests;
  wire             pipeline_bridge_PERIPHERALS_s1_read;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;
  wire             pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa;
  reg              pipeline_bridge_PERIPHERALS_s1_reg_firsttransfer;
  wire             pipeline_bridge_PERIPHERALS_s1_reset_n;
  reg              pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable;
  wire             pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable2;
  wire             pipeline_bridge_PERIPHERALS_s1_unreg_firsttransfer;
  wire             pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;
  wire             pipeline_bridge_PERIPHERALS_s1_waits_for_read;
  wire             pipeline_bridge_PERIPHERALS_s1_waits_for_write;
  wire             pipeline_bridge_PERIPHERALS_s1_write;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_s1_writedata;
  wire    [ 31: 0] shifted_address_to_pipeline_bridge_PERIPHERALS_s1_from_tiger_top_0_procMaster;
  wire             tiger_top_0_procMaster_arbiterlock;
  wire             tiger_top_0_procMaster_arbiterlock2;
  wire             tiger_top_0_procMaster_continuerequest;
  wire             tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_rdv_fifo_empty_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_rdv_fifo_output_from_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  wire             tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_saved_grant_pipeline_bridge_PERIPHERALS_s1;
  wire             wait_for_pipeline_bridge_PERIPHERALS_s1_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~pipeline_bridge_PERIPHERALS_s1_end_xfer;
    end


  assign pipeline_bridge_PERIPHERALS_s1_begins_xfer = ~d1_reasons_to_wait & ((tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1));
  //assign pipeline_bridge_PERIPHERALS_s1_readdata_from_sa = pipeline_bridge_PERIPHERALS_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_readdata_from_sa = pipeline_bridge_PERIPHERALS_s1_readdata;

  assign tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1 = ({tiger_top_0_procMaster_address_to_slave[31 : 13] , 13'b0} == 32'hf0000000) & (tiger_top_0_procMaster_read | tiger_top_0_procMaster_write);
  //assign pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa = pipeline_bridge_PERIPHERALS_s1_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa = pipeline_bridge_PERIPHERALS_s1_waitrequest;

  //assign pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa = pipeline_bridge_PERIPHERALS_s1_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa = pipeline_bridge_PERIPHERALS_s1_readdatavalid;

  //pipeline_bridge_PERIPHERALS_s1_arb_share_counter set values, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_arb_share_set_values = 1;

  //pipeline_bridge_PERIPHERALS_s1_non_bursting_master_requests mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_non_bursting_master_requests = tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1 |
    tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1;

  //pipeline_bridge_PERIPHERALS_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_any_bursting_master_saved_grant = 0;

  //pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value = pipeline_bridge_PERIPHERALS_s1_firsttransfer ? (pipeline_bridge_PERIPHERALS_s1_arb_share_set_values - 1) : |pipeline_bridge_PERIPHERALS_s1_arb_share_counter ? (pipeline_bridge_PERIPHERALS_s1_arb_share_counter - 1) : 0;

  //pipeline_bridge_PERIPHERALS_s1_allgrants all slave grants, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_allgrants = (|pipeline_bridge_PERIPHERALS_s1_grant_vector) |
    (|pipeline_bridge_PERIPHERALS_s1_grant_vector);

  //pipeline_bridge_PERIPHERALS_s1_end_xfer assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_end_xfer = ~(pipeline_bridge_PERIPHERALS_s1_waits_for_read | pipeline_bridge_PERIPHERALS_s1_waits_for_write);

  //end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1 arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1 = pipeline_bridge_PERIPHERALS_s1_end_xfer & (~pipeline_bridge_PERIPHERALS_s1_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //pipeline_bridge_PERIPHERALS_s1_arb_share_counter arbitration counter enable, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_arb_counter_enable = (end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1 & pipeline_bridge_PERIPHERALS_s1_allgrants) | (end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1 & ~pipeline_bridge_PERIPHERALS_s1_non_bursting_master_requests);

  //pipeline_bridge_PERIPHERALS_s1_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_s1_arb_share_counter <= 0;
      else if (pipeline_bridge_PERIPHERALS_s1_arb_counter_enable)
          pipeline_bridge_PERIPHERALS_s1_arb_share_counter <= pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value;
    end


  //pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable <= 0;
      else if ((|pipeline_bridge_PERIPHERALS_s1_master_qreq_vector & end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1) | (end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1 & ~pipeline_bridge_PERIPHERALS_s1_non_bursting_master_requests))
          pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable <= |pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value;
    end


  //tiger_top_0/procMaster pipeline_bridge_PERIPHERALS/s1 arbiterlock, which is an e_assign
  assign tiger_top_0_procMaster_arbiterlock = pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable & tiger_top_0_procMaster_continuerequest;

  //pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable2 = |pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value;

  //tiger_top_0/procMaster pipeline_bridge_PERIPHERALS/s1 arbiterlock2, which is an e_assign
  assign tiger_top_0_procMaster_arbiterlock2 = pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable2 & tiger_top_0_procMaster_continuerequest;

  //pipeline_bridge_PERIPHERALS_s1_any_continuerequest at least one master continues requesting, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_any_continuerequest = 1;

  //tiger_top_0_procMaster_continuerequest continued request, which is an e_assign
  assign tiger_top_0_procMaster_continuerequest = 1;

  assign tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1 = tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1 & ~((tiger_top_0_procMaster_read & ((tiger_top_0_procMaster_latency_counter != 0) | (1 < tiger_top_0_procMaster_latency_counter))));
  //unique name for pipeline_bridge_PERIPHERALS_s1_move_on_to_next_transaction, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_move_on_to_next_transaction = pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa;

  //rdv_fifo_for_tiger_top_0_procMaster_to_pipeline_bridge_PERIPHERALS_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_tiger_top_0_procMaster_to_pipeline_bridge_PERIPHERALS_s1_module rdv_fifo_for_tiger_top_0_procMaster_to_pipeline_bridge_PERIPHERALS_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1),
      .data_out             (tiger_top_0_procMaster_rdv_fifo_output_from_pipeline_bridge_PERIPHERALS_s1),
      .empty                (),
      .fifo_contains_ones_n (tiger_top_0_procMaster_rdv_fifo_empty_pipeline_bridge_PERIPHERALS_s1),
      .full                 (),
      .read                 (pipeline_bridge_PERIPHERALS_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~pipeline_bridge_PERIPHERALS_s1_waits_for_read)
    );

  assign tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register = ~tiger_top_0_procMaster_rdv_fifo_empty_pipeline_bridge_PERIPHERALS_s1;
  //local readdatavalid tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1, which is an e_mux
  assign tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1 = pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa;

  //pipeline_bridge_PERIPHERALS_s1_writedata mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_writedata = tiger_top_0_procMaster_writedata;

  //assign pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa = pipeline_bridge_PERIPHERALS_s1_endofpacket so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa = pipeline_bridge_PERIPHERALS_s1_endofpacket;

  //master is always granted when requested
  assign tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1 = tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1;

  //tiger_top_0/procMaster saved-grant pipeline_bridge_PERIPHERALS/s1, which is an e_assign
  assign tiger_top_0_procMaster_saved_grant_pipeline_bridge_PERIPHERALS_s1 = tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1;

  //allow new arb cycle for pipeline_bridge_PERIPHERALS/s1, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign pipeline_bridge_PERIPHERALS_s1_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign pipeline_bridge_PERIPHERALS_s1_master_qreq_vector = 1;

  //pipeline_bridge_PERIPHERALS_s1_reset_n assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_reset_n = reset_n;

  assign pipeline_bridge_PERIPHERALS_s1_chipselect = tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1;
  //pipeline_bridge_PERIPHERALS_s1_firsttransfer first transaction, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_firsttransfer = pipeline_bridge_PERIPHERALS_s1_begins_xfer ? pipeline_bridge_PERIPHERALS_s1_unreg_firsttransfer : pipeline_bridge_PERIPHERALS_s1_reg_firsttransfer;

  //pipeline_bridge_PERIPHERALS_s1_unreg_firsttransfer first transaction, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_unreg_firsttransfer = ~(pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable & pipeline_bridge_PERIPHERALS_s1_any_continuerequest);

  //pipeline_bridge_PERIPHERALS_s1_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_s1_reg_firsttransfer <= 1'b1;
      else if (pipeline_bridge_PERIPHERALS_s1_begins_xfer)
          pipeline_bridge_PERIPHERALS_s1_reg_firsttransfer <= pipeline_bridge_PERIPHERALS_s1_unreg_firsttransfer;
    end


  //pipeline_bridge_PERIPHERALS_s1_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_beginbursttransfer_internal = pipeline_bridge_PERIPHERALS_s1_begins_xfer;

  //pipeline_bridge_PERIPHERALS_s1_arbitration_holdoff_internal arbitration_holdoff, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_arbitration_holdoff_internal = pipeline_bridge_PERIPHERALS_s1_begins_xfer & pipeline_bridge_PERIPHERALS_s1_firsttransfer;

  //pipeline_bridge_PERIPHERALS_s1_read assignment, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_read = tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_top_0_procMaster_read;

  //pipeline_bridge_PERIPHERALS_s1_write assignment, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_write = tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_top_0_procMaster_write;

  assign shifted_address_to_pipeline_bridge_PERIPHERALS_s1_from_tiger_top_0_procMaster = tiger_top_0_procMaster_address_to_slave;
  //pipeline_bridge_PERIPHERALS_s1_address mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_address = shifted_address_to_pipeline_bridge_PERIPHERALS_s1_from_tiger_top_0_procMaster >> 2;

  //slaveid pipeline_bridge_PERIPHERALS_s1_nativeaddress nativeaddress mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_nativeaddress = tiger_top_0_procMaster_address_to_slave >> 2;

  //d1_pipeline_bridge_PERIPHERALS_s1_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_pipeline_bridge_PERIPHERALS_s1_end_xfer <= 1;
      else 
        d1_pipeline_bridge_PERIPHERALS_s1_end_xfer <= pipeline_bridge_PERIPHERALS_s1_end_xfer;
    end


  //pipeline_bridge_PERIPHERALS_s1_waits_for_read in a cycle, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_waits_for_read = pipeline_bridge_PERIPHERALS_s1_in_a_read_cycle & pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;

  //pipeline_bridge_PERIPHERALS_s1_in_a_read_cycle assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_in_a_read_cycle = tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_top_0_procMaster_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = pipeline_bridge_PERIPHERALS_s1_in_a_read_cycle;

  //pipeline_bridge_PERIPHERALS_s1_waits_for_write in a cycle, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_waits_for_write = pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle & pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;

  //pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle = tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_top_0_procMaster_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle;

  assign wait_for_pipeline_bridge_PERIPHERALS_s1_counter = 0;
  //pipeline_bridge_PERIPHERALS_s1_byteenable byte enable port mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_byteenable = (tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1)? tiger_top_0_procMaster_byteenable :
    -1;

  //burstcount mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_burstcount = 1;

  //pipeline_bridge_PERIPHERALS/s1 arbiterlock assigned from _handle_arbiterlock, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_arbiterlock = tiger_top_0_procMaster_arbiterlock;

  //pipeline_bridge_PERIPHERALS/s1 arbiterlock2 assigned from _handle_arbiterlock2, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_arbiterlock2 = tiger_top_0_procMaster_arbiterlock2;

  //debugaccess mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_debugaccess = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //pipeline_bridge_PERIPHERALS/s1 enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_PERIPHERALS_m1_arbitrator (
                                                   // inputs:
                                                    clk,
                                                    d1_uart_0_s1_end_xfer,
                                                    pipeline_bridge_PERIPHERALS_m1_address,
                                                    pipeline_bridge_PERIPHERALS_m1_burstcount,
                                                    pipeline_bridge_PERIPHERALS_m1_byteenable,
                                                    pipeline_bridge_PERIPHERALS_m1_chipselect,
                                                    pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_read,
                                                    pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_write,
                                                    pipeline_bridge_PERIPHERALS_m1_writedata,
                                                    reset_n,
                                                    uart_0_s1_readdata_from_sa,

                                                   // outputs:
                                                    pipeline_bridge_PERIPHERALS_m1_address_to_slave,
                                                    pipeline_bridge_PERIPHERALS_m1_latency_counter,
                                                    pipeline_bridge_PERIPHERALS_m1_readdata,
                                                    pipeline_bridge_PERIPHERALS_m1_readdatavalid,
                                                    pipeline_bridge_PERIPHERALS_m1_waitrequest
                                                 )
;

  output  [ 12: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  output           pipeline_bridge_PERIPHERALS_m1_latency_counter;
  output  [ 31: 0] pipeline_bridge_PERIPHERALS_m1_readdata;
  output           pipeline_bridge_PERIPHERALS_m1_readdatavalid;
  output           pipeline_bridge_PERIPHERALS_m1_waitrequest;
  input            clk;
  input            d1_uart_0_s1_end_xfer;
  input   [ 12: 0] pipeline_bridge_PERIPHERALS_m1_address;
  input            pipeline_bridge_PERIPHERALS_m1_burstcount;
  input   [  3: 0] pipeline_bridge_PERIPHERALS_m1_byteenable;
  input            pipeline_bridge_PERIPHERALS_m1_chipselect;
  input            pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_read;
  input            pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_write;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata;
  input            reset_n;
  input   [ 15: 0] uart_0_s1_readdata_from_sa;

  reg              active_and_waiting_last_time;
  reg     [ 12: 0] pipeline_bridge_PERIPHERALS_m1_address_last_time;
  wire    [ 12: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  reg              pipeline_bridge_PERIPHERALS_m1_burstcount_last_time;
  reg     [  3: 0] pipeline_bridge_PERIPHERALS_m1_byteenable_last_time;
  reg              pipeline_bridge_PERIPHERALS_m1_chipselect_last_time;
  wire             pipeline_bridge_PERIPHERALS_m1_latency_counter;
  reg              pipeline_bridge_PERIPHERALS_m1_read_last_time;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_m1_readdata;
  wire             pipeline_bridge_PERIPHERALS_m1_readdatavalid;
  wire             pipeline_bridge_PERIPHERALS_m1_run;
  wire             pipeline_bridge_PERIPHERALS_m1_waitrequest;
  reg              pipeline_bridge_PERIPHERALS_m1_write_last_time;
  reg     [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata_last_time;
  wire             pre_flush_pipeline_bridge_PERIPHERALS_m1_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 | ~pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & ~d1_uart_0_s1_end_xfer & pipeline_bridge_PERIPHERALS_m1_chipselect))) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & ~d1_uart_0_s1_end_xfer & pipeline_bridge_PERIPHERALS_m1_chipselect)));

  //cascaded wait assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign pipeline_bridge_PERIPHERALS_m1_address_to_slave = {8'b10000000,
    pipeline_bridge_PERIPHERALS_m1_address[4 : 0]};

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_pipeline_bridge_PERIPHERALS_m1_readdatavalid = 0;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_readdatavalid = 0 |
    pre_flush_pipeline_bridge_PERIPHERALS_m1_readdatavalid |
    pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;

  //pipeline_bridge_PERIPHERALS/m1 readdata mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_readdata = uart_0_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_waitrequest = ~pipeline_bridge_PERIPHERALS_m1_run;

  //latent max counter, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_latency_counter = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //pipeline_bridge_PERIPHERALS_m1_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_address_last_time <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_address_last_time <= pipeline_bridge_PERIPHERALS_m1_address;
    end


  //pipeline_bridge_PERIPHERALS/m1 waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= pipeline_bridge_PERIPHERALS_m1_waitrequest & pipeline_bridge_PERIPHERALS_m1_chipselect;
    end


  //pipeline_bridge_PERIPHERALS_m1_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_PERIPHERALS_m1_address != pipeline_bridge_PERIPHERALS_m1_address_last_time))
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS_m1_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_PERIPHERALS_m1_chipselect check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_chipselect_last_time <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_chipselect_last_time <= pipeline_bridge_PERIPHERALS_m1_chipselect;
    end


  //pipeline_bridge_PERIPHERALS_m1_chipselect matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_PERIPHERALS_m1_chipselect != pipeline_bridge_PERIPHERALS_m1_chipselect_last_time))
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS_m1_chipselect did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_PERIPHERALS_m1_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_burstcount_last_time <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_burstcount_last_time <= pipeline_bridge_PERIPHERALS_m1_burstcount;
    end


  //pipeline_bridge_PERIPHERALS_m1_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_PERIPHERALS_m1_burstcount != pipeline_bridge_PERIPHERALS_m1_burstcount_last_time))
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS_m1_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_PERIPHERALS_m1_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_byteenable_last_time <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_byteenable_last_time <= pipeline_bridge_PERIPHERALS_m1_byteenable;
    end


  //pipeline_bridge_PERIPHERALS_m1_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_PERIPHERALS_m1_byteenable != pipeline_bridge_PERIPHERALS_m1_byteenable_last_time))
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS_m1_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_PERIPHERALS_m1_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_read_last_time <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_read_last_time <= pipeline_bridge_PERIPHERALS_m1_read;
    end


  //pipeline_bridge_PERIPHERALS_m1_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_PERIPHERALS_m1_read != pipeline_bridge_PERIPHERALS_m1_read_last_time))
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS_m1_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_PERIPHERALS_m1_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_write_last_time <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_write_last_time <= pipeline_bridge_PERIPHERALS_m1_write;
    end


  //pipeline_bridge_PERIPHERALS_m1_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_PERIPHERALS_m1_write != pipeline_bridge_PERIPHERALS_m1_write_last_time))
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS_m1_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //pipeline_bridge_PERIPHERALS_m1_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_writedata_last_time <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_writedata_last_time <= pipeline_bridge_PERIPHERALS_m1_writedata;
    end


  //pipeline_bridge_PERIPHERALS_m1_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (pipeline_bridge_PERIPHERALS_m1_writedata != pipeline_bridge_PERIPHERALS_m1_writedata_last_time) & (pipeline_bridge_PERIPHERALS_m1_write & pipeline_bridge_PERIPHERALS_m1_chipselect))
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS_m1_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_PERIPHERALS_bridge_arbitrator 
;



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_top_0_PROC_arbitrator (
                                     // inputs:
                                      clk,
                                      data_cache_0_PROC_data,
                                      reset_n,

                                     // outputs:
                                      tiger_top_0_PROC_data
                                   )
;

  output  [  7: 0] tiger_top_0_PROC_data;
  input            clk;
  input   [  7: 0] data_cache_0_PROC_data;
  input            reset_n;

  wire    [  7: 0] tiger_top_0_PROC_data;
  //mux tiger_top_0_PROC_data, which is an e_mux
  assign tiger_top_0_PROC_data = data_cache_0_PROC_data;


endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_top_0_leapSlave_arbitrator (
                                          // inputs:
                                           clk,
                                           jtag_to_ava_master_bridge_latency_counter,
                                           jtag_to_ava_master_bridge_master_address_to_slave,
                                           jtag_to_ava_master_bridge_master_read,
                                           jtag_to_ava_master_bridge_master_write,
                                           jtag_to_ava_master_bridge_master_writedata,
                                           jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register,
                                           reset_n,
                                           tiger_top_0_leapSlave_readdata,

                                          // outputs:
                                           d1_tiger_top_0_leapSlave_end_xfer,
                                           jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave,
                                           jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave,
                                           jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave,
                                           jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave,
                                           tiger_top_0_leapSlave_address,
                                           tiger_top_0_leapSlave_chipselect,
                                           tiger_top_0_leapSlave_read,
                                           tiger_top_0_leapSlave_readdata_from_sa,
                                           tiger_top_0_leapSlave_write,
                                           tiger_top_0_leapSlave_writedata
                                        )
;

  output           d1_tiger_top_0_leapSlave_end_xfer;
  output           jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;
  output           jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave;
  output           jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;
  output           jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;
  output  [  7: 0] tiger_top_0_leapSlave_address;
  output           tiger_top_0_leapSlave_chipselect;
  output           tiger_top_0_leapSlave_read;
  output  [ 31: 0] tiger_top_0_leapSlave_readdata_from_sa;
  output           tiger_top_0_leapSlave_write;
  output  [ 31: 0] tiger_top_0_leapSlave_writedata;
  input            clk;
  input            jtag_to_ava_master_bridge_latency_counter;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  input            jtag_to_ava_master_bridge_master_read;
  input            jtag_to_ava_master_bridge_master_write;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_writedata;
  input            jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_leapSlave_readdata;

  reg              d1_reasons_to_wait;
  reg              d1_tiger_top_0_leapSlave_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_tiger_top_0_leapSlave;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_master_arbiterlock;
  wire             jtag_to_ava_master_bridge_master_arbiterlock2;
  wire             jtag_to_ava_master_bridge_master_continuerequest;
  wire             jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_saved_grant_tiger_top_0_leapSlave;
  wire    [ 31: 0] shifted_address_to_tiger_top_0_leapSlave_from_jtag_to_ava_master_bridge_master;
  wire    [  7: 0] tiger_top_0_leapSlave_address;
  wire             tiger_top_0_leapSlave_allgrants;
  wire             tiger_top_0_leapSlave_allow_new_arb_cycle;
  wire             tiger_top_0_leapSlave_any_bursting_master_saved_grant;
  wire             tiger_top_0_leapSlave_any_continuerequest;
  wire             tiger_top_0_leapSlave_arb_counter_enable;
  reg              tiger_top_0_leapSlave_arb_share_counter;
  wire             tiger_top_0_leapSlave_arb_share_counter_next_value;
  wire             tiger_top_0_leapSlave_arb_share_set_values;
  wire             tiger_top_0_leapSlave_beginbursttransfer_internal;
  wire             tiger_top_0_leapSlave_begins_xfer;
  wire             tiger_top_0_leapSlave_chipselect;
  wire             tiger_top_0_leapSlave_end_xfer;
  wire             tiger_top_0_leapSlave_firsttransfer;
  wire             tiger_top_0_leapSlave_grant_vector;
  wire             tiger_top_0_leapSlave_in_a_read_cycle;
  wire             tiger_top_0_leapSlave_in_a_write_cycle;
  wire             tiger_top_0_leapSlave_master_qreq_vector;
  wire             tiger_top_0_leapSlave_non_bursting_master_requests;
  wire             tiger_top_0_leapSlave_read;
  wire    [ 31: 0] tiger_top_0_leapSlave_readdata_from_sa;
  reg              tiger_top_0_leapSlave_reg_firsttransfer;
  reg              tiger_top_0_leapSlave_slavearbiterlockenable;
  wire             tiger_top_0_leapSlave_slavearbiterlockenable2;
  wire             tiger_top_0_leapSlave_unreg_firsttransfer;
  wire             tiger_top_0_leapSlave_waits_for_read;
  wire             tiger_top_0_leapSlave_waits_for_write;
  wire             tiger_top_0_leapSlave_write;
  wire    [ 31: 0] tiger_top_0_leapSlave_writedata;
  wire             wait_for_tiger_top_0_leapSlave_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~tiger_top_0_leapSlave_end_xfer;
    end


  assign tiger_top_0_leapSlave_begins_xfer = ~d1_reasons_to_wait & ((jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave));
  //assign tiger_top_0_leapSlave_readdata_from_sa = tiger_top_0_leapSlave_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_top_0_leapSlave_readdata_from_sa = tiger_top_0_leapSlave_readdata;

  assign jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave = ({jtag_to_ava_master_bridge_master_address_to_slave[31 : 10] , 10'b0} == 32'h80000000) & (jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write);
  //tiger_top_0_leapSlave_arb_share_counter set values, which is an e_mux
  assign tiger_top_0_leapSlave_arb_share_set_values = 1;

  //tiger_top_0_leapSlave_non_bursting_master_requests mux, which is an e_mux
  assign tiger_top_0_leapSlave_non_bursting_master_requests = jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;

  //tiger_top_0_leapSlave_any_bursting_master_saved_grant mux, which is an e_mux
  assign tiger_top_0_leapSlave_any_bursting_master_saved_grant = 0;

  //tiger_top_0_leapSlave_arb_share_counter_next_value assignment, which is an e_assign
  assign tiger_top_0_leapSlave_arb_share_counter_next_value = tiger_top_0_leapSlave_firsttransfer ? (tiger_top_0_leapSlave_arb_share_set_values - 1) : |tiger_top_0_leapSlave_arb_share_counter ? (tiger_top_0_leapSlave_arb_share_counter - 1) : 0;

  //tiger_top_0_leapSlave_allgrants all slave grants, which is an e_mux
  assign tiger_top_0_leapSlave_allgrants = |tiger_top_0_leapSlave_grant_vector;

  //tiger_top_0_leapSlave_end_xfer assignment, which is an e_assign
  assign tiger_top_0_leapSlave_end_xfer = ~(tiger_top_0_leapSlave_waits_for_read | tiger_top_0_leapSlave_waits_for_write);

  //end_xfer_arb_share_counter_term_tiger_top_0_leapSlave arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_tiger_top_0_leapSlave = tiger_top_0_leapSlave_end_xfer & (~tiger_top_0_leapSlave_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //tiger_top_0_leapSlave_arb_share_counter arbitration counter enable, which is an e_assign
  assign tiger_top_0_leapSlave_arb_counter_enable = (end_xfer_arb_share_counter_term_tiger_top_0_leapSlave & tiger_top_0_leapSlave_allgrants) | (end_xfer_arb_share_counter_term_tiger_top_0_leapSlave & ~tiger_top_0_leapSlave_non_bursting_master_requests);

  //tiger_top_0_leapSlave_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_leapSlave_arb_share_counter <= 0;
      else if (tiger_top_0_leapSlave_arb_counter_enable)
          tiger_top_0_leapSlave_arb_share_counter <= tiger_top_0_leapSlave_arb_share_counter_next_value;
    end


  //tiger_top_0_leapSlave_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_leapSlave_slavearbiterlockenable <= 0;
      else if ((|tiger_top_0_leapSlave_master_qreq_vector & end_xfer_arb_share_counter_term_tiger_top_0_leapSlave) | (end_xfer_arb_share_counter_term_tiger_top_0_leapSlave & ~tiger_top_0_leapSlave_non_bursting_master_requests))
          tiger_top_0_leapSlave_slavearbiterlockenable <= |tiger_top_0_leapSlave_arb_share_counter_next_value;
    end


  //jtag_to_ava_master_bridge/master tiger_top_0/leapSlave arbiterlock, which is an e_assign
  assign jtag_to_ava_master_bridge_master_arbiterlock = tiger_top_0_leapSlave_slavearbiterlockenable & jtag_to_ava_master_bridge_master_continuerequest;

  //tiger_top_0_leapSlave_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign tiger_top_0_leapSlave_slavearbiterlockenable2 = |tiger_top_0_leapSlave_arb_share_counter_next_value;

  //jtag_to_ava_master_bridge/master tiger_top_0/leapSlave arbiterlock2, which is an e_assign
  assign jtag_to_ava_master_bridge_master_arbiterlock2 = tiger_top_0_leapSlave_slavearbiterlockenable2 & jtag_to_ava_master_bridge_master_continuerequest;

  //tiger_top_0_leapSlave_any_continuerequest at least one master continues requesting, which is an e_assign
  assign tiger_top_0_leapSlave_any_continuerequest = 1;

  //jtag_to_ava_master_bridge_master_continuerequest continued request, which is an e_assign
  assign jtag_to_ava_master_bridge_master_continuerequest = 1;

  assign jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave = jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave & ~((jtag_to_ava_master_bridge_master_read & ((jtag_to_ava_master_bridge_latency_counter != 0) | (|jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register))));
  //local readdatavalid jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave, which is an e_mux
  assign jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave = jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave & jtag_to_ava_master_bridge_master_read & ~tiger_top_0_leapSlave_waits_for_read;

  //tiger_top_0_leapSlave_writedata mux, which is an e_mux
  assign tiger_top_0_leapSlave_writedata = jtag_to_ava_master_bridge_master_writedata;

  //master is always granted when requested
  assign jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave = jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave;

  //jtag_to_ava_master_bridge/master saved-grant tiger_top_0/leapSlave, which is an e_assign
  assign jtag_to_ava_master_bridge_saved_grant_tiger_top_0_leapSlave = jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;

  //allow new arb cycle for tiger_top_0/leapSlave, which is an e_assign
  assign tiger_top_0_leapSlave_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign tiger_top_0_leapSlave_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign tiger_top_0_leapSlave_master_qreq_vector = 1;

  assign tiger_top_0_leapSlave_chipselect = jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;
  //tiger_top_0_leapSlave_firsttransfer first transaction, which is an e_assign
  assign tiger_top_0_leapSlave_firsttransfer = tiger_top_0_leapSlave_begins_xfer ? tiger_top_0_leapSlave_unreg_firsttransfer : tiger_top_0_leapSlave_reg_firsttransfer;

  //tiger_top_0_leapSlave_unreg_firsttransfer first transaction, which is an e_assign
  assign tiger_top_0_leapSlave_unreg_firsttransfer = ~(tiger_top_0_leapSlave_slavearbiterlockenable & tiger_top_0_leapSlave_any_continuerequest);

  //tiger_top_0_leapSlave_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_leapSlave_reg_firsttransfer <= 1'b1;
      else if (tiger_top_0_leapSlave_begins_xfer)
          tiger_top_0_leapSlave_reg_firsttransfer <= tiger_top_0_leapSlave_unreg_firsttransfer;
    end


  //tiger_top_0_leapSlave_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign tiger_top_0_leapSlave_beginbursttransfer_internal = tiger_top_0_leapSlave_begins_xfer;

  //tiger_top_0_leapSlave_read assignment, which is an e_mux
  assign tiger_top_0_leapSlave_read = jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave & jtag_to_ava_master_bridge_master_read;

  //tiger_top_0_leapSlave_write assignment, which is an e_mux
  assign tiger_top_0_leapSlave_write = jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave & jtag_to_ava_master_bridge_master_write;

  assign shifted_address_to_tiger_top_0_leapSlave_from_jtag_to_ava_master_bridge_master = jtag_to_ava_master_bridge_master_address_to_slave;
  //tiger_top_0_leapSlave_address mux, which is an e_mux
  assign tiger_top_0_leapSlave_address = shifted_address_to_tiger_top_0_leapSlave_from_jtag_to_ava_master_bridge_master >> 2;

  //d1_tiger_top_0_leapSlave_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_tiger_top_0_leapSlave_end_xfer <= 1;
      else 
        d1_tiger_top_0_leapSlave_end_xfer <= tiger_top_0_leapSlave_end_xfer;
    end


  //tiger_top_0_leapSlave_waits_for_read in a cycle, which is an e_mux
  assign tiger_top_0_leapSlave_waits_for_read = tiger_top_0_leapSlave_in_a_read_cycle & tiger_top_0_leapSlave_begins_xfer;

  //tiger_top_0_leapSlave_in_a_read_cycle assignment, which is an e_assign
  assign tiger_top_0_leapSlave_in_a_read_cycle = jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave & jtag_to_ava_master_bridge_master_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = tiger_top_0_leapSlave_in_a_read_cycle;

  //tiger_top_0_leapSlave_waits_for_write in a cycle, which is an e_mux
  assign tiger_top_0_leapSlave_waits_for_write = tiger_top_0_leapSlave_in_a_write_cycle & 0;

  //tiger_top_0_leapSlave_in_a_write_cycle assignment, which is an e_assign
  assign tiger_top_0_leapSlave_in_a_write_cycle = jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave & jtag_to_ava_master_bridge_master_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = tiger_top_0_leapSlave_in_a_write_cycle;

  assign wait_for_tiger_top_0_leapSlave_counter = 0;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_top_0/leapSlave enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_top_0_CACHE_arbitrator (
                                      // inputs:
                                       clk,
                                       d1_data_cache_0_CACHE0_end_xfer,
                                       data_cache_0_CACHE0_readdata_from_sa,
                                       data_cache_0_CACHE0_waitrequest_from_sa,
                                       reset_n,
                                       tiger_top_0_CACHE_address,
                                       tiger_top_0_CACHE_granted_data_cache_0_CACHE0,
                                       tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0,
                                       tiger_top_0_CACHE_read,
                                       tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0,
                                       tiger_top_0_CACHE_requests_data_cache_0_CACHE0,
                                       tiger_top_0_CACHE_write,
                                       tiger_top_0_CACHE_writedata,

                                      // outputs:
                                       tiger_top_0_CACHE_address_to_slave,
                                       tiger_top_0_CACHE_readdata,
                                       tiger_top_0_CACHE_reset,
                                       tiger_top_0_CACHE_waitrequest
                                    )
;

  output  [ 31: 0] tiger_top_0_CACHE_address_to_slave;
  output  [127: 0] tiger_top_0_CACHE_readdata;
  output           tiger_top_0_CACHE_reset;
  output           tiger_top_0_CACHE_waitrequest;
  input            clk;
  input            d1_data_cache_0_CACHE0_end_xfer;
  input   [127: 0] data_cache_0_CACHE0_readdata_from_sa;
  input            data_cache_0_CACHE0_waitrequest_from_sa;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_CACHE_address;
  input            tiger_top_0_CACHE_granted_data_cache_0_CACHE0;
  input            tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0;
  input            tiger_top_0_CACHE_read;
  input            tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0;
  input            tiger_top_0_CACHE_requests_data_cache_0_CACHE0;
  input            tiger_top_0_CACHE_write;
  input   [127: 0] tiger_top_0_CACHE_writedata;

  reg              active_and_waiting_last_time;
  wire             r_0;
  reg     [ 31: 0] tiger_top_0_CACHE_address_last_time;
  wire    [ 31: 0] tiger_top_0_CACHE_address_to_slave;
  reg              tiger_top_0_CACHE_read_last_time;
  wire    [127: 0] tiger_top_0_CACHE_readdata;
  wire             tiger_top_0_CACHE_reset;
  wire             tiger_top_0_CACHE_run;
  wire             tiger_top_0_CACHE_waitrequest;
  reg              tiger_top_0_CACHE_write_last_time;
  reg     [127: 0] tiger_top_0_CACHE_writedata_last_time;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & ((~tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0 | ~(tiger_top_0_CACHE_read | tiger_top_0_CACHE_write) | (1 & ~data_cache_0_CACHE0_waitrequest_from_sa & (tiger_top_0_CACHE_read | tiger_top_0_CACHE_write)))) & ((~tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0 | ~(tiger_top_0_CACHE_read | tiger_top_0_CACHE_write) | (1 & ~data_cache_0_CACHE0_waitrequest_from_sa & (tiger_top_0_CACHE_read | tiger_top_0_CACHE_write))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_top_0_CACHE_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_top_0_CACHE_address_to_slave = {28'b0,
    tiger_top_0_CACHE_address[3 : 0]};

  //tiger_top_0/CACHE readdata mux, which is an e_mux
  assign tiger_top_0_CACHE_readdata = data_cache_0_CACHE0_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_top_0_CACHE_waitrequest = ~tiger_top_0_CACHE_run;

  //~tiger_top_0_CACHE_reset assignment, which is an e_assign
  assign tiger_top_0_CACHE_reset = ~reset_n;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_top_0_CACHE_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_CACHE_address_last_time <= 0;
      else 
        tiger_top_0_CACHE_address_last_time <= tiger_top_0_CACHE_address;
    end


  //tiger_top_0/CACHE waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= tiger_top_0_CACHE_waitrequest & (tiger_top_0_CACHE_read | tiger_top_0_CACHE_write);
    end


  //tiger_top_0_CACHE_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_CACHE_address != tiger_top_0_CACHE_address_last_time))
        begin
          $write("%0d ns: tiger_top_0_CACHE_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_CACHE_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_CACHE_read_last_time <= 0;
      else 
        tiger_top_0_CACHE_read_last_time <= tiger_top_0_CACHE_read;
    end


  //tiger_top_0_CACHE_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_CACHE_read != tiger_top_0_CACHE_read_last_time))
        begin
          $write("%0d ns: tiger_top_0_CACHE_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_CACHE_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_CACHE_write_last_time <= 0;
      else 
        tiger_top_0_CACHE_write_last_time <= tiger_top_0_CACHE_write;
    end


  //tiger_top_0_CACHE_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_CACHE_write != tiger_top_0_CACHE_write_last_time))
        begin
          $write("%0d ns: tiger_top_0_CACHE_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_CACHE_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_CACHE_writedata_last_time <= 0;
      else 
        tiger_top_0_CACHE_writedata_last_time <= tiger_top_0_CACHE_writedata;
    end


  //tiger_top_0_CACHE_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_CACHE_writedata != tiger_top_0_CACHE_writedata_last_time) & tiger_top_0_CACHE_write)
        begin
          $write("%0d ns: tiger_top_0_CACHE_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_top_0_instructionMaster_arbitrator (
                                                  // inputs:
                                                   clk,
                                                   d1_pipeline_bridge_MEMORY_s1_end_xfer,
                                                   pipeline_bridge_MEMORY_s1_readdata_from_sa,
                                                   pipeline_bridge_MEMORY_s1_waitrequest_from_sa,
                                                   reset_n,
                                                   tiger_top_0_instructionMaster_address,
                                                   tiger_top_0_instructionMaster_burstcount,
                                                   tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1,
                                                   tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1,
                                                   tiger_top_0_instructionMaster_read,
                                                   tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1,
                                                   tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                                   tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1,

                                                  // outputs:
                                                   tiger_top_0_instructionMaster_address_to_slave,
                                                   tiger_top_0_instructionMaster_latency_counter,
                                                   tiger_top_0_instructionMaster_readdata,
                                                   tiger_top_0_instructionMaster_readdatavalid,
                                                   tiger_top_0_instructionMaster_waitrequest
                                                )
;

  output  [ 31: 0] tiger_top_0_instructionMaster_address_to_slave;
  output           tiger_top_0_instructionMaster_latency_counter;
  output  [255: 0] tiger_top_0_instructionMaster_readdata;
  output           tiger_top_0_instructionMaster_readdatavalid;
  output           tiger_top_0_instructionMaster_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_MEMORY_s1_end_xfer;
  input   [255: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  input            pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_instructionMaster_address;
  input   [  5: 0] tiger_top_0_instructionMaster_burstcount;
  input            tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;
  input            tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  input            tiger_top_0_instructionMaster_read;
  input            tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  input            tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  input            tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1;

  reg              active_and_waiting_last_time;
  wire             latency_load_value;
  wire             p1_tiger_top_0_instructionMaster_latency_counter;
  wire             pre_flush_tiger_top_0_instructionMaster_readdatavalid;
  wire             r_0;
  reg     [ 31: 0] tiger_top_0_instructionMaster_address_last_time;
  wire    [ 31: 0] tiger_top_0_instructionMaster_address_to_slave;
  reg     [  5: 0] tiger_top_0_instructionMaster_burstcount_last_time;
  wire             tiger_top_0_instructionMaster_is_granted_some_slave;
  reg              tiger_top_0_instructionMaster_latency_counter;
  reg              tiger_top_0_instructionMaster_read_but_no_slave_selected;
  reg              tiger_top_0_instructionMaster_read_last_time;
  wire    [255: 0] tiger_top_0_instructionMaster_readdata;
  wire             tiger_top_0_instructionMaster_readdatavalid;
  wire             tiger_top_0_instructionMaster_run;
  wire             tiger_top_0_instructionMaster_waitrequest;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1) & (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 | ~tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1) & ((~tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~(tiger_top_0_instructionMaster_read) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (tiger_top_0_instructionMaster_read))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_top_0_instructionMaster_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_top_0_instructionMaster_address_to_slave = {1'b0,
    tiger_top_0_instructionMaster_address[30 : 0]};

  //tiger_top_0_instructionMaster_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_instructionMaster_read_but_no_slave_selected <= 0;
      else 
        tiger_top_0_instructionMaster_read_but_no_slave_selected <= tiger_top_0_instructionMaster_read & tiger_top_0_instructionMaster_run & ~tiger_top_0_instructionMaster_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign tiger_top_0_instructionMaster_is_granted_some_slave = tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_tiger_top_0_instructionMaster_readdatavalid = tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign tiger_top_0_instructionMaster_readdatavalid = tiger_top_0_instructionMaster_read_but_no_slave_selected |
    pre_flush_tiger_top_0_instructionMaster_readdatavalid;

  //tiger_top_0/instructionMaster readdata mux, which is an e_mux
  assign tiger_top_0_instructionMaster_readdata = pipeline_bridge_MEMORY_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_top_0_instructionMaster_waitrequest = ~tiger_top_0_instructionMaster_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_instructionMaster_latency_counter <= 0;
      else 
        tiger_top_0_instructionMaster_latency_counter <= p1_tiger_top_0_instructionMaster_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_tiger_top_0_instructionMaster_latency_counter = ((tiger_top_0_instructionMaster_run & tiger_top_0_instructionMaster_read))? latency_load_value :
    (tiger_top_0_instructionMaster_latency_counter)? tiger_top_0_instructionMaster_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_top_0_instructionMaster_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_instructionMaster_address_last_time <= 0;
      else 
        tiger_top_0_instructionMaster_address_last_time <= tiger_top_0_instructionMaster_address;
    end


  //tiger_top_0/instructionMaster waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= tiger_top_0_instructionMaster_waitrequest & (tiger_top_0_instructionMaster_read);
    end


  //tiger_top_0_instructionMaster_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_instructionMaster_address != tiger_top_0_instructionMaster_address_last_time))
        begin
          $write("%0d ns: tiger_top_0_instructionMaster_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_instructionMaster_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_instructionMaster_burstcount_last_time <= 0;
      else 
        tiger_top_0_instructionMaster_burstcount_last_time <= tiger_top_0_instructionMaster_burstcount;
    end


  //tiger_top_0_instructionMaster_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_instructionMaster_burstcount != tiger_top_0_instructionMaster_burstcount_last_time))
        begin
          $write("%0d ns: tiger_top_0_instructionMaster_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_instructionMaster_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_instructionMaster_read_last_time <= 0;
      else 
        tiger_top_0_instructionMaster_read_last_time <= tiger_top_0_instructionMaster_read;
    end


  //tiger_top_0_instructionMaster_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_instructionMaster_read != tiger_top_0_instructionMaster_read_last_time))
        begin
          $write("%0d ns: tiger_top_0_instructionMaster_read did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_top_0_procMaster_arbitrator (
                                           // inputs:
                                            clk,
                                            d1_pipeline_bridge_PERIPHERALS_s1_end_xfer,
                                            pipeline_bridge_PERIPHERALS_s1_readdata_from_sa,
                                            pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa,
                                            reset_n,
                                            tiger_top_0_procMaster_address,
                                            tiger_top_0_procMaster_byteenable,
                                            tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1,
                                            tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1,
                                            tiger_top_0_procMaster_read,
                                            tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1,
                                            tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register,
                                            tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1,
                                            tiger_top_0_procMaster_write,
                                            tiger_top_0_procMaster_writedata,

                                           // outputs:
                                            tiger_top_0_procMaster_address_to_slave,
                                            tiger_top_0_procMaster_latency_counter,
                                            tiger_top_0_procMaster_readdata,
                                            tiger_top_0_procMaster_readdatavalid,
                                            tiger_top_0_procMaster_waitrequest
                                         )
;

  output  [ 31: 0] tiger_top_0_procMaster_address_to_slave;
  output           tiger_top_0_procMaster_latency_counter;
  output  [ 31: 0] tiger_top_0_procMaster_readdata;
  output           tiger_top_0_procMaster_readdatavalid;
  output           tiger_top_0_procMaster_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;
  input            pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_procMaster_address;
  input   [  3: 0] tiger_top_0_procMaster_byteenable;
  input            tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_top_0_procMaster_read;
  input            tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  input            tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_top_0_procMaster_write;
  input   [ 31: 0] tiger_top_0_procMaster_writedata;

  reg              active_and_waiting_last_time;
  wire             pre_flush_tiger_top_0_procMaster_readdatavalid;
  wire             r_0;
  reg     [ 31: 0] tiger_top_0_procMaster_address_last_time;
  wire    [ 31: 0] tiger_top_0_procMaster_address_to_slave;
  reg     [  3: 0] tiger_top_0_procMaster_byteenable_last_time;
  wire             tiger_top_0_procMaster_latency_counter;
  reg              tiger_top_0_procMaster_read_last_time;
  wire    [ 31: 0] tiger_top_0_procMaster_readdata;
  wire             tiger_top_0_procMaster_readdatavalid;
  wire             tiger_top_0_procMaster_run;
  wire             tiger_top_0_procMaster_waitrequest;
  reg              tiger_top_0_procMaster_write_last_time;
  reg     [ 31: 0] tiger_top_0_procMaster_writedata_last_time;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1 | ~tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1) & ((~tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1 | ~(tiger_top_0_procMaster_read | tiger_top_0_procMaster_write) | (1 & ~pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa & (tiger_top_0_procMaster_read | tiger_top_0_procMaster_write)))) & ((~tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1 | ~(tiger_top_0_procMaster_read | tiger_top_0_procMaster_write) | (1 & ~pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa & (tiger_top_0_procMaster_read | tiger_top_0_procMaster_write))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_top_0_procMaster_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_top_0_procMaster_address_to_slave = {19'b1111000000000000000,
    tiger_top_0_procMaster_address[12 : 0]};

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_tiger_top_0_procMaster_readdatavalid = tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign tiger_top_0_procMaster_readdatavalid = 0 |
    pre_flush_tiger_top_0_procMaster_readdatavalid;

  //tiger_top_0/procMaster readdata mux, which is an e_mux
  assign tiger_top_0_procMaster_readdata = pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_top_0_procMaster_waitrequest = ~tiger_top_0_procMaster_run;

  //latent max counter, which is an e_assign
  assign tiger_top_0_procMaster_latency_counter = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_top_0_procMaster_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_procMaster_address_last_time <= 0;
      else 
        tiger_top_0_procMaster_address_last_time <= tiger_top_0_procMaster_address;
    end


  //tiger_top_0/procMaster waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= tiger_top_0_procMaster_waitrequest & (tiger_top_0_procMaster_read | tiger_top_0_procMaster_write);
    end


  //tiger_top_0_procMaster_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_procMaster_address != tiger_top_0_procMaster_address_last_time))
        begin
          $write("%0d ns: tiger_top_0_procMaster_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_procMaster_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_procMaster_byteenable_last_time <= 0;
      else 
        tiger_top_0_procMaster_byteenable_last_time <= tiger_top_0_procMaster_byteenable;
    end


  //tiger_top_0_procMaster_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_procMaster_byteenable != tiger_top_0_procMaster_byteenable_last_time))
        begin
          $write("%0d ns: tiger_top_0_procMaster_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_procMaster_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_procMaster_read_last_time <= 0;
      else 
        tiger_top_0_procMaster_read_last_time <= tiger_top_0_procMaster_read;
    end


  //tiger_top_0_procMaster_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_procMaster_read != tiger_top_0_procMaster_read_last_time))
        begin
          $write("%0d ns: tiger_top_0_procMaster_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_procMaster_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_procMaster_write_last_time <= 0;
      else 
        tiger_top_0_procMaster_write_last_time <= tiger_top_0_procMaster_write;
    end


  //tiger_top_0_procMaster_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_procMaster_write != tiger_top_0_procMaster_write_last_time))
        begin
          $write("%0d ns: tiger_top_0_procMaster_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_top_0_procMaster_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_top_0_procMaster_writedata_last_time <= 0;
      else 
        tiger_top_0_procMaster_writedata_last_time <= tiger_top_0_procMaster_writedata;
    end


  //tiger_top_0_procMaster_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_top_0_procMaster_writedata != tiger_top_0_procMaster_writedata_last_time) & tiger_top_0_procMaster_write)
        begin
          $write("%0d ns: tiger_top_0_procMaster_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module uart_0_s1_arbitrator (
                              // inputs:
                               clk,
                               pipeline_bridge_PERIPHERALS_m1_address_to_slave,
                               pipeline_bridge_PERIPHERALS_m1_burstcount,
                               pipeline_bridge_PERIPHERALS_m1_chipselect,
                               pipeline_bridge_PERIPHERALS_m1_latency_counter,
                               pipeline_bridge_PERIPHERALS_m1_read,
                               pipeline_bridge_PERIPHERALS_m1_write,
                               pipeline_bridge_PERIPHERALS_m1_writedata,
                               reset_n,
                               uart_0_s1_dataavailable,
                               uart_0_s1_readdata,
                               uart_0_s1_readyfordata,

                              // outputs:
                               d1_uart_0_s1_end_xfer,
                               pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1,
                               pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1,
                               pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1,
                               pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1,
                               uart_0_s1_address,
                               uart_0_s1_begintransfer,
                               uart_0_s1_chipselect,
                               uart_0_s1_dataavailable_from_sa,
                               uart_0_s1_read_n,
                               uart_0_s1_readdata_from_sa,
                               uart_0_s1_readyfordata_from_sa,
                               uart_0_s1_reset_n,
                               uart_0_s1_write_n,
                               uart_0_s1_writedata
                            )
;

  output           d1_uart_0_s1_end_xfer;
  output           pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;
  output           pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1;
  output           pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;
  output           pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;
  output  [  2: 0] uart_0_s1_address;
  output           uart_0_s1_begintransfer;
  output           uart_0_s1_chipselect;
  output           uart_0_s1_dataavailable_from_sa;
  output           uart_0_s1_read_n;
  output  [ 15: 0] uart_0_s1_readdata_from_sa;
  output           uart_0_s1_readyfordata_from_sa;
  output           uart_0_s1_reset_n;
  output           uart_0_s1_write_n;
  output  [ 15: 0] uart_0_s1_writedata;
  input            clk;
  input   [ 12: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  input            pipeline_bridge_PERIPHERALS_m1_burstcount;
  input            pipeline_bridge_PERIPHERALS_m1_chipselect;
  input            pipeline_bridge_PERIPHERALS_m1_latency_counter;
  input            pipeline_bridge_PERIPHERALS_m1_read;
  input            pipeline_bridge_PERIPHERALS_m1_write;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata;
  input            reset_n;
  input            uart_0_s1_dataavailable;
  input   [ 15: 0] uart_0_s1_readdata;
  input            uart_0_s1_readyfordata;

  reg              d1_reasons_to_wait;
  reg              d1_uart_0_s1_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_uart_0_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             pipeline_bridge_PERIPHERALS_m1_arbiterlock;
  wire             pipeline_bridge_PERIPHERALS_m1_arbiterlock2;
  wire             pipeline_bridge_PERIPHERALS_m1_continuerequest;
  wire             pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_saved_grant_uart_0_s1;
  wire    [ 12: 0] shifted_address_to_uart_0_s1_from_pipeline_bridge_PERIPHERALS_m1;
  wire    [  2: 0] uart_0_s1_address;
  wire             uart_0_s1_allgrants;
  wire             uart_0_s1_allow_new_arb_cycle;
  wire             uart_0_s1_any_bursting_master_saved_grant;
  wire             uart_0_s1_any_continuerequest;
  wire             uart_0_s1_arb_counter_enable;
  reg              uart_0_s1_arb_share_counter;
  wire             uart_0_s1_arb_share_counter_next_value;
  wire             uart_0_s1_arb_share_set_values;
  wire             uart_0_s1_beginbursttransfer_internal;
  wire             uart_0_s1_begins_xfer;
  wire             uart_0_s1_begintransfer;
  wire             uart_0_s1_chipselect;
  wire             uart_0_s1_dataavailable_from_sa;
  wire             uart_0_s1_end_xfer;
  wire             uart_0_s1_firsttransfer;
  wire             uart_0_s1_grant_vector;
  wire             uart_0_s1_in_a_read_cycle;
  wire             uart_0_s1_in_a_write_cycle;
  wire             uart_0_s1_master_qreq_vector;
  wire             uart_0_s1_non_bursting_master_requests;
  wire             uart_0_s1_read_n;
  wire    [ 15: 0] uart_0_s1_readdata_from_sa;
  wire             uart_0_s1_readyfordata_from_sa;
  reg              uart_0_s1_reg_firsttransfer;
  wire             uart_0_s1_reset_n;
  reg              uart_0_s1_slavearbiterlockenable;
  wire             uart_0_s1_slavearbiterlockenable2;
  wire             uart_0_s1_unreg_firsttransfer;
  wire             uart_0_s1_waits_for_read;
  wire             uart_0_s1_waits_for_write;
  wire             uart_0_s1_write_n;
  wire    [ 15: 0] uart_0_s1_writedata;
  wire             wait_for_uart_0_s1_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~uart_0_s1_end_xfer;
    end


  assign uart_0_s1_begins_xfer = ~d1_reasons_to_wait & ((pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1));
  //assign uart_0_s1_readdata_from_sa = uart_0_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign uart_0_s1_readdata_from_sa = uart_0_s1_readdata;

  assign pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1 = ({pipeline_bridge_PERIPHERALS_m1_address_to_slave[12 : 5] , 5'b0} == 13'h1000) & pipeline_bridge_PERIPHERALS_m1_chipselect;
  //assign uart_0_s1_dataavailable_from_sa = uart_0_s1_dataavailable so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign uart_0_s1_dataavailable_from_sa = uart_0_s1_dataavailable;

  //assign uart_0_s1_readyfordata_from_sa = uart_0_s1_readyfordata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign uart_0_s1_readyfordata_from_sa = uart_0_s1_readyfordata;

  //uart_0_s1_arb_share_counter set values, which is an e_mux
  assign uart_0_s1_arb_share_set_values = 1;

  //uart_0_s1_non_bursting_master_requests mux, which is an e_mux
  assign uart_0_s1_non_bursting_master_requests = pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;

  //uart_0_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign uart_0_s1_any_bursting_master_saved_grant = 0;

  //uart_0_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign uart_0_s1_arb_share_counter_next_value = uart_0_s1_firsttransfer ? (uart_0_s1_arb_share_set_values - 1) : |uart_0_s1_arb_share_counter ? (uart_0_s1_arb_share_counter - 1) : 0;

  //uart_0_s1_allgrants all slave grants, which is an e_mux
  assign uart_0_s1_allgrants = |uart_0_s1_grant_vector;

  //uart_0_s1_end_xfer assignment, which is an e_assign
  assign uart_0_s1_end_xfer = ~(uart_0_s1_waits_for_read | uart_0_s1_waits_for_write);

  //end_xfer_arb_share_counter_term_uart_0_s1 arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_uart_0_s1 = uart_0_s1_end_xfer & (~uart_0_s1_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //uart_0_s1_arb_share_counter arbitration counter enable, which is an e_assign
  assign uart_0_s1_arb_counter_enable = (end_xfer_arb_share_counter_term_uart_0_s1 & uart_0_s1_allgrants) | (end_xfer_arb_share_counter_term_uart_0_s1 & ~uart_0_s1_non_bursting_master_requests);

  //uart_0_s1_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          uart_0_s1_arb_share_counter <= 0;
      else if (uart_0_s1_arb_counter_enable)
          uart_0_s1_arb_share_counter <= uart_0_s1_arb_share_counter_next_value;
    end


  //uart_0_s1_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          uart_0_s1_slavearbiterlockenable <= 0;
      else if ((|uart_0_s1_master_qreq_vector & end_xfer_arb_share_counter_term_uart_0_s1) | (end_xfer_arb_share_counter_term_uart_0_s1 & ~uart_0_s1_non_bursting_master_requests))
          uart_0_s1_slavearbiterlockenable <= |uart_0_s1_arb_share_counter_next_value;
    end


  //pipeline_bridge_PERIPHERALS/m1 uart_0/s1 arbiterlock, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_arbiterlock = uart_0_s1_slavearbiterlockenable & pipeline_bridge_PERIPHERALS_m1_continuerequest;

  //uart_0_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign uart_0_s1_slavearbiterlockenable2 = |uart_0_s1_arb_share_counter_next_value;

  //pipeline_bridge_PERIPHERALS/m1 uart_0/s1 arbiterlock2, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_arbiterlock2 = uart_0_s1_slavearbiterlockenable2 & pipeline_bridge_PERIPHERALS_m1_continuerequest;

  //uart_0_s1_any_continuerequest at least one master continues requesting, which is an e_assign
  assign uart_0_s1_any_continuerequest = 1;

  //pipeline_bridge_PERIPHERALS_m1_continuerequest continued request, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_continuerequest = 1;

  assign pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 = pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1 & ~(((pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect) & ((pipeline_bridge_PERIPHERALS_m1_latency_counter != 0))));
  //local readdatavalid pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1 = pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1 & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect) & ~uart_0_s1_waits_for_read;

  //uart_0_s1_writedata mux, which is an e_mux
  assign uart_0_s1_writedata = pipeline_bridge_PERIPHERALS_m1_writedata;

  //master is always granted when requested
  assign pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1 = pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1;

  //pipeline_bridge_PERIPHERALS/m1 saved-grant uart_0/s1, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_saved_grant_uart_0_s1 = pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;

  //allow new arb cycle for uart_0/s1, which is an e_assign
  assign uart_0_s1_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign uart_0_s1_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign uart_0_s1_master_qreq_vector = 1;

  assign uart_0_s1_begintransfer = uart_0_s1_begins_xfer;
  //uart_0_s1_reset_n assignment, which is an e_assign
  assign uart_0_s1_reset_n = reset_n;

  assign uart_0_s1_chipselect = pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;
  //uart_0_s1_firsttransfer first transaction, which is an e_assign
  assign uart_0_s1_firsttransfer = uart_0_s1_begins_xfer ? uart_0_s1_unreg_firsttransfer : uart_0_s1_reg_firsttransfer;

  //uart_0_s1_unreg_firsttransfer first transaction, which is an e_assign
  assign uart_0_s1_unreg_firsttransfer = ~(uart_0_s1_slavearbiterlockenable & uart_0_s1_any_continuerequest);

  //uart_0_s1_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          uart_0_s1_reg_firsttransfer <= 1'b1;
      else if (uart_0_s1_begins_xfer)
          uart_0_s1_reg_firsttransfer <= uart_0_s1_unreg_firsttransfer;
    end


  //uart_0_s1_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign uart_0_s1_beginbursttransfer_internal = uart_0_s1_begins_xfer;

  //~uart_0_s1_read_n assignment, which is an e_mux
  assign uart_0_s1_read_n = ~(pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1 & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect));

  //~uart_0_s1_write_n assignment, which is an e_mux
  assign uart_0_s1_write_n = ~(pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1 & (pipeline_bridge_PERIPHERALS_m1_write & pipeline_bridge_PERIPHERALS_m1_chipselect));

  assign shifted_address_to_uart_0_s1_from_pipeline_bridge_PERIPHERALS_m1 = pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  //uart_0_s1_address mux, which is an e_mux
  assign uart_0_s1_address = shifted_address_to_uart_0_s1_from_pipeline_bridge_PERIPHERALS_m1 >> 2;

  //d1_uart_0_s1_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_uart_0_s1_end_xfer <= 1;
      else 
        d1_uart_0_s1_end_xfer <= uart_0_s1_end_xfer;
    end


  //uart_0_s1_waits_for_read in a cycle, which is an e_mux
  assign uart_0_s1_waits_for_read = uart_0_s1_in_a_read_cycle & uart_0_s1_begins_xfer;

  //uart_0_s1_in_a_read_cycle assignment, which is an e_assign
  assign uart_0_s1_in_a_read_cycle = pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1 & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = uart_0_s1_in_a_read_cycle;

  //uart_0_s1_waits_for_write in a cycle, which is an e_mux
  assign uart_0_s1_waits_for_write = uart_0_s1_in_a_write_cycle & uart_0_s1_begins_xfer;

  //uart_0_s1_in_a_write_cycle assignment, which is an e_assign
  assign uart_0_s1_in_a_write_cycle = pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1 & (pipeline_bridge_PERIPHERALS_m1_write & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = uart_0_s1_in_a_write_cycle;

  assign wait_for_uart_0_s1_counter = 0;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //uart_0/s1 enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end


  //pipeline_bridge_PERIPHERALS/m1 non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1 && (pipeline_bridge_PERIPHERALS_m1_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS/m1 drove 0 on its 'burstcount' port while accessing slave uart_0/s1", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_reset_ddr2_phy_clk_out_domain_synch_module (
                                                          // inputs:
                                                           clk,
                                                           data_in,
                                                           reset_n,

                                                          // outputs:
                                                           data_out
                                                        )
;

  output           data_out;
  input            clk;
  input            data_in;
  input            reset_n;

  reg              data_in_d1 /* synthesis ALTERA_ATTRIBUTE = "{-from \"*\"} CUT=ON ; PRESERVE_REGISTER=ON ; SUPPRESS_DA_RULE_INTERNAL=R101"  */;
  reg              data_out /* synthesis ALTERA_ATTRIBUTE = "PRESERVE_REGISTER=ON ; SUPPRESS_DA_RULE_INTERNAL=R101"  */;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_in_d1 <= 0;
      else 
        data_in_d1 <= data_in;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_out <= 0;
      else 
        data_out <= data_in_d1;
    end



endmodule


// synthesis translate_off
`timescale 1ns / 1ps
// synthesis translate_on

// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger (
               // 1) global signals:
                clk,
                ddr2_aux_full_rate_clk_out,
                ddr2_aux_half_rate_clk_out,
                ddr2_phy_clk_out,
                reset_n,

               // the_ddr2
                aux_scan_clk_from_the_ddr2,
                aux_scan_clk_reset_n_from_the_ddr2,
                dll_reference_clk_from_the_ddr2,
                dqs_delay_ctrl_export_from_the_ddr2,
                global_reset_n_to_the_ddr2,
                local_init_done_from_the_ddr2,
                local_refresh_ack_from_the_ddr2,
                local_wdata_req_from_the_ddr2,
                mem_addr_from_the_ddr2,
                mem_ba_from_the_ddr2,
                mem_cas_n_from_the_ddr2,
                mem_cke_from_the_ddr2,
                mem_clk_n_to_and_from_the_ddr2,
                mem_clk_to_and_from_the_ddr2,
                mem_cs_n_from_the_ddr2,
                mem_dm_from_the_ddr2,
                mem_dq_to_and_from_the_ddr2,
                mem_dqs_to_and_from_the_ddr2,
                mem_dqsn_to_and_from_the_ddr2,
                mem_odt_from_the_ddr2,
                mem_ras_n_from_the_ddr2,
                mem_we_n_from_the_ddr2,
                oct_ctl_rs_value_to_the_ddr2,
                oct_ctl_rt_value_to_the_ddr2,
                reset_phy_clk_n_from_the_ddr2,

               // the_tiger_top_0
                coe_debug_lights_from_the_tiger_top_0,
                coe_debug_select_to_the_tiger_top_0,
                coe_exe_end_from_the_tiger_top_0,
                coe_exe_start_from_the_tiger_top_0,

               // the_uart_0
                rxd_to_the_uart_0,
                txd_from_the_uart_0
             )
;

  output           aux_scan_clk_from_the_ddr2;
  output           aux_scan_clk_reset_n_from_the_ddr2;
  output  [ 17: 0] coe_debug_lights_from_the_tiger_top_0;
  output           coe_exe_end_from_the_tiger_top_0;
  output           coe_exe_start_from_the_tiger_top_0;
  output           ddr2_aux_full_rate_clk_out;
  output           ddr2_aux_half_rate_clk_out;
  output           ddr2_phy_clk_out;
  output           dll_reference_clk_from_the_ddr2;
  output  [  5: 0] dqs_delay_ctrl_export_from_the_ddr2;
  output           local_init_done_from_the_ddr2;
  output           local_refresh_ack_from_the_ddr2;
  output           local_wdata_req_from_the_ddr2;
  output  [ 13: 0] mem_addr_from_the_ddr2;
  output  [  2: 0] mem_ba_from_the_ddr2;
  output           mem_cas_n_from_the_ddr2;
  output           mem_cke_from_the_ddr2;
  inout   [  1: 0] mem_clk_n_to_and_from_the_ddr2;
  inout   [  1: 0] mem_clk_to_and_from_the_ddr2;
  output           mem_cs_n_from_the_ddr2;
  output  [  7: 0] mem_dm_from_the_ddr2;
  inout   [ 63: 0] mem_dq_to_and_from_the_ddr2;
  inout   [  7: 0] mem_dqs_to_and_from_the_ddr2;
  inout   [  7: 0] mem_dqsn_to_and_from_the_ddr2;
  output           mem_odt_from_the_ddr2;
  output           mem_ras_n_from_the_ddr2;
  output           mem_we_n_from_the_ddr2;
  output           reset_phy_clk_n_from_the_ddr2;
  output           txd_from_the_uart_0;
  input            clk;
  input   [  2: 0] coe_debug_select_to_the_tiger_top_0;
  input            global_reset_n_to_the_ddr2;
  input   [ 13: 0] oct_ctl_rs_value_to_the_ddr2;
  input   [ 13: 0] oct_ctl_rt_value_to_the_ddr2;
  input            reset_n;
  input            rxd_to_the_uart_0;

  wire             aux_scan_clk_from_the_ddr2;
  wire             aux_scan_clk_reset_n_from_the_ddr2;
  wire             clk_reset_n;
  wire    [ 17: 0] coe_debug_lights_from_the_tiger_top_0;
  wire             coe_exe_end_from_the_tiger_top_0;
  wire             coe_exe_start_from_the_tiger_top_0;
  wire             d1_data_cache_0_CACHE0_end_xfer;
  wire             d1_ddr2_s1_end_xfer;
  wire             d1_pipeline_bridge_MEMORY_s1_end_xfer;
  wire             d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  wire             d1_tiger_top_0_leapSlave_end_xfer;
  wire             d1_uart_0_s1_end_xfer;
  wire             data_cache_0_CACHE0_begintransfer;
  wire             data_cache_0_CACHE0_read;
  wire    [127: 0] data_cache_0_CACHE0_readdata;
  wire    [127: 0] data_cache_0_CACHE0_readdata_from_sa;
  wire             data_cache_0_CACHE0_waitrequest;
  wire             data_cache_0_CACHE0_waitrequest_from_sa;
  wire             data_cache_0_CACHE0_write;
  wire    [127: 0] data_cache_0_CACHE0_writedata;
  wire    [  7: 0] data_cache_0_PROC_data;
  wire             data_cache_0_PROC_reset_n;
  wire    [ 31: 0] data_cache_0_dataMaster0_address;
  wire    [ 31: 0] data_cache_0_dataMaster0_address_to_slave;
  wire             data_cache_0_dataMaster0_beginbursttransfer;
  wire    [  5: 0] data_cache_0_dataMaster0_burstcount;
  wire    [ 31: 0] data_cache_0_dataMaster0_byteenable;
  wire             data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_latency_counter;
  wire             data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_read;
  wire             data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire    [255: 0] data_cache_0_dataMaster0_readdata;
  wire             data_cache_0_dataMaster0_readdatavalid;
  wire             data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_waitrequest;
  wire             data_cache_0_dataMaster0_write;
  wire    [255: 0] data_cache_0_dataMaster0_writedata;
  wire             ddr2_aux_full_rate_clk_out;
  wire             ddr2_aux_half_rate_clk_out;
  wire             ddr2_phy_clk_out;
  wire             ddr2_phy_clk_out_reset_n;
  wire    [ 24: 0] ddr2_s1_address;
  wire             ddr2_s1_beginbursttransfer;
  wire    [  6: 0] ddr2_s1_burstcount;
  wire    [ 31: 0] ddr2_s1_byteenable;
  wire             ddr2_s1_read;
  wire    [255: 0] ddr2_s1_readdata;
  wire    [255: 0] ddr2_s1_readdata_from_sa;
  wire             ddr2_s1_readdatavalid;
  wire             ddr2_s1_resetrequest_n;
  wire             ddr2_s1_resetrequest_n_from_sa;
  wire             ddr2_s1_waitrequest_n;
  wire             ddr2_s1_waitrequest_n_from_sa;
  wire             ddr2_s1_write;
  wire    [255: 0] ddr2_s1_writedata;
  wire             dll_reference_clk_from_the_ddr2;
  wire    [  5: 0] dqs_delay_ctrl_export_from_the_ddr2;
  wire             jtag_to_ava_master_bridge_granted_ddr2_s1;
  wire             jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_latency_counter;
  wire    [ 31: 0] jtag_to_ava_master_bridge_master_address;
  wire    [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  wire    [  3: 0] jtag_to_ava_master_bridge_master_byteenable;
  wire             jtag_to_ava_master_bridge_master_read;
  wire    [ 31: 0] jtag_to_ava_master_bridge_master_readdata;
  wire             jtag_to_ava_master_bridge_master_readdatavalid;
  wire             jtag_to_ava_master_bridge_master_reset;
  wire             jtag_to_ava_master_bridge_master_resetrequest;
  wire             jtag_to_ava_master_bridge_master_waitrequest;
  wire             jtag_to_ava_master_bridge_master_write;
  wire    [ 31: 0] jtag_to_ava_master_bridge_master_writedata;
  wire             jtag_to_ava_master_bridge_qualified_request_ddr2_s1;
  wire             jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_read_data_valid_ddr2_s1;
  wire             jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register;
  wire             jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_requests_ddr2_s1;
  wire             jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;
  wire             local_init_done_from_the_ddr2;
  wire             local_refresh_ack_from_the_ddr2;
  wire             local_wdata_req_from_the_ddr2;
  wire    [ 13: 0] mem_addr_from_the_ddr2;
  wire    [  2: 0] mem_ba_from_the_ddr2;
  wire             mem_cas_n_from_the_ddr2;
  wire             mem_cke_from_the_ddr2;
  wire    [  1: 0] mem_clk_n_to_and_from_the_ddr2;
  wire    [  1: 0] mem_clk_to_and_from_the_ddr2;
  wire             mem_cs_n_from_the_ddr2;
  wire    [  7: 0] mem_dm_from_the_ddr2;
  wire    [ 63: 0] mem_dq_to_and_from_the_ddr2;
  wire    [  7: 0] mem_dqs_to_and_from_the_ddr2;
  wire    [  7: 0] mem_dqsn_to_and_from_the_ddr2;
  wire             mem_odt_from_the_ddr2;
  wire             mem_ras_n_from_the_ddr2;
  wire             mem_we_n_from_the_ddr2;
  wire             out_clk_ddr2_aux_full_rate_clk;
  wire             out_clk_ddr2_aux_half_rate_clk;
  wire             out_clk_ddr2_phy_clk;
  wire    [ 30: 0] pipeline_bridge_MEMORY_m1_address;
  wire    [ 30: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  wire    [  6: 0] pipeline_bridge_MEMORY_m1_burstcount;
  wire    [ 31: 0] pipeline_bridge_MEMORY_m1_byteenable;
  wire             pipeline_bridge_MEMORY_m1_chipselect;
  wire             pipeline_bridge_MEMORY_m1_debugaccess;
  wire             pipeline_bridge_MEMORY_m1_endofpacket;
  wire             pipeline_bridge_MEMORY_m1_granted_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_latency_counter;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_read;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register;
  wire    [255: 0] pipeline_bridge_MEMORY_m1_readdata;
  wire             pipeline_bridge_MEMORY_m1_readdatavalid;
  wire             pipeline_bridge_MEMORY_m1_requests_ddr2_s1;
  wire             pipeline_bridge_MEMORY_m1_waitrequest;
  wire             pipeline_bridge_MEMORY_m1_write;
  wire    [255: 0] pipeline_bridge_MEMORY_m1_writedata;
  wire    [ 25: 0] pipeline_bridge_MEMORY_s1_address;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock2;
  wire    [  6: 0] pipeline_bridge_MEMORY_s1_burstcount;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_byteenable;
  wire             pipeline_bridge_MEMORY_s1_chipselect;
  wire             pipeline_bridge_MEMORY_s1_debugaccess;
  wire             pipeline_bridge_MEMORY_s1_endofpacket;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire    [ 25: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  wire             pipeline_bridge_MEMORY_s1_read;
  wire    [255: 0] pipeline_bridge_MEMORY_s1_readdata;
  wire    [255: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  wire             pipeline_bridge_MEMORY_s1_readdatavalid;
  wire             pipeline_bridge_MEMORY_s1_reset_n;
  wire             pipeline_bridge_MEMORY_s1_waitrequest;
  wire             pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  wire             pipeline_bridge_MEMORY_s1_write;
  wire    [255: 0] pipeline_bridge_MEMORY_s1_writedata;
  wire    [ 12: 0] pipeline_bridge_PERIPHERALS_m1_address;
  wire    [ 12: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  wire             pipeline_bridge_PERIPHERALS_m1_burstcount;
  wire    [  3: 0] pipeline_bridge_PERIPHERALS_m1_byteenable;
  wire             pipeline_bridge_PERIPHERALS_m1_chipselect;
  wire             pipeline_bridge_PERIPHERALS_m1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_m1_endofpacket;
  wire             pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_latency_counter;
  wire             pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_read;
  wire             pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_m1_readdata;
  wire             pipeline_bridge_PERIPHERALS_m1_readdatavalid;
  wire             pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_waitrequest;
  wire             pipeline_bridge_PERIPHERALS_m1_write;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata;
  wire    [ 10: 0] pipeline_bridge_PERIPHERALS_s1_address;
  wire             pipeline_bridge_PERIPHERALS_s1_arbiterlock;
  wire             pipeline_bridge_PERIPHERALS_s1_arbiterlock2;
  wire             pipeline_bridge_PERIPHERALS_s1_burstcount;
  wire    [  3: 0] pipeline_bridge_PERIPHERALS_s1_byteenable;
  wire             pipeline_bridge_PERIPHERALS_s1_chipselect;
  wire             pipeline_bridge_PERIPHERALS_s1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  wire    [ 10: 0] pipeline_bridge_PERIPHERALS_s1_nativeaddress;
  wire             pipeline_bridge_PERIPHERALS_s1_read;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;
  wire             pipeline_bridge_PERIPHERALS_s1_readdatavalid;
  wire             pipeline_bridge_PERIPHERALS_s1_reset_n;
  wire             pipeline_bridge_PERIPHERALS_s1_waitrequest;
  wire             pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;
  wire             pipeline_bridge_PERIPHERALS_s1_write;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_s1_writedata;
  wire             reset_n_sources;
  wire             reset_phy_clk_n_from_the_ddr2;
  wire    [ 31: 0] tiger_top_0_CACHE_address;
  wire    [ 31: 0] tiger_top_0_CACHE_address_to_slave;
  wire             tiger_top_0_CACHE_granted_data_cache_0_CACHE0;
  wire             tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0;
  wire             tiger_top_0_CACHE_read;
  wire             tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0;
  wire    [127: 0] tiger_top_0_CACHE_readdata;
  wire             tiger_top_0_CACHE_requests_data_cache_0_CACHE0;
  wire             tiger_top_0_CACHE_reset;
  wire             tiger_top_0_CACHE_waitrequest;
  wire             tiger_top_0_CACHE_write;
  wire    [127: 0] tiger_top_0_CACHE_writedata;
  wire    [  7: 0] tiger_top_0_PROC_data;
  wire    [ 31: 0] tiger_top_0_instructionMaster_address;
  wire    [ 31: 0] tiger_top_0_instructionMaster_address_to_slave;
  wire             tiger_top_0_instructionMaster_beginbursttransfer;
  wire    [  5: 0] tiger_top_0_instructionMaster_burstcount;
  wire             tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_latency_counter;
  wire             tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_read;
  wire             tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire    [255: 0] tiger_top_0_instructionMaster_readdata;
  wire             tiger_top_0_instructionMaster_readdatavalid;
  wire             tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1;
  wire             tiger_top_0_instructionMaster_waitrequest;
  wire    [  7: 0] tiger_top_0_leapSlave_address;
  wire             tiger_top_0_leapSlave_chipselect;
  wire             tiger_top_0_leapSlave_read;
  wire    [ 31: 0] tiger_top_0_leapSlave_readdata;
  wire    [ 31: 0] tiger_top_0_leapSlave_readdata_from_sa;
  wire             tiger_top_0_leapSlave_write;
  wire    [ 31: 0] tiger_top_0_leapSlave_writedata;
  wire    [ 31: 0] tiger_top_0_procMaster_address;
  wire    [ 31: 0] tiger_top_0_procMaster_address_to_slave;
  wire    [  3: 0] tiger_top_0_procMaster_byteenable;
  wire             tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_latency_counter;
  wire             tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_read;
  wire             tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  wire    [ 31: 0] tiger_top_0_procMaster_readdata;
  wire             tiger_top_0_procMaster_readdatavalid;
  wire             tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_top_0_procMaster_waitrequest;
  wire             tiger_top_0_procMaster_write;
  wire    [ 31: 0] tiger_top_0_procMaster_writedata;
  wire             txd_from_the_uart_0;
  wire    [  2: 0] uart_0_s1_address;
  wire             uart_0_s1_begintransfer;
  wire             uart_0_s1_chipselect;
  wire             uart_0_s1_dataavailable;
  wire             uart_0_s1_dataavailable_from_sa;
  wire             uart_0_s1_irq;
  wire             uart_0_s1_read_n;
  wire    [ 15: 0] uart_0_s1_readdata;
  wire    [ 15: 0] uart_0_s1_readdata_from_sa;
  wire             uart_0_s1_readyfordata;
  wire             uart_0_s1_readyfordata_from_sa;
  wire             uart_0_s1_reset_n;
  wire             uart_0_s1_write_n;
  wire    [ 15: 0] uart_0_s1_writedata;
  data_cache_0_CACHE0_arbitrator the_data_cache_0_CACHE0
    (
      .clk                                                     (ddr2_phy_clk_out),
      .d1_data_cache_0_CACHE0_end_xfer                         (d1_data_cache_0_CACHE0_end_xfer),
      .data_cache_0_CACHE0_begintransfer                       (data_cache_0_CACHE0_begintransfer),
      .data_cache_0_CACHE0_read                                (data_cache_0_CACHE0_read),
      .data_cache_0_CACHE0_readdata                            (data_cache_0_CACHE0_readdata),
      .data_cache_0_CACHE0_readdata_from_sa                    (data_cache_0_CACHE0_readdata_from_sa),
      .data_cache_0_CACHE0_waitrequest                         (data_cache_0_CACHE0_waitrequest),
      .data_cache_0_CACHE0_waitrequest_from_sa                 (data_cache_0_CACHE0_waitrequest_from_sa),
      .data_cache_0_CACHE0_write                               (data_cache_0_CACHE0_write),
      .data_cache_0_CACHE0_writedata                           (data_cache_0_CACHE0_writedata),
      .reset_n                                                 (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_CACHE_address_to_slave                      (tiger_top_0_CACHE_address_to_slave),
      .tiger_top_0_CACHE_granted_data_cache_0_CACHE0           (tiger_top_0_CACHE_granted_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0 (tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_read                                  (tiger_top_0_CACHE_read),
      .tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0   (tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_requests_data_cache_0_CACHE0          (tiger_top_0_CACHE_requests_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_write                                 (tiger_top_0_CACHE_write),
      .tiger_top_0_CACHE_writedata                             (tiger_top_0_CACHE_writedata)
    );

  data_cache_0_PROC_arbitrator the_data_cache_0_PROC
    (
      .clk                       (ddr2_phy_clk_out),
      .data_cache_0_PROC_data    (data_cache_0_PROC_data),
      .data_cache_0_PROC_reset_n (data_cache_0_PROC_reset_n),
      .reset_n                   (ddr2_phy_clk_out_reset_n)
    );

  data_cache_0_dataMaster0_arbitrator the_data_cache_0_dataMaster0
    (
      .clk                                                                               (ddr2_phy_clk_out),
      .d1_pipeline_bridge_MEMORY_s1_end_xfer                                             (d1_pipeline_bridge_MEMORY_s1_end_xfer),
      .data_cache_0_dataMaster0_address                                                  (data_cache_0_dataMaster0_address),
      .data_cache_0_dataMaster0_address_to_slave                                         (data_cache_0_dataMaster0_address_to_slave),
      .data_cache_0_dataMaster0_burstcount                                               (data_cache_0_dataMaster0_burstcount),
      .data_cache_0_dataMaster0_byteenable                                               (data_cache_0_dataMaster0_byteenable),
      .data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1                        (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_latency_counter                                          (data_cache_0_dataMaster0_latency_counter),
      .data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1              (data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_read                                                     (data_cache_0_dataMaster0_read),
      .data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1                (data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register (data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .data_cache_0_dataMaster0_readdata                                                 (data_cache_0_dataMaster0_readdata),
      .data_cache_0_dataMaster0_readdatavalid                                            (data_cache_0_dataMaster0_readdatavalid),
      .data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1                       (data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_waitrequest                                              (data_cache_0_dataMaster0_waitrequest),
      .data_cache_0_dataMaster0_write                                                    (data_cache_0_dataMaster0_write),
      .data_cache_0_dataMaster0_writedata                                                (data_cache_0_dataMaster0_writedata),
      .pipeline_bridge_MEMORY_s1_readdata_from_sa                                        (pipeline_bridge_MEMORY_s1_readdata_from_sa),
      .pipeline_bridge_MEMORY_s1_waitrequest_from_sa                                     (pipeline_bridge_MEMORY_s1_waitrequest_from_sa),
      .reset_n                                                                           (ddr2_phy_clk_out_reset_n)
    );

  data_cache_0 the_data_cache_0
    (
      .aso_PROC_data                      (data_cache_0_PROC_data),
      .avm_dataMaster0_address            (data_cache_0_dataMaster0_address),
      .avm_dataMaster0_beginbursttransfer (data_cache_0_dataMaster0_beginbursttransfer),
      .avm_dataMaster0_burstcount         (data_cache_0_dataMaster0_burstcount),
      .avm_dataMaster0_byteenable         (data_cache_0_dataMaster0_byteenable),
      .avm_dataMaster0_read               (data_cache_0_dataMaster0_read),
      .avm_dataMaster0_readdata           (data_cache_0_dataMaster0_readdata),
      .avm_dataMaster0_readdatavalid      (data_cache_0_dataMaster0_readdatavalid),
      .avm_dataMaster0_waitrequest        (data_cache_0_dataMaster0_waitrequest),
      .avm_dataMaster0_write              (data_cache_0_dataMaster0_write),
      .avm_dataMaster0_writedata          (data_cache_0_dataMaster0_writedata),
      .avs_CACHE0_begintransfer           (data_cache_0_CACHE0_begintransfer),
      .avs_CACHE0_read                    (data_cache_0_CACHE0_read),
      .avs_CACHE0_readdata                (data_cache_0_CACHE0_readdata),
      .avs_CACHE0_waitrequest             (data_cache_0_CACHE0_waitrequest),
      .avs_CACHE0_write                   (data_cache_0_CACHE0_write),
      .avs_CACHE0_writedata               (data_cache_0_CACHE0_writedata),
      .csi_clockreset_clk                 (ddr2_phy_clk_out),
      .csi_clockreset_reset_n             (data_cache_0_PROC_reset_n)
    );

  ddr2_s1_arbitrator the_ddr2_s1
    (
      .clk                                                              (ddr2_phy_clk_out),
      .d1_ddr2_s1_end_xfer                                              (d1_ddr2_s1_end_xfer),
      .ddr2_s1_address                                                  (ddr2_s1_address),
      .ddr2_s1_beginbursttransfer                                       (ddr2_s1_beginbursttransfer),
      .ddr2_s1_burstcount                                               (ddr2_s1_burstcount),
      .ddr2_s1_byteenable                                               (ddr2_s1_byteenable),
      .ddr2_s1_read                                                     (ddr2_s1_read),
      .ddr2_s1_readdata                                                 (ddr2_s1_readdata),
      .ddr2_s1_readdata_from_sa                                         (ddr2_s1_readdata_from_sa),
      .ddr2_s1_readdatavalid                                            (ddr2_s1_readdatavalid),
      .ddr2_s1_resetrequest_n                                           (ddr2_s1_resetrequest_n),
      .ddr2_s1_resetrequest_n_from_sa                                   (ddr2_s1_resetrequest_n_from_sa),
      .ddr2_s1_waitrequest_n                                            (ddr2_s1_waitrequest_n),
      .ddr2_s1_waitrequest_n_from_sa                                    (ddr2_s1_waitrequest_n_from_sa),
      .ddr2_s1_write                                                    (ddr2_s1_write),
      .ddr2_s1_writedata                                                (ddr2_s1_writedata),
      .jtag_to_ava_master_bridge_granted_ddr2_s1                        (jtag_to_ava_master_bridge_granted_ddr2_s1),
      .jtag_to_ava_master_bridge_latency_counter                        (jtag_to_ava_master_bridge_latency_counter),
      .jtag_to_ava_master_bridge_master_address_to_slave                (jtag_to_ava_master_bridge_master_address_to_slave),
      .jtag_to_ava_master_bridge_master_byteenable                      (jtag_to_ava_master_bridge_master_byteenable),
      .jtag_to_ava_master_bridge_master_read                            (jtag_to_ava_master_bridge_master_read),
      .jtag_to_ava_master_bridge_master_write                           (jtag_to_ava_master_bridge_master_write),
      .jtag_to_ava_master_bridge_master_writedata                       (jtag_to_ava_master_bridge_master_writedata),
      .jtag_to_ava_master_bridge_qualified_request_ddr2_s1              (jtag_to_ava_master_bridge_qualified_request_ddr2_s1),
      .jtag_to_ava_master_bridge_read_data_valid_ddr2_s1                (jtag_to_ava_master_bridge_read_data_valid_ddr2_s1),
      .jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register (jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register),
      .jtag_to_ava_master_bridge_requests_ddr2_s1                       (jtag_to_ava_master_bridge_requests_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_address_to_slave                       (pipeline_bridge_MEMORY_m1_address_to_slave),
      .pipeline_bridge_MEMORY_m1_burstcount                             (pipeline_bridge_MEMORY_m1_burstcount),
      .pipeline_bridge_MEMORY_m1_byteenable                             (pipeline_bridge_MEMORY_m1_byteenable),
      .pipeline_bridge_MEMORY_m1_chipselect                             (pipeline_bridge_MEMORY_m1_chipselect),
      .pipeline_bridge_MEMORY_m1_granted_ddr2_s1                        (pipeline_bridge_MEMORY_m1_granted_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_latency_counter                        (pipeline_bridge_MEMORY_m1_latency_counter),
      .pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1              (pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_read                                   (pipeline_bridge_MEMORY_m1_read),
      .pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1                (pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register),
      .pipeline_bridge_MEMORY_m1_requests_ddr2_s1                       (pipeline_bridge_MEMORY_m1_requests_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_write                                  (pipeline_bridge_MEMORY_m1_write),
      .pipeline_bridge_MEMORY_m1_writedata                              (pipeline_bridge_MEMORY_m1_writedata),
      .reset_n                                                          (ddr2_phy_clk_out_reset_n)
    );

  //ddr2_aux_full_rate_clk_out out_clk assignment, which is an e_assign
  assign ddr2_aux_full_rate_clk_out = out_clk_ddr2_aux_full_rate_clk;

  //ddr2_aux_half_rate_clk_out out_clk assignment, which is an e_assign
  assign ddr2_aux_half_rate_clk_out = out_clk_ddr2_aux_half_rate_clk;

  //ddr2_phy_clk_out out_clk assignment, which is an e_assign
  assign ddr2_phy_clk_out = out_clk_ddr2_phy_clk;

  //reset is asserted asynchronously and deasserted synchronously
  tiger_reset_clk_domain_synch_module tiger_reset_clk_domain_synch
    (
      .clk      (clk),
      .data_in  (1'b1),
      .data_out (clk_reset_n),
      .reset_n  (reset_n_sources)
    );

  //reset sources mux, which is an e_mux
  assign reset_n_sources = ~(~reset_n |
    0 |
    0 |
    jtag_to_ava_master_bridge_master_resetrequest |
    ~ddr2_s1_resetrequest_n_from_sa |
    ~ddr2_s1_resetrequest_n_from_sa |
    jtag_to_ava_master_bridge_master_resetrequest);

  ddr2 the_ddr2
    (
      .aux_full_rate_clk     (out_clk_ddr2_aux_full_rate_clk),
      .aux_half_rate_clk     (out_clk_ddr2_aux_half_rate_clk),
      .aux_scan_clk          (aux_scan_clk_from_the_ddr2),
      .aux_scan_clk_reset_n  (aux_scan_clk_reset_n_from_the_ddr2),
      .dll_reference_clk     (dll_reference_clk_from_the_ddr2),
      .dqs_delay_ctrl_export (dqs_delay_ctrl_export_from_the_ddr2),
      .global_reset_n        (global_reset_n_to_the_ddr2),
      .local_address         (ddr2_s1_address),
      .local_be              (ddr2_s1_byteenable),
      .local_burstbegin      (ddr2_s1_beginbursttransfer),
      .local_init_done       (local_init_done_from_the_ddr2),
      .local_rdata           (ddr2_s1_readdata),
      .local_rdata_valid     (ddr2_s1_readdatavalid),
      .local_read_req        (ddr2_s1_read),
      .local_ready           (ddr2_s1_waitrequest_n),
      .local_refresh_ack     (local_refresh_ack_from_the_ddr2),
      .local_size            (ddr2_s1_burstcount),
      .local_wdata           (ddr2_s1_writedata),
      .local_wdata_req       (local_wdata_req_from_the_ddr2),
      .local_write_req       (ddr2_s1_write),
      .mem_addr              (mem_addr_from_the_ddr2),
      .mem_ba                (mem_ba_from_the_ddr2),
      .mem_cas_n             (mem_cas_n_from_the_ddr2),
      .mem_cke               (mem_cke_from_the_ddr2),
      .mem_clk               (mem_clk_to_and_from_the_ddr2),
      .mem_clk_n             (mem_clk_n_to_and_from_the_ddr2),
      .mem_cs_n              (mem_cs_n_from_the_ddr2),
      .mem_dm                (mem_dm_from_the_ddr2),
      .mem_dq                (mem_dq_to_and_from_the_ddr2),
      .mem_dqs               (mem_dqs_to_and_from_the_ddr2),
      .mem_dqsn              (mem_dqsn_to_and_from_the_ddr2),
      .mem_odt               (mem_odt_from_the_ddr2),
      .mem_ras_n             (mem_ras_n_from_the_ddr2),
      .mem_we_n              (mem_we_n_from_the_ddr2),
      .oct_ctl_rs_value      (oct_ctl_rs_value_to_the_ddr2),
      .oct_ctl_rt_value      (oct_ctl_rt_value_to_the_ddr2),
      .phy_clk               (out_clk_ddr2_phy_clk),
      .pll_ref_clk           (clk),
      .reset_phy_clk_n       (reset_phy_clk_n_from_the_ddr2),
      .reset_request_n       (ddr2_s1_resetrequest_n),
      .soft_reset_n          (clk_reset_n)
    );

  jtag_to_ava_master_bridge_master_arbitrator the_jtag_to_ava_master_bridge_master
    (
      .clk                                                               (ddr2_phy_clk_out),
      .d1_ddr2_s1_end_xfer                                               (d1_ddr2_s1_end_xfer),
      .d1_tiger_top_0_leapSlave_end_xfer                                 (d1_tiger_top_0_leapSlave_end_xfer),
      .ddr2_s1_readdata_from_sa                                          (ddr2_s1_readdata_from_sa),
      .ddr2_s1_waitrequest_n_from_sa                                     (ddr2_s1_waitrequest_n_from_sa),
      .jtag_to_ava_master_bridge_granted_ddr2_s1                         (jtag_to_ava_master_bridge_granted_ddr2_s1),
      .jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave           (jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_latency_counter                         (jtag_to_ava_master_bridge_latency_counter),
      .jtag_to_ava_master_bridge_master_address                          (jtag_to_ava_master_bridge_master_address),
      .jtag_to_ava_master_bridge_master_address_to_slave                 (jtag_to_ava_master_bridge_master_address_to_slave),
      .jtag_to_ava_master_bridge_master_byteenable                       (jtag_to_ava_master_bridge_master_byteenable),
      .jtag_to_ava_master_bridge_master_read                             (jtag_to_ava_master_bridge_master_read),
      .jtag_to_ava_master_bridge_master_readdata                         (jtag_to_ava_master_bridge_master_readdata),
      .jtag_to_ava_master_bridge_master_readdatavalid                    (jtag_to_ava_master_bridge_master_readdatavalid),
      .jtag_to_ava_master_bridge_master_reset                            (jtag_to_ava_master_bridge_master_reset),
      .jtag_to_ava_master_bridge_master_waitrequest                      (jtag_to_ava_master_bridge_master_waitrequest),
      .jtag_to_ava_master_bridge_master_write                            (jtag_to_ava_master_bridge_master_write),
      .jtag_to_ava_master_bridge_master_writedata                        (jtag_to_ava_master_bridge_master_writedata),
      .jtag_to_ava_master_bridge_qualified_request_ddr2_s1               (jtag_to_ava_master_bridge_qualified_request_ddr2_s1),
      .jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave (jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_read_data_valid_ddr2_s1                 (jtag_to_ava_master_bridge_read_data_valid_ddr2_s1),
      .jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register  (jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register),
      .jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave   (jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_requests_ddr2_s1                        (jtag_to_ava_master_bridge_requests_ddr2_s1),
      .jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave          (jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave),
      .reset_n                                                           (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_leapSlave_readdata_from_sa                            (tiger_top_0_leapSlave_readdata_from_sa)
    );

  jtag_to_ava_master_bridge the_jtag_to_ava_master_bridge
    (
      .clk_clk              (ddr2_phy_clk_out),
      .clk_reset_reset      (jtag_to_ava_master_bridge_master_reset),
      .master_address       (jtag_to_ava_master_bridge_master_address),
      .master_byteenable    (jtag_to_ava_master_bridge_master_byteenable),
      .master_read          (jtag_to_ava_master_bridge_master_read),
      .master_readdata      (jtag_to_ava_master_bridge_master_readdata),
      .master_readdatavalid (jtag_to_ava_master_bridge_master_readdatavalid),
      .master_reset_reset   (jtag_to_ava_master_bridge_master_resetrequest),
      .master_waitrequest   (jtag_to_ava_master_bridge_master_waitrequest),
      .master_write         (jtag_to_ava_master_bridge_master_write),
      .master_writedata     (jtag_to_ava_master_bridge_master_writedata)
    );

  pipeline_bridge_MEMORY_s1_arbitrator the_pipeline_bridge_MEMORY_s1
    (
      .clk                                                                                    (ddr2_phy_clk_out),
      .d1_pipeline_bridge_MEMORY_s1_end_xfer                                                  (d1_pipeline_bridge_MEMORY_s1_end_xfer),
      .data_cache_0_dataMaster0_address_to_slave                                              (data_cache_0_dataMaster0_address_to_slave),
      .data_cache_0_dataMaster0_burstcount                                                    (data_cache_0_dataMaster0_burstcount),
      .data_cache_0_dataMaster0_byteenable                                                    (data_cache_0_dataMaster0_byteenable),
      .data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1                             (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_latency_counter                                               (data_cache_0_dataMaster0_latency_counter),
      .data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1                   (data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_read                                                          (data_cache_0_dataMaster0_read),
      .data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1                     (data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register      (data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1                            (data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster0_write                                                         (data_cache_0_dataMaster0_write),
      .data_cache_0_dataMaster0_writedata                                                     (data_cache_0_dataMaster0_writedata),
      .pipeline_bridge_MEMORY_s1_address                                                      (pipeline_bridge_MEMORY_s1_address),
      .pipeline_bridge_MEMORY_s1_arbiterlock                                                  (pipeline_bridge_MEMORY_s1_arbiterlock),
      .pipeline_bridge_MEMORY_s1_arbiterlock2                                                 (pipeline_bridge_MEMORY_s1_arbiterlock2),
      .pipeline_bridge_MEMORY_s1_burstcount                                                   (pipeline_bridge_MEMORY_s1_burstcount),
      .pipeline_bridge_MEMORY_s1_byteenable                                                   (pipeline_bridge_MEMORY_s1_byteenable),
      .pipeline_bridge_MEMORY_s1_chipselect                                                   (pipeline_bridge_MEMORY_s1_chipselect),
      .pipeline_bridge_MEMORY_s1_debugaccess                                                  (pipeline_bridge_MEMORY_s1_debugaccess),
      .pipeline_bridge_MEMORY_s1_endofpacket                                                  (pipeline_bridge_MEMORY_s1_endofpacket),
      .pipeline_bridge_MEMORY_s1_endofpacket_from_sa                                          (pipeline_bridge_MEMORY_s1_endofpacket_from_sa),
      .pipeline_bridge_MEMORY_s1_nativeaddress                                                (pipeline_bridge_MEMORY_s1_nativeaddress),
      .pipeline_bridge_MEMORY_s1_read                                                         (pipeline_bridge_MEMORY_s1_read),
      .pipeline_bridge_MEMORY_s1_readdata                                                     (pipeline_bridge_MEMORY_s1_readdata),
      .pipeline_bridge_MEMORY_s1_readdata_from_sa                                             (pipeline_bridge_MEMORY_s1_readdata_from_sa),
      .pipeline_bridge_MEMORY_s1_readdatavalid                                                (pipeline_bridge_MEMORY_s1_readdatavalid),
      .pipeline_bridge_MEMORY_s1_reset_n                                                      (pipeline_bridge_MEMORY_s1_reset_n),
      .pipeline_bridge_MEMORY_s1_waitrequest                                                  (pipeline_bridge_MEMORY_s1_waitrequest),
      .pipeline_bridge_MEMORY_s1_waitrequest_from_sa                                          (pipeline_bridge_MEMORY_s1_waitrequest_from_sa),
      .pipeline_bridge_MEMORY_s1_write                                                        (pipeline_bridge_MEMORY_s1_write),
      .pipeline_bridge_MEMORY_s1_writedata                                                    (pipeline_bridge_MEMORY_s1_writedata),
      .reset_n                                                                                (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_instructionMaster_address_to_slave                                         (tiger_top_0_instructionMaster_address_to_slave),
      .tiger_top_0_instructionMaster_burstcount                                               (tiger_top_0_instructionMaster_burstcount),
      .tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1                        (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1),
      .tiger_top_0_instructionMaster_latency_counter                                          (tiger_top_0_instructionMaster_latency_counter),
      .tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1              (tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1),
      .tiger_top_0_instructionMaster_read                                                     (tiger_top_0_instructionMaster_read),
      .tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1                (tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1),
      .tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register (tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1                       (tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1)
    );

  pipeline_bridge_MEMORY_m1_arbitrator the_pipeline_bridge_MEMORY_m1
    (
      .clk                                                              (ddr2_phy_clk_out),
      .d1_ddr2_s1_end_xfer                                              (d1_ddr2_s1_end_xfer),
      .ddr2_s1_readdata_from_sa                                         (ddr2_s1_readdata_from_sa),
      .ddr2_s1_waitrequest_n_from_sa                                    (ddr2_s1_waitrequest_n_from_sa),
      .pipeline_bridge_MEMORY_m1_address                                (pipeline_bridge_MEMORY_m1_address),
      .pipeline_bridge_MEMORY_m1_address_to_slave                       (pipeline_bridge_MEMORY_m1_address_to_slave),
      .pipeline_bridge_MEMORY_m1_burstcount                             (pipeline_bridge_MEMORY_m1_burstcount),
      .pipeline_bridge_MEMORY_m1_byteenable                             (pipeline_bridge_MEMORY_m1_byteenable),
      .pipeline_bridge_MEMORY_m1_chipselect                             (pipeline_bridge_MEMORY_m1_chipselect),
      .pipeline_bridge_MEMORY_m1_granted_ddr2_s1                        (pipeline_bridge_MEMORY_m1_granted_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_latency_counter                        (pipeline_bridge_MEMORY_m1_latency_counter),
      .pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1              (pipeline_bridge_MEMORY_m1_qualified_request_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_read                                   (pipeline_bridge_MEMORY_m1_read),
      .pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1                (pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_ddr2_s1_shift_register),
      .pipeline_bridge_MEMORY_m1_readdata                               (pipeline_bridge_MEMORY_m1_readdata),
      .pipeline_bridge_MEMORY_m1_readdatavalid                          (pipeline_bridge_MEMORY_m1_readdatavalid),
      .pipeline_bridge_MEMORY_m1_requests_ddr2_s1                       (pipeline_bridge_MEMORY_m1_requests_ddr2_s1),
      .pipeline_bridge_MEMORY_m1_waitrequest                            (pipeline_bridge_MEMORY_m1_waitrequest),
      .pipeline_bridge_MEMORY_m1_write                                  (pipeline_bridge_MEMORY_m1_write),
      .pipeline_bridge_MEMORY_m1_writedata                              (pipeline_bridge_MEMORY_m1_writedata),
      .reset_n                                                          (ddr2_phy_clk_out_reset_n)
    );

  pipeline_bridge_MEMORY the_pipeline_bridge_MEMORY
    (
      .clk              (ddr2_phy_clk_out),
      .m1_address       (pipeline_bridge_MEMORY_m1_address),
      .m1_burstcount    (pipeline_bridge_MEMORY_m1_burstcount),
      .m1_byteenable    (pipeline_bridge_MEMORY_m1_byteenable),
      .m1_chipselect    (pipeline_bridge_MEMORY_m1_chipselect),
      .m1_debugaccess   (pipeline_bridge_MEMORY_m1_debugaccess),
      .m1_endofpacket   (pipeline_bridge_MEMORY_m1_endofpacket),
      .m1_read          (pipeline_bridge_MEMORY_m1_read),
      .m1_readdata      (pipeline_bridge_MEMORY_m1_readdata),
      .m1_readdatavalid (pipeline_bridge_MEMORY_m1_readdatavalid),
      .m1_waitrequest   (pipeline_bridge_MEMORY_m1_waitrequest),
      .m1_write         (pipeline_bridge_MEMORY_m1_write),
      .m1_writedata     (pipeline_bridge_MEMORY_m1_writedata),
      .reset_n          (pipeline_bridge_MEMORY_s1_reset_n),
      .s1_address       (pipeline_bridge_MEMORY_s1_address),
      .s1_arbiterlock   (pipeline_bridge_MEMORY_s1_arbiterlock),
      .s1_arbiterlock2  (pipeline_bridge_MEMORY_s1_arbiterlock2),
      .s1_burstcount    (pipeline_bridge_MEMORY_s1_burstcount),
      .s1_byteenable    (pipeline_bridge_MEMORY_s1_byteenable),
      .s1_chipselect    (pipeline_bridge_MEMORY_s1_chipselect),
      .s1_debugaccess   (pipeline_bridge_MEMORY_s1_debugaccess),
      .s1_endofpacket   (pipeline_bridge_MEMORY_s1_endofpacket),
      .s1_nativeaddress (pipeline_bridge_MEMORY_s1_nativeaddress),
      .s1_read          (pipeline_bridge_MEMORY_s1_read),
      .s1_readdata      (pipeline_bridge_MEMORY_s1_readdata),
      .s1_readdatavalid (pipeline_bridge_MEMORY_s1_readdatavalid),
      .s1_waitrequest   (pipeline_bridge_MEMORY_s1_waitrequest),
      .s1_write         (pipeline_bridge_MEMORY_s1_write),
      .s1_writedata     (pipeline_bridge_MEMORY_s1_writedata)
    );

  pipeline_bridge_PERIPHERALS_s1_arbitrator the_pipeline_bridge_PERIPHERALS_s1
    (
      .clk                                                                                  (ddr2_phy_clk_out),
      .d1_pipeline_bridge_PERIPHERALS_s1_end_xfer                                           (d1_pipeline_bridge_PERIPHERALS_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_s1_address                                               (pipeline_bridge_PERIPHERALS_s1_address),
      .pipeline_bridge_PERIPHERALS_s1_arbiterlock                                           (pipeline_bridge_PERIPHERALS_s1_arbiterlock),
      .pipeline_bridge_PERIPHERALS_s1_arbiterlock2                                          (pipeline_bridge_PERIPHERALS_s1_arbiterlock2),
      .pipeline_bridge_PERIPHERALS_s1_burstcount                                            (pipeline_bridge_PERIPHERALS_s1_burstcount),
      .pipeline_bridge_PERIPHERALS_s1_byteenable                                            (pipeline_bridge_PERIPHERALS_s1_byteenable),
      .pipeline_bridge_PERIPHERALS_s1_chipselect                                            (pipeline_bridge_PERIPHERALS_s1_chipselect),
      .pipeline_bridge_PERIPHERALS_s1_debugaccess                                           (pipeline_bridge_PERIPHERALS_s1_debugaccess),
      .pipeline_bridge_PERIPHERALS_s1_endofpacket                                           (pipeline_bridge_PERIPHERALS_s1_endofpacket),
      .pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa                                   (pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_nativeaddress                                         (pipeline_bridge_PERIPHERALS_s1_nativeaddress),
      .pipeline_bridge_PERIPHERALS_s1_read                                                  (pipeline_bridge_PERIPHERALS_s1_read),
      .pipeline_bridge_PERIPHERALS_s1_readdata                                              (pipeline_bridge_PERIPHERALS_s1_readdata),
      .pipeline_bridge_PERIPHERALS_s1_readdata_from_sa                                      (pipeline_bridge_PERIPHERALS_s1_readdata_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_readdatavalid                                         (pipeline_bridge_PERIPHERALS_s1_readdatavalid),
      .pipeline_bridge_PERIPHERALS_s1_reset_n                                               (pipeline_bridge_PERIPHERALS_s1_reset_n),
      .pipeline_bridge_PERIPHERALS_s1_waitrequest                                           (pipeline_bridge_PERIPHERALS_s1_waitrequest),
      .pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa                                   (pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_write                                                 (pipeline_bridge_PERIPHERALS_s1_write),
      .pipeline_bridge_PERIPHERALS_s1_writedata                                             (pipeline_bridge_PERIPHERALS_s1_writedata),
      .reset_n                                                                              (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_procMaster_address_to_slave                                              (tiger_top_0_procMaster_address_to_slave),
      .tiger_top_0_procMaster_byteenable                                                    (tiger_top_0_procMaster_byteenable),
      .tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1                        (tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_latency_counter                                               (tiger_top_0_procMaster_latency_counter),
      .tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1              (tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_read                                                          (tiger_top_0_procMaster_read),
      .tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1                (tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register (tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register),
      .tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1                       (tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_write                                                         (tiger_top_0_procMaster_write),
      .tiger_top_0_procMaster_writedata                                                     (tiger_top_0_procMaster_writedata)
    );

  pipeline_bridge_PERIPHERALS_m1_arbitrator the_pipeline_bridge_PERIPHERALS_m1
    (
      .clk                                                        (ddr2_phy_clk_out),
      .d1_uart_0_s1_end_xfer                                      (d1_uart_0_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_m1_address                     (pipeline_bridge_PERIPHERALS_m1_address),
      .pipeline_bridge_PERIPHERALS_m1_address_to_slave            (pipeline_bridge_PERIPHERALS_m1_address_to_slave),
      .pipeline_bridge_PERIPHERALS_m1_burstcount                  (pipeline_bridge_PERIPHERALS_m1_burstcount),
      .pipeline_bridge_PERIPHERALS_m1_byteenable                  (pipeline_bridge_PERIPHERALS_m1_byteenable),
      .pipeline_bridge_PERIPHERALS_m1_chipselect                  (pipeline_bridge_PERIPHERALS_m1_chipselect),
      .pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1           (pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_latency_counter             (pipeline_bridge_PERIPHERALS_m1_latency_counter),
      .pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 (pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_read                        (pipeline_bridge_PERIPHERALS_m1_read),
      .pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1   (pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_readdata                    (pipeline_bridge_PERIPHERALS_m1_readdata),
      .pipeline_bridge_PERIPHERALS_m1_readdatavalid               (pipeline_bridge_PERIPHERALS_m1_readdatavalid),
      .pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1          (pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_waitrequest                 (pipeline_bridge_PERIPHERALS_m1_waitrequest),
      .pipeline_bridge_PERIPHERALS_m1_write                       (pipeline_bridge_PERIPHERALS_m1_write),
      .pipeline_bridge_PERIPHERALS_m1_writedata                   (pipeline_bridge_PERIPHERALS_m1_writedata),
      .reset_n                                                    (ddr2_phy_clk_out_reset_n),
      .uart_0_s1_readdata_from_sa                                 (uart_0_s1_readdata_from_sa)
    );

  pipeline_bridge_PERIPHERALS the_pipeline_bridge_PERIPHERALS
    (
      .clk              (ddr2_phy_clk_out),
      .m1_address       (pipeline_bridge_PERIPHERALS_m1_address),
      .m1_burstcount    (pipeline_bridge_PERIPHERALS_m1_burstcount),
      .m1_byteenable    (pipeline_bridge_PERIPHERALS_m1_byteenable),
      .m1_chipselect    (pipeline_bridge_PERIPHERALS_m1_chipselect),
      .m1_debugaccess   (pipeline_bridge_PERIPHERALS_m1_debugaccess),
      .m1_endofpacket   (pipeline_bridge_PERIPHERALS_m1_endofpacket),
      .m1_read          (pipeline_bridge_PERIPHERALS_m1_read),
      .m1_readdata      (pipeline_bridge_PERIPHERALS_m1_readdata),
      .m1_readdatavalid (pipeline_bridge_PERIPHERALS_m1_readdatavalid),
      .m1_waitrequest   (pipeline_bridge_PERIPHERALS_m1_waitrequest),
      .m1_write         (pipeline_bridge_PERIPHERALS_m1_write),
      .m1_writedata     (pipeline_bridge_PERIPHERALS_m1_writedata),
      .reset_n          (pipeline_bridge_PERIPHERALS_s1_reset_n),
      .s1_address       (pipeline_bridge_PERIPHERALS_s1_address),
      .s1_arbiterlock   (pipeline_bridge_PERIPHERALS_s1_arbiterlock),
      .s1_arbiterlock2  (pipeline_bridge_PERIPHERALS_s1_arbiterlock2),
      .s1_burstcount    (pipeline_bridge_PERIPHERALS_s1_burstcount),
      .s1_byteenable    (pipeline_bridge_PERIPHERALS_s1_byteenable),
      .s1_chipselect    (pipeline_bridge_PERIPHERALS_s1_chipselect),
      .s1_debugaccess   (pipeline_bridge_PERIPHERALS_s1_debugaccess),
      .s1_endofpacket   (pipeline_bridge_PERIPHERALS_s1_endofpacket),
      .s1_nativeaddress (pipeline_bridge_PERIPHERALS_s1_nativeaddress),
      .s1_read          (pipeline_bridge_PERIPHERALS_s1_read),
      .s1_readdata      (pipeline_bridge_PERIPHERALS_s1_readdata),
      .s1_readdatavalid (pipeline_bridge_PERIPHERALS_s1_readdatavalid),
      .s1_waitrequest   (pipeline_bridge_PERIPHERALS_s1_waitrequest),
      .s1_write         (pipeline_bridge_PERIPHERALS_s1_write),
      .s1_writedata     (pipeline_bridge_PERIPHERALS_s1_writedata)
    );

  tiger_top_0_PROC_arbitrator the_tiger_top_0_PROC
    (
      .clk                    (ddr2_phy_clk_out),
      .data_cache_0_PROC_data (data_cache_0_PROC_data),
      .reset_n                (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_PROC_data  (tiger_top_0_PROC_data)
    );

  tiger_top_0_leapSlave_arbitrator the_tiger_top_0_leapSlave
    (
      .clk                                                               (ddr2_phy_clk_out),
      .d1_tiger_top_0_leapSlave_end_xfer                                 (d1_tiger_top_0_leapSlave_end_xfer),
      .jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave           (jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_latency_counter                         (jtag_to_ava_master_bridge_latency_counter),
      .jtag_to_ava_master_bridge_master_address_to_slave                 (jtag_to_ava_master_bridge_master_address_to_slave),
      .jtag_to_ava_master_bridge_master_read                             (jtag_to_ava_master_bridge_master_read),
      .jtag_to_ava_master_bridge_master_write                            (jtag_to_ava_master_bridge_master_write),
      .jtag_to_ava_master_bridge_master_writedata                        (jtag_to_ava_master_bridge_master_writedata),
      .jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave (jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register  (jtag_to_ava_master_bridge_read_data_valid_ddr2_s1_shift_register),
      .jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave   (jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave          (jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave),
      .reset_n                                                           (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_leapSlave_address                                     (tiger_top_0_leapSlave_address),
      .tiger_top_0_leapSlave_chipselect                                  (tiger_top_0_leapSlave_chipselect),
      .tiger_top_0_leapSlave_read                                        (tiger_top_0_leapSlave_read),
      .tiger_top_0_leapSlave_readdata                                    (tiger_top_0_leapSlave_readdata),
      .tiger_top_0_leapSlave_readdata_from_sa                            (tiger_top_0_leapSlave_readdata_from_sa),
      .tiger_top_0_leapSlave_write                                       (tiger_top_0_leapSlave_write),
      .tiger_top_0_leapSlave_writedata                                   (tiger_top_0_leapSlave_writedata)
    );

  tiger_top_0_CACHE_arbitrator the_tiger_top_0_CACHE
    (
      .clk                                                     (ddr2_phy_clk_out),
      .d1_data_cache_0_CACHE0_end_xfer                         (d1_data_cache_0_CACHE0_end_xfer),
      .data_cache_0_CACHE0_readdata_from_sa                    (data_cache_0_CACHE0_readdata_from_sa),
      .data_cache_0_CACHE0_waitrequest_from_sa                 (data_cache_0_CACHE0_waitrequest_from_sa),
      .reset_n                                                 (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_CACHE_address                               (tiger_top_0_CACHE_address),
      .tiger_top_0_CACHE_address_to_slave                      (tiger_top_0_CACHE_address_to_slave),
      .tiger_top_0_CACHE_granted_data_cache_0_CACHE0           (tiger_top_0_CACHE_granted_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0 (tiger_top_0_CACHE_qualified_request_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_read                                  (tiger_top_0_CACHE_read),
      .tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0   (tiger_top_0_CACHE_read_data_valid_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_readdata                              (tiger_top_0_CACHE_readdata),
      .tiger_top_0_CACHE_requests_data_cache_0_CACHE0          (tiger_top_0_CACHE_requests_data_cache_0_CACHE0),
      .tiger_top_0_CACHE_reset                                 (tiger_top_0_CACHE_reset),
      .tiger_top_0_CACHE_waitrequest                           (tiger_top_0_CACHE_waitrequest),
      .tiger_top_0_CACHE_write                                 (tiger_top_0_CACHE_write),
      .tiger_top_0_CACHE_writedata                             (tiger_top_0_CACHE_writedata)
    );

  tiger_top_0_instructionMaster_arbitrator the_tiger_top_0_instructionMaster
    (
      .clk                                                                                    (ddr2_phy_clk_out),
      .d1_pipeline_bridge_MEMORY_s1_end_xfer                                                  (d1_pipeline_bridge_MEMORY_s1_end_xfer),
      .pipeline_bridge_MEMORY_s1_readdata_from_sa                                             (pipeline_bridge_MEMORY_s1_readdata_from_sa),
      .pipeline_bridge_MEMORY_s1_waitrequest_from_sa                                          (pipeline_bridge_MEMORY_s1_waitrequest_from_sa),
      .reset_n                                                                                (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_instructionMaster_address                                                  (tiger_top_0_instructionMaster_address),
      .tiger_top_0_instructionMaster_address_to_slave                                         (tiger_top_0_instructionMaster_address_to_slave),
      .tiger_top_0_instructionMaster_burstcount                                               (tiger_top_0_instructionMaster_burstcount),
      .tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1                        (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1),
      .tiger_top_0_instructionMaster_latency_counter                                          (tiger_top_0_instructionMaster_latency_counter),
      .tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1              (tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1),
      .tiger_top_0_instructionMaster_read                                                     (tiger_top_0_instructionMaster_read),
      .tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1                (tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1),
      .tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register (tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .tiger_top_0_instructionMaster_readdata                                                 (tiger_top_0_instructionMaster_readdata),
      .tiger_top_0_instructionMaster_readdatavalid                                            (tiger_top_0_instructionMaster_readdatavalid),
      .tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1                       (tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1),
      .tiger_top_0_instructionMaster_waitrequest                                              (tiger_top_0_instructionMaster_waitrequest)
    );

  tiger_top_0_procMaster_arbitrator the_tiger_top_0_procMaster
    (
      .clk                                                                                  (ddr2_phy_clk_out),
      .d1_pipeline_bridge_PERIPHERALS_s1_end_xfer                                           (d1_pipeline_bridge_PERIPHERALS_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_s1_readdata_from_sa                                      (pipeline_bridge_PERIPHERALS_s1_readdata_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa                                   (pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa),
      .reset_n                                                                              (ddr2_phy_clk_out_reset_n),
      .tiger_top_0_procMaster_address                                                       (tiger_top_0_procMaster_address),
      .tiger_top_0_procMaster_address_to_slave                                              (tiger_top_0_procMaster_address_to_slave),
      .tiger_top_0_procMaster_byteenable                                                    (tiger_top_0_procMaster_byteenable),
      .tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1                        (tiger_top_0_procMaster_granted_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_latency_counter                                               (tiger_top_0_procMaster_latency_counter),
      .tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1              (tiger_top_0_procMaster_qualified_request_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_read                                                          (tiger_top_0_procMaster_read),
      .tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1                (tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register (tiger_top_0_procMaster_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register),
      .tiger_top_0_procMaster_readdata                                                      (tiger_top_0_procMaster_readdata),
      .tiger_top_0_procMaster_readdatavalid                                                 (tiger_top_0_procMaster_readdatavalid),
      .tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1                       (tiger_top_0_procMaster_requests_pipeline_bridge_PERIPHERALS_s1),
      .tiger_top_0_procMaster_waitrequest                                                   (tiger_top_0_procMaster_waitrequest),
      .tiger_top_0_procMaster_write                                                         (tiger_top_0_procMaster_write),
      .tiger_top_0_procMaster_writedata                                                     (tiger_top_0_procMaster_writedata)
    );

  tiger_top_0 the_tiger_top_0
    (
      .asi_PROC_data                            (tiger_top_0_PROC_data),
      .avm_CACHE_address                        (tiger_top_0_CACHE_address),
      .avm_CACHE_read                           (tiger_top_0_CACHE_read),
      .avm_CACHE_readdata                       (tiger_top_0_CACHE_readdata),
      .avm_CACHE_waitrequest                    (tiger_top_0_CACHE_waitrequest),
      .avm_CACHE_write                          (tiger_top_0_CACHE_write),
      .avm_CACHE_writedata                      (tiger_top_0_CACHE_writedata),
      .avm_instructionMaster_address            (tiger_top_0_instructionMaster_address),
      .avm_instructionMaster_beginbursttransfer (tiger_top_0_instructionMaster_beginbursttransfer),
      .avm_instructionMaster_burstcount         (tiger_top_0_instructionMaster_burstcount),
      .avm_instructionMaster_read               (tiger_top_0_instructionMaster_read),
      .avm_instructionMaster_readdata           (tiger_top_0_instructionMaster_readdata),
      .avm_instructionMaster_readdatavalid      (tiger_top_0_instructionMaster_readdatavalid),
      .avm_instructionMaster_waitrequest        (tiger_top_0_instructionMaster_waitrequest),
      .avm_procMaster_address                   (tiger_top_0_procMaster_address),
      .avm_procMaster_byteenable                (tiger_top_0_procMaster_byteenable),
      .avm_procMaster_read                      (tiger_top_0_procMaster_read),
      .avm_procMaster_readdata                  (tiger_top_0_procMaster_readdata),
      .avm_procMaster_readdatavalid             (tiger_top_0_procMaster_readdatavalid),
      .avm_procMaster_waitrequest               (tiger_top_0_procMaster_waitrequest),
      .avm_procMaster_write                     (tiger_top_0_procMaster_write),
      .avm_procMaster_writedata                 (tiger_top_0_procMaster_writedata),
      .avs_leapSlave_address                    (tiger_top_0_leapSlave_address),
      .avs_leapSlave_chipselect                 (tiger_top_0_leapSlave_chipselect),
      .avs_leapSlave_read                       (tiger_top_0_leapSlave_read),
      .avs_leapSlave_readdata                   (tiger_top_0_leapSlave_readdata),
      .avs_leapSlave_write                      (tiger_top_0_leapSlave_write),
      .avs_leapSlave_writedata                  (tiger_top_0_leapSlave_writedata),
      .clk                                      (ddr2_phy_clk_out),
      .coe_debug_lights                         (coe_debug_lights_from_the_tiger_top_0),
      .coe_debug_select                         (coe_debug_select_to_the_tiger_top_0),
      .coe_exe_end                              (coe_exe_end_from_the_tiger_top_0),
      .coe_exe_start                            (coe_exe_start_from_the_tiger_top_0),
      .reset                                    (tiger_top_0_CACHE_reset)
    );

  uart_0_s1_arbitrator the_uart_0_s1
    (
      .clk                                                        (ddr2_phy_clk_out),
      .d1_uart_0_s1_end_xfer                                      (d1_uart_0_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_m1_address_to_slave            (pipeline_bridge_PERIPHERALS_m1_address_to_slave),
      .pipeline_bridge_PERIPHERALS_m1_burstcount                  (pipeline_bridge_PERIPHERALS_m1_burstcount),
      .pipeline_bridge_PERIPHERALS_m1_chipselect                  (pipeline_bridge_PERIPHERALS_m1_chipselect),
      .pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1           (pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_latency_counter             (pipeline_bridge_PERIPHERALS_m1_latency_counter),
      .pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 (pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_read                        (pipeline_bridge_PERIPHERALS_m1_read),
      .pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1   (pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1          (pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_write                       (pipeline_bridge_PERIPHERALS_m1_write),
      .pipeline_bridge_PERIPHERALS_m1_writedata                   (pipeline_bridge_PERIPHERALS_m1_writedata),
      .reset_n                                                    (ddr2_phy_clk_out_reset_n),
      .uart_0_s1_address                                          (uart_0_s1_address),
      .uart_0_s1_begintransfer                                    (uart_0_s1_begintransfer),
      .uart_0_s1_chipselect                                       (uart_0_s1_chipselect),
      .uart_0_s1_dataavailable                                    (uart_0_s1_dataavailable),
      .uart_0_s1_dataavailable_from_sa                            (uart_0_s1_dataavailable_from_sa),
      .uart_0_s1_read_n                                           (uart_0_s1_read_n),
      .uart_0_s1_readdata                                         (uart_0_s1_readdata),
      .uart_0_s1_readdata_from_sa                                 (uart_0_s1_readdata_from_sa),
      .uart_0_s1_readyfordata                                     (uart_0_s1_readyfordata),
      .uart_0_s1_readyfordata_from_sa                             (uart_0_s1_readyfordata_from_sa),
      .uart_0_s1_reset_n                                          (uart_0_s1_reset_n),
      .uart_0_s1_write_n                                          (uart_0_s1_write_n),
      .uart_0_s1_writedata                                        (uart_0_s1_writedata)
    );

  uart_0 the_uart_0
    (
      .address       (uart_0_s1_address),
      .begintransfer (uart_0_s1_begintransfer),
      .chipselect    (uart_0_s1_chipselect),
      .clk           (ddr2_phy_clk_out),
      .dataavailable (uart_0_s1_dataavailable),
      .irq           (uart_0_s1_irq),
      .read_n        (uart_0_s1_read_n),
      .readdata      (uart_0_s1_readdata),
      .readyfordata  (uart_0_s1_readyfordata),
      .reset_n       (uart_0_s1_reset_n),
      .rxd           (rxd_to_the_uart_0),
      .txd           (txd_from_the_uart_0),
      .write_n       (uart_0_s1_write_n),
      .writedata     (uart_0_s1_writedata)
    );

  //reset is asserted asynchronously and deasserted synchronously
  tiger_reset_ddr2_phy_clk_out_domain_synch_module tiger_reset_ddr2_phy_clk_out_domain_synch
    (
      .clk      (ddr2_phy_clk_out),
      .data_in  (1'b1),
      .data_out (ddr2_phy_clk_out_reset_n),
      .reset_n  (reset_n_sources)
    );

  //pipeline_bridge_MEMORY_m1_endofpacket of type endofpacket does not connect to anything so wire it to default (0)
  assign pipeline_bridge_MEMORY_m1_endofpacket = 0;

  //pipeline_bridge_PERIPHERALS_m1_endofpacket of type endofpacket does not connect to anything so wire it to default (0)
  assign pipeline_bridge_PERIPHERALS_m1_endofpacket = 0;


endmodule


//synthesis translate_off



// <ALTERA_NOTE> CODE INSERTED BETWEEN HERE

// AND HERE WILL BE PRESERVED </ALTERA_NOTE>


// If user logic components use Altsync_Ram with convert_hex2ver.dll,
// set USE_convert_hex2ver in the user comments section above

// `ifdef USE_convert_hex2ver
// `else
// `define NO_PLI 1
// `endif

`include "data_cache.v"
`include "data_cache_0.v"
`include "ddr2_alt_ddrx_controller_wrapper.v"
`include "alt_ddrx_controller.v"
`include "alt_ddrx_input_if.v"
`include "alt_ddrx_state_machine.v"
`include "alt_ddrx_addr_cmd.v"
`include "alt_ddrx_afi_block.v"
`include "alt_ddrx_odt_gen.v"
`include "alt_ddrx_ecc.v"
`include "alt_ddrx_csr.v"
`include "alt_ddrx_avalon_if.v"
`include "alt_ddrx_cmd_queue.v"
`include "alt_ddrx_wdata_fifo.v"
`include "alt_ddrx_ddr2_odt_gen.v"
`include "alt_ddrx_ddr3_odt_gen.v"
`include "altera_avalon_half_rate_bridge.v"
`include "alt_ddrx_decoder.v"
`include "alt_ddrx_decoder_40.v"
`include "alt_ddrx_decoder_72.v"
`include "alt_ddrx_encoder.v"
`include "alt_ddrx_encoder_40.v"
`include "alt_ddrx_encoder_72.v"
`include "alt_ddrx_clock_and_reset.v"
`include "alt_ddrx_cmd_gen.v"
`include "alt_ddrx_bank_timer_wrapper.v"
`include "alt_ddrx_bank_timer.v"
`include "alt_ddrx_bank_timer_info.v"
`include "alt_ddrx_bypass.v"
`include "alt_ddrx_cache.v"
`include "alt_ddrx_timing_param.v"
`include "alt_ddrx_rank_monitor.v"
`include "ddr2_controller_phy.v"
`include "ddr2.v"
`include "ddr2_phy_alt_mem_phy_pll.v"
`include "ddr2_phy.v"
`include "ddr2_phy_alt_mem_phy.v"
`include "ddr2_phy_alt_mem_phy_seq_wrapper.vo"
`include "tiger_top.v"
`include "LEAP/AddressHash.v"
`include "LEAP/AddressStack.v"
`include "LEAP/CounterStack.v"
`include "LEAP/CounterStorage.v"
`include "LEAP/CountingBlock.v"
`include "LEAP/IncCounter.v"
`include "LEAP/LeapProfiler.v"
`include "LEAP/OpDecode.v"
`include "LEAP/tiger_leap_slave_handler.v"
`include "tiger_top_0.v"
`include "jtag_to_ava_master_bridge_sim/jtag_to_ava_master_bridge.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_avalon_master/jtag_to_ava_master_bridge_jtag_to_ava_master_bridge.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_avalon_st_jtag_interface.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_jtag_dc_streaming.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_jtag_sld_node.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_jtag_streaming.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_pli_streaming.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_avalon_st_clock_crosser.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_avalon_st_pipeline_base.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_avalon_st_idle_remover.v"
`include "jtag_to_ava_master_bridge_sim/altera_jtag_dc_streaming/altera_avalon_st_idle_inserter.v"
`include "jtag_to_ava_master_bridge_sim/timing_adapter/jtag_to_ava_master_bridge_timing_adt.v"
`include "jtag_to_ava_master_bridge_sim/altera_avalon_sc_fifo/altera_avalon_sc_fifo.v"
`include "jtag_to_ava_master_bridge_sim/altera_avalon_st_bytes_to_packets/altera_avalon_st_bytes_to_packets.v"
`include "jtag_to_ava_master_bridge_sim/altera_avalon_st_packets_to_bytes/altera_avalon_st_packets_to_bytes.v"
`include "jtag_to_ava_master_bridge_sim/altera_avalon_packets_to_master/altera_avalon_packets_to_master.v"
`include "jtag_to_ava_master_bridge_sim/channel_adapter/jtag_to_ava_master_bridge_b2p_adapter.v"
`include "jtag_to_ava_master_bridge_sim/channel_adapter/jtag_to_ava_master_bridge_p2b_adapter.v"
`include "ddr2_test_component.v"
`include "pipeline_bridge_MEMORY.v"
`include "pipeline_bridge_PERIPHERALS.v"
`include "uart_0.v"

`timescale 1ns / 1ps

module test_bench 
;


  wire             aux_scan_clk_from_the_ddr2;
  wire             aux_scan_clk_reset_n_from_the_ddr2;
  reg              clk;
  wire    [ 17: 0] coe_debug_lights_from_the_tiger_top_0;
  wire    [  2: 0] coe_debug_select_to_the_tiger_top_0;
  wire             coe_exe_end_from_the_tiger_top_0;
  wire             coe_exe_start_from_the_tiger_top_0;
  wire             data_cache_0_dataMaster0_beginbursttransfer;
  wire             ddr2_aux_full_rate_clk_out;
  wire             ddr2_aux_half_rate_clk_out;
  wire             ddr2_phy_clk_out;
  wire             dll_reference_clk_from_the_ddr2;
  wire    [  5: 0] dqs_delay_ctrl_export_from_the_ddr2;
  wire             global_reset_n_to_the_ddr2;
  wire             local_init_done_from_the_ddr2;
  wire             local_refresh_ack_from_the_ddr2;
  wire             local_wdata_req_from_the_ddr2;
  wire    [ 13: 0] mem_addr_from_the_ddr2;
  wire    [  2: 0] mem_ba_from_the_ddr2;
  wire             mem_cas_n_from_the_ddr2;
  wire             mem_cke_from_the_ddr2;
  wire    [  1: 0] mem_clk_n_to_and_from_the_ddr2;
  wire    [  1: 0] mem_clk_to_and_from_the_ddr2;
  wire             mem_cs_n_from_the_ddr2;
  wire    [  7: 0] mem_dm_from_the_ddr2;
  wire    [ 63: 0] mem_dq_to_and_from_the_ddr2;
  wire    [  7: 0] mem_dqs_n;
  wire    [  7: 0] mem_dqs_to_and_from_the_ddr2;
  wire    [  7: 0] mem_dqsn_to_and_from_the_ddr2;
  wire             mem_odt_from_the_ddr2;
  wire             mem_ras_n_from_the_ddr2;
  wire             mem_we_n_from_the_ddr2;
  wire    [ 13: 0] oct_ctl_rs_value_to_the_ddr2;
  wire    [ 13: 0] oct_ctl_rt_value_to_the_ddr2;
  wire             pipeline_bridge_MEMORY_m1_debugaccess;
  wire             pipeline_bridge_MEMORY_m1_endofpacket;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire             pipeline_bridge_PERIPHERALS_m1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_m1_endofpacket;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  reg              reset_n;
  wire             reset_phy_clk_n_from_the_ddr2;
  wire             rxd_to_the_uart_0;
  wire             tiger_top_0_instructionMaster_beginbursttransfer;
  wire             txd_from_the_uart_0;
  wire             uart_0_s1_dataavailable_from_sa;
  wire             uart_0_s1_irq;
  wire             uart_0_s1_readyfordata_from_sa;


// <ALTERA_NOTE> CODE INSERTED BETWEEN HERE
//  add your signals and additional architecture here
// AND HERE WILL BE PRESERVED </ALTERA_NOTE>

  //Set us up the Dut
  tiger DUT
    (
      .aux_scan_clk_from_the_ddr2            (aux_scan_clk_from_the_ddr2),
      .aux_scan_clk_reset_n_from_the_ddr2    (aux_scan_clk_reset_n_from_the_ddr2),
      .clk                                   (clk),
      .coe_debug_lights_from_the_tiger_top_0 (coe_debug_lights_from_the_tiger_top_0),
      .coe_debug_select_to_the_tiger_top_0   (coe_debug_select_to_the_tiger_top_0),
      .coe_exe_end_from_the_tiger_top_0      (coe_exe_end_from_the_tiger_top_0),
      .coe_exe_start_from_the_tiger_top_0    (coe_exe_start_from_the_tiger_top_0),
      .ddr2_aux_full_rate_clk_out            (ddr2_aux_full_rate_clk_out),
      .ddr2_aux_half_rate_clk_out            (ddr2_aux_half_rate_clk_out),
      .ddr2_phy_clk_out                      (ddr2_phy_clk_out),
      .dll_reference_clk_from_the_ddr2       (dll_reference_clk_from_the_ddr2),
      .dqs_delay_ctrl_export_from_the_ddr2   (dqs_delay_ctrl_export_from_the_ddr2),
      .global_reset_n_to_the_ddr2            (global_reset_n_to_the_ddr2),
      .local_init_done_from_the_ddr2         (local_init_done_from_the_ddr2),
      .local_refresh_ack_from_the_ddr2       (local_refresh_ack_from_the_ddr2),
      .local_wdata_req_from_the_ddr2         (local_wdata_req_from_the_ddr2),
      .mem_addr_from_the_ddr2                (mem_addr_from_the_ddr2),
      .mem_ba_from_the_ddr2                  (mem_ba_from_the_ddr2),
      .mem_cas_n_from_the_ddr2               (mem_cas_n_from_the_ddr2),
      .mem_cke_from_the_ddr2                 (mem_cke_from_the_ddr2),
      .mem_clk_n_to_and_from_the_ddr2        (mem_clk_n_to_and_from_the_ddr2),
      .mem_clk_to_and_from_the_ddr2          (mem_clk_to_and_from_the_ddr2),
      .mem_cs_n_from_the_ddr2                (mem_cs_n_from_the_ddr2),
      .mem_dm_from_the_ddr2                  (mem_dm_from_the_ddr2),
      .mem_dq_to_and_from_the_ddr2           (mem_dq_to_and_from_the_ddr2),
      .mem_dqs_to_and_from_the_ddr2          (mem_dqs_to_and_from_the_ddr2),
      .mem_dqsn_to_and_from_the_ddr2         (mem_dqsn_to_and_from_the_ddr2),
      .mem_odt_from_the_ddr2                 (mem_odt_from_the_ddr2),
      .mem_ras_n_from_the_ddr2               (mem_ras_n_from_the_ddr2),
      .mem_we_n_from_the_ddr2                (mem_we_n_from_the_ddr2),
      .oct_ctl_rs_value_to_the_ddr2          (oct_ctl_rs_value_to_the_ddr2),
      .oct_ctl_rt_value_to_the_ddr2          (oct_ctl_rt_value_to_the_ddr2),
      .reset_n                               (reset_n),
      .reset_phy_clk_n_from_the_ddr2         (reset_phy_clk_n_from_the_ddr2),
      .rxd_to_the_uart_0                     (rxd_to_the_uart_0),
      .txd_from_the_uart_0                   (txd_from_the_uart_0)
    );

  ddr2_test_component the_ddr2_test_component
    (
      .global_reset_n (global_reset_n_to_the_ddr2),
      .mem_addr       (mem_addr_from_the_ddr2),
      .mem_ba         (mem_ba_from_the_ddr2),
      .mem_cas_n      (mem_cas_n_from_the_ddr2),
      .mem_cke        (mem_cke_from_the_ddr2),
      .mem_clk        (mem_clk_to_and_from_the_ddr2),
      .mem_clk_n      (mem_clk_n_to_and_from_the_ddr2),
      .mem_cs_n       (mem_cs_n_from_the_ddr2),
      .mem_dm         (mem_dm_from_the_ddr2),
      .mem_dq         (mem_dq_to_and_from_the_ddr2),
      .mem_dqs        (mem_dqs_to_and_from_the_ddr2),
      .mem_dqs_n      (mem_dqs_n),
      .mem_odt        (mem_odt_from_the_ddr2),
      .mem_ras_n      (mem_ras_n_from_the_ddr2),
      .mem_we_n       (mem_we_n_from_the_ddr2)
    );

  initial
    clk = 1'b0;
  always
     if (clk == 1'b1) 
    #2 clk <= ~clk;
     else 
    #3 clk <= ~clk;
  
  initial 
    begin
      reset_n <= 0;
      #50 reset_n <= 1;
    end

endmodule


//synthesis translate_on
