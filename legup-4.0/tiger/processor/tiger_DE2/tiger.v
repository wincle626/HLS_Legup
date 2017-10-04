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
  output  [ 31: 0] data_cache_0_dataMaster0_readdata;
  output           data_cache_0_dataMaster0_readdatavalid;
  output           data_cache_0_dataMaster0_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_MEMORY_s1_end_xfer;
  input   [ 31: 0] data_cache_0_dataMaster0_address;
  input   [  5: 0] data_cache_0_dataMaster0_burstcount;
  input   [  3: 0] data_cache_0_dataMaster0_byteenable;
  input            data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_read;
  input            data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  input            data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster0_write;
  input   [ 31: 0] data_cache_0_dataMaster0_writedata;
  input   [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  input            pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  input            reset_n;

  reg              active_and_waiting_last_time;
  reg     [ 31: 0] data_cache_0_dataMaster0_address_last_time;
  wire    [ 31: 0] data_cache_0_dataMaster0_address_to_slave;
  reg     [  5: 0] data_cache_0_dataMaster0_burstcount_last_time;
  reg     [  3: 0] data_cache_0_dataMaster0_byteenable_last_time;
  wire             data_cache_0_dataMaster0_is_granted_some_slave;
  reg              data_cache_0_dataMaster0_latency_counter;
  reg              data_cache_0_dataMaster0_read_but_no_slave_selected;
  reg              data_cache_0_dataMaster0_read_last_time;
  wire    [ 31: 0] data_cache_0_dataMaster0_readdata;
  wire             data_cache_0_dataMaster0_readdatavalid;
  wire             data_cache_0_dataMaster0_run;
  wire             data_cache_0_dataMaster0_waitrequest;
  reg              data_cache_0_dataMaster0_write_last_time;
  reg     [ 31: 0] data_cache_0_dataMaster0_writedata_last_time;
  wire             latency_load_value;
  wire             p1_data_cache_0_dataMaster0_latency_counter;
  wire             pre_flush_data_cache_0_dataMaster0_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 | ~data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1) & (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1 | ~data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1) & ((~data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write)))) & ((~data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write))));

  //cascaded wait assignment, which is an e_assign
  assign data_cache_0_dataMaster0_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign data_cache_0_dataMaster0_address_to_slave = {8'b0,
    data_cache_0_dataMaster0_address[23 : 0]};

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

module jtag_to_ava_master_bridge_master_arbitrator (
                                                     // inputs:
                                                      clk,
                                                      d1_sdram_s1_end_xfer,
                                                      d1_tiger_top_0_leapSlave_end_xfer,
                                                      jtag_to_ava_master_bridge_byteenable_sdram_s1,
                                                      jtag_to_ava_master_bridge_granted_sdram_s1,
                                                      jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave,
                                                      jtag_to_ava_master_bridge_master_address,
                                                      jtag_to_ava_master_bridge_master_byteenable,
                                                      jtag_to_ava_master_bridge_master_read,
                                                      jtag_to_ava_master_bridge_master_write,
                                                      jtag_to_ava_master_bridge_master_writedata,
                                                      jtag_to_ava_master_bridge_qualified_request_sdram_s1,
                                                      jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave,
                                                      jtag_to_ava_master_bridge_read_data_valid_sdram_s1,
                                                      jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register,
                                                      jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave,
                                                      jtag_to_ava_master_bridge_requests_sdram_s1,
                                                      jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave,
                                                      reset_n,
                                                      sdram_s1_readdata_from_sa,
                                                      sdram_s1_waitrequest_from_sa,
                                                      tiger_top_0_leapSlave_readdata_from_sa,

                                                     // outputs:
                                                      jtag_to_ava_master_bridge_dbs_address,
                                                      jtag_to_ava_master_bridge_dbs_write_16,
                                                      jtag_to_ava_master_bridge_latency_counter,
                                                      jtag_to_ava_master_bridge_master_address_to_slave,
                                                      jtag_to_ava_master_bridge_master_readdata,
                                                      jtag_to_ava_master_bridge_master_readdatavalid,
                                                      jtag_to_ava_master_bridge_master_reset,
                                                      jtag_to_ava_master_bridge_master_waitrequest
                                                   )
;

  output  [  1: 0] jtag_to_ava_master_bridge_dbs_address;
  output  [ 15: 0] jtag_to_ava_master_bridge_dbs_write_16;
  output           jtag_to_ava_master_bridge_latency_counter;
  output  [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  output  [ 31: 0] jtag_to_ava_master_bridge_master_readdata;
  output           jtag_to_ava_master_bridge_master_readdatavalid;
  output           jtag_to_ava_master_bridge_master_reset;
  output           jtag_to_ava_master_bridge_master_waitrequest;
  input            clk;
  input            d1_sdram_s1_end_xfer;
  input            d1_tiger_top_0_leapSlave_end_xfer;
  input   [  1: 0] jtag_to_ava_master_bridge_byteenable_sdram_s1;
  input            jtag_to_ava_master_bridge_granted_sdram_s1;
  input            jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_address;
  input   [  3: 0] jtag_to_ava_master_bridge_master_byteenable;
  input            jtag_to_ava_master_bridge_master_read;
  input            jtag_to_ava_master_bridge_master_write;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_writedata;
  input            jtag_to_ava_master_bridge_qualified_request_sdram_s1;
  input            jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave;
  input            jtag_to_ava_master_bridge_read_data_valid_sdram_s1;
  input            jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register;
  input            jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;
  input            jtag_to_ava_master_bridge_requests_sdram_s1;
  input            jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;
  input            reset_n;
  input   [ 15: 0] sdram_s1_readdata_from_sa;
  input            sdram_s1_waitrequest_from_sa;
  input   [ 31: 0] tiger_top_0_leapSlave_readdata_from_sa;

  reg              active_and_waiting_last_time;
  wire             dbs_count_enable;
  wire             dbs_counter_overflow;
  reg     [ 15: 0] dbs_latent_16_reg_segment_0;
  wire             dbs_rdv_count_enable;
  wire             dbs_rdv_counter_overflow;
  reg     [  1: 0] jtag_to_ava_master_bridge_dbs_address;
  reg     [  1: 0] jtag_to_ava_master_bridge_dbs_rdv_counter;
  wire    [ 15: 0] jtag_to_ava_master_bridge_dbs_write_16;
  reg              jtag_to_ava_master_bridge_latency_counter;
  reg     [ 31: 0] jtag_to_ava_master_bridge_master_address_last_time;
  wire    [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  reg     [  3: 0] jtag_to_ava_master_bridge_master_byteenable_last_time;
  wire    [  1: 0] jtag_to_ava_master_bridge_master_dbs_increment;
  wire    [  1: 0] jtag_to_ava_master_bridge_master_dbs_rdv_counter_inc;
  wire             jtag_to_ava_master_bridge_master_is_granted_some_slave;
  wire    [  1: 0] jtag_to_ava_master_bridge_master_next_dbs_rdv_counter;
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
  wire    [  1: 0] next_dbs_address;
  wire    [ 15: 0] p1_dbs_latent_16_reg_segment_0;
  wire             p1_jtag_to_ava_master_bridge_latency_counter;
  wire             pre_dbs_count_enable;
  wire             pre_flush_jtag_to_ava_master_bridge_master_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (jtag_to_ava_master_bridge_qualified_request_sdram_s1 | (jtag_to_ava_master_bridge_master_write & !jtag_to_ava_master_bridge_byteenable_sdram_s1 & jtag_to_ava_master_bridge_dbs_address[1]) | ~jtag_to_ava_master_bridge_requests_sdram_s1) & (jtag_to_ava_master_bridge_granted_sdram_s1 | ~jtag_to_ava_master_bridge_qualified_request_sdram_s1) & ((~jtag_to_ava_master_bridge_qualified_request_sdram_s1 | ~jtag_to_ava_master_bridge_master_read | (1 & ~sdram_s1_waitrequest_from_sa & (jtag_to_ava_master_bridge_dbs_address[1]) & jtag_to_ava_master_bridge_master_read))) & ((~jtag_to_ava_master_bridge_qualified_request_sdram_s1 | ~jtag_to_ava_master_bridge_master_write | (1 & ~sdram_s1_waitrequest_from_sa & (jtag_to_ava_master_bridge_dbs_address[1]) & jtag_to_ava_master_bridge_master_write))) & 1 & (jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave | ~jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave) & ((~jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave | ~jtag_to_ava_master_bridge_master_read | (1 & ~d1_tiger_top_0_leapSlave_end_xfer & jtag_to_ava_master_bridge_master_read))) & ((~jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave | ~jtag_to_ava_master_bridge_master_write | (1 & jtag_to_ava_master_bridge_master_write)));

  //cascaded wait assignment, which is an e_assign
  assign jtag_to_ava_master_bridge_master_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign jtag_to_ava_master_bridge_master_address_to_slave = {6'b0,
    jtag_to_ava_master_bridge_master_address[25],
    1'b0,
    jtag_to_ava_master_bridge_master_address[23 : 0]};

  //pre dbs count enable, which is an e_mux
  assign pre_dbs_count_enable = (((~0) & jtag_to_ava_master_bridge_requests_sdram_s1 & jtag_to_ava_master_bridge_master_write & !jtag_to_ava_master_bridge_byteenable_sdram_s1)) |
    (jtag_to_ava_master_bridge_granted_sdram_s1 & jtag_to_ava_master_bridge_master_read & 1 & 1 & ~sdram_s1_waitrequest_from_sa) |
    (jtag_to_ava_master_bridge_granted_sdram_s1 & jtag_to_ava_master_bridge_master_write & 1 & 1 & ~sdram_s1_waitrequest_from_sa);

  //jtag_to_ava_master_bridge_master_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_master_read_but_no_slave_selected <= 0;
      else 
        jtag_to_ava_master_bridge_master_read_but_no_slave_selected <= jtag_to_ava_master_bridge_master_read & jtag_to_ava_master_bridge_master_run & ~jtag_to_ava_master_bridge_master_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign jtag_to_ava_master_bridge_master_is_granted_some_slave = jtag_to_ava_master_bridge_granted_sdram_s1 |
    jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_jtag_to_ava_master_bridge_master_readdatavalid = jtag_to_ava_master_bridge_read_data_valid_sdram_s1 & dbs_rdv_counter_overflow;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign jtag_to_ava_master_bridge_master_readdatavalid = jtag_to_ava_master_bridge_master_read_but_no_slave_selected |
    pre_flush_jtag_to_ava_master_bridge_master_readdatavalid |
    jtag_to_ava_master_bridge_master_read_but_no_slave_selected |
    pre_flush_jtag_to_ava_master_bridge_master_readdatavalid |
    jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;

  //input to latent dbs-16 stored 0, which is an e_mux
  assign p1_dbs_latent_16_reg_segment_0 = sdram_s1_readdata_from_sa;

  //dbs register for latent dbs-16 segment 0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          dbs_latent_16_reg_segment_0 <= 0;
      else if (dbs_rdv_count_enable & ((jtag_to_ava_master_bridge_dbs_rdv_counter[1]) == 0))
          dbs_latent_16_reg_segment_0 <= p1_dbs_latent_16_reg_segment_0;
    end


  //jtag_to_ava_master_bridge/master readdata mux, which is an e_mux
  assign jtag_to_ava_master_bridge_master_readdata = ({32 {~jtag_to_ava_master_bridge_read_data_valid_sdram_s1}} | {sdram_s1_readdata_from_sa[15 : 0],
    dbs_latent_16_reg_segment_0}) &
    ({32 {~(jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave & jtag_to_ava_master_bridge_master_read)}} | tiger_top_0_leapSlave_readdata_from_sa);

  //mux write dbs 1, which is an e_mux
  assign jtag_to_ava_master_bridge_dbs_write_16 = (jtag_to_ava_master_bridge_dbs_address[1])? jtag_to_ava_master_bridge_master_writedata[31 : 16] :
    jtag_to_ava_master_bridge_master_writedata[15 : 0];

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

  //dbs count increment, which is an e_mux
  assign jtag_to_ava_master_bridge_master_dbs_increment = (jtag_to_ava_master_bridge_requests_sdram_s1)? 2 :
    0;

  //dbs counter overflow, which is an e_assign
  assign dbs_counter_overflow = jtag_to_ava_master_bridge_dbs_address[1] & !(next_dbs_address[1]);

  //next master address, which is an e_assign
  assign next_dbs_address = jtag_to_ava_master_bridge_dbs_address + jtag_to_ava_master_bridge_master_dbs_increment;

  //dbs count enable, which is an e_mux
  assign dbs_count_enable = pre_dbs_count_enable;

  //dbs counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_dbs_address <= 0;
      else if (dbs_count_enable)
          jtag_to_ava_master_bridge_dbs_address <= next_dbs_address;
    end


  //p1 dbs rdv counter, which is an e_assign
  assign jtag_to_ava_master_bridge_master_next_dbs_rdv_counter = jtag_to_ava_master_bridge_dbs_rdv_counter + jtag_to_ava_master_bridge_master_dbs_rdv_counter_inc;

  //jtag_to_ava_master_bridge_master_rdv_inc_mux, which is an e_mux
  assign jtag_to_ava_master_bridge_master_dbs_rdv_counter_inc = 2;

  //master any slave rdv, which is an e_mux
  assign dbs_rdv_count_enable = jtag_to_ava_master_bridge_read_data_valid_sdram_s1;

  //dbs rdv counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          jtag_to_ava_master_bridge_dbs_rdv_counter <= 0;
      else if (dbs_rdv_count_enable)
          jtag_to_ava_master_bridge_dbs_rdv_counter <= jtag_to_ava_master_bridge_master_next_dbs_rdv_counter;
    end


  //dbs rdv counter overflow, which is an e_assign
  assign dbs_rdv_counter_overflow = jtag_to_ava_master_bridge_dbs_rdv_counter[1] & ~jtag_to_ava_master_bridge_master_next_dbs_rdv_counter[1];


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
  wire             full_10;
  reg              full_2;
  reg              full_3;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  4: 0] how_many_ones;
  wire    [  4: 0] one_count_minus_one;
  wire    [  4: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  5: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  5: 0] p1_stage_1;
  wire             p2_full_2;
  wire    [  5: 0] p2_stage_2;
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
  reg     [  5: 0] stage_2;
  reg     [  5: 0] stage_3;
  reg     [  5: 0] stage_4;
  reg     [  5: 0] stage_5;
  reg     [  5: 0] stage_6;
  reg     [  5: 0] stage_7;
  reg     [  5: 0] stage_8;
  reg     [  5: 0] stage_9;
  wire    [  4: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_9;
  assign empty = !full_0;
  assign full_10 = 0;
  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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
  wire             full_10;
  reg              full_2;
  reg              full_3;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  4: 0] how_many_ones;
  wire    [  4: 0] one_count_minus_one;
  wire    [  4: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p2_full_2;
  wire             p2_stage_2;
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
  reg              stage_2;
  reg              stage_3;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  reg              stage_7;
  reg              stage_8;
  reg              stage_9;
  wire    [  4: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_9;
  assign empty = !full_0;
  assign full_10 = 0;
  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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
  wire             full_10;
  reg              full_2;
  reg              full_3;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  reg              full_9;
  reg     [  4: 0] how_many_ones;
  wire    [  4: 0] one_count_minus_one;
  wire    [  4: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p2_full_2;
  wire             p2_stage_2;
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
  reg              stage_2;
  reg              stage_3;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  reg              stage_7;
  reg              stage_8;
  reg              stage_9;
  wire    [  4: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_9;
  assign empty = !full_0;
  assign full_10 = 0;
  //data_9, which is an e_mux
  assign p9_stage_9 = ((full_10 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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
  output  [ 21: 0] pipeline_bridge_MEMORY_s1_address;
  output           pipeline_bridge_MEMORY_s1_arbiterlock;
  output           pipeline_bridge_MEMORY_s1_arbiterlock2;
  output  [  6: 0] pipeline_bridge_MEMORY_s1_burstcount;
  output  [  3: 0] pipeline_bridge_MEMORY_s1_byteenable;
  output           pipeline_bridge_MEMORY_s1_chipselect;
  output           pipeline_bridge_MEMORY_s1_debugaccess;
  output           pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  output  [ 21: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  output           pipeline_bridge_MEMORY_s1_read;
  output  [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  output           pipeline_bridge_MEMORY_s1_reset_n;
  output           pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  output           pipeline_bridge_MEMORY_s1_write;
  output  [ 31: 0] pipeline_bridge_MEMORY_s1_writedata;
  output           tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;
  output           tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  output           tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  output           tiger_top_0_instructionMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  output           tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1;
  input            clk;
  input   [ 31: 0] data_cache_0_dataMaster0_address_to_slave;
  input   [  5: 0] data_cache_0_dataMaster0_burstcount;
  input   [  3: 0] data_cache_0_dataMaster0_byteenable;
  input            data_cache_0_dataMaster0_latency_counter;
  input            data_cache_0_dataMaster0_read;
  input            data_cache_0_dataMaster0_write;
  input   [ 31: 0] data_cache_0_dataMaster0_writedata;
  input            pipeline_bridge_MEMORY_s1_endofpacket;
  input   [ 31: 0] pipeline_bridge_MEMORY_s1_readdata;
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
  wire    [ 21: 0] pipeline_bridge_MEMORY_s1_address;
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
  wire    [  3: 0] pipeline_bridge_MEMORY_s1_byteenable;
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
  wire    [ 21: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_next_bbt_burstcount;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_next_burst_count;
  wire             pipeline_bridge_MEMORY_s1_non_bursting_master_requests;
  wire             pipeline_bridge_MEMORY_s1_read;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
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
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_writedata;
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

  assign data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1 = ({data_cache_0_dataMaster0_address_to_slave[31 : 24] , 24'b0} == 32'h0) & (data_cache_0_dataMaster0_read | data_cache_0_dataMaster0_write);
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

  assign tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1 = (({tiger_top_0_instructionMaster_address_to_slave[31 : 24] , 24'b0} == 32'h0) & (tiger_top_0_instructionMaster_read)) & tiger_top_0_instructionMaster_read;
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
  assign pipeline_bridge_MEMORY_s1_address = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? (shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_dataMaster0 >> 2) :
    (shifted_address_to_pipeline_bridge_MEMORY_s1_from_tiger_top_0_instructionMaster >> 2);

  assign shifted_address_to_pipeline_bridge_MEMORY_s1_from_tiger_top_0_instructionMaster = tiger_top_0_instructionMaster_address_to_slave;
  //slaveid pipeline_bridge_MEMORY_s1_nativeaddress nativeaddress mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_nativeaddress = (data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1)? (data_cache_0_dataMaster0_address_to_slave >> 2) :
    (tiger_top_0_instructionMaster_address_to_slave >> 2);

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
                                               d1_tiger_burst_0_upstream_end_xfer,
                                               pipeline_bridge_MEMORY_m1_address,
                                               pipeline_bridge_MEMORY_m1_burstcount,
                                               pipeline_bridge_MEMORY_m1_byteenable,
                                               pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_chipselect,
                                               pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_read,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register,
                                               pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_write,
                                               pipeline_bridge_MEMORY_m1_writedata,
                                               reset_n,
                                               tiger_burst_0_upstream_readdata_from_sa,
                                               tiger_burst_0_upstream_waitrequest_from_sa,

                                              // outputs:
                                               pipeline_bridge_MEMORY_m1_address_to_slave,
                                               pipeline_bridge_MEMORY_m1_dbs_address,
                                               pipeline_bridge_MEMORY_m1_dbs_write_16,
                                               pipeline_bridge_MEMORY_m1_latency_counter,
                                               pipeline_bridge_MEMORY_m1_readdata,
                                               pipeline_bridge_MEMORY_m1_readdatavalid,
                                               pipeline_bridge_MEMORY_m1_waitrequest
                                            )
;

  output  [ 23: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  output  [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  output  [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  output           pipeline_bridge_MEMORY_m1_latency_counter;
  output  [ 31: 0] pipeline_bridge_MEMORY_m1_readdata;
  output           pipeline_bridge_MEMORY_m1_readdatavalid;
  output           pipeline_bridge_MEMORY_m1_waitrequest;
  input            clk;
  input            d1_tiger_burst_0_upstream_end_xfer;
  input   [ 23: 0] pipeline_bridge_MEMORY_m1_address;
  input   [  6: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input   [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input            pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_write;
  input   [ 31: 0] pipeline_bridge_MEMORY_m1_writedata;
  input            reset_n;
  input   [ 15: 0] tiger_burst_0_upstream_readdata_from_sa;
  input            tiger_burst_0_upstream_waitrequest_from_sa;

  reg              active_and_waiting_last_time;
  wire             dbs_count_enable;
  wire             dbs_counter_overflow;
  reg     [ 15: 0] dbs_latent_16_reg_segment_0;
  wire             dbs_rdv_count_enable;
  wire             dbs_rdv_counter_overflow;
  wire    [  1: 0] next_dbs_address;
  wire    [ 15: 0] p1_dbs_latent_16_reg_segment_0;
  reg     [ 23: 0] pipeline_bridge_MEMORY_m1_address_last_time;
  wire    [ 23: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  reg     [  6: 0] pipeline_bridge_MEMORY_m1_burstcount_last_time;
  reg     [  3: 0] pipeline_bridge_MEMORY_m1_byteenable_last_time;
  reg              pipeline_bridge_MEMORY_m1_chipselect_last_time;
  reg     [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_dbs_increment;
  reg     [  1: 0] pipeline_bridge_MEMORY_m1_dbs_rdv_counter;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_dbs_rdv_counter_inc;
  wire    [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  wire             pipeline_bridge_MEMORY_m1_latency_counter;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_next_dbs_rdv_counter;
  reg              pipeline_bridge_MEMORY_m1_read_last_time;
  wire    [ 31: 0] pipeline_bridge_MEMORY_m1_readdata;
  wire             pipeline_bridge_MEMORY_m1_readdatavalid;
  wire             pipeline_bridge_MEMORY_m1_run;
  wire             pipeline_bridge_MEMORY_m1_waitrequest;
  reg              pipeline_bridge_MEMORY_m1_write_last_time;
  reg     [ 31: 0] pipeline_bridge_MEMORY_m1_writedata_last_time;
  wire             pre_dbs_count_enable;
  wire             pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream | ~pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream | ~(pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) | (1 & ~tiger_burst_0_upstream_waitrequest_from_sa & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect)))) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream | ~(pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect) | (1 & ~tiger_burst_0_upstream_waitrequest_from_sa & (pipeline_bridge_MEMORY_m1_dbs_address[1]) & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect))));

  //cascaded wait assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign pipeline_bridge_MEMORY_m1_address_to_slave = {1'b1,
    pipeline_bridge_MEMORY_m1_address[22 : 0]};

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid = pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream & dbs_rdv_counter_overflow;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_readdatavalid = 0 |
    pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid;

  //input to latent dbs-16 stored 0, which is an e_mux
  assign p1_dbs_latent_16_reg_segment_0 = tiger_burst_0_upstream_readdata_from_sa;

  //dbs register for latent dbs-16 segment 0, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          dbs_latent_16_reg_segment_0 <= 0;
      else if (dbs_rdv_count_enable & ((pipeline_bridge_MEMORY_m1_dbs_rdv_counter[1]) == 0))
          dbs_latent_16_reg_segment_0 <= p1_dbs_latent_16_reg_segment_0;
    end


  //pipeline_bridge_MEMORY/m1 readdata mux, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_readdata = {tiger_burst_0_upstream_readdata_from_sa[15 : 0],
    dbs_latent_16_reg_segment_0};

  //mux write dbs 1, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_dbs_write_16 = (pipeline_bridge_MEMORY_m1_dbs_address[1])? pipeline_bridge_MEMORY_m1_writedata[31 : 16] :
    pipeline_bridge_MEMORY_m1_writedata[15 : 0];

  //actual waitrequest port, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_waitrequest = ~pipeline_bridge_MEMORY_m1_run;

  //latent max counter, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_latency_counter = 0;

  //dbs count increment, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_dbs_increment = (pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream)? 2 :
    0;

  //dbs counter overflow, which is an e_assign
  assign dbs_counter_overflow = pipeline_bridge_MEMORY_m1_dbs_address[1] & !(next_dbs_address[1]);

  //next master address, which is an e_assign
  assign next_dbs_address = pipeline_bridge_MEMORY_m1_dbs_address + pipeline_bridge_MEMORY_m1_dbs_increment;

  //dbs count enable, which is an e_mux
  assign dbs_count_enable = pre_dbs_count_enable;

  //dbs counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_dbs_address <= 0;
      else if (dbs_count_enable)
          pipeline_bridge_MEMORY_m1_dbs_address <= next_dbs_address;
    end


  //p1 dbs rdv counter, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_next_dbs_rdv_counter = pipeline_bridge_MEMORY_m1_dbs_rdv_counter + pipeline_bridge_MEMORY_m1_dbs_rdv_counter_inc;

  //pipeline_bridge_MEMORY_m1_rdv_inc_mux, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_dbs_rdv_counter_inc = 2;

  //master any slave rdv, which is an e_mux
  assign dbs_rdv_count_enable = pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream;

  //dbs rdv counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_dbs_rdv_counter <= 0;
      else if (dbs_rdv_count_enable)
          pipeline_bridge_MEMORY_m1_dbs_rdv_counter <= pipeline_bridge_MEMORY_m1_next_dbs_rdv_counter;
    end


  //dbs rdv counter overflow, which is an e_assign
  assign dbs_rdv_counter_overflow = pipeline_bridge_MEMORY_m1_dbs_rdv_counter[1] & ~pipeline_bridge_MEMORY_m1_next_dbs_rdv_counter[1];

  //pre dbs count enable, which is an e_mux
  assign pre_dbs_count_enable = ((pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & 0 & 1 & ~tiger_burst_0_upstream_waitrequest_from_sa)) |
    ((pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect) & 1 & 1 & ~tiger_burst_0_upstream_waitrequest_from_sa));


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

module rdv_fifo_for_jtag_to_ava_master_bridge_master_to_sdram_s1_module (
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
  reg              full_2;
  reg              full_3;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  wire             full_7;
  reg     [  3: 0] how_many_ones;
  wire    [  3: 0] one_count_minus_one;
  wire    [  3: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p2_full_2;
  wire             p2_stage_2;
  wire             p3_full_3;
  wire             p3_stage_3;
  wire             p4_full_4;
  wire             p4_stage_4;
  wire             p5_full_5;
  wire             p5_stage_5;
  wire             p6_full_6;
  wire             p6_stage_6;
  reg              stage_0;
  reg              stage_1;
  reg              stage_2;
  reg              stage_3;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  wire    [  3: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_6;
  assign empty = !full_0;
  assign full_7 = 0;
  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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

module rdv_fifo_for_tiger_burst_0_downstream_to_sdram_s1_module (
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
  reg              full_2;
  reg              full_3;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  wire             full_7;
  reg     [  3: 0] how_many_ones;
  wire    [  3: 0] one_count_minus_one;
  wire    [  3: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p2_full_2;
  wire             p2_stage_2;
  wire             p3_full_3;
  wire             p3_stage_3;
  wire             p4_full_4;
  wire             p4_stage_4;
  wire             p5_full_5;
  wire             p5_stage_5;
  wire             p6_full_6;
  wire             p6_stage_6;
  reg              stage_0;
  reg              stage_1;
  reg              stage_2;
  reg              stage_3;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  wire    [  3: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_6;
  assign empty = !full_0;
  assign full_7 = 0;
  //data_6, which is an e_mux
  assign p6_stage_6 = ((full_7 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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

module sdram_s1_arbitrator (
                             // inputs:
                              clk,
                              jtag_to_ava_master_bridge_dbs_address,
                              jtag_to_ava_master_bridge_dbs_write_16,
                              jtag_to_ava_master_bridge_latency_counter,
                              jtag_to_ava_master_bridge_master_address_to_slave,
                              jtag_to_ava_master_bridge_master_byteenable,
                              jtag_to_ava_master_bridge_master_read,
                              jtag_to_ava_master_bridge_master_write,
                              reset_n,
                              sdram_s1_readdata,
                              sdram_s1_readdatavalid,
                              sdram_s1_waitrequest,
                              tiger_burst_0_downstream_address_to_slave,
                              tiger_burst_0_downstream_arbitrationshare,
                              tiger_burst_0_downstream_burstcount,
                              tiger_burst_0_downstream_byteenable,
                              tiger_burst_0_downstream_latency_counter,
                              tiger_burst_0_downstream_read,
                              tiger_burst_0_downstream_write,
                              tiger_burst_0_downstream_writedata,

                             // outputs:
                              d1_sdram_s1_end_xfer,
                              jtag_to_ava_master_bridge_byteenable_sdram_s1,
                              jtag_to_ava_master_bridge_granted_sdram_s1,
                              jtag_to_ava_master_bridge_qualified_request_sdram_s1,
                              jtag_to_ava_master_bridge_read_data_valid_sdram_s1,
                              jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register,
                              jtag_to_ava_master_bridge_requests_sdram_s1,
                              sdram_s1_address,
                              sdram_s1_byteenable_n,
                              sdram_s1_chipselect,
                              sdram_s1_read_n,
                              sdram_s1_readdata_from_sa,
                              sdram_s1_reset_n,
                              sdram_s1_waitrequest_from_sa,
                              sdram_s1_write_n,
                              sdram_s1_writedata,
                              tiger_burst_0_downstream_granted_sdram_s1,
                              tiger_burst_0_downstream_qualified_request_sdram_s1,
                              tiger_burst_0_downstream_read_data_valid_sdram_s1,
                              tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register,
                              tiger_burst_0_downstream_requests_sdram_s1
                           )
;

  output           d1_sdram_s1_end_xfer;
  output  [  1: 0] jtag_to_ava_master_bridge_byteenable_sdram_s1;
  output           jtag_to_ava_master_bridge_granted_sdram_s1;
  output           jtag_to_ava_master_bridge_qualified_request_sdram_s1;
  output           jtag_to_ava_master_bridge_read_data_valid_sdram_s1;
  output           jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register;
  output           jtag_to_ava_master_bridge_requests_sdram_s1;
  output  [ 21: 0] sdram_s1_address;
  output  [  1: 0] sdram_s1_byteenable_n;
  output           sdram_s1_chipselect;
  output           sdram_s1_read_n;
  output  [ 15: 0] sdram_s1_readdata_from_sa;
  output           sdram_s1_reset_n;
  output           sdram_s1_waitrequest_from_sa;
  output           sdram_s1_write_n;
  output  [ 15: 0] sdram_s1_writedata;
  output           tiger_burst_0_downstream_granted_sdram_s1;
  output           tiger_burst_0_downstream_qualified_request_sdram_s1;
  output           tiger_burst_0_downstream_read_data_valid_sdram_s1;
  output           tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register;
  output           tiger_burst_0_downstream_requests_sdram_s1;
  input            clk;
  input   [  1: 0] jtag_to_ava_master_bridge_dbs_address;
  input   [ 15: 0] jtag_to_ava_master_bridge_dbs_write_16;
  input            jtag_to_ava_master_bridge_latency_counter;
  input   [ 31: 0] jtag_to_ava_master_bridge_master_address_to_slave;
  input   [  3: 0] jtag_to_ava_master_bridge_master_byteenable;
  input            jtag_to_ava_master_bridge_master_read;
  input            jtag_to_ava_master_bridge_master_write;
  input            reset_n;
  input   [ 15: 0] sdram_s1_readdata;
  input            sdram_s1_readdatavalid;
  input            sdram_s1_waitrequest;
  input   [ 22: 0] tiger_burst_0_downstream_address_to_slave;
  input   [  7: 0] tiger_burst_0_downstream_arbitrationshare;
  input            tiger_burst_0_downstream_burstcount;
  input   [  1: 0] tiger_burst_0_downstream_byteenable;
  input            tiger_burst_0_downstream_latency_counter;
  input            tiger_burst_0_downstream_read;
  input            tiger_burst_0_downstream_write;
  input   [ 15: 0] tiger_burst_0_downstream_writedata;

  reg              d1_reasons_to_wait;
  reg              d1_sdram_s1_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_sdram_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire    [  1: 0] jtag_to_ava_master_bridge_byteenable_sdram_s1;
  wire    [  1: 0] jtag_to_ava_master_bridge_byteenable_sdram_s1_segment_0;
  wire    [  1: 0] jtag_to_ava_master_bridge_byteenable_sdram_s1_segment_1;
  wire             jtag_to_ava_master_bridge_granted_sdram_s1;
  wire             jtag_to_ava_master_bridge_master_arbiterlock;
  wire             jtag_to_ava_master_bridge_master_arbiterlock2;
  wire             jtag_to_ava_master_bridge_master_continuerequest;
  wire             jtag_to_ava_master_bridge_qualified_request_sdram_s1;
  wire             jtag_to_ava_master_bridge_rdv_fifo_empty_sdram_s1;
  wire             jtag_to_ava_master_bridge_rdv_fifo_output_from_sdram_s1;
  wire             jtag_to_ava_master_bridge_read_data_valid_sdram_s1;
  wire             jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register;
  wire             jtag_to_ava_master_bridge_requests_sdram_s1;
  wire             jtag_to_ava_master_bridge_saved_grant_sdram_s1;
  reg              last_cycle_jtag_to_ava_master_bridge_master_granted_slave_sdram_s1;
  reg              last_cycle_tiger_burst_0_downstream_granted_slave_sdram_s1;
  wire    [ 21: 0] sdram_s1_address;
  wire             sdram_s1_allgrants;
  wire             sdram_s1_allow_new_arb_cycle;
  wire             sdram_s1_any_bursting_master_saved_grant;
  wire             sdram_s1_any_continuerequest;
  reg     [  1: 0] sdram_s1_arb_addend;
  wire             sdram_s1_arb_counter_enable;
  reg     [  7: 0] sdram_s1_arb_share_counter;
  wire    [  7: 0] sdram_s1_arb_share_counter_next_value;
  wire    [  7: 0] sdram_s1_arb_share_set_values;
  wire    [  1: 0] sdram_s1_arb_winner;
  wire             sdram_s1_arbitration_holdoff_internal;
  wire             sdram_s1_beginbursttransfer_internal;
  wire             sdram_s1_begins_xfer;
  wire    [  1: 0] sdram_s1_byteenable_n;
  wire             sdram_s1_chipselect;
  wire    [  3: 0] sdram_s1_chosen_master_double_vector;
  wire    [  1: 0] sdram_s1_chosen_master_rot_left;
  wire             sdram_s1_end_xfer;
  wire             sdram_s1_firsttransfer;
  wire    [  1: 0] sdram_s1_grant_vector;
  wire             sdram_s1_in_a_read_cycle;
  wire             sdram_s1_in_a_write_cycle;
  wire    [  1: 0] sdram_s1_master_qreq_vector;
  wire             sdram_s1_move_on_to_next_transaction;
  wire             sdram_s1_non_bursting_master_requests;
  wire             sdram_s1_read_n;
  wire    [ 15: 0] sdram_s1_readdata_from_sa;
  wire             sdram_s1_readdatavalid_from_sa;
  reg              sdram_s1_reg_firsttransfer;
  wire             sdram_s1_reset_n;
  reg     [  1: 0] sdram_s1_saved_chosen_master_vector;
  reg              sdram_s1_slavearbiterlockenable;
  wire             sdram_s1_slavearbiterlockenable2;
  wire             sdram_s1_unreg_firsttransfer;
  wire             sdram_s1_waitrequest_from_sa;
  wire             sdram_s1_waits_for_read;
  wire             sdram_s1_waits_for_write;
  wire             sdram_s1_write_n;
  wire    [ 15: 0] sdram_s1_writedata;
  wire    [ 31: 0] shifted_address_to_sdram_s1_from_jtag_to_ava_master_bridge_master;
  wire    [ 22: 0] shifted_address_to_sdram_s1_from_tiger_burst_0_downstream;
  wire             tiger_burst_0_downstream_arbiterlock;
  wire             tiger_burst_0_downstream_arbiterlock2;
  wire             tiger_burst_0_downstream_continuerequest;
  wire             tiger_burst_0_downstream_granted_sdram_s1;
  wire             tiger_burst_0_downstream_qualified_request_sdram_s1;
  wire             tiger_burst_0_downstream_rdv_fifo_empty_sdram_s1;
  wire             tiger_burst_0_downstream_rdv_fifo_output_from_sdram_s1;
  wire             tiger_burst_0_downstream_read_data_valid_sdram_s1;
  wire             tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register;
  wire             tiger_burst_0_downstream_requests_sdram_s1;
  wire             tiger_burst_0_downstream_saved_grant_sdram_s1;
  wire             wait_for_sdram_s1_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~sdram_s1_end_xfer;
    end


  assign sdram_s1_begins_xfer = ~d1_reasons_to_wait & ((jtag_to_ava_master_bridge_qualified_request_sdram_s1 | tiger_burst_0_downstream_qualified_request_sdram_s1));
  //assign sdram_s1_readdata_from_sa = sdram_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign sdram_s1_readdata_from_sa = sdram_s1_readdata;

  assign jtag_to_ava_master_bridge_requests_sdram_s1 = ({jtag_to_ava_master_bridge_master_address_to_slave[31 : 23] , 23'b0} == 32'h800000) & (jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write);
  //assign sdram_s1_waitrequest_from_sa = sdram_s1_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign sdram_s1_waitrequest_from_sa = sdram_s1_waitrequest;

  //assign sdram_s1_readdatavalid_from_sa = sdram_s1_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign sdram_s1_readdatavalid_from_sa = sdram_s1_readdatavalid;

  //sdram_s1_arb_share_counter set values, which is an e_mux
  assign sdram_s1_arb_share_set_values = (jtag_to_ava_master_bridge_granted_sdram_s1)? 2 :
    (tiger_burst_0_downstream_granted_sdram_s1)? tiger_burst_0_downstream_arbitrationshare :
    (jtag_to_ava_master_bridge_granted_sdram_s1)? 2 :
    (tiger_burst_0_downstream_granted_sdram_s1)? tiger_burst_0_downstream_arbitrationshare :
    1;

  //sdram_s1_non_bursting_master_requests mux, which is an e_mux
  assign sdram_s1_non_bursting_master_requests = jtag_to_ava_master_bridge_requests_sdram_s1 |
    jtag_to_ava_master_bridge_requests_sdram_s1;

  //sdram_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign sdram_s1_any_bursting_master_saved_grant = 0 |
    tiger_burst_0_downstream_saved_grant_sdram_s1 |
    tiger_burst_0_downstream_saved_grant_sdram_s1;

  //sdram_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign sdram_s1_arb_share_counter_next_value = sdram_s1_firsttransfer ? (sdram_s1_arb_share_set_values - 1) : |sdram_s1_arb_share_counter ? (sdram_s1_arb_share_counter - 1) : 0;

  //sdram_s1_allgrants all slave grants, which is an e_mux
  assign sdram_s1_allgrants = (|sdram_s1_grant_vector) |
    (|sdram_s1_grant_vector) |
    (|sdram_s1_grant_vector) |
    (|sdram_s1_grant_vector);

  //sdram_s1_end_xfer assignment, which is an e_assign
  assign sdram_s1_end_xfer = ~(sdram_s1_waits_for_read | sdram_s1_waits_for_write);

  //end_xfer_arb_share_counter_term_sdram_s1 arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_sdram_s1 = sdram_s1_end_xfer & (~sdram_s1_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //sdram_s1_arb_share_counter arbitration counter enable, which is an e_assign
  assign sdram_s1_arb_counter_enable = (end_xfer_arb_share_counter_term_sdram_s1 & sdram_s1_allgrants) | (end_xfer_arb_share_counter_term_sdram_s1 & ~sdram_s1_non_bursting_master_requests);

  //sdram_s1_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          sdram_s1_arb_share_counter <= 0;
      else if (sdram_s1_arb_counter_enable)
          sdram_s1_arb_share_counter <= sdram_s1_arb_share_counter_next_value;
    end


  //sdram_s1_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          sdram_s1_slavearbiterlockenable <= 0;
      else if ((|sdram_s1_master_qreq_vector & end_xfer_arb_share_counter_term_sdram_s1) | (end_xfer_arb_share_counter_term_sdram_s1 & ~sdram_s1_non_bursting_master_requests))
          sdram_s1_slavearbiterlockenable <= |sdram_s1_arb_share_counter_next_value;
    end


  //jtag_to_ava_master_bridge/master sdram/s1 arbiterlock, which is an e_assign
  assign jtag_to_ava_master_bridge_master_arbiterlock = sdram_s1_slavearbiterlockenable & jtag_to_ava_master_bridge_master_continuerequest;

  //sdram_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign sdram_s1_slavearbiterlockenable2 = |sdram_s1_arb_share_counter_next_value;

  //jtag_to_ava_master_bridge/master sdram/s1 arbiterlock2, which is an e_assign
  assign jtag_to_ava_master_bridge_master_arbiterlock2 = sdram_s1_slavearbiterlockenable2 & jtag_to_ava_master_bridge_master_continuerequest;

  //tiger_burst_0/downstream sdram/s1 arbiterlock, which is an e_assign
  assign tiger_burst_0_downstream_arbiterlock = sdram_s1_slavearbiterlockenable & tiger_burst_0_downstream_continuerequest;

  //tiger_burst_0/downstream sdram/s1 arbiterlock2, which is an e_assign
  assign tiger_burst_0_downstream_arbiterlock2 = sdram_s1_slavearbiterlockenable2 & tiger_burst_0_downstream_continuerequest;

  //tiger_burst_0/downstream granted sdram/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_tiger_burst_0_downstream_granted_slave_sdram_s1 <= 0;
      else 
        last_cycle_tiger_burst_0_downstream_granted_slave_sdram_s1 <= tiger_burst_0_downstream_saved_grant_sdram_s1 ? 1 : (sdram_s1_arbitration_holdoff_internal | ~tiger_burst_0_downstream_requests_sdram_s1) ? 0 : last_cycle_tiger_burst_0_downstream_granted_slave_sdram_s1;
    end


  //tiger_burst_0_downstream_continuerequest continued request, which is an e_mux
  assign tiger_burst_0_downstream_continuerequest = last_cycle_tiger_burst_0_downstream_granted_slave_sdram_s1 & 1;

  //sdram_s1_any_continuerequest at least one master continues requesting, which is an e_mux
  assign sdram_s1_any_continuerequest = tiger_burst_0_downstream_continuerequest |
    jtag_to_ava_master_bridge_master_continuerequest;

  assign jtag_to_ava_master_bridge_qualified_request_sdram_s1 = jtag_to_ava_master_bridge_requests_sdram_s1 & ~((jtag_to_ava_master_bridge_master_read & ((jtag_to_ava_master_bridge_latency_counter != 0) | (1 < jtag_to_ava_master_bridge_latency_counter))) | ((!jtag_to_ava_master_bridge_byteenable_sdram_s1) & jtag_to_ava_master_bridge_master_write) | tiger_burst_0_downstream_arbiterlock);
  //unique name for sdram_s1_move_on_to_next_transaction, which is an e_assign
  assign sdram_s1_move_on_to_next_transaction = sdram_s1_readdatavalid_from_sa;

  //rdv_fifo_for_jtag_to_ava_master_bridge_master_to_sdram_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_jtag_to_ava_master_bridge_master_to_sdram_s1_module rdv_fifo_for_jtag_to_ava_master_bridge_master_to_sdram_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (jtag_to_ava_master_bridge_granted_sdram_s1),
      .data_out             (jtag_to_ava_master_bridge_rdv_fifo_output_from_sdram_s1),
      .empty                (),
      .fifo_contains_ones_n (jtag_to_ava_master_bridge_rdv_fifo_empty_sdram_s1),
      .full                 (),
      .read                 (sdram_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~sdram_s1_waits_for_read)
    );

  assign jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register = ~jtag_to_ava_master_bridge_rdv_fifo_empty_sdram_s1;
  //local readdatavalid jtag_to_ava_master_bridge_read_data_valid_sdram_s1, which is an e_mux
  assign jtag_to_ava_master_bridge_read_data_valid_sdram_s1 = (sdram_s1_readdatavalid_from_sa & jtag_to_ava_master_bridge_rdv_fifo_output_from_sdram_s1) & ~ jtag_to_ava_master_bridge_rdv_fifo_empty_sdram_s1;

  //sdram_s1_writedata mux, which is an e_mux
  assign sdram_s1_writedata = (jtag_to_ava_master_bridge_granted_sdram_s1)? jtag_to_ava_master_bridge_dbs_write_16 :
    tiger_burst_0_downstream_writedata;

  assign tiger_burst_0_downstream_requests_sdram_s1 = (1) & (tiger_burst_0_downstream_read | tiger_burst_0_downstream_write);
  //jtag_to_ava_master_bridge/master granted sdram/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_jtag_to_ava_master_bridge_master_granted_slave_sdram_s1 <= 0;
      else 
        last_cycle_jtag_to_ava_master_bridge_master_granted_slave_sdram_s1 <= jtag_to_ava_master_bridge_saved_grant_sdram_s1 ? 1 : (sdram_s1_arbitration_holdoff_internal | 0) ? 0 : last_cycle_jtag_to_ava_master_bridge_master_granted_slave_sdram_s1;
    end


  //jtag_to_ava_master_bridge_master_continuerequest continued request, which is an e_mux
  assign jtag_to_ava_master_bridge_master_continuerequest = last_cycle_jtag_to_ava_master_bridge_master_granted_slave_sdram_s1 & jtag_to_ava_master_bridge_requests_sdram_s1;

  assign tiger_burst_0_downstream_qualified_request_sdram_s1 = tiger_burst_0_downstream_requests_sdram_s1 & ~((tiger_burst_0_downstream_read & ((tiger_burst_0_downstream_latency_counter != 0) | (1 < tiger_burst_0_downstream_latency_counter))) | jtag_to_ava_master_bridge_master_arbiterlock);
  //rdv_fifo_for_tiger_burst_0_downstream_to_sdram_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_tiger_burst_0_downstream_to_sdram_s1_module rdv_fifo_for_tiger_burst_0_downstream_to_sdram_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_burst_0_downstream_granted_sdram_s1),
      .data_out             (tiger_burst_0_downstream_rdv_fifo_output_from_sdram_s1),
      .empty                (),
      .fifo_contains_ones_n (tiger_burst_0_downstream_rdv_fifo_empty_sdram_s1),
      .full                 (),
      .read                 (sdram_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~sdram_s1_waits_for_read)
    );

  assign tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register = ~tiger_burst_0_downstream_rdv_fifo_empty_sdram_s1;
  //local readdatavalid tiger_burst_0_downstream_read_data_valid_sdram_s1, which is an e_mux
  assign tiger_burst_0_downstream_read_data_valid_sdram_s1 = (sdram_s1_readdatavalid_from_sa & tiger_burst_0_downstream_rdv_fifo_output_from_sdram_s1) & ~ tiger_burst_0_downstream_rdv_fifo_empty_sdram_s1;

  //allow new arb cycle for sdram/s1, which is an e_assign
  assign sdram_s1_allow_new_arb_cycle = ~jtag_to_ava_master_bridge_master_arbiterlock & ~tiger_burst_0_downstream_arbiterlock;

  //tiger_burst_0/downstream assignment into master qualified-requests vector for sdram/s1, which is an e_assign
  assign sdram_s1_master_qreq_vector[0] = tiger_burst_0_downstream_qualified_request_sdram_s1;

  //tiger_burst_0/downstream grant sdram/s1, which is an e_assign
  assign tiger_burst_0_downstream_granted_sdram_s1 = sdram_s1_grant_vector[0];

  //tiger_burst_0/downstream saved-grant sdram/s1, which is an e_assign
  assign tiger_burst_0_downstream_saved_grant_sdram_s1 = sdram_s1_arb_winner[0];

  //jtag_to_ava_master_bridge/master assignment into master qualified-requests vector for sdram/s1, which is an e_assign
  assign sdram_s1_master_qreq_vector[1] = jtag_to_ava_master_bridge_qualified_request_sdram_s1;

  //jtag_to_ava_master_bridge/master grant sdram/s1, which is an e_assign
  assign jtag_to_ava_master_bridge_granted_sdram_s1 = sdram_s1_grant_vector[1];

  //jtag_to_ava_master_bridge/master saved-grant sdram/s1, which is an e_assign
  assign jtag_to_ava_master_bridge_saved_grant_sdram_s1 = sdram_s1_arb_winner[1] && jtag_to_ava_master_bridge_requests_sdram_s1;

  //sdram/s1 chosen-master double-vector, which is an e_assign
  assign sdram_s1_chosen_master_double_vector = {sdram_s1_master_qreq_vector, sdram_s1_master_qreq_vector} & ({~sdram_s1_master_qreq_vector, ~sdram_s1_master_qreq_vector} + sdram_s1_arb_addend);

  //stable onehot encoding of arb winner
  assign sdram_s1_arb_winner = (sdram_s1_allow_new_arb_cycle & | sdram_s1_grant_vector) ? sdram_s1_grant_vector : sdram_s1_saved_chosen_master_vector;

  //saved sdram_s1_grant_vector, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          sdram_s1_saved_chosen_master_vector <= 0;
      else if (sdram_s1_allow_new_arb_cycle)
          sdram_s1_saved_chosen_master_vector <= |sdram_s1_grant_vector ? sdram_s1_grant_vector : sdram_s1_saved_chosen_master_vector;
    end


  //onehot encoding of chosen master
  assign sdram_s1_grant_vector = {(sdram_s1_chosen_master_double_vector[1] | sdram_s1_chosen_master_double_vector[3]),
    (sdram_s1_chosen_master_double_vector[0] | sdram_s1_chosen_master_double_vector[2])};

  //sdram/s1 chosen master rotated left, which is an e_assign
  assign sdram_s1_chosen_master_rot_left = (sdram_s1_arb_winner << 1) ? (sdram_s1_arb_winner << 1) : 1;

  //sdram/s1's addend for next-master-grant
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          sdram_s1_arb_addend <= 1;
      else if (|sdram_s1_grant_vector)
          sdram_s1_arb_addend <= sdram_s1_end_xfer? sdram_s1_chosen_master_rot_left : sdram_s1_grant_vector;
    end


  //sdram_s1_reset_n assignment, which is an e_assign
  assign sdram_s1_reset_n = reset_n;

  assign sdram_s1_chipselect = jtag_to_ava_master_bridge_granted_sdram_s1 | tiger_burst_0_downstream_granted_sdram_s1;
  //sdram_s1_firsttransfer first transaction, which is an e_assign
  assign sdram_s1_firsttransfer = sdram_s1_begins_xfer ? sdram_s1_unreg_firsttransfer : sdram_s1_reg_firsttransfer;

  //sdram_s1_unreg_firsttransfer first transaction, which is an e_assign
  assign sdram_s1_unreg_firsttransfer = ~(sdram_s1_slavearbiterlockenable & sdram_s1_any_continuerequest);

  //sdram_s1_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          sdram_s1_reg_firsttransfer <= 1'b1;
      else if (sdram_s1_begins_xfer)
          sdram_s1_reg_firsttransfer <= sdram_s1_unreg_firsttransfer;
    end


  //sdram_s1_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign sdram_s1_beginbursttransfer_internal = sdram_s1_begins_xfer;

  //sdram_s1_arbitration_holdoff_internal arbitration_holdoff, which is an e_assign
  assign sdram_s1_arbitration_holdoff_internal = sdram_s1_begins_xfer & sdram_s1_firsttransfer;

  //~sdram_s1_read_n assignment, which is an e_mux
  assign sdram_s1_read_n = ~((jtag_to_ava_master_bridge_granted_sdram_s1 & jtag_to_ava_master_bridge_master_read) | (tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_read));

  //~sdram_s1_write_n assignment, which is an e_mux
  assign sdram_s1_write_n = ~((jtag_to_ava_master_bridge_granted_sdram_s1 & jtag_to_ava_master_bridge_master_write) | (tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_write));

  assign shifted_address_to_sdram_s1_from_jtag_to_ava_master_bridge_master = {jtag_to_ava_master_bridge_master_address_to_slave >> 2,
    jtag_to_ava_master_bridge_dbs_address[1],
    {1 {1'b0}}};

  //sdram_s1_address mux, which is an e_mux
  assign sdram_s1_address = (jtag_to_ava_master_bridge_granted_sdram_s1)? (shifted_address_to_sdram_s1_from_jtag_to_ava_master_bridge_master >> 1) :
    (shifted_address_to_sdram_s1_from_tiger_burst_0_downstream >> 1);

  assign shifted_address_to_sdram_s1_from_tiger_burst_0_downstream = tiger_burst_0_downstream_address_to_slave;
  //d1_sdram_s1_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_sdram_s1_end_xfer <= 1;
      else 
        d1_sdram_s1_end_xfer <= sdram_s1_end_xfer;
    end


  //sdram_s1_waits_for_read in a cycle, which is an e_mux
  assign sdram_s1_waits_for_read = sdram_s1_in_a_read_cycle & sdram_s1_waitrequest_from_sa;

  //sdram_s1_in_a_read_cycle assignment, which is an e_assign
  assign sdram_s1_in_a_read_cycle = (jtag_to_ava_master_bridge_granted_sdram_s1 & jtag_to_ava_master_bridge_master_read) | (tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_read);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = sdram_s1_in_a_read_cycle;

  //sdram_s1_waits_for_write in a cycle, which is an e_mux
  assign sdram_s1_waits_for_write = sdram_s1_in_a_write_cycle & sdram_s1_waitrequest_from_sa;

  //sdram_s1_in_a_write_cycle assignment, which is an e_assign
  assign sdram_s1_in_a_write_cycle = (jtag_to_ava_master_bridge_granted_sdram_s1 & jtag_to_ava_master_bridge_master_write) | (tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_write);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = sdram_s1_in_a_write_cycle;

  assign wait_for_sdram_s1_counter = 0;
  //~sdram_s1_byteenable_n byte enable port mux, which is an e_mux
  assign sdram_s1_byteenable_n = ~((jtag_to_ava_master_bridge_granted_sdram_s1)? jtag_to_ava_master_bridge_byteenable_sdram_s1 :
    (tiger_burst_0_downstream_granted_sdram_s1)? tiger_burst_0_downstream_byteenable :
    -1);

  assign {jtag_to_ava_master_bridge_byteenable_sdram_s1_segment_1,
jtag_to_ava_master_bridge_byteenable_sdram_s1_segment_0} = jtag_to_ava_master_bridge_master_byteenable;
  assign jtag_to_ava_master_bridge_byteenable_sdram_s1 = ((jtag_to_ava_master_bridge_dbs_address[1] == 0))? jtag_to_ava_master_bridge_byteenable_sdram_s1_segment_0 :
    jtag_to_ava_master_bridge_byteenable_sdram_s1_segment_1;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //sdram/s1 enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end


  //tiger_burst_0/downstream non-zero arbitrationshare assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_0_downstream_requests_sdram_s1 && (tiger_burst_0_downstream_arbitrationshare == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_0/downstream drove 0 on its 'arbitrationshare' port while accessing slave sdram/s1", $time);
          $stop;
        end
    end


  //tiger_burst_0/downstream non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_0_downstream_requests_sdram_s1 && (tiger_burst_0_downstream_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_0/downstream drove 0 on its 'burstcount' port while accessing slave sdram/s1", $time);
          $stop;
        end
    end


  //grant signals are active simultaneously, which is an e_process
  always @(posedge clk)
    begin
      if (jtag_to_ava_master_bridge_granted_sdram_s1 + tiger_burst_0_downstream_granted_sdram_s1 > 1)
        begin
          $write("%0d ns: > 1 of grant signals are active simultaneously", $time);
          $stop;
        end
    end


  //saved_grant signals are active simultaneously, which is an e_process
  always @(posedge clk)
    begin
      if (jtag_to_ava_master_bridge_saved_grant_sdram_s1 + tiger_burst_0_downstream_saved_grant_sdram_s1 > 1)
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

module burstcount_fifo_for_tiger_burst_0_upstream_module (
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

  output  [  7: 0] data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input   [  7: 0] data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire    [  7: 0] data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  reg              full_2;
  reg              full_3;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  wire             full_9;
  reg     [  4: 0] how_many_ones;
  wire    [  4: 0] one_count_minus_one;
  wire    [  4: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  7: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  7: 0] p1_stage_1;
  wire             p2_full_2;
  wire    [  7: 0] p2_stage_2;
  wire             p3_full_3;
  wire    [  7: 0] p3_stage_3;
  wire             p4_full_4;
  wire    [  7: 0] p4_stage_4;
  wire             p5_full_5;
  wire    [  7: 0] p5_stage_5;
  wire             p6_full_6;
  wire    [  7: 0] p6_stage_6;
  wire             p7_full_7;
  wire    [  7: 0] p7_stage_7;
  wire             p8_full_8;
  wire    [  7: 0] p8_stage_8;
  reg     [  7: 0] stage_0;
  reg     [  7: 0] stage_1;
  reg     [  7: 0] stage_2;
  reg     [  7: 0] stage_3;
  reg     [  7: 0] stage_4;
  reg     [  7: 0] stage_5;
  reg     [  7: 0] stage_6;
  reg     [  7: 0] stage_7;
  reg     [  7: 0] stage_8;
  wire    [  4: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_8;
  assign empty = !full_0;
  assign full_9 = 0;
  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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

module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_0_upstream_module (
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
  reg              full_2;
  reg              full_3;
  reg              full_4;
  reg              full_5;
  reg              full_6;
  reg              full_7;
  reg              full_8;
  wire             full_9;
  reg     [  4: 0] how_many_ones;
  wire    [  4: 0] one_count_minus_one;
  wire    [  4: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p2_full_2;
  wire             p2_stage_2;
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
  reg              stage_0;
  reg              stage_1;
  reg              stage_2;
  reg              stage_3;
  reg              stage_4;
  reg              stage_5;
  reg              stage_6;
  reg              stage_7;
  reg              stage_8;
  wire    [  4: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_8;
  assign empty = !full_0;
  assign full_9 = 0;
  //data_8, which is an e_mux
  assign p8_stage_8 = ((full_9 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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

module tiger_burst_0_upstream_arbitrator (
                                           // inputs:
                                            clk,
                                            pipeline_bridge_MEMORY_m1_address_to_slave,
                                            pipeline_bridge_MEMORY_m1_burstcount,
                                            pipeline_bridge_MEMORY_m1_byteenable,
                                            pipeline_bridge_MEMORY_m1_chipselect,
                                            pipeline_bridge_MEMORY_m1_dbs_address,
                                            pipeline_bridge_MEMORY_m1_dbs_write_16,
                                            pipeline_bridge_MEMORY_m1_debugaccess,
                                            pipeline_bridge_MEMORY_m1_latency_counter,
                                            pipeline_bridge_MEMORY_m1_read,
                                            pipeline_bridge_MEMORY_m1_write,
                                            reset_n,
                                            tiger_burst_0_upstream_readdata,
                                            tiger_burst_0_upstream_readdatavalid,
                                            tiger_burst_0_upstream_waitrequest,

                                           // outputs:
                                            d1_tiger_burst_0_upstream_end_xfer,
                                            pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream,
                                            pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream,
                                            pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream,
                                            tiger_burst_0_upstream_address,
                                            tiger_burst_0_upstream_burstcount,
                                            tiger_burst_0_upstream_byteaddress,
                                            tiger_burst_0_upstream_byteenable,
                                            tiger_burst_0_upstream_debugaccess,
                                            tiger_burst_0_upstream_read,
                                            tiger_burst_0_upstream_readdata_from_sa,
                                            tiger_burst_0_upstream_waitrequest_from_sa,
                                            tiger_burst_0_upstream_write,
                                            tiger_burst_0_upstream_writedata
                                         )
;

  output           d1_tiger_burst_0_upstream_end_xfer;
  output  [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream;
  output           pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream;
  output           pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  output           pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream;
  output  [ 22: 0] tiger_burst_0_upstream_address;
  output  [  6: 0] tiger_burst_0_upstream_burstcount;
  output  [ 23: 0] tiger_burst_0_upstream_byteaddress;
  output  [  1: 0] tiger_burst_0_upstream_byteenable;
  output           tiger_burst_0_upstream_debugaccess;
  output           tiger_burst_0_upstream_read;
  output  [ 15: 0] tiger_burst_0_upstream_readdata_from_sa;
  output           tiger_burst_0_upstream_waitrequest_from_sa;
  output           tiger_burst_0_upstream_write;
  output  [ 15: 0] tiger_burst_0_upstream_writedata;
  input            clk;
  input   [ 23: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  input   [  6: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input   [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  input   [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  input            pipeline_bridge_MEMORY_m1_debugaccess;
  input            pipeline_bridge_MEMORY_m1_latency_counter;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_write;
  input            reset_n;
  input   [ 15: 0] tiger_burst_0_upstream_readdata;
  input            tiger_burst_0_upstream_readdatavalid;
  input            tiger_burst_0_upstream_waitrequest;

  reg              d1_reasons_to_wait;
  reg              d1_tiger_burst_0_upstream_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_tiger_burst_0_upstream;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             p0_tiger_burst_0_upstream_load_fifo;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock2;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream_segment_0;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream_segment_1;
  wire             pipeline_bridge_MEMORY_m1_continuerequest;
  wire             pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  wire             pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_0_upstream;
  wire    [ 22: 0] tiger_burst_0_upstream_address;
  wire             tiger_burst_0_upstream_allgrants;
  wire             tiger_burst_0_upstream_allow_new_arb_cycle;
  wire             tiger_burst_0_upstream_any_bursting_master_saved_grant;
  wire             tiger_burst_0_upstream_any_continuerequest;
  wire             tiger_burst_0_upstream_arb_counter_enable;
  reg     [  8: 0] tiger_burst_0_upstream_arb_share_counter;
  wire    [  8: 0] tiger_burst_0_upstream_arb_share_counter_next_value;
  wire    [  8: 0] tiger_burst_0_upstream_arb_share_set_values;
  reg     [  5: 0] tiger_burst_0_upstream_bbt_burstcounter;
  wire             tiger_burst_0_upstream_beginbursttransfer_internal;
  wire             tiger_burst_0_upstream_begins_xfer;
  wire    [  6: 0] tiger_burst_0_upstream_burstcount;
  wire             tiger_burst_0_upstream_burstcount_fifo_empty;
  wire    [ 23: 0] tiger_burst_0_upstream_byteaddress;
  wire    [  1: 0] tiger_burst_0_upstream_byteenable;
  reg     [  7: 0] tiger_burst_0_upstream_current_burst;
  wire    [  7: 0] tiger_burst_0_upstream_current_burst_minus_one;
  wire             tiger_burst_0_upstream_debugaccess;
  wire             tiger_burst_0_upstream_end_xfer;
  wire             tiger_burst_0_upstream_firsttransfer;
  wire             tiger_burst_0_upstream_grant_vector;
  wire             tiger_burst_0_upstream_in_a_read_cycle;
  wire             tiger_burst_0_upstream_in_a_write_cycle;
  reg              tiger_burst_0_upstream_load_fifo;
  wire             tiger_burst_0_upstream_master_qreq_vector;
  wire             tiger_burst_0_upstream_move_on_to_next_transaction;
  wire    [  5: 0] tiger_burst_0_upstream_next_bbt_burstcount;
  wire    [  7: 0] tiger_burst_0_upstream_next_burst_count;
  wire             tiger_burst_0_upstream_non_bursting_master_requests;
  wire             tiger_burst_0_upstream_read;
  wire    [ 15: 0] tiger_burst_0_upstream_readdata_from_sa;
  wire             tiger_burst_0_upstream_readdatavalid_from_sa;
  reg              tiger_burst_0_upstream_reg_firsttransfer;
  wire    [  7: 0] tiger_burst_0_upstream_selected_burstcount;
  reg              tiger_burst_0_upstream_slavearbiterlockenable;
  wire             tiger_burst_0_upstream_slavearbiterlockenable2;
  wire             tiger_burst_0_upstream_this_cycle_is_the_last_burst;
  wire    [  7: 0] tiger_burst_0_upstream_transaction_burst_count;
  wire             tiger_burst_0_upstream_unreg_firsttransfer;
  wire             tiger_burst_0_upstream_waitrequest_from_sa;
  wire             tiger_burst_0_upstream_waits_for_read;
  wire             tiger_burst_0_upstream_waits_for_write;
  wire             tiger_burst_0_upstream_write;
  wire    [ 15: 0] tiger_burst_0_upstream_writedata;
  wire             wait_for_tiger_burst_0_upstream_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~tiger_burst_0_upstream_end_xfer;
    end


  assign tiger_burst_0_upstream_begins_xfer = ~d1_reasons_to_wait & ((pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream));
  //assign tiger_burst_0_upstream_readdata_from_sa = tiger_burst_0_upstream_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_0_upstream_readdata_from_sa = tiger_burst_0_upstream_readdata;

  assign pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream = ({pipeline_bridge_MEMORY_m1_address_to_slave[23] , 23'b0} == 24'h800000) & pipeline_bridge_MEMORY_m1_chipselect;
  //assign tiger_burst_0_upstream_waitrequest_from_sa = tiger_burst_0_upstream_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_0_upstream_waitrequest_from_sa = tiger_burst_0_upstream_waitrequest;

  //assign tiger_burst_0_upstream_readdatavalid_from_sa = tiger_burst_0_upstream_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_0_upstream_readdatavalid_from_sa = tiger_burst_0_upstream_readdatavalid;

  //tiger_burst_0_upstream_arb_share_counter set values, which is an e_mux
  assign tiger_burst_0_upstream_arb_share_set_values = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream)? ((((pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect)) ? pipeline_bridge_MEMORY_m1_burstcount<< 1 : 1)) :
    1;

  //tiger_burst_0_upstream_non_bursting_master_requests mux, which is an e_mux
  assign tiger_burst_0_upstream_non_bursting_master_requests = 0;

  //tiger_burst_0_upstream_any_bursting_master_saved_grant mux, which is an e_mux
  assign tiger_burst_0_upstream_any_bursting_master_saved_grant = pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_0_upstream;

  //tiger_burst_0_upstream_arb_share_counter_next_value assignment, which is an e_assign
  assign tiger_burst_0_upstream_arb_share_counter_next_value = tiger_burst_0_upstream_firsttransfer ? (tiger_burst_0_upstream_arb_share_set_values - 1) : |tiger_burst_0_upstream_arb_share_counter ? (tiger_burst_0_upstream_arb_share_counter - 1) : 0;

  //tiger_burst_0_upstream_allgrants all slave grants, which is an e_mux
  assign tiger_burst_0_upstream_allgrants = |tiger_burst_0_upstream_grant_vector;

  //tiger_burst_0_upstream_end_xfer assignment, which is an e_assign
  assign tiger_burst_0_upstream_end_xfer = ~(tiger_burst_0_upstream_waits_for_read | tiger_burst_0_upstream_waits_for_write);

  //end_xfer_arb_share_counter_term_tiger_burst_0_upstream arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_tiger_burst_0_upstream = tiger_burst_0_upstream_end_xfer & (~tiger_burst_0_upstream_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //tiger_burst_0_upstream_arb_share_counter arbitration counter enable, which is an e_assign
  assign tiger_burst_0_upstream_arb_counter_enable = (end_xfer_arb_share_counter_term_tiger_burst_0_upstream & tiger_burst_0_upstream_allgrants) | (end_xfer_arb_share_counter_term_tiger_burst_0_upstream & ~tiger_burst_0_upstream_non_bursting_master_requests);

  //tiger_burst_0_upstream_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_upstream_arb_share_counter <= 0;
      else if (tiger_burst_0_upstream_arb_counter_enable)
          tiger_burst_0_upstream_arb_share_counter <= tiger_burst_0_upstream_arb_share_counter_next_value;
    end


  //tiger_burst_0_upstream_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_upstream_slavearbiterlockenable <= 0;
      else if ((|tiger_burst_0_upstream_master_qreq_vector & end_xfer_arb_share_counter_term_tiger_burst_0_upstream) | (end_xfer_arb_share_counter_term_tiger_burst_0_upstream & ~tiger_burst_0_upstream_non_bursting_master_requests))
          tiger_burst_0_upstream_slavearbiterlockenable <= |tiger_burst_0_upstream_arb_share_counter_next_value;
    end


  //pipeline_bridge_MEMORY/m1 tiger_burst_0/upstream arbiterlock, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock = tiger_burst_0_upstream_slavearbiterlockenable & pipeline_bridge_MEMORY_m1_continuerequest;

  //tiger_burst_0_upstream_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign tiger_burst_0_upstream_slavearbiterlockenable2 = |tiger_burst_0_upstream_arb_share_counter_next_value;

  //pipeline_bridge_MEMORY/m1 tiger_burst_0/upstream arbiterlock2, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock2 = tiger_burst_0_upstream_slavearbiterlockenable2 & pipeline_bridge_MEMORY_m1_continuerequest;

  //tiger_burst_0_upstream_any_continuerequest at least one master continues requesting, which is an e_assign
  assign tiger_burst_0_upstream_any_continuerequest = 1;

  //pipeline_bridge_MEMORY_m1_continuerequest continued request, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_continuerequest = 1;

  assign pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream = pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream & ~(((pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & ((pipeline_bridge_MEMORY_m1_latency_counter != 0) | (1 < pipeline_bridge_MEMORY_m1_latency_counter))));
  //unique name for tiger_burst_0_upstream_move_on_to_next_transaction, which is an e_assign
  assign tiger_burst_0_upstream_move_on_to_next_transaction = tiger_burst_0_upstream_this_cycle_is_the_last_burst & tiger_burst_0_upstream_load_fifo;

  //the currently selected burstcount for tiger_burst_0_upstream, which is an e_mux
  assign tiger_burst_0_upstream_selected_burstcount = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream)? pipeline_bridge_MEMORY_m1_burstcount :
    1;

  //burstcount_fifo_for_tiger_burst_0_upstream, which is an e_fifo_with_registered_outputs
  burstcount_fifo_for_tiger_burst_0_upstream_module burstcount_fifo_for_tiger_burst_0_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_burst_0_upstream_selected_burstcount),
      .data_out             (tiger_burst_0_upstream_transaction_burst_count),
      .empty                (tiger_burst_0_upstream_burstcount_fifo_empty),
      .fifo_contains_ones_n (),
      .full                 (),
      .read                 (tiger_burst_0_upstream_this_cycle_is_the_last_burst),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_0_upstream_waits_for_read & tiger_burst_0_upstream_load_fifo & ~(tiger_burst_0_upstream_this_cycle_is_the_last_burst & tiger_burst_0_upstream_burstcount_fifo_empty))
    );

  //tiger_burst_0_upstream current burst minus one, which is an e_assign
  assign tiger_burst_0_upstream_current_burst_minus_one = tiger_burst_0_upstream_current_burst - 1;

  //what to load in current_burst, for tiger_burst_0_upstream, which is an e_mux
  assign tiger_burst_0_upstream_next_burst_count = (((in_a_read_cycle & ~tiger_burst_0_upstream_waits_for_read) & ~tiger_burst_0_upstream_load_fifo))? {tiger_burst_0_upstream_selected_burstcount, 1'b0} :
    ((in_a_read_cycle & ~tiger_burst_0_upstream_waits_for_read & tiger_burst_0_upstream_this_cycle_is_the_last_burst & tiger_burst_0_upstream_burstcount_fifo_empty))? {tiger_burst_0_upstream_selected_burstcount, 1'b0} :
    (tiger_burst_0_upstream_this_cycle_is_the_last_burst)? {tiger_burst_0_upstream_transaction_burst_count,  1'b0} :
    tiger_burst_0_upstream_current_burst_minus_one;

  //the current burst count for tiger_burst_0_upstream, to be decremented, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_upstream_current_burst <= 0;
      else if (tiger_burst_0_upstream_readdatavalid_from_sa | (~tiger_burst_0_upstream_load_fifo & (in_a_read_cycle & ~tiger_burst_0_upstream_waits_for_read)))
          tiger_burst_0_upstream_current_burst <= tiger_burst_0_upstream_next_burst_count;
    end


  //a 1 or burstcount fifo empty, to initialize the counter, which is an e_mux
  assign p0_tiger_burst_0_upstream_load_fifo = (~tiger_burst_0_upstream_load_fifo)? 1 :
    (((in_a_read_cycle & ~tiger_burst_0_upstream_waits_for_read) & tiger_burst_0_upstream_load_fifo))? 1 :
    ~tiger_burst_0_upstream_burstcount_fifo_empty;

  //whether to load directly to the counter or to the fifo, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_upstream_load_fifo <= 0;
      else if ((in_a_read_cycle & ~tiger_burst_0_upstream_waits_for_read) & ~tiger_burst_0_upstream_load_fifo | tiger_burst_0_upstream_this_cycle_is_the_last_burst)
          tiger_burst_0_upstream_load_fifo <= p0_tiger_burst_0_upstream_load_fifo;
    end


  //the last cycle in the burst for tiger_burst_0_upstream, which is an e_assign
  assign tiger_burst_0_upstream_this_cycle_is_the_last_burst = ~(|tiger_burst_0_upstream_current_burst_minus_one) & tiger_burst_0_upstream_readdatavalid_from_sa;

  //rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_0_upstream, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_0_upstream_module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_0_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream),
      .data_out             (pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_tiger_burst_0_upstream),
      .empty                (),
      .fifo_contains_ones_n (pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_0_upstream),
      .full                 (),
      .read                 (tiger_burst_0_upstream_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_0_upstream_waits_for_read)
    );

  assign pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register = ~pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_0_upstream;
  //local readdatavalid pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream = tiger_burst_0_upstream_readdatavalid_from_sa;

  //tiger_burst_0_upstream_writedata mux, which is an e_mux
  assign tiger_burst_0_upstream_writedata = pipeline_bridge_MEMORY_m1_dbs_write_16;

  //byteaddress mux for tiger_burst_0/upstream, which is an e_mux
  assign tiger_burst_0_upstream_byteaddress = pipeline_bridge_MEMORY_m1_address_to_slave;

  //master is always granted when requested
  assign pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream = pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream;

  //pipeline_bridge_MEMORY/m1 saved-grant tiger_burst_0/upstream, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_0_upstream = pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream;

  //allow new arb cycle for tiger_burst_0/upstream, which is an e_assign
  assign tiger_burst_0_upstream_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign tiger_burst_0_upstream_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign tiger_burst_0_upstream_master_qreq_vector = 1;

  //tiger_burst_0_upstream_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_0_upstream_firsttransfer = tiger_burst_0_upstream_begins_xfer ? tiger_burst_0_upstream_unreg_firsttransfer : tiger_burst_0_upstream_reg_firsttransfer;

  //tiger_burst_0_upstream_unreg_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_0_upstream_unreg_firsttransfer = ~(tiger_burst_0_upstream_slavearbiterlockenable & tiger_burst_0_upstream_any_continuerequest);

  //tiger_burst_0_upstream_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_upstream_reg_firsttransfer <= 1'b1;
      else if (tiger_burst_0_upstream_begins_xfer)
          tiger_burst_0_upstream_reg_firsttransfer <= tiger_burst_0_upstream_unreg_firsttransfer;
    end


  //tiger_burst_0_upstream_next_bbt_burstcount next_bbt_burstcount, which is an e_mux
  assign tiger_burst_0_upstream_next_bbt_burstcount = ((((tiger_burst_0_upstream_write) && (tiger_burst_0_upstream_bbt_burstcounter == 0))))? (tiger_burst_0_upstream_burstcount - 1) :
    ((((tiger_burst_0_upstream_read) && (tiger_burst_0_upstream_bbt_burstcounter == 0))))? 0 :
    (tiger_burst_0_upstream_bbt_burstcounter - 1);

  //tiger_burst_0_upstream_bbt_burstcounter bbt_burstcounter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_upstream_bbt_burstcounter <= 0;
      else if (tiger_burst_0_upstream_begins_xfer)
          tiger_burst_0_upstream_bbt_burstcounter <= tiger_burst_0_upstream_next_bbt_burstcount;
    end


  //tiger_burst_0_upstream_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign tiger_burst_0_upstream_beginbursttransfer_internal = tiger_burst_0_upstream_begins_xfer & (tiger_burst_0_upstream_bbt_burstcounter == 0);

  //tiger_burst_0_upstream_read assignment, which is an e_mux
  assign tiger_burst_0_upstream_read = pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect);

  //tiger_burst_0_upstream_write assignment, which is an e_mux
  assign tiger_burst_0_upstream_write = pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect);

  //tiger_burst_0_upstream_address mux, which is an e_mux
  assign tiger_burst_0_upstream_address = {pipeline_bridge_MEMORY_m1_address_to_slave >> 2,
    pipeline_bridge_MEMORY_m1_dbs_address[1],
    {1 {1'b0}}};

  //d1_tiger_burst_0_upstream_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_tiger_burst_0_upstream_end_xfer <= 1;
      else 
        d1_tiger_burst_0_upstream_end_xfer <= tiger_burst_0_upstream_end_xfer;
    end


  //tiger_burst_0_upstream_waits_for_read in a cycle, which is an e_mux
  assign tiger_burst_0_upstream_waits_for_read = tiger_burst_0_upstream_in_a_read_cycle & tiger_burst_0_upstream_waitrequest_from_sa;

  //tiger_burst_0_upstream_in_a_read_cycle assignment, which is an e_assign
  assign tiger_burst_0_upstream_in_a_read_cycle = pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = tiger_burst_0_upstream_in_a_read_cycle;

  //tiger_burst_0_upstream_waits_for_write in a cycle, which is an e_mux
  assign tiger_burst_0_upstream_waits_for_write = tiger_burst_0_upstream_in_a_write_cycle & tiger_burst_0_upstream_waitrequest_from_sa;

  //tiger_burst_0_upstream_in_a_write_cycle assignment, which is an e_assign
  assign tiger_burst_0_upstream_in_a_write_cycle = pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = tiger_burst_0_upstream_in_a_write_cycle;

  assign wait_for_tiger_burst_0_upstream_counter = 0;
  //tiger_burst_0_upstream_byteenable byte enable port mux, which is an e_mux
  assign tiger_burst_0_upstream_byteenable = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream)? pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream :
    -1;

  assign {pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream_segment_1,
pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream_segment_0} = pipeline_bridge_MEMORY_m1_byteenable;
  assign pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream = ((pipeline_bridge_MEMORY_m1_dbs_address[1] == 0))? pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream_segment_0 :
    pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream_segment_1;

  //burstcount mux, which is an e_mux
  assign tiger_burst_0_upstream_burstcount = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream)? pipeline_bridge_MEMORY_m1_burstcount :
    1;

  //debugaccess mux, which is an e_mux
  assign tiger_burst_0_upstream_debugaccess = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream)? pipeline_bridge_MEMORY_m1_debugaccess :
    0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_0/upstream enable non-zero assertions, which is an e_register
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
      if (pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream && (pipeline_bridge_MEMORY_m1_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: pipeline_bridge_MEMORY/m1 drove 0 on its 'burstcount' port while accessing slave tiger_burst_0/upstream", $time);
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

module tiger_burst_0_downstream_arbitrator (
                                             // inputs:
                                              clk,
                                              d1_sdram_s1_end_xfer,
                                              reset_n,
                                              sdram_s1_readdata_from_sa,
                                              sdram_s1_waitrequest_from_sa,
                                              tiger_burst_0_downstream_address,
                                              tiger_burst_0_downstream_burstcount,
                                              tiger_burst_0_downstream_byteenable,
                                              tiger_burst_0_downstream_granted_sdram_s1,
                                              tiger_burst_0_downstream_qualified_request_sdram_s1,
                                              tiger_burst_0_downstream_read,
                                              tiger_burst_0_downstream_read_data_valid_sdram_s1,
                                              tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register,
                                              tiger_burst_0_downstream_requests_sdram_s1,
                                              tiger_burst_0_downstream_write,
                                              tiger_burst_0_downstream_writedata,

                                             // outputs:
                                              tiger_burst_0_downstream_address_to_slave,
                                              tiger_burst_0_downstream_latency_counter,
                                              tiger_burst_0_downstream_readdata,
                                              tiger_burst_0_downstream_readdatavalid,
                                              tiger_burst_0_downstream_reset_n,
                                              tiger_burst_0_downstream_waitrequest
                                           )
;

  output  [ 22: 0] tiger_burst_0_downstream_address_to_slave;
  output           tiger_burst_0_downstream_latency_counter;
  output  [ 15: 0] tiger_burst_0_downstream_readdata;
  output           tiger_burst_0_downstream_readdatavalid;
  output           tiger_burst_0_downstream_reset_n;
  output           tiger_burst_0_downstream_waitrequest;
  input            clk;
  input            d1_sdram_s1_end_xfer;
  input            reset_n;
  input   [ 15: 0] sdram_s1_readdata_from_sa;
  input            sdram_s1_waitrequest_from_sa;
  input   [ 22: 0] tiger_burst_0_downstream_address;
  input            tiger_burst_0_downstream_burstcount;
  input   [  1: 0] tiger_burst_0_downstream_byteenable;
  input            tiger_burst_0_downstream_granted_sdram_s1;
  input            tiger_burst_0_downstream_qualified_request_sdram_s1;
  input            tiger_burst_0_downstream_read;
  input            tiger_burst_0_downstream_read_data_valid_sdram_s1;
  input            tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register;
  input            tiger_burst_0_downstream_requests_sdram_s1;
  input            tiger_burst_0_downstream_write;
  input   [ 15: 0] tiger_burst_0_downstream_writedata;

  reg              active_and_waiting_last_time;
  wire             latency_load_value;
  wire             p1_tiger_burst_0_downstream_latency_counter;
  wire             pre_flush_tiger_burst_0_downstream_readdatavalid;
  wire             r_0;
  reg     [ 22: 0] tiger_burst_0_downstream_address_last_time;
  wire    [ 22: 0] tiger_burst_0_downstream_address_to_slave;
  reg              tiger_burst_0_downstream_burstcount_last_time;
  reg     [  1: 0] tiger_burst_0_downstream_byteenable_last_time;
  wire             tiger_burst_0_downstream_is_granted_some_slave;
  reg              tiger_burst_0_downstream_latency_counter;
  reg              tiger_burst_0_downstream_read_but_no_slave_selected;
  reg              tiger_burst_0_downstream_read_last_time;
  wire    [ 15: 0] tiger_burst_0_downstream_readdata;
  wire             tiger_burst_0_downstream_readdatavalid;
  wire             tiger_burst_0_downstream_reset_n;
  wire             tiger_burst_0_downstream_run;
  wire             tiger_burst_0_downstream_waitrequest;
  reg              tiger_burst_0_downstream_write_last_time;
  reg     [ 15: 0] tiger_burst_0_downstream_writedata_last_time;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (tiger_burst_0_downstream_qualified_request_sdram_s1 | ~tiger_burst_0_downstream_requests_sdram_s1) & (tiger_burst_0_downstream_granted_sdram_s1 | ~tiger_burst_0_downstream_qualified_request_sdram_s1) & ((~tiger_burst_0_downstream_qualified_request_sdram_s1 | ~tiger_burst_0_downstream_read | (1 & ~sdram_s1_waitrequest_from_sa & tiger_burst_0_downstream_read))) & ((~tiger_burst_0_downstream_qualified_request_sdram_s1 | ~tiger_burst_0_downstream_write | (1 & ~sdram_s1_waitrequest_from_sa & tiger_burst_0_downstream_write)));

  //cascaded wait assignment, which is an e_assign
  assign tiger_burst_0_downstream_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_burst_0_downstream_address_to_slave = tiger_burst_0_downstream_address;

  //tiger_burst_0_downstream_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_read_but_no_slave_selected <= 0;
      else 
        tiger_burst_0_downstream_read_but_no_slave_selected <= tiger_burst_0_downstream_read & tiger_burst_0_downstream_run & ~tiger_burst_0_downstream_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign tiger_burst_0_downstream_is_granted_some_slave = tiger_burst_0_downstream_granted_sdram_s1;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_tiger_burst_0_downstream_readdatavalid = tiger_burst_0_downstream_read_data_valid_sdram_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign tiger_burst_0_downstream_readdatavalid = tiger_burst_0_downstream_read_but_no_slave_selected |
    pre_flush_tiger_burst_0_downstream_readdatavalid;

  //tiger_burst_0/downstream readdata mux, which is an e_mux
  assign tiger_burst_0_downstream_readdata = sdram_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_burst_0_downstream_waitrequest = ~tiger_burst_0_downstream_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_latency_counter <= 0;
      else 
        tiger_burst_0_downstream_latency_counter <= p1_tiger_burst_0_downstream_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_tiger_burst_0_downstream_latency_counter = ((tiger_burst_0_downstream_run & tiger_burst_0_downstream_read))? latency_load_value :
    (tiger_burst_0_downstream_latency_counter)? tiger_burst_0_downstream_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;

  //tiger_burst_0_downstream_reset_n assignment, which is an e_assign
  assign tiger_burst_0_downstream_reset_n = reset_n;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_0_downstream_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_address_last_time <= 0;
      else 
        tiger_burst_0_downstream_address_last_time <= tiger_burst_0_downstream_address;
    end


  //tiger_burst_0/downstream waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= tiger_burst_0_downstream_waitrequest & (tiger_burst_0_downstream_read | tiger_burst_0_downstream_write);
    end


  //tiger_burst_0_downstream_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_0_downstream_address != tiger_burst_0_downstream_address_last_time))
        begin
          $write("%0d ns: tiger_burst_0_downstream_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_0_downstream_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_burstcount_last_time <= 0;
      else 
        tiger_burst_0_downstream_burstcount_last_time <= tiger_burst_0_downstream_burstcount;
    end


  //tiger_burst_0_downstream_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_0_downstream_burstcount != tiger_burst_0_downstream_burstcount_last_time))
        begin
          $write("%0d ns: tiger_burst_0_downstream_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_0_downstream_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_byteenable_last_time <= 0;
      else 
        tiger_burst_0_downstream_byteenable_last_time <= tiger_burst_0_downstream_byteenable;
    end


  //tiger_burst_0_downstream_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_0_downstream_byteenable != tiger_burst_0_downstream_byteenable_last_time))
        begin
          $write("%0d ns: tiger_burst_0_downstream_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_0_downstream_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_read_last_time <= 0;
      else 
        tiger_burst_0_downstream_read_last_time <= tiger_burst_0_downstream_read;
    end


  //tiger_burst_0_downstream_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_0_downstream_read != tiger_burst_0_downstream_read_last_time))
        begin
          $write("%0d ns: tiger_burst_0_downstream_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_0_downstream_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_write_last_time <= 0;
      else 
        tiger_burst_0_downstream_write_last_time <= tiger_burst_0_downstream_write;
    end


  //tiger_burst_0_downstream_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_0_downstream_write != tiger_burst_0_downstream_write_last_time))
        begin
          $write("%0d ns: tiger_burst_0_downstream_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_0_downstream_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_0_downstream_writedata_last_time <= 0;
      else 
        tiger_burst_0_downstream_writedata_last_time <= tiger_burst_0_downstream_writedata;
    end


  //tiger_burst_0_downstream_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_0_downstream_writedata != tiger_burst_0_downstream_writedata_last_time) & tiger_burst_0_downstream_write)
        begin
          $write("%0d ns: tiger_burst_0_downstream_writedata did not heed wait!!!", $time);
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
                                           jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register,
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
  input            jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register;
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
  reg     [  1: 0] tiger_top_0_leapSlave_arb_share_counter;
  wire    [  1: 0] tiger_top_0_leapSlave_arb_share_counter_next_value;
  wire    [  1: 0] tiger_top_0_leapSlave_arb_share_set_values;
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

  assign jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave = ({jtag_to_ava_master_bridge_master_address_to_slave[31 : 10] , 10'b0} == 32'h2000000) & (jtag_to_ava_master_bridge_master_read | jtag_to_ava_master_bridge_master_write);
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

  assign jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave = jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave & ~((jtag_to_ava_master_bridge_master_read & ((jtag_to_ava_master_bridge_latency_counter != 0) | (|jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register))));
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
  output  [ 31: 0] tiger_top_0_instructionMaster_readdata;
  output           tiger_top_0_instructionMaster_readdatavalid;
  output           tiger_top_0_instructionMaster_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_MEMORY_s1_end_xfer;
  input   [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
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
  wire    [ 31: 0] tiger_top_0_instructionMaster_readdata;
  wire             tiger_top_0_instructionMaster_readdatavalid;
  wire             tiger_top_0_instructionMaster_run;
  wire             tiger_top_0_instructionMaster_waitrequest;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1) & (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 | ~tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1) & ((~tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~(tiger_top_0_instructionMaster_read) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (tiger_top_0_instructionMaster_read))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_top_0_instructionMaster_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_top_0_instructionMaster_address_to_slave = {8'b0,
    tiger_top_0_instructionMaster_address[23 : 0]};

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

module tiger (
               // 1) global signals:
                clk,
                reset_n,

               // the_sdram
                zs_addr_from_the_sdram,
                zs_ba_from_the_sdram,
                zs_cas_n_from_the_sdram,
                zs_cke_from_the_sdram,
                zs_cs_n_from_the_sdram,
                zs_dq_to_and_from_the_sdram,
                zs_dqm_from_the_sdram,
                zs_ras_n_from_the_sdram,
                zs_we_n_from_the_sdram,

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

  output  [ 17: 0] coe_debug_lights_from_the_tiger_top_0;
  output           coe_exe_end_from_the_tiger_top_0;
  output           coe_exe_start_from_the_tiger_top_0;
  output           txd_from_the_uart_0;
  output  [ 11: 0] zs_addr_from_the_sdram;
  output  [  1: 0] zs_ba_from_the_sdram;
  output           zs_cas_n_from_the_sdram;
  output           zs_cke_from_the_sdram;
  output           zs_cs_n_from_the_sdram;
  inout   [ 15: 0] zs_dq_to_and_from_the_sdram;
  output  [  1: 0] zs_dqm_from_the_sdram;
  output           zs_ras_n_from_the_sdram;
  output           zs_we_n_from_the_sdram;
  input            clk;
  input   [  2: 0] coe_debug_select_to_the_tiger_top_0;
  input            reset_n;
  input            rxd_to_the_uart_0;

  wire             clk_reset_n;
  wire    [ 17: 0] coe_debug_lights_from_the_tiger_top_0;
  wire             coe_exe_end_from_the_tiger_top_0;
  wire             coe_exe_start_from_the_tiger_top_0;
  wire             d1_data_cache_0_CACHE0_end_xfer;
  wire             d1_pipeline_bridge_MEMORY_s1_end_xfer;
  wire             d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  wire             d1_sdram_s1_end_xfer;
  wire             d1_tiger_burst_0_upstream_end_xfer;
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
  wire    [  3: 0] data_cache_0_dataMaster0_byteenable;
  wire             data_cache_0_dataMaster0_granted_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_latency_counter;
  wire             data_cache_0_dataMaster0_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_read;
  wire             data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire    [ 31: 0] data_cache_0_dataMaster0_readdata;
  wire             data_cache_0_dataMaster0_readdatavalid;
  wire             data_cache_0_dataMaster0_requests_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster0_waitrequest;
  wire             data_cache_0_dataMaster0_write;
  wire    [ 31: 0] data_cache_0_dataMaster0_writedata;
  wire    [  1: 0] jtag_to_ava_master_bridge_byteenable_sdram_s1;
  wire    [  1: 0] jtag_to_ava_master_bridge_dbs_address;
  wire    [ 15: 0] jtag_to_ava_master_bridge_dbs_write_16;
  wire             jtag_to_ava_master_bridge_granted_sdram_s1;
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
  wire             jtag_to_ava_master_bridge_qualified_request_sdram_s1;
  wire             jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_read_data_valid_sdram_s1;
  wire             jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register;
  wire             jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave;
  wire             jtag_to_ava_master_bridge_requests_sdram_s1;
  wire             jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave;
  wire    [ 23: 0] pipeline_bridge_MEMORY_m1_address;
  wire    [ 23: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  wire    [  6: 0] pipeline_bridge_MEMORY_m1_burstcount;
  wire    [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_chipselect;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  wire    [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  wire             pipeline_bridge_MEMORY_m1_debugaccess;
  wire             pipeline_bridge_MEMORY_m1_endofpacket;
  wire             pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_latency_counter;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_read;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  wire    [ 31: 0] pipeline_bridge_MEMORY_m1_readdata;
  wire             pipeline_bridge_MEMORY_m1_readdatavalid;
  wire             pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_waitrequest;
  wire             pipeline_bridge_MEMORY_m1_write;
  wire    [ 31: 0] pipeline_bridge_MEMORY_m1_writedata;
  wire    [ 21: 0] pipeline_bridge_MEMORY_s1_address;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock2;
  wire    [  6: 0] pipeline_bridge_MEMORY_s1_burstcount;
  wire    [  3: 0] pipeline_bridge_MEMORY_s1_byteenable;
  wire             pipeline_bridge_MEMORY_s1_chipselect;
  wire             pipeline_bridge_MEMORY_s1_debugaccess;
  wire             pipeline_bridge_MEMORY_s1_endofpacket;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire    [ 21: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  wire             pipeline_bridge_MEMORY_s1_read;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_readdata;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  wire             pipeline_bridge_MEMORY_s1_readdatavalid;
  wire             pipeline_bridge_MEMORY_s1_reset_n;
  wire             pipeline_bridge_MEMORY_s1_waitrequest;
  wire             pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  wire             pipeline_bridge_MEMORY_s1_write;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_writedata;
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
  wire    [ 21: 0] sdram_s1_address;
  wire    [  1: 0] sdram_s1_byteenable_n;
  wire             sdram_s1_chipselect;
  wire             sdram_s1_read_n;
  wire    [ 15: 0] sdram_s1_readdata;
  wire    [ 15: 0] sdram_s1_readdata_from_sa;
  wire             sdram_s1_readdatavalid;
  wire             sdram_s1_reset_n;
  wire             sdram_s1_waitrequest;
  wire             sdram_s1_waitrequest_from_sa;
  wire             sdram_s1_write_n;
  wire    [ 15: 0] sdram_s1_writedata;
  wire    [ 22: 0] tiger_burst_0_downstream_address;
  wire    [ 22: 0] tiger_burst_0_downstream_address_to_slave;
  wire    [  7: 0] tiger_burst_0_downstream_arbitrationshare;
  wire             tiger_burst_0_downstream_burstcount;
  wire    [  1: 0] tiger_burst_0_downstream_byteenable;
  wire             tiger_burst_0_downstream_debugaccess;
  wire             tiger_burst_0_downstream_granted_sdram_s1;
  wire             tiger_burst_0_downstream_latency_counter;
  wire    [ 22: 0] tiger_burst_0_downstream_nativeaddress;
  wire             tiger_burst_0_downstream_qualified_request_sdram_s1;
  wire             tiger_burst_0_downstream_read;
  wire             tiger_burst_0_downstream_read_data_valid_sdram_s1;
  wire             tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register;
  wire    [ 15: 0] tiger_burst_0_downstream_readdata;
  wire             tiger_burst_0_downstream_readdatavalid;
  wire             tiger_burst_0_downstream_requests_sdram_s1;
  wire             tiger_burst_0_downstream_reset_n;
  wire             tiger_burst_0_downstream_waitrequest;
  wire             tiger_burst_0_downstream_write;
  wire    [ 15: 0] tiger_burst_0_downstream_writedata;
  wire    [ 22: 0] tiger_burst_0_upstream_address;
  wire    [  6: 0] tiger_burst_0_upstream_burstcount;
  wire    [ 23: 0] tiger_burst_0_upstream_byteaddress;
  wire    [  1: 0] tiger_burst_0_upstream_byteenable;
  wire             tiger_burst_0_upstream_debugaccess;
  wire             tiger_burst_0_upstream_read;
  wire    [ 15: 0] tiger_burst_0_upstream_readdata;
  wire    [ 15: 0] tiger_burst_0_upstream_readdata_from_sa;
  wire             tiger_burst_0_upstream_readdatavalid;
  wire             tiger_burst_0_upstream_waitrequest;
  wire             tiger_burst_0_upstream_waitrequest_from_sa;
  wire             tiger_burst_0_upstream_write;
  wire    [ 15: 0] tiger_burst_0_upstream_writedata;
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
  wire    [ 31: 0] tiger_top_0_instructionMaster_readdata;
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
  wire    [ 11: 0] zs_addr_from_the_sdram;
  wire    [  1: 0] zs_ba_from_the_sdram;
  wire             zs_cas_n_from_the_sdram;
  wire             zs_cke_from_the_sdram;
  wire             zs_cs_n_from_the_sdram;
  wire    [ 15: 0] zs_dq_to_and_from_the_sdram;
  wire    [  1: 0] zs_dqm_from_the_sdram;
  wire             zs_ras_n_from_the_sdram;
  wire             zs_we_n_from_the_sdram;
  data_cache_0_CACHE0_arbitrator the_data_cache_0_CACHE0
    (
      .clk                                                     (clk),
      .d1_data_cache_0_CACHE0_end_xfer                         (d1_data_cache_0_CACHE0_end_xfer),
      .data_cache_0_CACHE0_begintransfer                       (data_cache_0_CACHE0_begintransfer),
      .data_cache_0_CACHE0_read                                (data_cache_0_CACHE0_read),
      .data_cache_0_CACHE0_readdata                            (data_cache_0_CACHE0_readdata),
      .data_cache_0_CACHE0_readdata_from_sa                    (data_cache_0_CACHE0_readdata_from_sa),
      .data_cache_0_CACHE0_waitrequest                         (data_cache_0_CACHE0_waitrequest),
      .data_cache_0_CACHE0_waitrequest_from_sa                 (data_cache_0_CACHE0_waitrequest_from_sa),
      .data_cache_0_CACHE0_write                               (data_cache_0_CACHE0_write),
      .data_cache_0_CACHE0_writedata                           (data_cache_0_CACHE0_writedata),
      .reset_n                                                 (clk_reset_n),
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
      .clk                       (clk),
      .data_cache_0_PROC_data    (data_cache_0_PROC_data),
      .data_cache_0_PROC_reset_n (data_cache_0_PROC_reset_n),
      .reset_n                   (clk_reset_n)
    );

  data_cache_0_dataMaster0_arbitrator the_data_cache_0_dataMaster0
    (
      .clk                                                                               (clk),
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
      .reset_n                                                                           (clk_reset_n)
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
      .csi_clockreset_clk                 (clk),
      .csi_clockreset_reset_n             (data_cache_0_PROC_reset_n)
    );

  jtag_to_ava_master_bridge_master_arbitrator the_jtag_to_ava_master_bridge_master
    (
      .clk                                                               (clk),
      .d1_sdram_s1_end_xfer                                              (d1_sdram_s1_end_xfer),
      .d1_tiger_top_0_leapSlave_end_xfer                                 (d1_tiger_top_0_leapSlave_end_xfer),
      .jtag_to_ava_master_bridge_byteenable_sdram_s1                     (jtag_to_ava_master_bridge_byteenable_sdram_s1),
      .jtag_to_ava_master_bridge_dbs_address                             (jtag_to_ava_master_bridge_dbs_address),
      .jtag_to_ava_master_bridge_dbs_write_16                            (jtag_to_ava_master_bridge_dbs_write_16),
      .jtag_to_ava_master_bridge_granted_sdram_s1                        (jtag_to_ava_master_bridge_granted_sdram_s1),
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
      .jtag_to_ava_master_bridge_qualified_request_sdram_s1              (jtag_to_ava_master_bridge_qualified_request_sdram_s1),
      .jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave (jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_read_data_valid_sdram_s1                (jtag_to_ava_master_bridge_read_data_valid_sdram_s1),
      .jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register (jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register),
      .jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave   (jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_requests_sdram_s1                       (jtag_to_ava_master_bridge_requests_sdram_s1),
      .jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave          (jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave),
      .reset_n                                                           (clk_reset_n),
      .sdram_s1_readdata_from_sa                                         (sdram_s1_readdata_from_sa),
      .sdram_s1_waitrequest_from_sa                                      (sdram_s1_waitrequest_from_sa),
      .tiger_top_0_leapSlave_readdata_from_sa                            (tiger_top_0_leapSlave_readdata_from_sa)
    );

  jtag_to_ava_master_bridge the_jtag_to_ava_master_bridge
    (
      .clk_clk              (clk),
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
      .clk                                                                                    (clk),
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
      .reset_n                                                                                (clk_reset_n),
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
      .clk                                                                             (clk),
      .d1_tiger_burst_0_upstream_end_xfer                                              (d1_tiger_burst_0_upstream_end_xfer),
      .pipeline_bridge_MEMORY_m1_address                                               (pipeline_bridge_MEMORY_m1_address),
      .pipeline_bridge_MEMORY_m1_address_to_slave                                      (pipeline_bridge_MEMORY_m1_address_to_slave),
      .pipeline_bridge_MEMORY_m1_burstcount                                            (pipeline_bridge_MEMORY_m1_burstcount),
      .pipeline_bridge_MEMORY_m1_byteenable                                            (pipeline_bridge_MEMORY_m1_byteenable),
      .pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream                     (pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_chipselect                                            (pipeline_bridge_MEMORY_m1_chipselect),
      .pipeline_bridge_MEMORY_m1_dbs_address                                           (pipeline_bridge_MEMORY_m1_dbs_address),
      .pipeline_bridge_MEMORY_m1_dbs_write_16                                          (pipeline_bridge_MEMORY_m1_dbs_write_16),
      .pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream                        (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_latency_counter                                       (pipeline_bridge_MEMORY_m1_latency_counter),
      .pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream              (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_read                                                  (pipeline_bridge_MEMORY_m1_read),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream                (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_readdata                                              (pipeline_bridge_MEMORY_m1_readdata),
      .pipeline_bridge_MEMORY_m1_readdatavalid                                         (pipeline_bridge_MEMORY_m1_readdatavalid),
      .pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream                       (pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_waitrequest                                           (pipeline_bridge_MEMORY_m1_waitrequest),
      .pipeline_bridge_MEMORY_m1_write                                                 (pipeline_bridge_MEMORY_m1_write),
      .pipeline_bridge_MEMORY_m1_writedata                                             (pipeline_bridge_MEMORY_m1_writedata),
      .reset_n                                                                         (clk_reset_n),
      .tiger_burst_0_upstream_readdata_from_sa                                         (tiger_burst_0_upstream_readdata_from_sa),
      .tiger_burst_0_upstream_waitrequest_from_sa                                      (tiger_burst_0_upstream_waitrequest_from_sa)
    );

  pipeline_bridge_MEMORY the_pipeline_bridge_MEMORY
    (
      .clk              (clk),
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
      .clk                                                                                  (clk),
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
      .reset_n                                                                              (clk_reset_n),
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
      .clk                                                        (clk),
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
      .reset_n                                                    (clk_reset_n),
      .uart_0_s1_readdata_from_sa                                 (uart_0_s1_readdata_from_sa)
    );

  pipeline_bridge_PERIPHERALS the_pipeline_bridge_PERIPHERALS
    (
      .clk              (clk),
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

  sdram_s1_arbitrator the_sdram_s1
    (
      .clk                                                               (clk),
      .d1_sdram_s1_end_xfer                                              (d1_sdram_s1_end_xfer),
      .jtag_to_ava_master_bridge_byteenable_sdram_s1                     (jtag_to_ava_master_bridge_byteenable_sdram_s1),
      .jtag_to_ava_master_bridge_dbs_address                             (jtag_to_ava_master_bridge_dbs_address),
      .jtag_to_ava_master_bridge_dbs_write_16                            (jtag_to_ava_master_bridge_dbs_write_16),
      .jtag_to_ava_master_bridge_granted_sdram_s1                        (jtag_to_ava_master_bridge_granted_sdram_s1),
      .jtag_to_ava_master_bridge_latency_counter                         (jtag_to_ava_master_bridge_latency_counter),
      .jtag_to_ava_master_bridge_master_address_to_slave                 (jtag_to_ava_master_bridge_master_address_to_slave),
      .jtag_to_ava_master_bridge_master_byteenable                       (jtag_to_ava_master_bridge_master_byteenable),
      .jtag_to_ava_master_bridge_master_read                             (jtag_to_ava_master_bridge_master_read),
      .jtag_to_ava_master_bridge_master_write                            (jtag_to_ava_master_bridge_master_write),
      .jtag_to_ava_master_bridge_qualified_request_sdram_s1              (jtag_to_ava_master_bridge_qualified_request_sdram_s1),
      .jtag_to_ava_master_bridge_read_data_valid_sdram_s1                (jtag_to_ava_master_bridge_read_data_valid_sdram_s1),
      .jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register (jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register),
      .jtag_to_ava_master_bridge_requests_sdram_s1                       (jtag_to_ava_master_bridge_requests_sdram_s1),
      .reset_n                                                           (clk_reset_n),
      .sdram_s1_address                                                  (sdram_s1_address),
      .sdram_s1_byteenable_n                                             (sdram_s1_byteenable_n),
      .sdram_s1_chipselect                                               (sdram_s1_chipselect),
      .sdram_s1_read_n                                                   (sdram_s1_read_n),
      .sdram_s1_readdata                                                 (sdram_s1_readdata),
      .sdram_s1_readdata_from_sa                                         (sdram_s1_readdata_from_sa),
      .sdram_s1_readdatavalid                                            (sdram_s1_readdatavalid),
      .sdram_s1_reset_n                                                  (sdram_s1_reset_n),
      .sdram_s1_waitrequest                                              (sdram_s1_waitrequest),
      .sdram_s1_waitrequest_from_sa                                      (sdram_s1_waitrequest_from_sa),
      .sdram_s1_write_n                                                  (sdram_s1_write_n),
      .sdram_s1_writedata                                                (sdram_s1_writedata),
      .tiger_burst_0_downstream_address_to_slave                         (tiger_burst_0_downstream_address_to_slave),
      .tiger_burst_0_downstream_arbitrationshare                         (tiger_burst_0_downstream_arbitrationshare),
      .tiger_burst_0_downstream_burstcount                               (tiger_burst_0_downstream_burstcount),
      .tiger_burst_0_downstream_byteenable                               (tiger_burst_0_downstream_byteenable),
      .tiger_burst_0_downstream_granted_sdram_s1                         (tiger_burst_0_downstream_granted_sdram_s1),
      .tiger_burst_0_downstream_latency_counter                          (tiger_burst_0_downstream_latency_counter),
      .tiger_burst_0_downstream_qualified_request_sdram_s1               (tiger_burst_0_downstream_qualified_request_sdram_s1),
      .tiger_burst_0_downstream_read                                     (tiger_burst_0_downstream_read),
      .tiger_burst_0_downstream_read_data_valid_sdram_s1                 (tiger_burst_0_downstream_read_data_valid_sdram_s1),
      .tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register  (tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register),
      .tiger_burst_0_downstream_requests_sdram_s1                        (tiger_burst_0_downstream_requests_sdram_s1),
      .tiger_burst_0_downstream_write                                    (tiger_burst_0_downstream_write),
      .tiger_burst_0_downstream_writedata                                (tiger_burst_0_downstream_writedata)
    );

  sdram the_sdram
    (
      .az_addr        (sdram_s1_address),
      .az_be_n        (sdram_s1_byteenable_n),
      .az_cs          (sdram_s1_chipselect),
      .az_data        (sdram_s1_writedata),
      .az_rd_n        (sdram_s1_read_n),
      .az_wr_n        (sdram_s1_write_n),
      .clk            (clk),
      .reset_n        (sdram_s1_reset_n),
      .za_data        (sdram_s1_readdata),
      .za_valid       (sdram_s1_readdatavalid),
      .za_waitrequest (sdram_s1_waitrequest),
      .zs_addr        (zs_addr_from_the_sdram),
      .zs_ba          (zs_ba_from_the_sdram),
      .zs_cas_n       (zs_cas_n_from_the_sdram),
      .zs_cke         (zs_cke_from_the_sdram),
      .zs_cs_n        (zs_cs_n_from_the_sdram),
      .zs_dq          (zs_dq_to_and_from_the_sdram),
      .zs_dqm         (zs_dqm_from_the_sdram),
      .zs_ras_n       (zs_ras_n_from_the_sdram),
      .zs_we_n        (zs_we_n_from_the_sdram)
    );

  tiger_burst_0_upstream_arbitrator the_tiger_burst_0_upstream
    (
      .clk                                                                             (clk),
      .d1_tiger_burst_0_upstream_end_xfer                                              (d1_tiger_burst_0_upstream_end_xfer),
      .pipeline_bridge_MEMORY_m1_address_to_slave                                      (pipeline_bridge_MEMORY_m1_address_to_slave),
      .pipeline_bridge_MEMORY_m1_burstcount                                            (pipeline_bridge_MEMORY_m1_burstcount),
      .pipeline_bridge_MEMORY_m1_byteenable                                            (pipeline_bridge_MEMORY_m1_byteenable),
      .pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream                     (pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_chipselect                                            (pipeline_bridge_MEMORY_m1_chipselect),
      .pipeline_bridge_MEMORY_m1_dbs_address                                           (pipeline_bridge_MEMORY_m1_dbs_address),
      .pipeline_bridge_MEMORY_m1_dbs_write_16                                          (pipeline_bridge_MEMORY_m1_dbs_write_16),
      .pipeline_bridge_MEMORY_m1_debugaccess                                           (pipeline_bridge_MEMORY_m1_debugaccess),
      .pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream                        (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_latency_counter                                       (pipeline_bridge_MEMORY_m1_latency_counter),
      .pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream              (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_read                                                  (pipeline_bridge_MEMORY_m1_read),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream                (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream                       (pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_write                                                 (pipeline_bridge_MEMORY_m1_write),
      .reset_n                                                                         (clk_reset_n),
      .tiger_burst_0_upstream_address                                                  (tiger_burst_0_upstream_address),
      .tiger_burst_0_upstream_burstcount                                               (tiger_burst_0_upstream_burstcount),
      .tiger_burst_0_upstream_byteaddress                                              (tiger_burst_0_upstream_byteaddress),
      .tiger_burst_0_upstream_byteenable                                               (tiger_burst_0_upstream_byteenable),
      .tiger_burst_0_upstream_debugaccess                                              (tiger_burst_0_upstream_debugaccess),
      .tiger_burst_0_upstream_read                                                     (tiger_burst_0_upstream_read),
      .tiger_burst_0_upstream_readdata                                                 (tiger_burst_0_upstream_readdata),
      .tiger_burst_0_upstream_readdata_from_sa                                         (tiger_burst_0_upstream_readdata_from_sa),
      .tiger_burst_0_upstream_readdatavalid                                            (tiger_burst_0_upstream_readdatavalid),
      .tiger_burst_0_upstream_waitrequest                                              (tiger_burst_0_upstream_waitrequest),
      .tiger_burst_0_upstream_waitrequest_from_sa                                      (tiger_burst_0_upstream_waitrequest_from_sa),
      .tiger_burst_0_upstream_write                                                    (tiger_burst_0_upstream_write),
      .tiger_burst_0_upstream_writedata                                                (tiger_burst_0_upstream_writedata)
    );

  tiger_burst_0_downstream_arbitrator the_tiger_burst_0_downstream
    (
      .clk                                                              (clk),
      .d1_sdram_s1_end_xfer                                             (d1_sdram_s1_end_xfer),
      .reset_n                                                          (clk_reset_n),
      .sdram_s1_readdata_from_sa                                        (sdram_s1_readdata_from_sa),
      .sdram_s1_waitrequest_from_sa                                     (sdram_s1_waitrequest_from_sa),
      .tiger_burst_0_downstream_address                                 (tiger_burst_0_downstream_address),
      .tiger_burst_0_downstream_address_to_slave                        (tiger_burst_0_downstream_address_to_slave),
      .tiger_burst_0_downstream_burstcount                              (tiger_burst_0_downstream_burstcount),
      .tiger_burst_0_downstream_byteenable                              (tiger_burst_0_downstream_byteenable),
      .tiger_burst_0_downstream_granted_sdram_s1                        (tiger_burst_0_downstream_granted_sdram_s1),
      .tiger_burst_0_downstream_latency_counter                         (tiger_burst_0_downstream_latency_counter),
      .tiger_burst_0_downstream_qualified_request_sdram_s1              (tiger_burst_0_downstream_qualified_request_sdram_s1),
      .tiger_burst_0_downstream_read                                    (tiger_burst_0_downstream_read),
      .tiger_burst_0_downstream_read_data_valid_sdram_s1                (tiger_burst_0_downstream_read_data_valid_sdram_s1),
      .tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register (tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register),
      .tiger_burst_0_downstream_readdata                                (tiger_burst_0_downstream_readdata),
      .tiger_burst_0_downstream_readdatavalid                           (tiger_burst_0_downstream_readdatavalid),
      .tiger_burst_0_downstream_requests_sdram_s1                       (tiger_burst_0_downstream_requests_sdram_s1),
      .tiger_burst_0_downstream_reset_n                                 (tiger_burst_0_downstream_reset_n),
      .tiger_burst_0_downstream_waitrequest                             (tiger_burst_0_downstream_waitrequest),
      .tiger_burst_0_downstream_write                                   (tiger_burst_0_downstream_write),
      .tiger_burst_0_downstream_writedata                               (tiger_burst_0_downstream_writedata)
    );

  tiger_burst_0 the_tiger_burst_0
    (
      .clk                         (clk),
      .downstream_address          (tiger_burst_0_downstream_address),
      .downstream_arbitrationshare (tiger_burst_0_downstream_arbitrationshare),
      .downstream_burstcount       (tiger_burst_0_downstream_burstcount),
      .downstream_byteenable       (tiger_burst_0_downstream_byteenable),
      .downstream_debugaccess      (tiger_burst_0_downstream_debugaccess),
      .downstream_nativeaddress    (tiger_burst_0_downstream_nativeaddress),
      .downstream_read             (tiger_burst_0_downstream_read),
      .downstream_readdata         (tiger_burst_0_downstream_readdata),
      .downstream_readdatavalid    (tiger_burst_0_downstream_readdatavalid),
      .downstream_waitrequest      (tiger_burst_0_downstream_waitrequest),
      .downstream_write            (tiger_burst_0_downstream_write),
      .downstream_writedata        (tiger_burst_0_downstream_writedata),
      .reset_n                     (tiger_burst_0_downstream_reset_n),
      .upstream_address            (tiger_burst_0_upstream_byteaddress),
      .upstream_burstcount         (tiger_burst_0_upstream_burstcount),
      .upstream_byteenable         (tiger_burst_0_upstream_byteenable),
      .upstream_debugaccess        (tiger_burst_0_upstream_debugaccess),
      .upstream_nativeaddress      (tiger_burst_0_upstream_address),
      .upstream_read               (tiger_burst_0_upstream_read),
      .upstream_readdata           (tiger_burst_0_upstream_readdata),
      .upstream_readdatavalid      (tiger_burst_0_upstream_readdatavalid),
      .upstream_waitrequest        (tiger_burst_0_upstream_waitrequest),
      .upstream_write              (tiger_burst_0_upstream_write),
      .upstream_writedata          (tiger_burst_0_upstream_writedata)
    );

  tiger_top_0_PROC_arbitrator the_tiger_top_0_PROC
    (
      .clk                    (clk),
      .data_cache_0_PROC_data (data_cache_0_PROC_data),
      .reset_n                (clk_reset_n),
      .tiger_top_0_PROC_data  (tiger_top_0_PROC_data)
    );

  tiger_top_0_leapSlave_arbitrator the_tiger_top_0_leapSlave
    (
      .clk                                                               (clk),
      .d1_tiger_top_0_leapSlave_end_xfer                                 (d1_tiger_top_0_leapSlave_end_xfer),
      .jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave           (jtag_to_ava_master_bridge_granted_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_latency_counter                         (jtag_to_ava_master_bridge_latency_counter),
      .jtag_to_ava_master_bridge_master_address_to_slave                 (jtag_to_ava_master_bridge_master_address_to_slave),
      .jtag_to_ava_master_bridge_master_read                             (jtag_to_ava_master_bridge_master_read),
      .jtag_to_ava_master_bridge_master_write                            (jtag_to_ava_master_bridge_master_write),
      .jtag_to_ava_master_bridge_master_writedata                        (jtag_to_ava_master_bridge_master_writedata),
      .jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave (jtag_to_ava_master_bridge_qualified_request_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register (jtag_to_ava_master_bridge_read_data_valid_sdram_s1_shift_register),
      .jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave   (jtag_to_ava_master_bridge_read_data_valid_tiger_top_0_leapSlave),
      .jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave          (jtag_to_ava_master_bridge_requests_tiger_top_0_leapSlave),
      .reset_n                                                           (clk_reset_n),
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
      .clk                                                     (clk),
      .d1_data_cache_0_CACHE0_end_xfer                         (d1_data_cache_0_CACHE0_end_xfer),
      .data_cache_0_CACHE0_readdata_from_sa                    (data_cache_0_CACHE0_readdata_from_sa),
      .data_cache_0_CACHE0_waitrequest_from_sa                 (data_cache_0_CACHE0_waitrequest_from_sa),
      .reset_n                                                 (clk_reset_n),
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
      .clk                                                                                    (clk),
      .d1_pipeline_bridge_MEMORY_s1_end_xfer                                                  (d1_pipeline_bridge_MEMORY_s1_end_xfer),
      .pipeline_bridge_MEMORY_s1_readdata_from_sa                                             (pipeline_bridge_MEMORY_s1_readdata_from_sa),
      .pipeline_bridge_MEMORY_s1_waitrequest_from_sa                                          (pipeline_bridge_MEMORY_s1_waitrequest_from_sa),
      .reset_n                                                                                (clk_reset_n),
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
      .clk                                                                                  (clk),
      .d1_pipeline_bridge_PERIPHERALS_s1_end_xfer                                           (d1_pipeline_bridge_PERIPHERALS_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_s1_readdata_from_sa                                      (pipeline_bridge_PERIPHERALS_s1_readdata_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa                                   (pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa),
      .reset_n                                                                              (clk_reset_n),
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
      .clk                                      (clk),
      .coe_debug_lights                         (coe_debug_lights_from_the_tiger_top_0),
      .coe_debug_select                         (coe_debug_select_to_the_tiger_top_0),
      .coe_exe_end                              (coe_exe_end_from_the_tiger_top_0),
      .coe_exe_start                            (coe_exe_start_from_the_tiger_top_0),
      .reset                                    (tiger_top_0_CACHE_reset)
    );

  uart_0_s1_arbitrator the_uart_0_s1
    (
      .clk                                                        (clk),
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
      .reset_n                                                    (clk_reset_n),
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
      .clk           (clk),
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
    jtag_to_ava_master_bridge_master_resetrequest |
    jtag_to_ava_master_bridge_master_resetrequest);

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
`include "data_cache.v"
`include "data_cache_0.v"
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
`include "tiger_burst_0.v"
`include "pipeline_bridge_MEMORY.v"
`include "sdram.v"
`include "sdram_test_component.v"
`include "uart_0.v"
`include "pipeline_bridge_PERIPHERALS.v"

`timescale 1ns / 1ps

module test_bench 
;


  reg              clk;
  wire    [ 17: 0] coe_debug_lights_from_the_tiger_top_0;
  wire    [  2: 0] coe_debug_select_to_the_tiger_top_0;
  wire             coe_exe_end_from_the_tiger_top_0;
  wire             coe_exe_start_from_the_tiger_top_0;
  wire             data_cache_0_dataMaster0_beginbursttransfer;
  wire             pipeline_bridge_MEMORY_m1_endofpacket;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire             pipeline_bridge_PERIPHERALS_m1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_m1_endofpacket;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  reg              reset_n;
  wire             rxd_to_the_uart_0;
  wire             tiger_burst_0_downstream_debugaccess;
  wire    [ 22: 0] tiger_burst_0_downstream_nativeaddress;
  wire             tiger_top_0_instructionMaster_beginbursttransfer;
  wire             txd_from_the_uart_0;
  wire             uart_0_s1_dataavailable_from_sa;
  wire             uart_0_s1_irq;
  wire             uart_0_s1_readyfordata_from_sa;
  wire    [ 11: 0] zs_addr_from_the_sdram;
  wire    [  1: 0] zs_ba_from_the_sdram;
  wire             zs_cas_n_from_the_sdram;
  wire             zs_cke_from_the_sdram;
  wire             zs_cs_n_from_the_sdram;
  wire    [ 15: 0] zs_dq_to_and_from_the_sdram;
  wire    [  1: 0] zs_dqm_from_the_sdram;
  wire             zs_ras_n_from_the_sdram;
  wire             zs_we_n_from_the_sdram;


// <ALTERA_NOTE> CODE INSERTED BETWEEN HERE
//  add your signals and additional architecture here
// AND HERE WILL BE PRESERVED </ALTERA_NOTE>

  //Set us up the Dut
  tiger DUT
    (
      .clk                                   (clk),
      .coe_debug_lights_from_the_tiger_top_0 (coe_debug_lights_from_the_tiger_top_0),
      .coe_debug_select_to_the_tiger_top_0   (coe_debug_select_to_the_tiger_top_0),
      .coe_exe_end_from_the_tiger_top_0      (coe_exe_end_from_the_tiger_top_0),
      .coe_exe_start_from_the_tiger_top_0    (coe_exe_start_from_the_tiger_top_0),
      .reset_n                               (reset_n),
      .rxd_to_the_uart_0                     (rxd_to_the_uart_0),
      .txd_from_the_uart_0                   (txd_from_the_uart_0),
      .zs_addr_from_the_sdram                (zs_addr_from_the_sdram),
      .zs_ba_from_the_sdram                  (zs_ba_from_the_sdram),
      .zs_cas_n_from_the_sdram               (zs_cas_n_from_the_sdram),
      .zs_cke_from_the_sdram                 (zs_cke_from_the_sdram),
      .zs_cs_n_from_the_sdram                (zs_cs_n_from_the_sdram),
      .zs_dq_to_and_from_the_sdram           (zs_dq_to_and_from_the_sdram),
      .zs_dqm_from_the_sdram                 (zs_dqm_from_the_sdram),
      .zs_ras_n_from_the_sdram               (zs_ras_n_from_the_sdram),
      .zs_we_n_from_the_sdram                (zs_we_n_from_the_sdram)
    );

  sdram_test_component the_sdram_test_component
    (
      .clk      (clk),
      .zs_addr  (zs_addr_from_the_sdram),
      .zs_ba    (zs_ba_from_the_sdram),
      .zs_cas_n (zs_cas_n_from_the_sdram),
      .zs_cke   (zs_cke_from_the_sdram),
      .zs_cs_n  (zs_cs_n_from_the_sdram),
      .zs_dq    (zs_dq_to_and_from_the_sdram),
      .zs_dqm   (zs_dqm_from_the_sdram),
      .zs_ras_n (zs_ras_n_from_the_sdram),
      .zs_we_n  (zs_we_n_from_the_sdram)
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
