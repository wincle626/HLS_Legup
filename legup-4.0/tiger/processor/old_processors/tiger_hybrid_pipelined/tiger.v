//megafunction wizard: %Altera SOPC Builder%
//GENERATION: STANDARD
//VERSION: WM1.0


//Legal Notice: (C)2011 Altera Corporation. All rights reserved.  Your
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

module data_cache_0_ACCEL_arbitrator (
                                       // inputs:
                                        clk,
                                        data_cache_0_ACCEL_readdata,
                                        data_cache_0_ACCEL_waitrequest,
                                        reset_n,
                                        tiger_burst_2_downstream_address_to_slave,
                                        tiger_burst_2_downstream_arbitrationshare,
                                        tiger_burst_2_downstream_burstcount,
                                        tiger_burst_2_downstream_latency_counter,
                                        tiger_burst_2_downstream_read,
                                        tiger_burst_2_downstream_write,
                                        tiger_burst_2_downstream_writedata,

                                       // outputs:
                                        d1_data_cache_0_ACCEL_end_xfer,
                                        data_cache_0_ACCEL_address,
                                        data_cache_0_ACCEL_begintransfer,
                                        data_cache_0_ACCEL_read,
                                        data_cache_0_ACCEL_readdata_from_sa,
                                        data_cache_0_ACCEL_waitrequest_from_sa,
                                        data_cache_0_ACCEL_write,
                                        data_cache_0_ACCEL_writedata,
                                        tiger_burst_2_downstream_granted_data_cache_0_ACCEL,
                                        tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL,
                                        tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL,
                                        tiger_burst_2_downstream_requests_data_cache_0_ACCEL
                                     )
;

  output           d1_data_cache_0_ACCEL_end_xfer;
  output  [  2: 0] data_cache_0_ACCEL_address;
  output           data_cache_0_ACCEL_begintransfer;
  output           data_cache_0_ACCEL_read;
  output  [127: 0] data_cache_0_ACCEL_readdata_from_sa;
  output           data_cache_0_ACCEL_waitrequest_from_sa;
  output           data_cache_0_ACCEL_write;
  output  [127: 0] data_cache_0_ACCEL_writedata;
  output           tiger_burst_2_downstream_granted_data_cache_0_ACCEL;
  output           tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL;
  output           tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL;
  output           tiger_burst_2_downstream_requests_data_cache_0_ACCEL;
  input            clk;
  input   [127: 0] data_cache_0_ACCEL_readdata;
  input            data_cache_0_ACCEL_waitrequest;
  input            reset_n;
  input   [  6: 0] tiger_burst_2_downstream_address_to_slave;
  input   [  2: 0] tiger_burst_2_downstream_arbitrationshare;
  input            tiger_burst_2_downstream_burstcount;
  input            tiger_burst_2_downstream_latency_counter;
  input            tiger_burst_2_downstream_read;
  input            tiger_burst_2_downstream_write;
  input   [127: 0] tiger_burst_2_downstream_writedata;

  reg              d1_data_cache_0_ACCEL_end_xfer;
  reg              d1_reasons_to_wait;
  wire    [  2: 0] data_cache_0_ACCEL_address;
  wire             data_cache_0_ACCEL_allgrants;
  wire             data_cache_0_ACCEL_allow_new_arb_cycle;
  wire             data_cache_0_ACCEL_any_bursting_master_saved_grant;
  wire             data_cache_0_ACCEL_any_continuerequest;
  wire             data_cache_0_ACCEL_arb_counter_enable;
  reg     [  2: 0] data_cache_0_ACCEL_arb_share_counter;
  wire    [  2: 0] data_cache_0_ACCEL_arb_share_counter_next_value;
  wire    [  2: 0] data_cache_0_ACCEL_arb_share_set_values;
  wire             data_cache_0_ACCEL_beginbursttransfer_internal;
  wire             data_cache_0_ACCEL_begins_xfer;
  wire             data_cache_0_ACCEL_begintransfer;
  wire             data_cache_0_ACCEL_end_xfer;
  wire             data_cache_0_ACCEL_firsttransfer;
  wire             data_cache_0_ACCEL_grant_vector;
  wire             data_cache_0_ACCEL_in_a_read_cycle;
  wire             data_cache_0_ACCEL_in_a_write_cycle;
  wire             data_cache_0_ACCEL_master_qreq_vector;
  wire             data_cache_0_ACCEL_non_bursting_master_requests;
  wire             data_cache_0_ACCEL_read;
  wire    [127: 0] data_cache_0_ACCEL_readdata_from_sa;
  reg              data_cache_0_ACCEL_reg_firsttransfer;
  reg              data_cache_0_ACCEL_slavearbiterlockenable;
  wire             data_cache_0_ACCEL_slavearbiterlockenable2;
  wire             data_cache_0_ACCEL_unreg_firsttransfer;
  wire             data_cache_0_ACCEL_waitrequest_from_sa;
  wire             data_cache_0_ACCEL_waits_for_read;
  wire             data_cache_0_ACCEL_waits_for_write;
  wire             data_cache_0_ACCEL_write;
  wire    [127: 0] data_cache_0_ACCEL_writedata;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_data_cache_0_ACCEL;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire    [  6: 0] shifted_address_to_data_cache_0_ACCEL_from_tiger_burst_2_downstream;
  wire             tiger_burst_2_downstream_arbiterlock;
  wire             tiger_burst_2_downstream_arbiterlock2;
  wire             tiger_burst_2_downstream_continuerequest;
  wire             tiger_burst_2_downstream_granted_data_cache_0_ACCEL;
  wire             tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL;
  wire             tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL;
  wire             tiger_burst_2_downstream_requests_data_cache_0_ACCEL;
  wire             tiger_burst_2_downstream_saved_grant_data_cache_0_ACCEL;
  wire             wait_for_data_cache_0_ACCEL_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~data_cache_0_ACCEL_end_xfer;
    end


  assign data_cache_0_ACCEL_begins_xfer = ~d1_reasons_to_wait & ((tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL));
  //assign data_cache_0_ACCEL_readdata_from_sa = data_cache_0_ACCEL_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign data_cache_0_ACCEL_readdata_from_sa = data_cache_0_ACCEL_readdata;

  assign tiger_burst_2_downstream_requests_data_cache_0_ACCEL = (1) & (tiger_burst_2_downstream_read | tiger_burst_2_downstream_write);
  //assign data_cache_0_ACCEL_waitrequest_from_sa = data_cache_0_ACCEL_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign data_cache_0_ACCEL_waitrequest_from_sa = data_cache_0_ACCEL_waitrequest;

  //data_cache_0_ACCEL_arb_share_counter set values, which is an e_mux
  assign data_cache_0_ACCEL_arb_share_set_values = (tiger_burst_2_downstream_granted_data_cache_0_ACCEL)? tiger_burst_2_downstream_arbitrationshare :
    1;

  //data_cache_0_ACCEL_non_bursting_master_requests mux, which is an e_mux
  assign data_cache_0_ACCEL_non_bursting_master_requests = 0;

  //data_cache_0_ACCEL_any_bursting_master_saved_grant mux, which is an e_mux
  assign data_cache_0_ACCEL_any_bursting_master_saved_grant = tiger_burst_2_downstream_saved_grant_data_cache_0_ACCEL;

  //data_cache_0_ACCEL_arb_share_counter_next_value assignment, which is an e_assign
  assign data_cache_0_ACCEL_arb_share_counter_next_value = data_cache_0_ACCEL_firsttransfer ? (data_cache_0_ACCEL_arb_share_set_values - 1) : |data_cache_0_ACCEL_arb_share_counter ? (data_cache_0_ACCEL_arb_share_counter - 1) : 0;

  //data_cache_0_ACCEL_allgrants all slave grants, which is an e_mux
  assign data_cache_0_ACCEL_allgrants = |data_cache_0_ACCEL_grant_vector;

  //data_cache_0_ACCEL_end_xfer assignment, which is an e_assign
  assign data_cache_0_ACCEL_end_xfer = ~(data_cache_0_ACCEL_waits_for_read | data_cache_0_ACCEL_waits_for_write);

  //end_xfer_arb_share_counter_term_data_cache_0_ACCEL arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_data_cache_0_ACCEL = data_cache_0_ACCEL_end_xfer & (~data_cache_0_ACCEL_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //data_cache_0_ACCEL_arb_share_counter arbitration counter enable, which is an e_assign
  assign data_cache_0_ACCEL_arb_counter_enable = (end_xfer_arb_share_counter_term_data_cache_0_ACCEL & data_cache_0_ACCEL_allgrants) | (end_xfer_arb_share_counter_term_data_cache_0_ACCEL & ~data_cache_0_ACCEL_non_bursting_master_requests);

  //data_cache_0_ACCEL_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_ACCEL_arb_share_counter <= 0;
      else if (data_cache_0_ACCEL_arb_counter_enable)
          data_cache_0_ACCEL_arb_share_counter <= data_cache_0_ACCEL_arb_share_counter_next_value;
    end


  //data_cache_0_ACCEL_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_ACCEL_slavearbiterlockenable <= 0;
      else if ((|data_cache_0_ACCEL_master_qreq_vector & end_xfer_arb_share_counter_term_data_cache_0_ACCEL) | (end_xfer_arb_share_counter_term_data_cache_0_ACCEL & ~data_cache_0_ACCEL_non_bursting_master_requests))
          data_cache_0_ACCEL_slavearbiterlockenable <= |data_cache_0_ACCEL_arb_share_counter_next_value;
    end


  //tiger_burst_2/downstream data_cache_0/ACCEL arbiterlock, which is an e_assign
  assign tiger_burst_2_downstream_arbiterlock = data_cache_0_ACCEL_slavearbiterlockenable & tiger_burst_2_downstream_continuerequest;

  //data_cache_0_ACCEL_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign data_cache_0_ACCEL_slavearbiterlockenable2 = |data_cache_0_ACCEL_arb_share_counter_next_value;

  //tiger_burst_2/downstream data_cache_0/ACCEL arbiterlock2, which is an e_assign
  assign tiger_burst_2_downstream_arbiterlock2 = data_cache_0_ACCEL_slavearbiterlockenable2 & tiger_burst_2_downstream_continuerequest;

  //data_cache_0_ACCEL_any_continuerequest at least one master continues requesting, which is an e_assign
  assign data_cache_0_ACCEL_any_continuerequest = 1;

  //tiger_burst_2_downstream_continuerequest continued request, which is an e_assign
  assign tiger_burst_2_downstream_continuerequest = 1;

  assign tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL = tiger_burst_2_downstream_requests_data_cache_0_ACCEL & ~((tiger_burst_2_downstream_read & ((tiger_burst_2_downstream_latency_counter != 0))));
  //local readdatavalid tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL, which is an e_mux
  assign tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL = tiger_burst_2_downstream_granted_data_cache_0_ACCEL & tiger_burst_2_downstream_read & ~data_cache_0_ACCEL_waits_for_read;

  //data_cache_0_ACCEL_writedata mux, which is an e_mux
  assign data_cache_0_ACCEL_writedata = tiger_burst_2_downstream_writedata;

  //master is always granted when requested
  assign tiger_burst_2_downstream_granted_data_cache_0_ACCEL = tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL;

  //tiger_burst_2/downstream saved-grant data_cache_0/ACCEL, which is an e_assign
  assign tiger_burst_2_downstream_saved_grant_data_cache_0_ACCEL = tiger_burst_2_downstream_requests_data_cache_0_ACCEL;

  //allow new arb cycle for data_cache_0/ACCEL, which is an e_assign
  assign data_cache_0_ACCEL_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign data_cache_0_ACCEL_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign data_cache_0_ACCEL_master_qreq_vector = 1;

  assign data_cache_0_ACCEL_begintransfer = data_cache_0_ACCEL_begins_xfer;
  //data_cache_0_ACCEL_firsttransfer first transaction, which is an e_assign
  assign data_cache_0_ACCEL_firsttransfer = data_cache_0_ACCEL_begins_xfer ? data_cache_0_ACCEL_unreg_firsttransfer : data_cache_0_ACCEL_reg_firsttransfer;

  //data_cache_0_ACCEL_unreg_firsttransfer first transaction, which is an e_assign
  assign data_cache_0_ACCEL_unreg_firsttransfer = ~(data_cache_0_ACCEL_slavearbiterlockenable & data_cache_0_ACCEL_any_continuerequest);

  //data_cache_0_ACCEL_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_ACCEL_reg_firsttransfer <= 1'b1;
      else if (data_cache_0_ACCEL_begins_xfer)
          data_cache_0_ACCEL_reg_firsttransfer <= data_cache_0_ACCEL_unreg_firsttransfer;
    end


  //data_cache_0_ACCEL_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign data_cache_0_ACCEL_beginbursttransfer_internal = data_cache_0_ACCEL_begins_xfer;

  //data_cache_0_ACCEL_read assignment, which is an e_mux
  assign data_cache_0_ACCEL_read = tiger_burst_2_downstream_granted_data_cache_0_ACCEL & tiger_burst_2_downstream_read;

  //data_cache_0_ACCEL_write assignment, which is an e_mux
  assign data_cache_0_ACCEL_write = tiger_burst_2_downstream_granted_data_cache_0_ACCEL & tiger_burst_2_downstream_write;

  assign shifted_address_to_data_cache_0_ACCEL_from_tiger_burst_2_downstream = tiger_burst_2_downstream_address_to_slave;
  //data_cache_0_ACCEL_address mux, which is an e_mux
  assign data_cache_0_ACCEL_address = shifted_address_to_data_cache_0_ACCEL_from_tiger_burst_2_downstream >> 4;

  //d1_data_cache_0_ACCEL_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_data_cache_0_ACCEL_end_xfer <= 1;
      else 
        d1_data_cache_0_ACCEL_end_xfer <= data_cache_0_ACCEL_end_xfer;
    end


  //data_cache_0_ACCEL_waits_for_read in a cycle, which is an e_mux
  assign data_cache_0_ACCEL_waits_for_read = data_cache_0_ACCEL_in_a_read_cycle & data_cache_0_ACCEL_waitrequest_from_sa;

  //data_cache_0_ACCEL_in_a_read_cycle assignment, which is an e_assign
  assign data_cache_0_ACCEL_in_a_read_cycle = tiger_burst_2_downstream_granted_data_cache_0_ACCEL & tiger_burst_2_downstream_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = data_cache_0_ACCEL_in_a_read_cycle;

  //data_cache_0_ACCEL_waits_for_write in a cycle, which is an e_mux
  assign data_cache_0_ACCEL_waits_for_write = data_cache_0_ACCEL_in_a_write_cycle & data_cache_0_ACCEL_waitrequest_from_sa;

  //data_cache_0_ACCEL_in_a_write_cycle assignment, which is an e_assign
  assign data_cache_0_ACCEL_in_a_write_cycle = tiger_burst_2_downstream_granted_data_cache_0_ACCEL & tiger_burst_2_downstream_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = data_cache_0_ACCEL_in_a_write_cycle;

  assign wait_for_data_cache_0_ACCEL_counter = 0;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //data_cache_0/ACCEL enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end


  //tiger_burst_2/downstream non-zero arbitrationshare assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_2_downstream_requests_data_cache_0_ACCEL && (tiger_burst_2_downstream_arbitrationshare == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_2/downstream drove 0 on its 'arbitrationshare' port while accessing slave data_cache_0/ACCEL", $time);
          $stop;
        end
    end


  //tiger_burst_2/downstream non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_2_downstream_requests_data_cache_0_ACCEL && (tiger_burst_2_downstream_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_2/downstream drove 0 on its 'burstcount' port while accessing slave data_cache_0/ACCEL", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module data_cache_0_TigertoCache_arbitrator (
                                              // inputs:
                                               clk,
                                               reset_n,
                                               tiger_top_0_TigertoCache_data,

                                              // outputs:
                                               data_cache_0_TigertoCache_data,
                                               data_cache_0_TigertoCache_reset_n
                                            )
;

  output  [ 71: 0] data_cache_0_TigertoCache_data;
  output           data_cache_0_TigertoCache_reset_n;
  input            clk;
  input            reset_n;
  input   [ 71: 0] tiger_top_0_TigertoCache_data;

  wire    [ 71: 0] data_cache_0_TigertoCache_data;
  wire             data_cache_0_TigertoCache_reset_n;
  //mux data_cache_0_TigertoCache_data, which is an e_mux
  assign data_cache_0_TigertoCache_data = tiger_top_0_TigertoCache_data;

  //data_cache_0_TigertoCache_reset_n assignment, which is an e_assign
  assign data_cache_0_TigertoCache_reset_n = reset_n;


endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module data_cache_0_AccelMaster_arbitrator (
                                             // inputs:
                                              clk,
                                              d1_pipeline_bridge_MEMORY_s1_end_xfer,
                                              data_cache_0_AccelMaster_address,
                                              data_cache_0_AccelMaster_burstcount,
                                              data_cache_0_AccelMaster_byteenable,
                                              data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_AccelMaster_read,
                                              data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                              data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1,
                                              data_cache_0_AccelMaster_write,
                                              data_cache_0_AccelMaster_writedata,
                                              pipeline_bridge_MEMORY_s1_readdata_from_sa,
                                              pipeline_bridge_MEMORY_s1_waitrequest_from_sa,
                                              reset_n,

                                             // outputs:
                                              data_cache_0_AccelMaster_address_to_slave,
                                              data_cache_0_AccelMaster_latency_counter,
                                              data_cache_0_AccelMaster_readdata,
                                              data_cache_0_AccelMaster_readdatavalid,
                                              data_cache_0_AccelMaster_waitrequest
                                           )
;

  output  [ 31: 0] data_cache_0_AccelMaster_address_to_slave;
  output           data_cache_0_AccelMaster_latency_counter;
  output  [ 31: 0] data_cache_0_AccelMaster_readdata;
  output           data_cache_0_AccelMaster_readdatavalid;
  output           data_cache_0_AccelMaster_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_MEMORY_s1_end_xfer;
  input   [ 31: 0] data_cache_0_AccelMaster_address;
  input   [  2: 0] data_cache_0_AccelMaster_burstcount;
  input   [  3: 0] data_cache_0_AccelMaster_byteenable;
  input            data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_AccelMaster_read;
  input            data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  input            data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_AccelMaster_write;
  input   [ 31: 0] data_cache_0_AccelMaster_writedata;
  input   [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  input            pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  input            reset_n;

  reg              active_and_waiting_last_time;
  reg     [ 31: 0] data_cache_0_AccelMaster_address_last_time;
  wire    [ 31: 0] data_cache_0_AccelMaster_address_to_slave;
  reg     [  2: 0] data_cache_0_AccelMaster_burstcount_last_time;
  reg     [  3: 0] data_cache_0_AccelMaster_byteenable_last_time;
  wire             data_cache_0_AccelMaster_is_granted_some_slave;
  reg              data_cache_0_AccelMaster_latency_counter;
  reg              data_cache_0_AccelMaster_read_but_no_slave_selected;
  reg              data_cache_0_AccelMaster_read_last_time;
  wire    [ 31: 0] data_cache_0_AccelMaster_readdata;
  wire             data_cache_0_AccelMaster_readdatavalid;
  wire             data_cache_0_AccelMaster_run;
  wire             data_cache_0_AccelMaster_waitrequest;
  reg              data_cache_0_AccelMaster_write_last_time;
  reg     [ 31: 0] data_cache_0_AccelMaster_writedata_last_time;
  wire             latency_load_value;
  wire             p1_data_cache_0_AccelMaster_latency_counter;
  wire             pre_flush_data_cache_0_AccelMaster_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1) & (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 | ~data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1) & ((~data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_AccelMaster_read | data_cache_0_AccelMaster_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_AccelMaster_read | data_cache_0_AccelMaster_write)))) & ((~data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_AccelMaster_read | data_cache_0_AccelMaster_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_AccelMaster_read | data_cache_0_AccelMaster_write))));

  //cascaded wait assignment, which is an e_assign
  assign data_cache_0_AccelMaster_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign data_cache_0_AccelMaster_address_to_slave = {7'b0,
    data_cache_0_AccelMaster_address[24 : 0]};

  //data_cache_0_AccelMaster_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_read_but_no_slave_selected <= 0;
      else 
        data_cache_0_AccelMaster_read_but_no_slave_selected <= data_cache_0_AccelMaster_read & data_cache_0_AccelMaster_run & ~data_cache_0_AccelMaster_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign data_cache_0_AccelMaster_is_granted_some_slave = data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_data_cache_0_AccelMaster_readdatavalid = data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign data_cache_0_AccelMaster_readdatavalid = data_cache_0_AccelMaster_read_but_no_slave_selected |
    pre_flush_data_cache_0_AccelMaster_readdatavalid;

  //data_cache_0/AccelMaster readdata mux, which is an e_mux
  assign data_cache_0_AccelMaster_readdata = pipeline_bridge_MEMORY_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign data_cache_0_AccelMaster_waitrequest = ~data_cache_0_AccelMaster_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_latency_counter <= 0;
      else 
        data_cache_0_AccelMaster_latency_counter <= p1_data_cache_0_AccelMaster_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_data_cache_0_AccelMaster_latency_counter = ((data_cache_0_AccelMaster_run & data_cache_0_AccelMaster_read))? latency_load_value :
    (data_cache_0_AccelMaster_latency_counter)? data_cache_0_AccelMaster_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //data_cache_0_AccelMaster_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_address_last_time <= 0;
      else 
        data_cache_0_AccelMaster_address_last_time <= data_cache_0_AccelMaster_address;
    end


  //data_cache_0/AccelMaster waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= data_cache_0_AccelMaster_waitrequest & (data_cache_0_AccelMaster_read | data_cache_0_AccelMaster_write);
    end


  //data_cache_0_AccelMaster_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_AccelMaster_address != data_cache_0_AccelMaster_address_last_time))
        begin
          $write("%0d ns: data_cache_0_AccelMaster_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_AccelMaster_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_burstcount_last_time <= 0;
      else 
        data_cache_0_AccelMaster_burstcount_last_time <= data_cache_0_AccelMaster_burstcount;
    end


  //data_cache_0_AccelMaster_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_AccelMaster_burstcount != data_cache_0_AccelMaster_burstcount_last_time))
        begin
          $write("%0d ns: data_cache_0_AccelMaster_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_AccelMaster_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_byteenable_last_time <= 0;
      else 
        data_cache_0_AccelMaster_byteenable_last_time <= data_cache_0_AccelMaster_byteenable;
    end


  //data_cache_0_AccelMaster_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_AccelMaster_byteenable != data_cache_0_AccelMaster_byteenable_last_time))
        begin
          $write("%0d ns: data_cache_0_AccelMaster_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_AccelMaster_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_read_last_time <= 0;
      else 
        data_cache_0_AccelMaster_read_last_time <= data_cache_0_AccelMaster_read;
    end


  //data_cache_0_AccelMaster_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_AccelMaster_read != data_cache_0_AccelMaster_read_last_time))
        begin
          $write("%0d ns: data_cache_0_AccelMaster_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_AccelMaster_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_write_last_time <= 0;
      else 
        data_cache_0_AccelMaster_write_last_time <= data_cache_0_AccelMaster_write;
    end


  //data_cache_0_AccelMaster_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_AccelMaster_write != data_cache_0_AccelMaster_write_last_time))
        begin
          $write("%0d ns: data_cache_0_AccelMaster_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_AccelMaster_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_AccelMaster_writedata_last_time <= 0;
      else 
        data_cache_0_AccelMaster_writedata_last_time <= data_cache_0_AccelMaster_writedata;
    end


  //data_cache_0_AccelMaster_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_AccelMaster_writedata != data_cache_0_AccelMaster_writedata_last_time) & data_cache_0_AccelMaster_write)
        begin
          $write("%0d ns: data_cache_0_AccelMaster_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module data_cache_0_CachetoTiger_arbitrator (
                                              // inputs:
                                               clk,
                                               data_cache_0_CachetoTiger_data,
                                               reset_n
                                            )
;

  input            clk;
  input   [ 39: 0] data_cache_0_CachetoTiger_data;
  input            reset_n;


endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module data_cache_0_dataMaster_arbitrator (
                                            // inputs:
                                             clk,
                                             d1_pipeline_bridge_MEMORY_s1_end_xfer,
                                             d1_tiger_burst_3_upstream_end_xfer,
                                             data_cache_0_dataMaster_address,
                                             data_cache_0_dataMaster_burstcount,
                                             data_cache_0_dataMaster_byteenable,
                                             data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1,
                                             data_cache_0_dataMaster_granted_tiger_burst_3_upstream,
                                             data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1,
                                             data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream,
                                             data_cache_0_dataMaster_read,
                                             data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1,
                                             data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                             data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream,
                                             data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register,
                                             data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1,
                                             data_cache_0_dataMaster_requests_tiger_burst_3_upstream,
                                             data_cache_0_dataMaster_write,
                                             data_cache_0_dataMaster_writedata,
                                             pipeline_bridge_MEMORY_s1_readdata_from_sa,
                                             pipeline_bridge_MEMORY_s1_waitrequest_from_sa,
                                             reset_n,
                                             tiger_burst_3_upstream_readdata_from_sa,
                                             tiger_burst_3_upstream_waitrequest_from_sa,

                                            // outputs:
                                             data_cache_0_dataMaster_address_to_slave,
                                             data_cache_0_dataMaster_latency_counter,
                                             data_cache_0_dataMaster_readdata,
                                             data_cache_0_dataMaster_readdatavalid,
                                             data_cache_0_dataMaster_waitrequest
                                          )
;

  output  [ 31: 0] data_cache_0_dataMaster_address_to_slave;
  output           data_cache_0_dataMaster_latency_counter;
  output  [ 31: 0] data_cache_0_dataMaster_readdata;
  output           data_cache_0_dataMaster_readdatavalid;
  output           data_cache_0_dataMaster_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_MEMORY_s1_end_xfer;
  input            d1_tiger_burst_3_upstream_end_xfer;
  input   [ 31: 0] data_cache_0_dataMaster_address;
  input   [  2: 0] data_cache_0_dataMaster_burstcount;
  input   [  3: 0] data_cache_0_dataMaster_byteenable;
  input            data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster_granted_tiger_burst_3_upstream;
  input            data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream;
  input            data_cache_0_dataMaster_read;
  input            data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  input            data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream;
  input            data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register;
  input            data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1;
  input            data_cache_0_dataMaster_requests_tiger_burst_3_upstream;
  input            data_cache_0_dataMaster_write;
  input   [ 31: 0] data_cache_0_dataMaster_writedata;
  input   [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  input            pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  input            reset_n;
  input   [ 31: 0] tiger_burst_3_upstream_readdata_from_sa;
  input            tiger_burst_3_upstream_waitrequest_from_sa;

  reg              active_and_waiting_last_time;
  reg     [ 31: 0] data_cache_0_dataMaster_address_last_time;
  wire    [ 31: 0] data_cache_0_dataMaster_address_to_slave;
  reg     [  2: 0] data_cache_0_dataMaster_burstcount_last_time;
  reg     [  3: 0] data_cache_0_dataMaster_byteenable_last_time;
  wire             data_cache_0_dataMaster_is_granted_some_slave;
  reg              data_cache_0_dataMaster_latency_counter;
  reg              data_cache_0_dataMaster_read_but_no_slave_selected;
  reg              data_cache_0_dataMaster_read_last_time;
  wire    [ 31: 0] data_cache_0_dataMaster_readdata;
  wire             data_cache_0_dataMaster_readdatavalid;
  wire             data_cache_0_dataMaster_run;
  wire             data_cache_0_dataMaster_waitrequest;
  reg              data_cache_0_dataMaster_write_last_time;
  reg     [ 31: 0] data_cache_0_dataMaster_writedata_last_time;
  wire             latency_load_value;
  wire             p1_data_cache_0_dataMaster_latency_counter;
  wire             pre_flush_data_cache_0_dataMaster_readdatavalid;
  wire             r_0;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1) & (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 | ~data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1) & ((~data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_dataMaster_read | data_cache_0_dataMaster_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_dataMaster_read | data_cache_0_dataMaster_write)))) & ((~data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1 | ~(data_cache_0_dataMaster_read | data_cache_0_dataMaster_write) | (1 & ~pipeline_bridge_MEMORY_s1_waitrequest_from_sa & (data_cache_0_dataMaster_read | data_cache_0_dataMaster_write)))) & 1 & (data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream | ~data_cache_0_dataMaster_requests_tiger_burst_3_upstream) & ((~data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream | ~(data_cache_0_dataMaster_read | data_cache_0_dataMaster_write) | (1 & ~tiger_burst_3_upstream_waitrequest_from_sa & (data_cache_0_dataMaster_read | data_cache_0_dataMaster_write)))) & ((~data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream | ~(data_cache_0_dataMaster_read | data_cache_0_dataMaster_write) | (1 & ~tiger_burst_3_upstream_waitrequest_from_sa & (data_cache_0_dataMaster_read | data_cache_0_dataMaster_write))));

  //cascaded wait assignment, which is an e_assign
  assign data_cache_0_dataMaster_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign data_cache_0_dataMaster_address_to_slave = {data_cache_0_dataMaster_address[31 : 28],
    3'b0,
    data_cache_0_dataMaster_address[24 : 0]};

  //data_cache_0_dataMaster_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_read_but_no_slave_selected <= 0;
      else 
        data_cache_0_dataMaster_read_but_no_slave_selected <= data_cache_0_dataMaster_read & data_cache_0_dataMaster_run & ~data_cache_0_dataMaster_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign data_cache_0_dataMaster_is_granted_some_slave = data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster_granted_tiger_burst_3_upstream;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_data_cache_0_dataMaster_readdatavalid = data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign data_cache_0_dataMaster_readdatavalid = data_cache_0_dataMaster_read_but_no_slave_selected |
    pre_flush_data_cache_0_dataMaster_readdatavalid |
    data_cache_0_dataMaster_read_but_no_slave_selected |
    pre_flush_data_cache_0_dataMaster_readdatavalid;

  //data_cache_0/dataMaster readdata mux, which is an e_mux
  assign data_cache_0_dataMaster_readdata = ({32 {~data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1}} | pipeline_bridge_MEMORY_s1_readdata_from_sa) &
    ({32 {~data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream}} | tiger_burst_3_upstream_readdata_from_sa);

  //actual waitrequest port, which is an e_assign
  assign data_cache_0_dataMaster_waitrequest = ~data_cache_0_dataMaster_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_latency_counter <= 0;
      else 
        data_cache_0_dataMaster_latency_counter <= p1_data_cache_0_dataMaster_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_data_cache_0_dataMaster_latency_counter = ((data_cache_0_dataMaster_run & data_cache_0_dataMaster_read))? latency_load_value :
    (data_cache_0_dataMaster_latency_counter)? data_cache_0_dataMaster_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //data_cache_0_dataMaster_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_address_last_time <= 0;
      else 
        data_cache_0_dataMaster_address_last_time <= data_cache_0_dataMaster_address;
    end


  //data_cache_0/dataMaster waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= data_cache_0_dataMaster_waitrequest & (data_cache_0_dataMaster_read | data_cache_0_dataMaster_write);
    end


  //data_cache_0_dataMaster_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster_address != data_cache_0_dataMaster_address_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_burstcount_last_time <= 0;
      else 
        data_cache_0_dataMaster_burstcount_last_time <= data_cache_0_dataMaster_burstcount;
    end


  //data_cache_0_dataMaster_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster_burstcount != data_cache_0_dataMaster_burstcount_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_byteenable_last_time <= 0;
      else 
        data_cache_0_dataMaster_byteenable_last_time <= data_cache_0_dataMaster_byteenable;
    end


  //data_cache_0_dataMaster_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster_byteenable != data_cache_0_dataMaster_byteenable_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_read_last_time <= 0;
      else 
        data_cache_0_dataMaster_read_last_time <= data_cache_0_dataMaster_read;
    end


  //data_cache_0_dataMaster_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster_read != data_cache_0_dataMaster_read_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_write_last_time <= 0;
      else 
        data_cache_0_dataMaster_write_last_time <= data_cache_0_dataMaster_write;
    end


  //data_cache_0_dataMaster_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster_write != data_cache_0_dataMaster_write_last_time))
        begin
          $write("%0d ns: data_cache_0_dataMaster_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //data_cache_0_dataMaster_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_cache_0_dataMaster_writedata_last_time <= 0;
      else 
        data_cache_0_dataMaster_writedata_last_time <= data_cache_0_dataMaster_writedata;
    end


  //data_cache_0_dataMaster_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (data_cache_0_dataMaster_writedata != data_cache_0_dataMaster_writedata_last_time) & data_cache_0_dataMaster_write)
        begin
          $write("%0d ns: data_cache_0_dataMaster_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module onchip_mem_s1_arbitrator (
                                  // inputs:
                                   clk,
                                   onchip_mem_s1_readdata,
                                   reset_n,
                                   tiger_burst_1_downstream_address_to_slave,
                                   tiger_burst_1_downstream_arbitrationshare,
                                   tiger_burst_1_downstream_burstcount,
                                   tiger_burst_1_downstream_byteenable,
                                   tiger_burst_1_downstream_debugaccess,
                                   tiger_burst_1_downstream_latency_counter,
                                   tiger_burst_1_downstream_read,
                                   tiger_burst_1_downstream_write,
                                   tiger_burst_1_downstream_writedata,

                                  // outputs:
                                   d1_onchip_mem_s1_end_xfer,
                                   onchip_mem_s1_address,
                                   onchip_mem_s1_byteenable,
                                   onchip_mem_s1_chipselect,
                                   onchip_mem_s1_clken,
                                   onchip_mem_s1_debugaccess,
                                   onchip_mem_s1_readdata_from_sa,
                                   onchip_mem_s1_reset,
                                   onchip_mem_s1_write,
                                   onchip_mem_s1_writedata,
                                   tiger_burst_1_downstream_granted_onchip_mem_s1,
                                   tiger_burst_1_downstream_qualified_request_onchip_mem_s1,
                                   tiger_burst_1_downstream_read_data_valid_onchip_mem_s1,
                                   tiger_burst_1_downstream_requests_onchip_mem_s1
                                )
;

  output           d1_onchip_mem_s1_end_xfer;
  output  [ 10: 0] onchip_mem_s1_address;
  output  [  3: 0] onchip_mem_s1_byteenable;
  output           onchip_mem_s1_chipselect;
  output           onchip_mem_s1_clken;
  output           onchip_mem_s1_debugaccess;
  output  [ 31: 0] onchip_mem_s1_readdata_from_sa;
  output           onchip_mem_s1_reset;
  output           onchip_mem_s1_write;
  output  [ 31: 0] onchip_mem_s1_writedata;
  output           tiger_burst_1_downstream_granted_onchip_mem_s1;
  output           tiger_burst_1_downstream_qualified_request_onchip_mem_s1;
  output           tiger_burst_1_downstream_read_data_valid_onchip_mem_s1;
  output           tiger_burst_1_downstream_requests_onchip_mem_s1;
  input            clk;
  input   [ 31: 0] onchip_mem_s1_readdata;
  input            reset_n;
  input   [ 12: 0] tiger_burst_1_downstream_address_to_slave;
  input   [  2: 0] tiger_burst_1_downstream_arbitrationshare;
  input            tiger_burst_1_downstream_burstcount;
  input   [  3: 0] tiger_burst_1_downstream_byteenable;
  input            tiger_burst_1_downstream_debugaccess;
  input            tiger_burst_1_downstream_latency_counter;
  input            tiger_burst_1_downstream_read;
  input            tiger_burst_1_downstream_write;
  input   [ 31: 0] tiger_burst_1_downstream_writedata;

  reg              d1_onchip_mem_s1_end_xfer;
  reg              d1_reasons_to_wait;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_onchip_mem_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire    [ 10: 0] onchip_mem_s1_address;
  wire             onchip_mem_s1_allgrants;
  wire             onchip_mem_s1_allow_new_arb_cycle;
  wire             onchip_mem_s1_any_bursting_master_saved_grant;
  wire             onchip_mem_s1_any_continuerequest;
  wire             onchip_mem_s1_arb_counter_enable;
  reg     [  2: 0] onchip_mem_s1_arb_share_counter;
  wire    [  2: 0] onchip_mem_s1_arb_share_counter_next_value;
  wire    [  2: 0] onchip_mem_s1_arb_share_set_values;
  wire             onchip_mem_s1_beginbursttransfer_internal;
  wire             onchip_mem_s1_begins_xfer;
  wire    [  3: 0] onchip_mem_s1_byteenable;
  wire             onchip_mem_s1_chipselect;
  wire             onchip_mem_s1_clken;
  wire             onchip_mem_s1_debugaccess;
  wire             onchip_mem_s1_end_xfer;
  wire             onchip_mem_s1_firsttransfer;
  wire             onchip_mem_s1_grant_vector;
  wire             onchip_mem_s1_in_a_read_cycle;
  wire             onchip_mem_s1_in_a_write_cycle;
  wire             onchip_mem_s1_master_qreq_vector;
  wire             onchip_mem_s1_non_bursting_master_requests;
  wire    [ 31: 0] onchip_mem_s1_readdata_from_sa;
  reg              onchip_mem_s1_reg_firsttransfer;
  wire             onchip_mem_s1_reset;
  reg              onchip_mem_s1_slavearbiterlockenable;
  wire             onchip_mem_s1_slavearbiterlockenable2;
  wire             onchip_mem_s1_unreg_firsttransfer;
  wire             onchip_mem_s1_waits_for_read;
  wire             onchip_mem_s1_waits_for_write;
  wire             onchip_mem_s1_write;
  wire    [ 31: 0] onchip_mem_s1_writedata;
  wire             p1_tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register;
  wire    [ 12: 0] shifted_address_to_onchip_mem_s1_from_tiger_burst_1_downstream;
  wire             tiger_burst_1_downstream_arbiterlock;
  wire             tiger_burst_1_downstream_arbiterlock2;
  wire             tiger_burst_1_downstream_continuerequest;
  wire             tiger_burst_1_downstream_granted_onchip_mem_s1;
  wire             tiger_burst_1_downstream_qualified_request_onchip_mem_s1;
  wire             tiger_burst_1_downstream_read_data_valid_onchip_mem_s1;
  reg              tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register;
  wire             tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register_in;
  wire             tiger_burst_1_downstream_requests_onchip_mem_s1;
  wire             tiger_burst_1_downstream_saved_grant_onchip_mem_s1;
  wire             wait_for_onchip_mem_s1_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~onchip_mem_s1_end_xfer;
    end


  assign onchip_mem_s1_begins_xfer = ~d1_reasons_to_wait & ((tiger_burst_1_downstream_qualified_request_onchip_mem_s1));
  //assign onchip_mem_s1_readdata_from_sa = onchip_mem_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign onchip_mem_s1_readdata_from_sa = onchip_mem_s1_readdata;

  assign tiger_burst_1_downstream_requests_onchip_mem_s1 = (1) & (tiger_burst_1_downstream_read | tiger_burst_1_downstream_write);
  //onchip_mem_s1_arb_share_counter set values, which is an e_mux
  assign onchip_mem_s1_arb_share_set_values = (tiger_burst_1_downstream_granted_onchip_mem_s1)? tiger_burst_1_downstream_arbitrationshare :
    1;

  //onchip_mem_s1_non_bursting_master_requests mux, which is an e_mux
  assign onchip_mem_s1_non_bursting_master_requests = 0;

  //onchip_mem_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign onchip_mem_s1_any_bursting_master_saved_grant = tiger_burst_1_downstream_saved_grant_onchip_mem_s1;

  //onchip_mem_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign onchip_mem_s1_arb_share_counter_next_value = onchip_mem_s1_firsttransfer ? (onchip_mem_s1_arb_share_set_values - 1) : |onchip_mem_s1_arb_share_counter ? (onchip_mem_s1_arb_share_counter - 1) : 0;

  //onchip_mem_s1_allgrants all slave grants, which is an e_mux
  assign onchip_mem_s1_allgrants = |onchip_mem_s1_grant_vector;

  //onchip_mem_s1_end_xfer assignment, which is an e_assign
  assign onchip_mem_s1_end_xfer = ~(onchip_mem_s1_waits_for_read | onchip_mem_s1_waits_for_write);

  //end_xfer_arb_share_counter_term_onchip_mem_s1 arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_onchip_mem_s1 = onchip_mem_s1_end_xfer & (~onchip_mem_s1_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //onchip_mem_s1_arb_share_counter arbitration counter enable, which is an e_assign
  assign onchip_mem_s1_arb_counter_enable = (end_xfer_arb_share_counter_term_onchip_mem_s1 & onchip_mem_s1_allgrants) | (end_xfer_arb_share_counter_term_onchip_mem_s1 & ~onchip_mem_s1_non_bursting_master_requests);

  //onchip_mem_s1_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          onchip_mem_s1_arb_share_counter <= 0;
      else if (onchip_mem_s1_arb_counter_enable)
          onchip_mem_s1_arb_share_counter <= onchip_mem_s1_arb_share_counter_next_value;
    end


  //onchip_mem_s1_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          onchip_mem_s1_slavearbiterlockenable <= 0;
      else if ((|onchip_mem_s1_master_qreq_vector & end_xfer_arb_share_counter_term_onchip_mem_s1) | (end_xfer_arb_share_counter_term_onchip_mem_s1 & ~onchip_mem_s1_non_bursting_master_requests))
          onchip_mem_s1_slavearbiterlockenable <= |onchip_mem_s1_arb_share_counter_next_value;
    end


  //tiger_burst_1/downstream onchip_mem/s1 arbiterlock, which is an e_assign
  assign tiger_burst_1_downstream_arbiterlock = onchip_mem_s1_slavearbiterlockenable & tiger_burst_1_downstream_continuerequest;

  //onchip_mem_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign onchip_mem_s1_slavearbiterlockenable2 = |onchip_mem_s1_arb_share_counter_next_value;

  //tiger_burst_1/downstream onchip_mem/s1 arbiterlock2, which is an e_assign
  assign tiger_burst_1_downstream_arbiterlock2 = onchip_mem_s1_slavearbiterlockenable2 & tiger_burst_1_downstream_continuerequest;

  //onchip_mem_s1_any_continuerequest at least one master continues requesting, which is an e_assign
  assign onchip_mem_s1_any_continuerequest = 1;

  //tiger_burst_1_downstream_continuerequest continued request, which is an e_assign
  assign tiger_burst_1_downstream_continuerequest = 1;

  assign tiger_burst_1_downstream_qualified_request_onchip_mem_s1 = tiger_burst_1_downstream_requests_onchip_mem_s1;
  //tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register_in mux for readlatency shift register, which is an e_mux
  assign tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register_in = tiger_burst_1_downstream_granted_onchip_mem_s1 & tiger_burst_1_downstream_read & ~onchip_mem_s1_waits_for_read;

  //shift register p1 tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register in if flush, otherwise shift left, which is an e_mux
  assign p1_tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register = {tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register, tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register_in};

  //tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register for remembering which master asked for a fixed latency read, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register <= 0;
      else 
        tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register <= p1_tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register;
    end


  //local readdatavalid tiger_burst_1_downstream_read_data_valid_onchip_mem_s1, which is an e_mux
  assign tiger_burst_1_downstream_read_data_valid_onchip_mem_s1 = tiger_burst_1_downstream_read_data_valid_onchip_mem_s1_shift_register;

  //onchip_mem_s1_writedata mux, which is an e_mux
  assign onchip_mem_s1_writedata = tiger_burst_1_downstream_writedata;

  //mux onchip_mem_s1_clken, which is an e_mux
  assign onchip_mem_s1_clken = 1'b1;

  //master is always granted when requested
  assign tiger_burst_1_downstream_granted_onchip_mem_s1 = tiger_burst_1_downstream_qualified_request_onchip_mem_s1;

  //tiger_burst_1/downstream saved-grant onchip_mem/s1, which is an e_assign
  assign tiger_burst_1_downstream_saved_grant_onchip_mem_s1 = tiger_burst_1_downstream_requests_onchip_mem_s1;

  //allow new arb cycle for onchip_mem/s1, which is an e_assign
  assign onchip_mem_s1_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign onchip_mem_s1_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign onchip_mem_s1_master_qreq_vector = 1;

  //~onchip_mem_s1_reset assignment, which is an e_assign
  assign onchip_mem_s1_reset = ~reset_n;

  assign onchip_mem_s1_chipselect = tiger_burst_1_downstream_granted_onchip_mem_s1;
  //onchip_mem_s1_firsttransfer first transaction, which is an e_assign
  assign onchip_mem_s1_firsttransfer = onchip_mem_s1_begins_xfer ? onchip_mem_s1_unreg_firsttransfer : onchip_mem_s1_reg_firsttransfer;

  //onchip_mem_s1_unreg_firsttransfer first transaction, which is an e_assign
  assign onchip_mem_s1_unreg_firsttransfer = ~(onchip_mem_s1_slavearbiterlockenable & onchip_mem_s1_any_continuerequest);

  //onchip_mem_s1_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          onchip_mem_s1_reg_firsttransfer <= 1'b1;
      else if (onchip_mem_s1_begins_xfer)
          onchip_mem_s1_reg_firsttransfer <= onchip_mem_s1_unreg_firsttransfer;
    end


  //onchip_mem_s1_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign onchip_mem_s1_beginbursttransfer_internal = onchip_mem_s1_begins_xfer;

  //onchip_mem_s1_write assignment, which is an e_mux
  assign onchip_mem_s1_write = tiger_burst_1_downstream_granted_onchip_mem_s1 & tiger_burst_1_downstream_write;

  assign shifted_address_to_onchip_mem_s1_from_tiger_burst_1_downstream = tiger_burst_1_downstream_address_to_slave;
  //onchip_mem_s1_address mux, which is an e_mux
  assign onchip_mem_s1_address = shifted_address_to_onchip_mem_s1_from_tiger_burst_1_downstream >> 2;

  //d1_onchip_mem_s1_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_onchip_mem_s1_end_xfer <= 1;
      else 
        d1_onchip_mem_s1_end_xfer <= onchip_mem_s1_end_xfer;
    end


  //onchip_mem_s1_waits_for_read in a cycle, which is an e_mux
  assign onchip_mem_s1_waits_for_read = onchip_mem_s1_in_a_read_cycle & 0;

  //onchip_mem_s1_in_a_read_cycle assignment, which is an e_assign
  assign onchip_mem_s1_in_a_read_cycle = tiger_burst_1_downstream_granted_onchip_mem_s1 & tiger_burst_1_downstream_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = onchip_mem_s1_in_a_read_cycle;

  //onchip_mem_s1_waits_for_write in a cycle, which is an e_mux
  assign onchip_mem_s1_waits_for_write = onchip_mem_s1_in_a_write_cycle & 0;

  //onchip_mem_s1_in_a_write_cycle assignment, which is an e_assign
  assign onchip_mem_s1_in_a_write_cycle = tiger_burst_1_downstream_granted_onchip_mem_s1 & tiger_burst_1_downstream_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = onchip_mem_s1_in_a_write_cycle;

  assign wait_for_onchip_mem_s1_counter = 0;
  //onchip_mem_s1_byteenable byte enable port mux, which is an e_mux
  assign onchip_mem_s1_byteenable = (tiger_burst_1_downstream_granted_onchip_mem_s1)? tiger_burst_1_downstream_byteenable :
    -1;

  //debugaccess mux, which is an e_mux
  assign onchip_mem_s1_debugaccess = (tiger_burst_1_downstream_granted_onchip_mem_s1)? tiger_burst_1_downstream_debugaccess :
    0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //onchip_mem/s1 enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end


  //tiger_burst_1/downstream non-zero arbitrationshare assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_1_downstream_requests_onchip_mem_s1 && (tiger_burst_1_downstream_arbitrationshare == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_1/downstream drove 0 on its 'arbitrationshare' port while accessing slave onchip_mem/s1", $time);
          $stop;
        end
    end


  //tiger_burst_1/downstream non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_1_downstream_requests_onchip_mem_s1 && (tiger_burst_1_downstream_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_1/downstream drove 0 on its 'burstcount' port while accessing slave onchip_mem/s1", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



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
  wire    [  2: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  2: 0] p1_stage_1;
  wire             p2_full_2;
  wire    [  2: 0] p2_stage_2;
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
  reg     [  2: 0] stage_2;
  reg     [  2: 0] stage_3;
  reg     [  2: 0] stage_4;
  reg     [  2: 0] stage_5;
  reg     [  2: 0] stage_6;
  reg     [  2: 0] stage_7;
  reg     [  2: 0] stage_8;
  reg     [  2: 0] stage_9;
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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_data_cache_0_AccelMaster_to_pipeline_bridge_MEMORY_s1_module (
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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_data_cache_0_dataMaster_to_pipeline_bridge_MEMORY_s1_module (
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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_MEMORY_s1_arbitrator (
                                              // inputs:
                                               clk,
                                               data_cache_0_AccelMaster_address_to_slave,
                                               data_cache_0_AccelMaster_burstcount,
                                               data_cache_0_AccelMaster_byteenable,
                                               data_cache_0_AccelMaster_latency_counter,
                                               data_cache_0_AccelMaster_read,
                                               data_cache_0_AccelMaster_write,
                                               data_cache_0_AccelMaster_writedata,
                                               data_cache_0_dataMaster_address_to_slave,
                                               data_cache_0_dataMaster_burstcount,
                                               data_cache_0_dataMaster_byteenable,
                                               data_cache_0_dataMaster_latency_counter,
                                               data_cache_0_dataMaster_read,
                                               data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register,
                                               data_cache_0_dataMaster_write,
                                               data_cache_0_dataMaster_writedata,
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
                                               data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                               data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1,
                                               data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                               data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1,
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
  output           data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  output           data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  output           data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  output           data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1;
  output  [ 22: 0] pipeline_bridge_MEMORY_s1_address;
  output           pipeline_bridge_MEMORY_s1_arbiterlock;
  output           pipeline_bridge_MEMORY_s1_arbiterlock2;
  output  [  2: 0] pipeline_bridge_MEMORY_s1_burstcount;
  output  [  3: 0] pipeline_bridge_MEMORY_s1_byteenable;
  output           pipeline_bridge_MEMORY_s1_chipselect;
  output           pipeline_bridge_MEMORY_s1_debugaccess;
  output           pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  output  [ 22: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
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
  input   [ 31: 0] data_cache_0_AccelMaster_address_to_slave;
  input   [  2: 0] data_cache_0_AccelMaster_burstcount;
  input   [  3: 0] data_cache_0_AccelMaster_byteenable;
  input            data_cache_0_AccelMaster_latency_counter;
  input            data_cache_0_AccelMaster_read;
  input            data_cache_0_AccelMaster_write;
  input   [ 31: 0] data_cache_0_AccelMaster_writedata;
  input   [ 31: 0] data_cache_0_dataMaster_address_to_slave;
  input   [  2: 0] data_cache_0_dataMaster_burstcount;
  input   [  3: 0] data_cache_0_dataMaster_byteenable;
  input            data_cache_0_dataMaster_latency_counter;
  input            data_cache_0_dataMaster_read;
  input            data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register;
  input            data_cache_0_dataMaster_write;
  input   [ 31: 0] data_cache_0_dataMaster_writedata;
  input            pipeline_bridge_MEMORY_s1_endofpacket;
  input   [ 31: 0] pipeline_bridge_MEMORY_s1_readdata;
  input            pipeline_bridge_MEMORY_s1_readdatavalid;
  input            pipeline_bridge_MEMORY_s1_waitrequest;
  input            reset_n;
  input   [ 31: 0] tiger_top_0_instructionMaster_address_to_slave;
  input   [  2: 0] tiger_top_0_instructionMaster_burstcount;
  input            tiger_top_0_instructionMaster_latency_counter;
  input            tiger_top_0_instructionMaster_read;

  reg              d1_pipeline_bridge_MEMORY_s1_end_xfer;
  reg              d1_reasons_to_wait;
  wire             data_cache_0_AccelMaster_arbiterlock;
  wire             data_cache_0_AccelMaster_arbiterlock2;
  wire             data_cache_0_AccelMaster_continuerequest;
  wire             data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire             data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_arbiterlock;
  wire             data_cache_0_dataMaster_arbiterlock2;
  wire             data_cache_0_dataMaster_continuerequest;
  wire             data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire             data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_pipeline_bridge_MEMORY_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  reg              last_cycle_data_cache_0_AccelMaster_granted_slave_pipeline_bridge_MEMORY_s1;
  reg              last_cycle_data_cache_0_dataMaster_granted_slave_pipeline_bridge_MEMORY_s1;
  reg              last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1;
  wire             p0_pipeline_bridge_MEMORY_s1_load_fifo;
  wire    [ 22: 0] pipeline_bridge_MEMORY_s1_address;
  wire             pipeline_bridge_MEMORY_s1_allgrants;
  wire             pipeline_bridge_MEMORY_s1_allow_new_arb_cycle;
  wire             pipeline_bridge_MEMORY_s1_any_bursting_master_saved_grant;
  wire             pipeline_bridge_MEMORY_s1_any_continuerequest;
  reg     [  2: 0] pipeline_bridge_MEMORY_s1_arb_addend;
  wire             pipeline_bridge_MEMORY_s1_arb_counter_enable;
  reg     [  2: 0] pipeline_bridge_MEMORY_s1_arb_share_counter;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_arb_share_counter_next_value;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_arb_share_set_values;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_arb_winner;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock2;
  wire             pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal;
  reg     [  1: 0] pipeline_bridge_MEMORY_s1_bbt_burstcounter;
  wire             pipeline_bridge_MEMORY_s1_beginbursttransfer_internal;
  wire             pipeline_bridge_MEMORY_s1_begins_xfer;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_burstcount;
  wire             pipeline_bridge_MEMORY_s1_burstcount_fifo_empty;
  wire    [  3: 0] pipeline_bridge_MEMORY_s1_byteenable;
  wire             pipeline_bridge_MEMORY_s1_chipselect;
  wire    [  5: 0] pipeline_bridge_MEMORY_s1_chosen_master_double_vector;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_chosen_master_rot_left;
  reg     [  2: 0] pipeline_bridge_MEMORY_s1_current_burst;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_current_burst_minus_one;
  wire             pipeline_bridge_MEMORY_s1_debugaccess;
  wire             pipeline_bridge_MEMORY_s1_end_xfer;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire             pipeline_bridge_MEMORY_s1_firsttransfer;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_grant_vector;
  wire             pipeline_bridge_MEMORY_s1_in_a_read_cycle;
  wire             pipeline_bridge_MEMORY_s1_in_a_write_cycle;
  reg              pipeline_bridge_MEMORY_s1_load_fifo;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_master_qreq_vector;
  wire             pipeline_bridge_MEMORY_s1_move_on_to_next_transaction;
  wire    [ 22: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  wire    [  1: 0] pipeline_bridge_MEMORY_s1_next_bbt_burstcount;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_next_burst_count;
  wire             pipeline_bridge_MEMORY_s1_non_bursting_master_requests;
  wire             pipeline_bridge_MEMORY_s1_read;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  wire             pipeline_bridge_MEMORY_s1_readdatavalid_from_sa;
  reg              pipeline_bridge_MEMORY_s1_reg_firsttransfer;
  wire             pipeline_bridge_MEMORY_s1_reset_n;
  reg     [  2: 0] pipeline_bridge_MEMORY_s1_saved_chosen_master_vector;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_selected_burstcount;
  reg              pipeline_bridge_MEMORY_s1_slavearbiterlockenable;
  wire             pipeline_bridge_MEMORY_s1_slavearbiterlockenable2;
  wire             pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_transaction_burst_count;
  wire             pipeline_bridge_MEMORY_s1_unreg_firsttransfer;
  wire             pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  wire             pipeline_bridge_MEMORY_s1_waits_for_read;
  wire             pipeline_bridge_MEMORY_s1_waits_for_write;
  wire             pipeline_bridge_MEMORY_s1_write;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_writedata;
  wire    [ 31: 0] shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_AccelMaster;
  wire    [ 31: 0] shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_dataMaster;
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


  assign pipeline_bridge_MEMORY_s1_begins_xfer = ~d1_reasons_to_wait & ((data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1 | data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1 | tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1));
  //assign pipeline_bridge_MEMORY_s1_readdata_from_sa = pipeline_bridge_MEMORY_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_readdata_from_sa = pipeline_bridge_MEMORY_s1_readdata;

  assign data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1 = ({data_cache_0_AccelMaster_address_to_slave[31 : 25] , 25'b0} == 32'h0) & (data_cache_0_AccelMaster_read | data_cache_0_AccelMaster_write);
  //assign pipeline_bridge_MEMORY_s1_waitrequest_from_sa = pipeline_bridge_MEMORY_s1_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_waitrequest_from_sa = pipeline_bridge_MEMORY_s1_waitrequest;

  //assign pipeline_bridge_MEMORY_s1_readdatavalid_from_sa = pipeline_bridge_MEMORY_s1_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_readdatavalid_from_sa = pipeline_bridge_MEMORY_s1_readdatavalid;

  //pipeline_bridge_MEMORY_s1_arb_share_counter set values, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_arb_share_set_values = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_AccelMaster_write) ? data_cache_0_AccelMaster_burstcount : 1)) :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_dataMaster_write) ? data_cache_0_dataMaster_burstcount : 1)) :
    (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_AccelMaster_write) ? data_cache_0_AccelMaster_burstcount : 1)) :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_dataMaster_write) ? data_cache_0_dataMaster_burstcount : 1)) :
    (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_AccelMaster_write) ? data_cache_0_AccelMaster_burstcount : 1)) :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_dataMaster_write) ? data_cache_0_dataMaster_burstcount : 1)) :
    (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_AccelMaster_write) ? data_cache_0_AccelMaster_burstcount : 1)) :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? (((data_cache_0_dataMaster_write) ? data_cache_0_dataMaster_burstcount : 1)) :
    1;

  //pipeline_bridge_MEMORY_s1_non_bursting_master_requests mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_non_bursting_master_requests = 0;

  //pipeline_bridge_MEMORY_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_any_bursting_master_saved_grant = data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1 |
    tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1;

  //pipeline_bridge_MEMORY_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_arb_share_counter_next_value = pipeline_bridge_MEMORY_s1_firsttransfer ? (pipeline_bridge_MEMORY_s1_arb_share_set_values - 1) : |pipeline_bridge_MEMORY_s1_arb_share_counter ? (pipeline_bridge_MEMORY_s1_arb_share_counter - 1) : 0;

  //pipeline_bridge_MEMORY_s1_allgrants all slave grants, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_allgrants = (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
    (|pipeline_bridge_MEMORY_s1_grant_vector) |
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


  //data_cache_0/AccelMaster pipeline_bridge_MEMORY/s1 arbiterlock, which is an e_assign
  assign data_cache_0_AccelMaster_arbiterlock = pipeline_bridge_MEMORY_s1_slavearbiterlockenable & data_cache_0_AccelMaster_continuerequest;

  //pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 = |pipeline_bridge_MEMORY_s1_arb_share_counter_next_value;

  //data_cache_0/AccelMaster pipeline_bridge_MEMORY/s1 arbiterlock2, which is an e_assign
  assign data_cache_0_AccelMaster_arbiterlock2 = pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 & data_cache_0_AccelMaster_continuerequest;

  //data_cache_0/dataMaster pipeline_bridge_MEMORY/s1 arbiterlock, which is an e_assign
  assign data_cache_0_dataMaster_arbiterlock = pipeline_bridge_MEMORY_s1_slavearbiterlockenable & data_cache_0_dataMaster_continuerequest;

  //data_cache_0/dataMaster pipeline_bridge_MEMORY/s1 arbiterlock2, which is an e_assign
  assign data_cache_0_dataMaster_arbiterlock2 = pipeline_bridge_MEMORY_s1_slavearbiterlockenable2 & data_cache_0_dataMaster_continuerequest;

  //data_cache_0/dataMaster granted pipeline_bridge_MEMORY/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_data_cache_0_dataMaster_granted_slave_pipeline_bridge_MEMORY_s1 <= 0;
      else 
        last_cycle_data_cache_0_dataMaster_granted_slave_pipeline_bridge_MEMORY_s1 <= data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1 ? 1 : (pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal | 0) ? 0 : last_cycle_data_cache_0_dataMaster_granted_slave_pipeline_bridge_MEMORY_s1;
    end


  //data_cache_0_dataMaster_continuerequest continued request, which is an e_mux
  assign data_cache_0_dataMaster_continuerequest = (last_cycle_data_cache_0_dataMaster_granted_slave_pipeline_bridge_MEMORY_s1 & 1) |
    (last_cycle_data_cache_0_dataMaster_granted_slave_pipeline_bridge_MEMORY_s1 & 1);

  //pipeline_bridge_MEMORY_s1_any_continuerequest at least one master continues requesting, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_any_continuerequest = data_cache_0_dataMaster_continuerequest |
    tiger_top_0_instructionMaster_continuerequest |
    data_cache_0_AccelMaster_continuerequest |
    tiger_top_0_instructionMaster_continuerequest |
    data_cache_0_AccelMaster_continuerequest |
    data_cache_0_dataMaster_continuerequest;

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
  assign tiger_top_0_instructionMaster_continuerequest = (last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1 & 1) |
    (last_cycle_tiger_top_0_instructionMaster_granted_slave_pipeline_bridge_MEMORY_s1 & 1);

  assign data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1 = data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1 & ~((data_cache_0_AccelMaster_read & ((data_cache_0_AccelMaster_latency_counter != 0) | (1 < data_cache_0_AccelMaster_latency_counter))) | data_cache_0_dataMaster_arbiterlock | tiger_top_0_instructionMaster_arbiterlock);
  //unique name for pipeline_bridge_MEMORY_s1_move_on_to_next_transaction, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_move_on_to_next_transaction = pipeline_bridge_MEMORY_s1_this_cycle_is_the_last_burst & pipeline_bridge_MEMORY_s1_load_fifo;

  //the currently selected burstcount for pipeline_bridge_MEMORY_s1, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_selected_burstcount = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_AccelMaster_burstcount :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_dataMaster_burstcount :
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

  //rdv_fifo_for_data_cache_0_AccelMaster_to_pipeline_bridge_MEMORY_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_data_cache_0_AccelMaster_to_pipeline_bridge_MEMORY_s1_module rdv_fifo_for_data_cache_0_AccelMaster_to_pipeline_bridge_MEMORY_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1),
      .data_out             (data_cache_0_AccelMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1),
      .empty                (),
      .fifo_contains_ones_n (data_cache_0_AccelMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1),
      .full                 (),
      .read                 (pipeline_bridge_MEMORY_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read)
    );

  assign data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register = ~data_cache_0_AccelMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  //local readdatavalid data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1, which is an e_mux
  assign data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1 = (pipeline_bridge_MEMORY_s1_readdatavalid_from_sa & data_cache_0_AccelMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1) & ~ data_cache_0_AccelMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;

  //pipeline_bridge_MEMORY_s1_writedata mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_writedata = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_AccelMaster_writedata :
    data_cache_0_dataMaster_writedata;

  //assign pipeline_bridge_MEMORY_s1_endofpacket_from_sa = pipeline_bridge_MEMORY_s1_endofpacket so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_endofpacket_from_sa = pipeline_bridge_MEMORY_s1_endofpacket;

  assign data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1 = ({data_cache_0_dataMaster_address_to_slave[31 : 25] , 25'b0} == 32'h0) & (data_cache_0_dataMaster_read | data_cache_0_dataMaster_write);
  //data_cache_0/AccelMaster granted pipeline_bridge_MEMORY/s1 last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_cycle_data_cache_0_AccelMaster_granted_slave_pipeline_bridge_MEMORY_s1 <= 0;
      else 
        last_cycle_data_cache_0_AccelMaster_granted_slave_pipeline_bridge_MEMORY_s1 <= data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1 ? 1 : (pipeline_bridge_MEMORY_s1_arbitration_holdoff_internal | 0) ? 0 : last_cycle_data_cache_0_AccelMaster_granted_slave_pipeline_bridge_MEMORY_s1;
    end


  //data_cache_0_AccelMaster_continuerequest continued request, which is an e_mux
  assign data_cache_0_AccelMaster_continuerequest = (last_cycle_data_cache_0_AccelMaster_granted_slave_pipeline_bridge_MEMORY_s1 & 1) |
    (last_cycle_data_cache_0_AccelMaster_granted_slave_pipeline_bridge_MEMORY_s1 & 1);

  assign data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1 = data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1 & ~((data_cache_0_dataMaster_read & ((data_cache_0_dataMaster_latency_counter != 0) | (1 < data_cache_0_dataMaster_latency_counter) | (|data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register))) | data_cache_0_AccelMaster_arbiterlock | tiger_top_0_instructionMaster_arbiterlock);
  //rdv_fifo_for_data_cache_0_dataMaster_to_pipeline_bridge_MEMORY_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_data_cache_0_dataMaster_to_pipeline_bridge_MEMORY_s1_module rdv_fifo_for_data_cache_0_dataMaster_to_pipeline_bridge_MEMORY_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1),
      .data_out             (data_cache_0_dataMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1),
      .empty                (),
      .fifo_contains_ones_n (data_cache_0_dataMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1),
      .full                 (),
      .read                 (pipeline_bridge_MEMORY_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~pipeline_bridge_MEMORY_s1_waits_for_read)
    );

  assign data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register = ~data_cache_0_dataMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;
  //local readdatavalid data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1, which is an e_mux
  assign data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1 = (pipeline_bridge_MEMORY_s1_readdatavalid_from_sa & data_cache_0_dataMaster_rdv_fifo_output_from_pipeline_bridge_MEMORY_s1) & ~ data_cache_0_dataMaster_rdv_fifo_empty_pipeline_bridge_MEMORY_s1;

  assign tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1 = (({tiger_top_0_instructionMaster_address_to_slave[31 : 25] , 25'b0} == 32'h0) & (tiger_top_0_instructionMaster_read)) & tiger_top_0_instructionMaster_read;
  assign tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1 = tiger_top_0_instructionMaster_requests_pipeline_bridge_MEMORY_s1 & ~((tiger_top_0_instructionMaster_read & ((tiger_top_0_instructionMaster_latency_counter != 0) | (1 < tiger_top_0_instructionMaster_latency_counter))) | data_cache_0_AccelMaster_arbiterlock | data_cache_0_dataMaster_arbiterlock);
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
  assign pipeline_bridge_MEMORY_s1_allow_new_arb_cycle = ~data_cache_0_AccelMaster_arbiterlock & ~data_cache_0_dataMaster_arbiterlock & ~tiger_top_0_instructionMaster_arbiterlock;

  //tiger_top_0/instructionMaster assignment into master qualified-requests vector for pipeline_bridge_MEMORY/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_master_qreq_vector[0] = tiger_top_0_instructionMaster_qualified_request_pipeline_bridge_MEMORY_s1;

  //tiger_top_0/instructionMaster grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_grant_vector[0];

  //tiger_top_0/instructionMaster saved-grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_arb_winner[0];

  //data_cache_0/dataMaster assignment into master qualified-requests vector for pipeline_bridge_MEMORY/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_master_qreq_vector[1] = data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1;

  //data_cache_0/dataMaster grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_grant_vector[1];

  //data_cache_0/dataMaster saved-grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_arb_winner[1];

  //data_cache_0/AccelMaster assignment into master qualified-requests vector for pipeline_bridge_MEMORY/s1, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_master_qreq_vector[2] = data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1;

  //data_cache_0/AccelMaster grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_grant_vector[2];

  //data_cache_0/AccelMaster saved-grant pipeline_bridge_MEMORY/s1, which is an e_assign
  assign data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1 = pipeline_bridge_MEMORY_s1_arb_winner[2];

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
  assign pipeline_bridge_MEMORY_s1_grant_vector = {(pipeline_bridge_MEMORY_s1_chosen_master_double_vector[2] | pipeline_bridge_MEMORY_s1_chosen_master_double_vector[5]),
    (pipeline_bridge_MEMORY_s1_chosen_master_double_vector[1] | pipeline_bridge_MEMORY_s1_chosen_master_double_vector[4]),
    (pipeline_bridge_MEMORY_s1_chosen_master_double_vector[0] | pipeline_bridge_MEMORY_s1_chosen_master_double_vector[3])};

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

  assign pipeline_bridge_MEMORY_s1_chipselect = data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 | data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 | tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1;
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
  assign pipeline_bridge_MEMORY_s1_read = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_AccelMaster_read) | (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster_read) | (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 & tiger_top_0_instructionMaster_read);

  //pipeline_bridge_MEMORY_s1_write assignment, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_write = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_AccelMaster_write) | (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster_write);

  assign shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_AccelMaster = data_cache_0_AccelMaster_address_to_slave;
  //pipeline_bridge_MEMORY_s1_address mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_address = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? (shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_AccelMaster >> 2) :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? (shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_dataMaster >> 2) :
    (shifted_address_to_pipeline_bridge_MEMORY_s1_from_tiger_top_0_instructionMaster >> 2);

  assign shifted_address_to_pipeline_bridge_MEMORY_s1_from_data_cache_0_dataMaster = data_cache_0_dataMaster_address_to_slave;
  assign shifted_address_to_pipeline_bridge_MEMORY_s1_from_tiger_top_0_instructionMaster = tiger_top_0_instructionMaster_address_to_slave;
  //slaveid pipeline_bridge_MEMORY_s1_nativeaddress nativeaddress mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_nativeaddress = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? (data_cache_0_AccelMaster_address_to_slave >> 2) :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? (data_cache_0_dataMaster_address_to_slave >> 2) :
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
  assign pipeline_bridge_MEMORY_s1_in_a_read_cycle = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_AccelMaster_read) | (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster_read) | (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 & tiger_top_0_instructionMaster_read);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = pipeline_bridge_MEMORY_s1_in_a_read_cycle;

  //pipeline_bridge_MEMORY_s1_waits_for_write in a cycle, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_waits_for_write = pipeline_bridge_MEMORY_s1_in_a_write_cycle & pipeline_bridge_MEMORY_s1_waitrequest_from_sa;

  //pipeline_bridge_MEMORY_s1_in_a_write_cycle assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_s1_in_a_write_cycle = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_AccelMaster_write) | (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 & data_cache_0_dataMaster_write);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = pipeline_bridge_MEMORY_s1_in_a_write_cycle;

  assign wait_for_pipeline_bridge_MEMORY_s1_counter = 0;
  //pipeline_bridge_MEMORY_s1_byteenable byte enable port mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_byteenable = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_AccelMaster_byteenable :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_dataMaster_byteenable :
    -1;

  //burstcount mux, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_burstcount = (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_AccelMaster_burstcount :
    (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1)? data_cache_0_dataMaster_burstcount :
    (tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1)? tiger_top_0_instructionMaster_burstcount :
    1;

  //pipeline_bridge_MEMORY/s1 arbiterlock assigned from _handle_arbiterlock, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_arbiterlock = (data_cache_0_AccelMaster_arbiterlock)? data_cache_0_AccelMaster_arbiterlock :
    (data_cache_0_dataMaster_arbiterlock)? data_cache_0_dataMaster_arbiterlock :
    tiger_top_0_instructionMaster_arbiterlock;

  //pipeline_bridge_MEMORY/s1 arbiterlock2 assigned from _handle_arbiterlock2, which is an e_mux
  assign pipeline_bridge_MEMORY_s1_arbiterlock2 = (data_cache_0_AccelMaster_arbiterlock2)? data_cache_0_AccelMaster_arbiterlock2 :
    (data_cache_0_dataMaster_arbiterlock2)? data_cache_0_dataMaster_arbiterlock2 :
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


  //data_cache_0/AccelMaster non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1 && (data_cache_0_AccelMaster_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: data_cache_0/AccelMaster drove 0 on its 'burstcount' port while accessing slave pipeline_bridge_MEMORY/s1", $time);
          $stop;
        end
    end


  //data_cache_0/dataMaster non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1 && (data_cache_0_dataMaster_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: data_cache_0/dataMaster drove 0 on its 'burstcount' port while accessing slave pipeline_bridge_MEMORY/s1", $time);
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
      if (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1 + data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1 + tiger_top_0_instructionMaster_granted_pipeline_bridge_MEMORY_s1 > 1)
        begin
          $write("%0d ns: > 1 of grant signals are active simultaneously", $time);
          $stop;
        end
    end


  //saved_grant signals are active simultaneously, which is an e_process
  always @(posedge clk)
    begin
      if (data_cache_0_AccelMaster_saved_grant_pipeline_bridge_MEMORY_s1 + data_cache_0_dataMaster_saved_grant_pipeline_bridge_MEMORY_s1 + tiger_top_0_instructionMaster_saved_grant_pipeline_bridge_MEMORY_s1 > 1)
        begin
          $write("%0d ns: > 1 of saved_grant signals are active simultaneously", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_module (
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

  output  [  1: 0] data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input   [  1: 0] data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire    [  1: 0] data_out;
  wire             empty;
  reg              fifo_contains_ones_n;
  wire             full;
  reg              full_0;
  reg              full_1;
  wire             full_2;
  reg     [  2: 0] how_many_ones;
  wire    [  2: 0] one_count_minus_one;
  wire    [  2: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  1: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  1: 0] p1_stage_1;
  reg     [  1: 0] stage_0;
  reg     [  1: 0] stage_1;
  wire    [  2: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_1;
  assign empty = !full_0;
  assign full_2 = 0;
  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_MEMORY_m1_arbitrator (
                                              // inputs:
                                               clk,
                                               d1_tiger_burst_0_upstream_end_xfer,
                                               d1_tiger_burst_1_upstream_end_xfer,
                                               d1_tiger_burst_2_upstream_end_xfer,
                                               pipeline_bridge_MEMORY_m1_address,
                                               pipeline_bridge_MEMORY_m1_burstcount,
                                               pipeline_bridge_MEMORY_m1_byteenable,
                                               pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_chipselect,
                                               pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream,
                                               pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream,
                                               pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream,
                                               pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream,
                                               pipeline_bridge_MEMORY_m1_read,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream,
                                               pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register,
                                               pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream,
                                               pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream,
                                               pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream,
                                               pipeline_bridge_MEMORY_m1_write,
                                               pipeline_bridge_MEMORY_m1_writedata,
                                               reset_n,
                                               tiger_burst_0_upstream_readdata_from_sa,
                                               tiger_burst_0_upstream_waitrequest_from_sa,
                                               tiger_burst_1_upstream_readdata_from_sa,
                                               tiger_burst_1_upstream_waitrequest_from_sa,
                                               tiger_burst_2_upstream_readdata_from_sa,
                                               tiger_burst_2_upstream_waitrequest_from_sa,

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

  output  [ 24: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  output  [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  output  [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  output           pipeline_bridge_MEMORY_m1_latency_counter;
  output  [ 31: 0] pipeline_bridge_MEMORY_m1_readdata;
  output           pipeline_bridge_MEMORY_m1_readdatavalid;
  output           pipeline_bridge_MEMORY_m1_waitrequest;
  input            clk;
  input            d1_tiger_burst_0_upstream_end_xfer;
  input            d1_tiger_burst_1_upstream_end_xfer;
  input            d1_tiger_burst_2_upstream_end_xfer;
  input   [ 24: 0] pipeline_bridge_MEMORY_m1_address;
  input   [  2: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input   [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input            pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream;
  input            pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream;
  input            pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream;
  input            pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream;
  input            pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream;
  input            pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream;
  input            pipeline_bridge_MEMORY_m1_write;
  input   [ 31: 0] pipeline_bridge_MEMORY_m1_writedata;
  input            reset_n;
  input   [ 15: 0] tiger_burst_0_upstream_readdata_from_sa;
  input            tiger_burst_0_upstream_waitrequest_from_sa;
  input   [ 31: 0] tiger_burst_1_upstream_readdata_from_sa;
  input            tiger_burst_1_upstream_waitrequest_from_sa;
  input   [127: 0] tiger_burst_2_upstream_readdata_from_sa;
  input            tiger_burst_2_upstream_waitrequest_from_sa;

  reg              active_and_waiting_last_time;
  wire             dbs_count_enable;
  wire             dbs_counter_overflow;
  reg     [ 15: 0] dbs_latent_16_reg_segment_0;
  wire             dbs_rdv_count_enable;
  wire             dbs_rdv_counter_overflow;
  wire             empty_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo;
  wire             full_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo;
  wire             latency_load_value;
  wire    [  1: 0] next_dbs_address;
  wire    [ 15: 0] p1_dbs_latent_16_reg_segment_0;
  wire             p1_pipeline_bridge_MEMORY_m1_latency_counter;
  reg     [ 24: 0] pipeline_bridge_MEMORY_m1_address_last_time;
  wire    [ 24: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  reg     [  2: 0] pipeline_bridge_MEMORY_m1_burstcount_last_time;
  reg     [  3: 0] pipeline_bridge_MEMORY_m1_byteenable_last_time;
  reg              pipeline_bridge_MEMORY_m1_chipselect_last_time;
  reg     [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_dbs_increment;
  reg     [  1: 0] pipeline_bridge_MEMORY_m1_dbs_rdv_counter;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_dbs_rdv_counter_inc;
  wire    [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  wire             pipeline_bridge_MEMORY_m1_is_granted_some_slave;
  reg              pipeline_bridge_MEMORY_m1_latency_counter;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_next_dbs_rdv_counter;
  reg              pipeline_bridge_MEMORY_m1_read_but_no_slave_selected;
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
  wire             read_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo;
  wire    [  1: 0] selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_output;
  wire    [  1: 0] selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_output_tiger_burst_2_upstream;
  wire    [ 31: 0] tiger_burst_2_upstream_readdata_from_sa_part_selected_by_negative_dbs;
  wire             write_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream | ~pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream | ~(pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) | (1 & ~tiger_burst_0_upstream_waitrequest_from_sa & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect)))) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream | ~(pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect) | (1 & ~tiger_burst_0_upstream_waitrequest_from_sa & (pipeline_bridge_MEMORY_m1_dbs_address[1]) & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect)))) & 1 & (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream | ~pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream | ~pipeline_bridge_MEMORY_m1_chipselect | (1 & ~tiger_burst_1_upstream_waitrequest_from_sa & pipeline_bridge_MEMORY_m1_chipselect))) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream | ~pipeline_bridge_MEMORY_m1_chipselect | (1 & ~tiger_burst_1_upstream_waitrequest_from_sa & pipeline_bridge_MEMORY_m1_chipselect))) & 1 & (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream | ~pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream | ~pipeline_bridge_MEMORY_m1_chipselect | (1 & ~tiger_burst_2_upstream_waitrequest_from_sa & pipeline_bridge_MEMORY_m1_chipselect))) & ((~pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream | ~pipeline_bridge_MEMORY_m1_chipselect | (1 & ~tiger_burst_2_upstream_waitrequest_from_sa & pipeline_bridge_MEMORY_m1_chipselect)));

  //cascaded wait assignment, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign pipeline_bridge_MEMORY_m1_address_to_slave = pipeline_bridge_MEMORY_m1_address[24 : 0];

  //pipeline_bridge_MEMORY_m1_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_read_but_no_slave_selected <= 0;
      else 
        pipeline_bridge_MEMORY_m1_read_but_no_slave_selected <= (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & pipeline_bridge_MEMORY_m1_run & ~pipeline_bridge_MEMORY_m1_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_is_granted_some_slave = pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream |
    pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream |
    pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid = (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream & dbs_rdv_counter_overflow) |
    pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream |
    pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_readdatavalid = pipeline_bridge_MEMORY_m1_read_but_no_slave_selected |
    pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid |
    pipeline_bridge_MEMORY_m1_read_but_no_slave_selected |
    pre_flush_pipeline_bridge_MEMORY_m1_readdatavalid |
    pipeline_bridge_MEMORY_m1_read_but_no_slave_selected |
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
  assign pipeline_bridge_MEMORY_m1_readdata = ({32 {~pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream}} | {tiger_burst_0_upstream_readdata_from_sa[15 : 0],
    dbs_latent_16_reg_segment_0}) &
    ({32 {~pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream}} | tiger_burst_1_upstream_readdata_from_sa) &
    ({32 {~pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream}} | tiger_burst_2_upstream_readdata_from_sa_part_selected_by_negative_dbs);

  //mux write dbs 1, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_dbs_write_16 = (pipeline_bridge_MEMORY_m1_dbs_address[1])? pipeline_bridge_MEMORY_m1_writedata[31 : 16] :
    pipeline_bridge_MEMORY_m1_writedata[15 : 0];

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

  //Negative Dynamic Bus-sizing mux.
  //this mux selects the correct fourth of the 
  //wide data coming from the slave tiger_burst_2/upstream 
  assign tiger_burst_2_upstream_readdata_from_sa_part_selected_by_negative_dbs = ((selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_output_tiger_burst_2_upstream == 0))? tiger_burst_2_upstream_readdata_from_sa[31 : 0] :
    ((selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_output_tiger_burst_2_upstream == 1))? tiger_burst_2_upstream_readdata_from_sa[63 : 32] :
    ((selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_output_tiger_burst_2_upstream == 2))? tiger_burst_2_upstream_readdata_from_sa[95 : 64] :
    tiger_burst_2_upstream_readdata_from_sa[127 : 96];

  //read_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo fifo read, which is an e_mux
  assign read_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo = pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream;

  //write_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo fifo write, which is an e_mux
  assign write_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo = (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & pipeline_bridge_MEMORY_m1_run & pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream;

  assign selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_output_tiger_burst_2_upstream = 2'b0;
  //selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo, which is an e_fifo_with_registered_outputs
  selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_module selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo
    (
      .clear_fifo           (1'b1),
      .clk                  (clk),
      .data_in              (pipeline_bridge_MEMORY_m1_address_to_slave[3 : 2]),
      .data_out             (selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo_output),
      .empty                (empty_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo),
      .fifo_contains_ones_n (),
      .full                 (full_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo),
      .read                 (1'b0),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (1'b0)
    );


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


  //selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo read when empty, which is an e_process
  always @(posedge clk)
    begin
      if (empty_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo & 1'b0)
        begin
          $write("%0d ns: pipeline_bridge_MEMORY/m1 negative rdv fifo selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo: read AND empty.\n", $time);
          $stop;
        end
    end


  //selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo write when full, which is an e_process
  always @(posedge clk)
    begin
      if (full_selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo & 1'b0 & ~1'b0)
        begin
          $write("%0d ns: pipeline_bridge_MEMORY/m1 negative rdv fifo selecto_nrdv_pipeline_bridge_MEMORY_m1_2_tiger_burst_2_upstream_fifo: write AND full.\n", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_MEMORY_bridge_arbitrator 
;



endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_tiger_burst_3_downstream_to_pipeline_bridge_PERIPHERALS_s1_module (
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
                                                    tiger_burst_3_downstream_address_to_slave,
                                                    tiger_burst_3_downstream_arbitrationshare,
                                                    tiger_burst_3_downstream_burstcount,
                                                    tiger_burst_3_downstream_byteenable,
                                                    tiger_burst_3_downstream_debugaccess,
                                                    tiger_burst_3_downstream_latency_counter,
                                                    tiger_burst_3_downstream_nativeaddress,
                                                    tiger_burst_3_downstream_read,
                                                    tiger_burst_3_downstream_write,
                                                    tiger_burst_3_downstream_writedata,

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
                                                    tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1,
                                                    tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1,
                                                    tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1,
                                                    tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register,
                                                    tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1
                                                 )
;

  output           d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  output  [ 11: 0] pipeline_bridge_PERIPHERALS_s1_address;
  output           pipeline_bridge_PERIPHERALS_s1_arbiterlock;
  output           pipeline_bridge_PERIPHERALS_s1_arbiterlock2;
  output           pipeline_bridge_PERIPHERALS_s1_burstcount;
  output  [  3: 0] pipeline_bridge_PERIPHERALS_s1_byteenable;
  output           pipeline_bridge_PERIPHERALS_s1_chipselect;
  output           pipeline_bridge_PERIPHERALS_s1_debugaccess;
  output           pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  output  [ 11: 0] pipeline_bridge_PERIPHERALS_s1_nativeaddress;
  output           pipeline_bridge_PERIPHERALS_s1_read;
  output  [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;
  output           pipeline_bridge_PERIPHERALS_s1_reset_n;
  output           pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;
  output           pipeline_bridge_PERIPHERALS_s1_write;
  output  [ 31: 0] pipeline_bridge_PERIPHERALS_s1_writedata;
  output           tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1;
  output           tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  output           tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  output           tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  output           tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1;
  input            clk;
  input            pipeline_bridge_PERIPHERALS_s1_endofpacket;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata;
  input            pipeline_bridge_PERIPHERALS_s1_readdatavalid;
  input            pipeline_bridge_PERIPHERALS_s1_waitrequest;
  input            reset_n;
  input   [ 13: 0] tiger_burst_3_downstream_address_to_slave;
  input   [  2: 0] tiger_burst_3_downstream_arbitrationshare;
  input            tiger_burst_3_downstream_burstcount;
  input   [  3: 0] tiger_burst_3_downstream_byteenable;
  input            tiger_burst_3_downstream_debugaccess;
  input            tiger_burst_3_downstream_latency_counter;
  input   [ 13: 0] tiger_burst_3_downstream_nativeaddress;
  input            tiger_burst_3_downstream_read;
  input            tiger_burst_3_downstream_write;
  input   [ 31: 0] tiger_burst_3_downstream_writedata;

  reg              d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  reg              d1_reasons_to_wait;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_pipeline_bridge_PERIPHERALS_s1;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire    [ 11: 0] pipeline_bridge_PERIPHERALS_s1_address;
  wire             pipeline_bridge_PERIPHERALS_s1_allgrants;
  wire             pipeline_bridge_PERIPHERALS_s1_allow_new_arb_cycle;
  wire             pipeline_bridge_PERIPHERALS_s1_any_bursting_master_saved_grant;
  wire             pipeline_bridge_PERIPHERALS_s1_any_continuerequest;
  wire             pipeline_bridge_PERIPHERALS_s1_arb_counter_enable;
  reg     [  2: 0] pipeline_bridge_PERIPHERALS_s1_arb_share_counter;
  wire    [  2: 0] pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value;
  wire    [  2: 0] pipeline_bridge_PERIPHERALS_s1_arb_share_set_values;
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
  wire    [ 11: 0] pipeline_bridge_PERIPHERALS_s1_nativeaddress;
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
  wire    [ 13: 0] shifted_address_to_pipeline_bridge_PERIPHERALS_s1_from_tiger_burst_3_downstream;
  wire             tiger_burst_3_downstream_arbiterlock;
  wire             tiger_burst_3_downstream_arbiterlock2;
  wire             tiger_burst_3_downstream_continuerequest;
  wire             tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_rdv_fifo_empty_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_rdv_fifo_output_from_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  wire             tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_saved_grant_pipeline_bridge_PERIPHERALS_s1;
  wire             wait_for_pipeline_bridge_PERIPHERALS_s1_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~pipeline_bridge_PERIPHERALS_s1_end_xfer;
    end


  assign pipeline_bridge_PERIPHERALS_s1_begins_xfer = ~d1_reasons_to_wait & ((tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1));
  //assign pipeline_bridge_PERIPHERALS_s1_readdata_from_sa = pipeline_bridge_PERIPHERALS_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_readdata_from_sa = pipeline_bridge_PERIPHERALS_s1_readdata;

  assign tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1 = (1) & (tiger_burst_3_downstream_read | tiger_burst_3_downstream_write);
  //assign pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa = pipeline_bridge_PERIPHERALS_s1_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa = pipeline_bridge_PERIPHERALS_s1_waitrequest;

  //assign pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa = pipeline_bridge_PERIPHERALS_s1_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa = pipeline_bridge_PERIPHERALS_s1_readdatavalid;

  //pipeline_bridge_PERIPHERALS_s1_arb_share_counter set values, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_arb_share_set_values = (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1)? tiger_burst_3_downstream_arbitrationshare :
    (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1)? tiger_burst_3_downstream_arbitrationshare :
    1;

  //pipeline_bridge_PERIPHERALS_s1_non_bursting_master_requests mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_non_bursting_master_requests = 0;

  //pipeline_bridge_PERIPHERALS_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_any_bursting_master_saved_grant = tiger_burst_3_downstream_saved_grant_pipeline_bridge_PERIPHERALS_s1 |
    tiger_burst_3_downstream_saved_grant_pipeline_bridge_PERIPHERALS_s1;

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


  //tiger_burst_3/downstream pipeline_bridge_PERIPHERALS/s1 arbiterlock, which is an e_assign
  assign tiger_burst_3_downstream_arbiterlock = pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable & tiger_burst_3_downstream_continuerequest;

  //pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable2 = |pipeline_bridge_PERIPHERALS_s1_arb_share_counter_next_value;

  //tiger_burst_3/downstream pipeline_bridge_PERIPHERALS/s1 arbiterlock2, which is an e_assign
  assign tiger_burst_3_downstream_arbiterlock2 = pipeline_bridge_PERIPHERALS_s1_slavearbiterlockenable2 & tiger_burst_3_downstream_continuerequest;

  //pipeline_bridge_PERIPHERALS_s1_any_continuerequest at least one master continues requesting, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_any_continuerequest = 1;

  //tiger_burst_3_downstream_continuerequest continued request, which is an e_assign
  assign tiger_burst_3_downstream_continuerequest = 1;

  assign tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1 = tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1 & ~((tiger_burst_3_downstream_read & ((tiger_burst_3_downstream_latency_counter != 0) | (1 < tiger_burst_3_downstream_latency_counter))));
  //unique name for pipeline_bridge_PERIPHERALS_s1_move_on_to_next_transaction, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_move_on_to_next_transaction = pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa;

  //rdv_fifo_for_tiger_burst_3_downstream_to_pipeline_bridge_PERIPHERALS_s1, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_tiger_burst_3_downstream_to_pipeline_bridge_PERIPHERALS_s1_module rdv_fifo_for_tiger_burst_3_downstream_to_pipeline_bridge_PERIPHERALS_s1
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1),
      .data_out             (tiger_burst_3_downstream_rdv_fifo_output_from_pipeline_bridge_PERIPHERALS_s1),
      .empty                (),
      .fifo_contains_ones_n (tiger_burst_3_downstream_rdv_fifo_empty_pipeline_bridge_PERIPHERALS_s1),
      .full                 (),
      .read                 (pipeline_bridge_PERIPHERALS_s1_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~pipeline_bridge_PERIPHERALS_s1_waits_for_read)
    );

  assign tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register = ~tiger_burst_3_downstream_rdv_fifo_empty_pipeline_bridge_PERIPHERALS_s1;
  //local readdatavalid tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1, which is an e_mux
  assign tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1 = pipeline_bridge_PERIPHERALS_s1_readdatavalid_from_sa;

  //pipeline_bridge_PERIPHERALS_s1_writedata mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_writedata = tiger_burst_3_downstream_writedata;

  //assign pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa = pipeline_bridge_PERIPHERALS_s1_endofpacket so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa = pipeline_bridge_PERIPHERALS_s1_endofpacket;

  //master is always granted when requested
  assign tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1 = tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1;

  //tiger_burst_3/downstream saved-grant pipeline_bridge_PERIPHERALS/s1, which is an e_assign
  assign tiger_burst_3_downstream_saved_grant_pipeline_bridge_PERIPHERALS_s1 = tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1;

  //allow new arb cycle for pipeline_bridge_PERIPHERALS/s1, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign pipeline_bridge_PERIPHERALS_s1_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign pipeline_bridge_PERIPHERALS_s1_master_qreq_vector = 1;

  //pipeline_bridge_PERIPHERALS_s1_reset_n assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_reset_n = reset_n;

  assign pipeline_bridge_PERIPHERALS_s1_chipselect = tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1;
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
  assign pipeline_bridge_PERIPHERALS_s1_read = tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_burst_3_downstream_read;

  //pipeline_bridge_PERIPHERALS_s1_write assignment, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_write = tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_burst_3_downstream_write;

  assign shifted_address_to_pipeline_bridge_PERIPHERALS_s1_from_tiger_burst_3_downstream = tiger_burst_3_downstream_address_to_slave;
  //pipeline_bridge_PERIPHERALS_s1_address mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_address = shifted_address_to_pipeline_bridge_PERIPHERALS_s1_from_tiger_burst_3_downstream >> 2;

  //slaveid pipeline_bridge_PERIPHERALS_s1_nativeaddress nativeaddress mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_nativeaddress = tiger_burst_3_downstream_nativeaddress;

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
  assign pipeline_bridge_PERIPHERALS_s1_in_a_read_cycle = tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_burst_3_downstream_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = pipeline_bridge_PERIPHERALS_s1_in_a_read_cycle;

  //pipeline_bridge_PERIPHERALS_s1_waits_for_write in a cycle, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_waits_for_write = pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle & pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;

  //pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle = tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1 & tiger_burst_3_downstream_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = pipeline_bridge_PERIPHERALS_s1_in_a_write_cycle;

  assign wait_for_pipeline_bridge_PERIPHERALS_s1_counter = 0;
  //pipeline_bridge_PERIPHERALS_s1_byteenable byte enable port mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_byteenable = (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1)? tiger_burst_3_downstream_byteenable :
    -1;

  //burstcount mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_burstcount = (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1)? tiger_burst_3_downstream_burstcount :
    1;

  //pipeline_bridge_PERIPHERALS/s1 arbiterlock assigned from _handle_arbiterlock, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_arbiterlock = tiger_burst_3_downstream_arbiterlock;

  //pipeline_bridge_PERIPHERALS/s1 arbiterlock2 assigned from _handle_arbiterlock2, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_arbiterlock2 = tiger_burst_3_downstream_arbiterlock2;

  //debugaccess mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_s1_debugaccess = (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1)? tiger_burst_3_downstream_debugaccess :
    0;


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


  //tiger_burst_3/downstream non-zero arbitrationshare assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1 && (tiger_burst_3_downstream_arbitrationshare == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_3/downstream drove 0 on its 'arbitrationshare' port while accessing slave pipeline_bridge_PERIPHERALS/s1", $time);
          $stop;
        end
    end


  //tiger_burst_3/downstream non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1 && (tiger_burst_3_downstream_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: tiger_burst_3/downstream drove 0 on its 'burstcount' port while accessing slave pipeline_bridge_PERIPHERALS/s1", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_PERIPHERALS_m1_arbitrator (
                                                   // inputs:
                                                    clk,
                                                    d1_tigers_jtag_uart_1_controlSlave_end_xfer,
                                                    d1_tigers_jtag_uart_controlSlave_end_xfer,
                                                    d1_uart_0_s1_end_xfer,
                                                    pipeline_bridge_PERIPHERALS_m1_address,
                                                    pipeline_bridge_PERIPHERALS_m1_burstcount,
                                                    pipeline_bridge_PERIPHERALS_m1_byteenable,
                                                    pipeline_bridge_PERIPHERALS_m1_chipselect,
                                                    pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_read,
                                                    pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave,
                                                    pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1,
                                                    pipeline_bridge_PERIPHERALS_m1_write,
                                                    pipeline_bridge_PERIPHERALS_m1_writedata,
                                                    reset_n,
                                                    tigers_jtag_uart_1_controlSlave_readdata_from_sa,
                                                    tigers_jtag_uart_controlSlave_readdata_from_sa,
                                                    uart_0_s1_readdata_from_sa,

                                                   // outputs:
                                                    pipeline_bridge_PERIPHERALS_m1_address_to_slave,
                                                    pipeline_bridge_PERIPHERALS_m1_latency_counter,
                                                    pipeline_bridge_PERIPHERALS_m1_readdata,
                                                    pipeline_bridge_PERIPHERALS_m1_readdatavalid,
                                                    pipeline_bridge_PERIPHERALS_m1_waitrequest
                                                 )
;

  output  [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  output           pipeline_bridge_PERIPHERALS_m1_latency_counter;
  output  [ 31: 0] pipeline_bridge_PERIPHERALS_m1_readdata;
  output           pipeline_bridge_PERIPHERALS_m1_readdatavalid;
  output           pipeline_bridge_PERIPHERALS_m1_waitrequest;
  input            clk;
  input            d1_tigers_jtag_uart_1_controlSlave_end_xfer;
  input            d1_tigers_jtag_uart_controlSlave_end_xfer;
  input            d1_uart_0_s1_end_xfer;
  input   [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address;
  input            pipeline_bridge_PERIPHERALS_m1_burstcount;
  input   [  3: 0] pipeline_bridge_PERIPHERALS_m1_byteenable;
  input            pipeline_bridge_PERIPHERALS_m1_chipselect;
  input            pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_read;
  input            pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave;
  input            pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;
  input            pipeline_bridge_PERIPHERALS_m1_write;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata;
  input            reset_n;
  input   [ 31: 0] tigers_jtag_uart_1_controlSlave_readdata_from_sa;
  input   [ 31: 0] tigers_jtag_uart_controlSlave_readdata_from_sa;
  input   [ 15: 0] uart_0_s1_readdata_from_sa;

  reg              active_and_waiting_last_time;
  wire             latency_load_value;
  wire             p1_pipeline_bridge_PERIPHERALS_m1_latency_counter;
  reg     [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address_last_time;
  wire    [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  reg              pipeline_bridge_PERIPHERALS_m1_burstcount_last_time;
  reg     [  3: 0] pipeline_bridge_PERIPHERALS_m1_byteenable_last_time;
  reg              pipeline_bridge_PERIPHERALS_m1_chipselect_last_time;
  wire             pipeline_bridge_PERIPHERALS_m1_is_granted_some_slave;
  reg              pipeline_bridge_PERIPHERALS_m1_latency_counter;
  reg              pipeline_bridge_PERIPHERALS_m1_read_but_no_slave_selected;
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
  assign r_0 = 1 & (pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave | ~pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & pipeline_bridge_PERIPHERALS_m1_chipselect))) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & pipeline_bridge_PERIPHERALS_m1_chipselect))) & 1 & (pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave | ~pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & pipeline_bridge_PERIPHERALS_m1_chipselect))) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & pipeline_bridge_PERIPHERALS_m1_chipselect))) & 1 & (pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 | ~pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & ~d1_uart_0_s1_end_xfer & pipeline_bridge_PERIPHERALS_m1_chipselect))) & ((~pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 | ~pipeline_bridge_PERIPHERALS_m1_chipselect | (1 & ~d1_uart_0_s1_end_xfer & pipeline_bridge_PERIPHERALS_m1_chipselect)));

  //cascaded wait assignment, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign pipeline_bridge_PERIPHERALS_m1_address_to_slave = {pipeline_bridge_PERIPHERALS_m1_address[13 : 11],
    3'b0,
    pipeline_bridge_PERIPHERALS_m1_address[7],
    2'b0,
    pipeline_bridge_PERIPHERALS_m1_address[4 : 0]};

  //pipeline_bridge_PERIPHERALS_m1_read_but_no_slave_selected assignment, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_read_but_no_slave_selected <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_read_but_no_slave_selected <= (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect) & pipeline_bridge_PERIPHERALS_m1_run & ~pipeline_bridge_PERIPHERALS_m1_is_granted_some_slave;
    end


  //some slave is getting selected, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_is_granted_some_slave = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave |
    pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave |
    pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_pipeline_bridge_PERIPHERALS_m1_readdatavalid = 0;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_readdatavalid = pipeline_bridge_PERIPHERALS_m1_read_but_no_slave_selected |
    pre_flush_pipeline_bridge_PERIPHERALS_m1_readdatavalid |
    pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave |
    pipeline_bridge_PERIPHERALS_m1_read_but_no_slave_selected |
    pre_flush_pipeline_bridge_PERIPHERALS_m1_readdatavalid |
    pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave |
    pipeline_bridge_PERIPHERALS_m1_read_but_no_slave_selected |
    pre_flush_pipeline_bridge_PERIPHERALS_m1_readdatavalid |
    pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;

  //pipeline_bridge_PERIPHERALS/m1 readdata mux, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_readdata = ({32 {~((pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect)))}} | tigers_jtag_uart_controlSlave_readdata_from_sa) &
    ({32 {~((pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect)))}} | tigers_jtag_uart_1_controlSlave_readdata_from_sa) &
    ({32 {~((pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1 & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect)))}} | uart_0_s1_readdata_from_sa);

  //actual waitrequest port, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_waitrequest = ~pipeline_bridge_PERIPHERALS_m1_run;

  //latent max counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_PERIPHERALS_m1_latency_counter <= 0;
      else 
        pipeline_bridge_PERIPHERALS_m1_latency_counter <= p1_pipeline_bridge_PERIPHERALS_m1_latency_counter;
    end


  //latency counter load mux, which is an e_mux
  assign p1_pipeline_bridge_PERIPHERALS_m1_latency_counter = ((pipeline_bridge_PERIPHERALS_m1_run & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect)))? latency_load_value :
    (pipeline_bridge_PERIPHERALS_m1_latency_counter)? pipeline_bridge_PERIPHERALS_m1_latency_counter - 1 :
    0;

  //read latency load values, which is an e_mux
  assign latency_load_value = 0;


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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module pipeline_bridge_PERIPHERALS_bridge_arbitrator 
;



endmodule



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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module sdram_s1_arbitrator (
                             // inputs:
                              clk,
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
  input            reset_n;
  input   [ 15: 0] sdram_s1_readdata;
  input            sdram_s1_readdatavalid;
  input            sdram_s1_waitrequest;
  input   [ 22: 0] tiger_burst_0_downstream_address_to_slave;
  input   [  3: 0] tiger_burst_0_downstream_arbitrationshare;
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
  wire    [ 21: 0] sdram_s1_address;
  wire             sdram_s1_allgrants;
  wire             sdram_s1_allow_new_arb_cycle;
  wire             sdram_s1_any_bursting_master_saved_grant;
  wire             sdram_s1_any_continuerequest;
  wire             sdram_s1_arb_counter_enable;
  reg     [  3: 0] sdram_s1_arb_share_counter;
  wire    [  3: 0] sdram_s1_arb_share_counter_next_value;
  wire    [  3: 0] sdram_s1_arb_share_set_values;
  wire             sdram_s1_beginbursttransfer_internal;
  wire             sdram_s1_begins_xfer;
  wire    [  1: 0] sdram_s1_byteenable_n;
  wire             sdram_s1_chipselect;
  wire             sdram_s1_end_xfer;
  wire             sdram_s1_firsttransfer;
  wire             sdram_s1_grant_vector;
  wire             sdram_s1_in_a_read_cycle;
  wire             sdram_s1_in_a_write_cycle;
  wire             sdram_s1_master_qreq_vector;
  wire             sdram_s1_move_on_to_next_transaction;
  wire             sdram_s1_non_bursting_master_requests;
  wire             sdram_s1_read_n;
  wire    [ 15: 0] sdram_s1_readdata_from_sa;
  wire             sdram_s1_readdatavalid_from_sa;
  reg              sdram_s1_reg_firsttransfer;
  wire             sdram_s1_reset_n;
  reg              sdram_s1_slavearbiterlockenable;
  wire             sdram_s1_slavearbiterlockenable2;
  wire             sdram_s1_unreg_firsttransfer;
  wire             sdram_s1_waitrequest_from_sa;
  wire             sdram_s1_waits_for_read;
  wire             sdram_s1_waits_for_write;
  wire             sdram_s1_write_n;
  wire    [ 15: 0] sdram_s1_writedata;
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


  assign sdram_s1_begins_xfer = ~d1_reasons_to_wait & ((tiger_burst_0_downstream_qualified_request_sdram_s1));
  //assign sdram_s1_readdata_from_sa = sdram_s1_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign sdram_s1_readdata_from_sa = sdram_s1_readdata;

  assign tiger_burst_0_downstream_requests_sdram_s1 = (1) & (tiger_burst_0_downstream_read | tiger_burst_0_downstream_write);
  //assign sdram_s1_waitrequest_from_sa = sdram_s1_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign sdram_s1_waitrequest_from_sa = sdram_s1_waitrequest;

  //assign sdram_s1_readdatavalid_from_sa = sdram_s1_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign sdram_s1_readdatavalid_from_sa = sdram_s1_readdatavalid;

  //sdram_s1_arb_share_counter set values, which is an e_mux
  assign sdram_s1_arb_share_set_values = (tiger_burst_0_downstream_granted_sdram_s1)? tiger_burst_0_downstream_arbitrationshare :
    1;

  //sdram_s1_non_bursting_master_requests mux, which is an e_mux
  assign sdram_s1_non_bursting_master_requests = 0;

  //sdram_s1_any_bursting_master_saved_grant mux, which is an e_mux
  assign sdram_s1_any_bursting_master_saved_grant = tiger_burst_0_downstream_saved_grant_sdram_s1;

  //sdram_s1_arb_share_counter_next_value assignment, which is an e_assign
  assign sdram_s1_arb_share_counter_next_value = sdram_s1_firsttransfer ? (sdram_s1_arb_share_set_values - 1) : |sdram_s1_arb_share_counter ? (sdram_s1_arb_share_counter - 1) : 0;

  //sdram_s1_allgrants all slave grants, which is an e_mux
  assign sdram_s1_allgrants = |sdram_s1_grant_vector;

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


  //tiger_burst_0/downstream sdram/s1 arbiterlock, which is an e_assign
  assign tiger_burst_0_downstream_arbiterlock = sdram_s1_slavearbiterlockenable & tiger_burst_0_downstream_continuerequest;

  //sdram_s1_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign sdram_s1_slavearbiterlockenable2 = |sdram_s1_arb_share_counter_next_value;

  //tiger_burst_0/downstream sdram/s1 arbiterlock2, which is an e_assign
  assign tiger_burst_0_downstream_arbiterlock2 = sdram_s1_slavearbiterlockenable2 & tiger_burst_0_downstream_continuerequest;

  //sdram_s1_any_continuerequest at least one master continues requesting, which is an e_assign
  assign sdram_s1_any_continuerequest = 1;

  //tiger_burst_0_downstream_continuerequest continued request, which is an e_assign
  assign tiger_burst_0_downstream_continuerequest = 1;

  assign tiger_burst_0_downstream_qualified_request_sdram_s1 = tiger_burst_0_downstream_requests_sdram_s1 & ~((tiger_burst_0_downstream_read & ((tiger_burst_0_downstream_latency_counter != 0) | (1 < tiger_burst_0_downstream_latency_counter))));
  //unique name for sdram_s1_move_on_to_next_transaction, which is an e_assign
  assign sdram_s1_move_on_to_next_transaction = sdram_s1_readdatavalid_from_sa;

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
  assign tiger_burst_0_downstream_read_data_valid_sdram_s1 = sdram_s1_readdatavalid_from_sa;

  //sdram_s1_writedata mux, which is an e_mux
  assign sdram_s1_writedata = tiger_burst_0_downstream_writedata;

  //master is always granted when requested
  assign tiger_burst_0_downstream_granted_sdram_s1 = tiger_burst_0_downstream_qualified_request_sdram_s1;

  //tiger_burst_0/downstream saved-grant sdram/s1, which is an e_assign
  assign tiger_burst_0_downstream_saved_grant_sdram_s1 = tiger_burst_0_downstream_requests_sdram_s1;

  //allow new arb cycle for sdram/s1, which is an e_assign
  assign sdram_s1_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign sdram_s1_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign sdram_s1_master_qreq_vector = 1;

  //sdram_s1_reset_n assignment, which is an e_assign
  assign sdram_s1_reset_n = reset_n;

  assign sdram_s1_chipselect = tiger_burst_0_downstream_granted_sdram_s1;
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

  //~sdram_s1_read_n assignment, which is an e_mux
  assign sdram_s1_read_n = ~(tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_read);

  //~sdram_s1_write_n assignment, which is an e_mux
  assign sdram_s1_write_n = ~(tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_write);

  assign shifted_address_to_sdram_s1_from_tiger_burst_0_downstream = tiger_burst_0_downstream_address_to_slave;
  //sdram_s1_address mux, which is an e_mux
  assign sdram_s1_address = shifted_address_to_sdram_s1_from_tiger_burst_0_downstream >> 1;

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
  assign sdram_s1_in_a_read_cycle = tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = sdram_s1_in_a_read_cycle;

  //sdram_s1_waits_for_write in a cycle, which is an e_mux
  assign sdram_s1_waits_for_write = sdram_s1_in_a_write_cycle & sdram_s1_waitrequest_from_sa;

  //sdram_s1_in_a_write_cycle assignment, which is an e_assign
  assign sdram_s1_in_a_write_cycle = tiger_burst_0_downstream_granted_sdram_s1 & tiger_burst_0_downstream_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = sdram_s1_in_a_write_cycle;

  assign wait_for_sdram_s1_counter = 0;
  //~sdram_s1_byteenable_n byte enable port mux, which is an e_mux
  assign sdram_s1_byteenable_n = ~((tiger_burst_0_downstream_granted_sdram_s1)? tiger_burst_0_downstream_byteenable :
    -1);


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



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



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

  output  [  3: 0] data_out;
  output           empty;
  output           fifo_contains_ones_n;
  output           full;
  input            clear_fifo;
  input            clk;
  input   [  3: 0] data_in;
  input            read;
  input            reset_n;
  input            sync_reset;
  input            write;

  wire    [  3: 0] data_out;
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
  wire    [  3: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  3: 0] p1_stage_1;
  wire             p2_full_2;
  wire    [  3: 0] p2_stage_2;
  wire             p3_full_3;
  wire    [  3: 0] p3_stage_3;
  wire             p4_full_4;
  wire    [  3: 0] p4_stage_4;
  wire             p5_full_5;
  wire    [  3: 0] p5_stage_5;
  wire             p6_full_6;
  wire    [  3: 0] p6_stage_6;
  wire             p7_full_7;
  wire    [  3: 0] p7_stage_7;
  wire             p8_full_8;
  wire    [  3: 0] p8_stage_8;
  reg     [  3: 0] stage_0;
  reg     [  3: 0] stage_1;
  reg     [  3: 0] stage_2;
  reg     [  3: 0] stage_3;
  reg     [  3: 0] stage_4;
  reg     [  3: 0] stage_5;
  reg     [  3: 0] stage_6;
  reg     [  3: 0] stage_7;
  reg     [  3: 0] stage_8;
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
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register,
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
  output  [  2: 0] tiger_burst_0_upstream_burstcount;
  output  [ 23: 0] tiger_burst_0_upstream_byteaddress;
  output  [  1: 0] tiger_burst_0_upstream_byteenable;
  output           tiger_burst_0_upstream_debugaccess;
  output           tiger_burst_0_upstream_read;
  output  [ 15: 0] tiger_burst_0_upstream_readdata_from_sa;
  output           tiger_burst_0_upstream_waitrequest_from_sa;
  output           tiger_burst_0_upstream_write;
  output  [ 15: 0] tiger_burst_0_upstream_writedata;
  input            clk;
  input   [ 24: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  input   [  2: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input   [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  input   [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  input            pipeline_bridge_MEMORY_m1_debugaccess;
  input            pipeline_bridge_MEMORY_m1_latency_counter;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register;
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
  reg     [  4: 0] tiger_burst_0_upstream_arb_share_counter;
  wire    [  4: 0] tiger_burst_0_upstream_arb_share_counter_next_value;
  wire    [  4: 0] tiger_burst_0_upstream_arb_share_set_values;
  reg     [  1: 0] tiger_burst_0_upstream_bbt_burstcounter;
  wire             tiger_burst_0_upstream_beginbursttransfer_internal;
  wire             tiger_burst_0_upstream_begins_xfer;
  wire    [  2: 0] tiger_burst_0_upstream_burstcount;
  wire             tiger_burst_0_upstream_burstcount_fifo_empty;
  wire    [ 23: 0] tiger_burst_0_upstream_byteaddress;
  wire    [  1: 0] tiger_burst_0_upstream_byteenable;
  reg     [  3: 0] tiger_burst_0_upstream_current_burst;
  wire    [  3: 0] tiger_burst_0_upstream_current_burst_minus_one;
  wire             tiger_burst_0_upstream_debugaccess;
  wire             tiger_burst_0_upstream_end_xfer;
  wire             tiger_burst_0_upstream_firsttransfer;
  wire             tiger_burst_0_upstream_grant_vector;
  wire             tiger_burst_0_upstream_in_a_read_cycle;
  wire             tiger_burst_0_upstream_in_a_write_cycle;
  reg              tiger_burst_0_upstream_load_fifo;
  wire             tiger_burst_0_upstream_master_qreq_vector;
  wire             tiger_burst_0_upstream_move_on_to_next_transaction;
  wire    [  1: 0] tiger_burst_0_upstream_next_bbt_burstcount;
  wire    [  3: 0] tiger_burst_0_upstream_next_burst_count;
  wire             tiger_burst_0_upstream_non_bursting_master_requests;
  wire             tiger_burst_0_upstream_read;
  wire    [ 15: 0] tiger_burst_0_upstream_readdata_from_sa;
  wire             tiger_burst_0_upstream_readdatavalid_from_sa;
  reg              tiger_burst_0_upstream_reg_firsttransfer;
  wire    [  3: 0] tiger_burst_0_upstream_selected_burstcount;
  reg              tiger_burst_0_upstream_slavearbiterlockenable;
  wire             tiger_burst_0_upstream_slavearbiterlockenable2;
  wire             tiger_burst_0_upstream_this_cycle_is_the_last_burst;
  wire    [  3: 0] tiger_burst_0_upstream_transaction_burst_count;
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

  assign pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream = ({pipeline_bridge_MEMORY_m1_address_to_slave[24 : 23] , 23'b0} == 25'h800000) & pipeline_bridge_MEMORY_m1_chipselect;
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

  assign pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream = pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream & ~(((pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & ((pipeline_bridge_MEMORY_m1_latency_counter != 0) | (1 < pipeline_bridge_MEMORY_m1_latency_counter) | (|pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register) | (|pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register))));
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
  wire             pre_flush_tiger_burst_0_downstream_readdatavalid;
  wire             r_0;
  reg     [ 22: 0] tiger_burst_0_downstream_address_last_time;
  wire    [ 22: 0] tiger_burst_0_downstream_address_to_slave;
  reg              tiger_burst_0_downstream_burstcount_last_time;
  reg     [  1: 0] tiger_burst_0_downstream_byteenable_last_time;
  wire             tiger_burst_0_downstream_latency_counter;
  reg              tiger_burst_0_downstream_read_last_time;
  wire    [ 15: 0] tiger_burst_0_downstream_readdata;
  wire             tiger_burst_0_downstream_readdatavalid;
  wire             tiger_burst_0_downstream_reset_n;
  wire             tiger_burst_0_downstream_run;
  wire             tiger_burst_0_downstream_waitrequest;
  reg              tiger_burst_0_downstream_write_last_time;
  reg     [ 15: 0] tiger_burst_0_downstream_writedata_last_time;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (tiger_burst_0_downstream_qualified_request_sdram_s1 | ~tiger_burst_0_downstream_requests_sdram_s1) & ((~tiger_burst_0_downstream_qualified_request_sdram_s1 | ~(tiger_burst_0_downstream_read | tiger_burst_0_downstream_write) | (1 & ~sdram_s1_waitrequest_from_sa & (tiger_burst_0_downstream_read | tiger_burst_0_downstream_write)))) & ((~tiger_burst_0_downstream_qualified_request_sdram_s1 | ~(tiger_burst_0_downstream_read | tiger_burst_0_downstream_write) | (1 & ~sdram_s1_waitrequest_from_sa & (tiger_burst_0_downstream_read | tiger_burst_0_downstream_write))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_burst_0_downstream_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_burst_0_downstream_address_to_slave = tiger_burst_0_downstream_address;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_tiger_burst_0_downstream_readdatavalid = tiger_burst_0_downstream_read_data_valid_sdram_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign tiger_burst_0_downstream_readdatavalid = 0 |
    pre_flush_tiger_burst_0_downstream_readdatavalid;

  //tiger_burst_0/downstream readdata mux, which is an e_mux
  assign tiger_burst_0_downstream_readdata = sdram_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_burst_0_downstream_waitrequest = ~tiger_burst_0_downstream_run;

  //latent max counter, which is an e_assign
  assign tiger_burst_0_downstream_latency_counter = 0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module burstcount_fifo_for_tiger_burst_1_upstream_module (
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
  reg              full_2;
  wire             full_3;
  reg     [  2: 0] how_many_ones;
  wire    [  2: 0] one_count_minus_one;
  wire    [  2: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  2: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  2: 0] p1_stage_1;
  wire             p2_full_2;
  wire    [  2: 0] p2_stage_2;
  reg     [  2: 0] stage_0;
  reg     [  2: 0] stage_1;
  reg     [  2: 0] stage_2;
  wire    [  2: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_2;
  assign empty = !full_0;
  assign full_3 = 0;
  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_1_upstream_module (
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
  wire             full_3;
  reg     [  2: 0] how_many_ones;
  wire    [  2: 0] one_count_minus_one;
  wire    [  2: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p2_full_2;
  wire             p2_stage_2;
  reg              stage_0;
  reg              stage_1;
  reg              stage_2;
  wire    [  2: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_2;
  assign empty = !full_0;
  assign full_3 = 0;
  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_burst_1_upstream_arbitrator (
                                           // inputs:
                                            clk,
                                            pipeline_bridge_MEMORY_m1_address_to_slave,
                                            pipeline_bridge_MEMORY_m1_burstcount,
                                            pipeline_bridge_MEMORY_m1_byteenable,
                                            pipeline_bridge_MEMORY_m1_chipselect,
                                            pipeline_bridge_MEMORY_m1_debugaccess,
                                            pipeline_bridge_MEMORY_m1_latency_counter,
                                            pipeline_bridge_MEMORY_m1_read,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_write,
                                            pipeline_bridge_MEMORY_m1_writedata,
                                            reset_n,
                                            tiger_burst_1_upstream_readdata,
                                            tiger_burst_1_upstream_readdatavalid,
                                            tiger_burst_1_upstream_waitrequest,

                                           // outputs:
                                            d1_tiger_burst_1_upstream_end_xfer,
                                            pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream,
                                            pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream,
                                            tiger_burst_1_upstream_address,
                                            tiger_burst_1_upstream_burstcount,
                                            tiger_burst_1_upstream_byteaddress,
                                            tiger_burst_1_upstream_byteenable,
                                            tiger_burst_1_upstream_debugaccess,
                                            tiger_burst_1_upstream_read,
                                            tiger_burst_1_upstream_readdata_from_sa,
                                            tiger_burst_1_upstream_waitrequest_from_sa,
                                            tiger_burst_1_upstream_write,
                                            tiger_burst_1_upstream_writedata
                                         )
;

  output           d1_tiger_burst_1_upstream_end_xfer;
  output           pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream;
  output           pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register;
  output           pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream;
  output  [ 12: 0] tiger_burst_1_upstream_address;
  output  [  2: 0] tiger_burst_1_upstream_burstcount;
  output  [ 14: 0] tiger_burst_1_upstream_byteaddress;
  output  [  3: 0] tiger_burst_1_upstream_byteenable;
  output           tiger_burst_1_upstream_debugaccess;
  output           tiger_burst_1_upstream_read;
  output  [ 31: 0] tiger_burst_1_upstream_readdata_from_sa;
  output           tiger_burst_1_upstream_waitrequest_from_sa;
  output           tiger_burst_1_upstream_write;
  output  [ 31: 0] tiger_burst_1_upstream_writedata;
  input            clk;
  input   [ 24: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  input   [  2: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input            pipeline_bridge_MEMORY_m1_debugaccess;
  input            pipeline_bridge_MEMORY_m1_latency_counter;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_write;
  input   [ 31: 0] pipeline_bridge_MEMORY_m1_writedata;
  input            reset_n;
  input   [ 31: 0] tiger_burst_1_upstream_readdata;
  input            tiger_burst_1_upstream_readdatavalid;
  input            tiger_burst_1_upstream_waitrequest;

  reg              d1_reasons_to_wait;
  reg              d1_tiger_burst_1_upstream_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_tiger_burst_1_upstream;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             p0_tiger_burst_1_upstream_load_fifo;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock2;
  wire             pipeline_bridge_MEMORY_m1_continuerequest;
  wire             pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register;
  wire             pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_1_upstream;
  wire    [ 12: 0] tiger_burst_1_upstream_address;
  wire             tiger_burst_1_upstream_allgrants;
  wire             tiger_burst_1_upstream_allow_new_arb_cycle;
  wire             tiger_burst_1_upstream_any_bursting_master_saved_grant;
  wire             tiger_burst_1_upstream_any_continuerequest;
  wire             tiger_burst_1_upstream_arb_counter_enable;
  reg     [  4: 0] tiger_burst_1_upstream_arb_share_counter;
  wire    [  4: 0] tiger_burst_1_upstream_arb_share_counter_next_value;
  wire    [  4: 0] tiger_burst_1_upstream_arb_share_set_values;
  reg     [  1: 0] tiger_burst_1_upstream_bbt_burstcounter;
  wire             tiger_burst_1_upstream_beginbursttransfer_internal;
  wire             tiger_burst_1_upstream_begins_xfer;
  wire    [  2: 0] tiger_burst_1_upstream_burstcount;
  wire             tiger_burst_1_upstream_burstcount_fifo_empty;
  wire    [ 14: 0] tiger_burst_1_upstream_byteaddress;
  wire    [  3: 0] tiger_burst_1_upstream_byteenable;
  reg     [  2: 0] tiger_burst_1_upstream_current_burst;
  wire    [  2: 0] tiger_burst_1_upstream_current_burst_minus_one;
  wire             tiger_burst_1_upstream_debugaccess;
  wire             tiger_burst_1_upstream_end_xfer;
  wire             tiger_burst_1_upstream_firsttransfer;
  wire             tiger_burst_1_upstream_grant_vector;
  wire             tiger_burst_1_upstream_in_a_read_cycle;
  wire             tiger_burst_1_upstream_in_a_write_cycle;
  reg              tiger_burst_1_upstream_load_fifo;
  wire             tiger_burst_1_upstream_master_qreq_vector;
  wire             tiger_burst_1_upstream_move_on_to_next_transaction;
  wire    [  1: 0] tiger_burst_1_upstream_next_bbt_burstcount;
  wire    [  2: 0] tiger_burst_1_upstream_next_burst_count;
  wire             tiger_burst_1_upstream_non_bursting_master_requests;
  wire             tiger_burst_1_upstream_read;
  wire    [ 31: 0] tiger_burst_1_upstream_readdata_from_sa;
  wire             tiger_burst_1_upstream_readdatavalid_from_sa;
  reg              tiger_burst_1_upstream_reg_firsttransfer;
  wire    [  2: 0] tiger_burst_1_upstream_selected_burstcount;
  reg              tiger_burst_1_upstream_slavearbiterlockenable;
  wire             tiger_burst_1_upstream_slavearbiterlockenable2;
  wire             tiger_burst_1_upstream_this_cycle_is_the_last_burst;
  wire    [  2: 0] tiger_burst_1_upstream_transaction_burst_count;
  wire             tiger_burst_1_upstream_unreg_firsttransfer;
  wire             tiger_burst_1_upstream_waitrequest_from_sa;
  wire             tiger_burst_1_upstream_waits_for_read;
  wire             tiger_burst_1_upstream_waits_for_write;
  wire             tiger_burst_1_upstream_write;
  wire    [ 31: 0] tiger_burst_1_upstream_writedata;
  wire             wait_for_tiger_burst_1_upstream_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~tiger_burst_1_upstream_end_xfer;
    end


  assign tiger_burst_1_upstream_begins_xfer = ~d1_reasons_to_wait & ((pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream));
  //assign tiger_burst_1_upstream_readdatavalid_from_sa = tiger_burst_1_upstream_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_1_upstream_readdatavalid_from_sa = tiger_burst_1_upstream_readdatavalid;

  //assign tiger_burst_1_upstream_readdata_from_sa = tiger_burst_1_upstream_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_1_upstream_readdata_from_sa = tiger_burst_1_upstream_readdata;

  assign pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream = ({pipeline_bridge_MEMORY_m1_address_to_slave[24 : 13] , 13'b0} == 25'h0) & pipeline_bridge_MEMORY_m1_chipselect;
  //assign tiger_burst_1_upstream_waitrequest_from_sa = tiger_burst_1_upstream_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_1_upstream_waitrequest_from_sa = tiger_burst_1_upstream_waitrequest;

  //tiger_burst_1_upstream_arb_share_counter set values, which is an e_mux
  assign tiger_burst_1_upstream_arb_share_set_values = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream)? ((((pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect)) ? pipeline_bridge_MEMORY_m1_burstcount : 1)) :
    1;

  //tiger_burst_1_upstream_non_bursting_master_requests mux, which is an e_mux
  assign tiger_burst_1_upstream_non_bursting_master_requests = 0;

  //tiger_burst_1_upstream_any_bursting_master_saved_grant mux, which is an e_mux
  assign tiger_burst_1_upstream_any_bursting_master_saved_grant = pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_1_upstream;

  //tiger_burst_1_upstream_arb_share_counter_next_value assignment, which is an e_assign
  assign tiger_burst_1_upstream_arb_share_counter_next_value = tiger_burst_1_upstream_firsttransfer ? (tiger_burst_1_upstream_arb_share_set_values - 1) : |tiger_burst_1_upstream_arb_share_counter ? (tiger_burst_1_upstream_arb_share_counter - 1) : 0;

  //tiger_burst_1_upstream_allgrants all slave grants, which is an e_mux
  assign tiger_burst_1_upstream_allgrants = |tiger_burst_1_upstream_grant_vector;

  //tiger_burst_1_upstream_end_xfer assignment, which is an e_assign
  assign tiger_burst_1_upstream_end_xfer = ~(tiger_burst_1_upstream_waits_for_read | tiger_burst_1_upstream_waits_for_write);

  //end_xfer_arb_share_counter_term_tiger_burst_1_upstream arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_tiger_burst_1_upstream = tiger_burst_1_upstream_end_xfer & (~tiger_burst_1_upstream_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //tiger_burst_1_upstream_arb_share_counter arbitration counter enable, which is an e_assign
  assign tiger_burst_1_upstream_arb_counter_enable = (end_xfer_arb_share_counter_term_tiger_burst_1_upstream & tiger_burst_1_upstream_allgrants) | (end_xfer_arb_share_counter_term_tiger_burst_1_upstream & ~tiger_burst_1_upstream_non_bursting_master_requests);

  //tiger_burst_1_upstream_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_upstream_arb_share_counter <= 0;
      else if (tiger_burst_1_upstream_arb_counter_enable)
          tiger_burst_1_upstream_arb_share_counter <= tiger_burst_1_upstream_arb_share_counter_next_value;
    end


  //tiger_burst_1_upstream_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_upstream_slavearbiterlockenable <= 0;
      else if ((|tiger_burst_1_upstream_master_qreq_vector & end_xfer_arb_share_counter_term_tiger_burst_1_upstream) | (end_xfer_arb_share_counter_term_tiger_burst_1_upstream & ~tiger_burst_1_upstream_non_bursting_master_requests))
          tiger_burst_1_upstream_slavearbiterlockenable <= |tiger_burst_1_upstream_arb_share_counter_next_value;
    end


  //pipeline_bridge_MEMORY/m1 tiger_burst_1/upstream arbiterlock, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock = tiger_burst_1_upstream_slavearbiterlockenable & pipeline_bridge_MEMORY_m1_continuerequest;

  //tiger_burst_1_upstream_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign tiger_burst_1_upstream_slavearbiterlockenable2 = |tiger_burst_1_upstream_arb_share_counter_next_value;

  //pipeline_bridge_MEMORY/m1 tiger_burst_1/upstream arbiterlock2, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock2 = tiger_burst_1_upstream_slavearbiterlockenable2 & pipeline_bridge_MEMORY_m1_continuerequest;

  //tiger_burst_1_upstream_any_continuerequest at least one master continues requesting, which is an e_assign
  assign tiger_burst_1_upstream_any_continuerequest = 1;

  //pipeline_bridge_MEMORY_m1_continuerequest continued request, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_continuerequest = 1;

  assign pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream = pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream & ~(((pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & ((pipeline_bridge_MEMORY_m1_latency_counter != 0) | (1 < pipeline_bridge_MEMORY_m1_latency_counter) | (|pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register) | (|pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register))));
  //unique name for tiger_burst_1_upstream_move_on_to_next_transaction, which is an e_assign
  assign tiger_burst_1_upstream_move_on_to_next_transaction = tiger_burst_1_upstream_this_cycle_is_the_last_burst & tiger_burst_1_upstream_load_fifo;

  //the currently selected burstcount for tiger_burst_1_upstream, which is an e_mux
  assign tiger_burst_1_upstream_selected_burstcount = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream)? pipeline_bridge_MEMORY_m1_burstcount :
    1;

  //burstcount_fifo_for_tiger_burst_1_upstream, which is an e_fifo_with_registered_outputs
  burstcount_fifo_for_tiger_burst_1_upstream_module burstcount_fifo_for_tiger_burst_1_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_burst_1_upstream_selected_burstcount),
      .data_out             (tiger_burst_1_upstream_transaction_burst_count),
      .empty                (tiger_burst_1_upstream_burstcount_fifo_empty),
      .fifo_contains_ones_n (),
      .full                 (),
      .read                 (tiger_burst_1_upstream_this_cycle_is_the_last_burst),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_1_upstream_waits_for_read & tiger_burst_1_upstream_load_fifo & ~(tiger_burst_1_upstream_this_cycle_is_the_last_burst & tiger_burst_1_upstream_burstcount_fifo_empty))
    );

  //tiger_burst_1_upstream current burst minus one, which is an e_assign
  assign tiger_burst_1_upstream_current_burst_minus_one = tiger_burst_1_upstream_current_burst - 1;

  //what to load in current_burst, for tiger_burst_1_upstream, which is an e_mux
  assign tiger_burst_1_upstream_next_burst_count = (((in_a_read_cycle & ~tiger_burst_1_upstream_waits_for_read) & ~tiger_burst_1_upstream_load_fifo))? tiger_burst_1_upstream_selected_burstcount :
    ((in_a_read_cycle & ~tiger_burst_1_upstream_waits_for_read & tiger_burst_1_upstream_this_cycle_is_the_last_burst & tiger_burst_1_upstream_burstcount_fifo_empty))? tiger_burst_1_upstream_selected_burstcount :
    (tiger_burst_1_upstream_this_cycle_is_the_last_burst)? tiger_burst_1_upstream_transaction_burst_count :
    tiger_burst_1_upstream_current_burst_minus_one;

  //the current burst count for tiger_burst_1_upstream, to be decremented, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_upstream_current_burst <= 0;
      else if (tiger_burst_1_upstream_readdatavalid_from_sa | (~tiger_burst_1_upstream_load_fifo & (in_a_read_cycle & ~tiger_burst_1_upstream_waits_for_read)))
          tiger_burst_1_upstream_current_burst <= tiger_burst_1_upstream_next_burst_count;
    end


  //a 1 or burstcount fifo empty, to initialize the counter, which is an e_mux
  assign p0_tiger_burst_1_upstream_load_fifo = (~tiger_burst_1_upstream_load_fifo)? 1 :
    (((in_a_read_cycle & ~tiger_burst_1_upstream_waits_for_read) & tiger_burst_1_upstream_load_fifo))? 1 :
    ~tiger_burst_1_upstream_burstcount_fifo_empty;

  //whether to load directly to the counter or to the fifo, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_upstream_load_fifo <= 0;
      else if ((in_a_read_cycle & ~tiger_burst_1_upstream_waits_for_read) & ~tiger_burst_1_upstream_load_fifo | tiger_burst_1_upstream_this_cycle_is_the_last_burst)
          tiger_burst_1_upstream_load_fifo <= p0_tiger_burst_1_upstream_load_fifo;
    end


  //the last cycle in the burst for tiger_burst_1_upstream, which is an e_assign
  assign tiger_burst_1_upstream_this_cycle_is_the_last_burst = ~(|tiger_burst_1_upstream_current_burst_minus_one) & tiger_burst_1_upstream_readdatavalid_from_sa;

  //rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_1_upstream, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_1_upstream_module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_1_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream),
      .data_out             (pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_tiger_burst_1_upstream),
      .empty                (),
      .fifo_contains_ones_n (pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_1_upstream),
      .full                 (),
      .read                 (tiger_burst_1_upstream_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_1_upstream_waits_for_read)
    );

  assign pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register = ~pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_1_upstream;
  //local readdatavalid pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream = tiger_burst_1_upstream_readdatavalid_from_sa;

  //tiger_burst_1_upstream_writedata mux, which is an e_mux
  assign tiger_burst_1_upstream_writedata = pipeline_bridge_MEMORY_m1_writedata;

  //byteaddress mux for tiger_burst_1/upstream, which is an e_mux
  assign tiger_burst_1_upstream_byteaddress = pipeline_bridge_MEMORY_m1_address_to_slave;

  //master is always granted when requested
  assign pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream = pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream;

  //pipeline_bridge_MEMORY/m1 saved-grant tiger_burst_1/upstream, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_1_upstream = pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream;

  //allow new arb cycle for tiger_burst_1/upstream, which is an e_assign
  assign tiger_burst_1_upstream_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign tiger_burst_1_upstream_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign tiger_burst_1_upstream_master_qreq_vector = 1;

  //tiger_burst_1_upstream_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_1_upstream_firsttransfer = tiger_burst_1_upstream_begins_xfer ? tiger_burst_1_upstream_unreg_firsttransfer : tiger_burst_1_upstream_reg_firsttransfer;

  //tiger_burst_1_upstream_unreg_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_1_upstream_unreg_firsttransfer = ~(tiger_burst_1_upstream_slavearbiterlockenable & tiger_burst_1_upstream_any_continuerequest);

  //tiger_burst_1_upstream_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_upstream_reg_firsttransfer <= 1'b1;
      else if (tiger_burst_1_upstream_begins_xfer)
          tiger_burst_1_upstream_reg_firsttransfer <= tiger_burst_1_upstream_unreg_firsttransfer;
    end


  //tiger_burst_1_upstream_next_bbt_burstcount next_bbt_burstcount, which is an e_mux
  assign tiger_burst_1_upstream_next_bbt_burstcount = ((((tiger_burst_1_upstream_write) && (tiger_burst_1_upstream_bbt_burstcounter == 0))))? (tiger_burst_1_upstream_burstcount - 1) :
    ((((tiger_burst_1_upstream_read) && (tiger_burst_1_upstream_bbt_burstcounter == 0))))? 0 :
    (tiger_burst_1_upstream_bbt_burstcounter - 1);

  //tiger_burst_1_upstream_bbt_burstcounter bbt_burstcounter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_upstream_bbt_burstcounter <= 0;
      else if (tiger_burst_1_upstream_begins_xfer)
          tiger_burst_1_upstream_bbt_burstcounter <= tiger_burst_1_upstream_next_bbt_burstcount;
    end


  //tiger_burst_1_upstream_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign tiger_burst_1_upstream_beginbursttransfer_internal = tiger_burst_1_upstream_begins_xfer & (tiger_burst_1_upstream_bbt_burstcounter == 0);

  //tiger_burst_1_upstream_read assignment, which is an e_mux
  assign tiger_burst_1_upstream_read = pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect);

  //tiger_burst_1_upstream_write assignment, which is an e_mux
  assign tiger_burst_1_upstream_write = pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect);

  //tiger_burst_1_upstream_address mux, which is an e_mux
  assign tiger_burst_1_upstream_address = pipeline_bridge_MEMORY_m1_address_to_slave;

  //d1_tiger_burst_1_upstream_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_tiger_burst_1_upstream_end_xfer <= 1;
      else 
        d1_tiger_burst_1_upstream_end_xfer <= tiger_burst_1_upstream_end_xfer;
    end


  //tiger_burst_1_upstream_waits_for_read in a cycle, which is an e_mux
  assign tiger_burst_1_upstream_waits_for_read = tiger_burst_1_upstream_in_a_read_cycle & tiger_burst_1_upstream_waitrequest_from_sa;

  //tiger_burst_1_upstream_in_a_read_cycle assignment, which is an e_assign
  assign tiger_burst_1_upstream_in_a_read_cycle = pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = tiger_burst_1_upstream_in_a_read_cycle;

  //tiger_burst_1_upstream_waits_for_write in a cycle, which is an e_mux
  assign tiger_burst_1_upstream_waits_for_write = tiger_burst_1_upstream_in_a_write_cycle & tiger_burst_1_upstream_waitrequest_from_sa;

  //tiger_burst_1_upstream_in_a_write_cycle assignment, which is an e_assign
  assign tiger_burst_1_upstream_in_a_write_cycle = pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = tiger_burst_1_upstream_in_a_write_cycle;

  assign wait_for_tiger_burst_1_upstream_counter = 0;
  //tiger_burst_1_upstream_byteenable byte enable port mux, which is an e_mux
  assign tiger_burst_1_upstream_byteenable = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream)? pipeline_bridge_MEMORY_m1_byteenable :
    -1;

  //burstcount mux, which is an e_mux
  assign tiger_burst_1_upstream_burstcount = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream)? pipeline_bridge_MEMORY_m1_burstcount :
    1;

  //debugaccess mux, which is an e_mux
  assign tiger_burst_1_upstream_debugaccess = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream)? pipeline_bridge_MEMORY_m1_debugaccess :
    0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_1/upstream enable non-zero assertions, which is an e_register
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
      if (pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream && (pipeline_bridge_MEMORY_m1_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: pipeline_bridge_MEMORY/m1 drove 0 on its 'burstcount' port while accessing slave tiger_burst_1/upstream", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_burst_1_downstream_arbitrator (
                                             // inputs:
                                              clk,
                                              d1_onchip_mem_s1_end_xfer,
                                              onchip_mem_s1_readdata_from_sa,
                                              reset_n,
                                              tiger_burst_1_downstream_address,
                                              tiger_burst_1_downstream_burstcount,
                                              tiger_burst_1_downstream_byteenable,
                                              tiger_burst_1_downstream_granted_onchip_mem_s1,
                                              tiger_burst_1_downstream_qualified_request_onchip_mem_s1,
                                              tiger_burst_1_downstream_read,
                                              tiger_burst_1_downstream_read_data_valid_onchip_mem_s1,
                                              tiger_burst_1_downstream_requests_onchip_mem_s1,
                                              tiger_burst_1_downstream_write,
                                              tiger_burst_1_downstream_writedata,

                                             // outputs:
                                              tiger_burst_1_downstream_address_to_slave,
                                              tiger_burst_1_downstream_latency_counter,
                                              tiger_burst_1_downstream_readdata,
                                              tiger_burst_1_downstream_readdatavalid,
                                              tiger_burst_1_downstream_reset_n,
                                              tiger_burst_1_downstream_waitrequest
                                           )
;

  output  [ 12: 0] tiger_burst_1_downstream_address_to_slave;
  output           tiger_burst_1_downstream_latency_counter;
  output  [ 31: 0] tiger_burst_1_downstream_readdata;
  output           tiger_burst_1_downstream_readdatavalid;
  output           tiger_burst_1_downstream_reset_n;
  output           tiger_burst_1_downstream_waitrequest;
  input            clk;
  input            d1_onchip_mem_s1_end_xfer;
  input   [ 31: 0] onchip_mem_s1_readdata_from_sa;
  input            reset_n;
  input   [ 12: 0] tiger_burst_1_downstream_address;
  input            tiger_burst_1_downstream_burstcount;
  input   [  3: 0] tiger_burst_1_downstream_byteenable;
  input            tiger_burst_1_downstream_granted_onchip_mem_s1;
  input            tiger_burst_1_downstream_qualified_request_onchip_mem_s1;
  input            tiger_burst_1_downstream_read;
  input            tiger_burst_1_downstream_read_data_valid_onchip_mem_s1;
  input            tiger_burst_1_downstream_requests_onchip_mem_s1;
  input            tiger_burst_1_downstream_write;
  input   [ 31: 0] tiger_burst_1_downstream_writedata;

  reg              active_and_waiting_last_time;
  wire             pre_flush_tiger_burst_1_downstream_readdatavalid;
  wire             r_0;
  reg     [ 12: 0] tiger_burst_1_downstream_address_last_time;
  wire    [ 12: 0] tiger_burst_1_downstream_address_to_slave;
  reg              tiger_burst_1_downstream_burstcount_last_time;
  reg     [  3: 0] tiger_burst_1_downstream_byteenable_last_time;
  wire             tiger_burst_1_downstream_latency_counter;
  reg              tiger_burst_1_downstream_read_last_time;
  wire    [ 31: 0] tiger_burst_1_downstream_readdata;
  wire             tiger_burst_1_downstream_readdatavalid;
  wire             tiger_burst_1_downstream_reset_n;
  wire             tiger_burst_1_downstream_run;
  wire             tiger_burst_1_downstream_waitrequest;
  reg              tiger_burst_1_downstream_write_last_time;
  reg     [ 31: 0] tiger_burst_1_downstream_writedata_last_time;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & ((~tiger_burst_1_downstream_qualified_request_onchip_mem_s1 | ~(tiger_burst_1_downstream_read | tiger_burst_1_downstream_write) | (1 & (tiger_burst_1_downstream_read | tiger_burst_1_downstream_write)))) & ((~tiger_burst_1_downstream_qualified_request_onchip_mem_s1 | ~(tiger_burst_1_downstream_read | tiger_burst_1_downstream_write) | (1 & (tiger_burst_1_downstream_read | tiger_burst_1_downstream_write))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_burst_1_downstream_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_burst_1_downstream_address_to_slave = tiger_burst_1_downstream_address;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_tiger_burst_1_downstream_readdatavalid = tiger_burst_1_downstream_read_data_valid_onchip_mem_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign tiger_burst_1_downstream_readdatavalid = 0 |
    pre_flush_tiger_burst_1_downstream_readdatavalid;

  //tiger_burst_1/downstream readdata mux, which is an e_mux
  assign tiger_burst_1_downstream_readdata = onchip_mem_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_burst_1_downstream_waitrequest = ~tiger_burst_1_downstream_run;

  //latent max counter, which is an e_assign
  assign tiger_burst_1_downstream_latency_counter = 0;

  //tiger_burst_1_downstream_reset_n assignment, which is an e_assign
  assign tiger_burst_1_downstream_reset_n = reset_n;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_1_downstream_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_downstream_address_last_time <= 0;
      else 
        tiger_burst_1_downstream_address_last_time <= tiger_burst_1_downstream_address;
    end


  //tiger_burst_1/downstream waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= tiger_burst_1_downstream_waitrequest & (tiger_burst_1_downstream_read | tiger_burst_1_downstream_write);
    end


  //tiger_burst_1_downstream_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_1_downstream_address != tiger_burst_1_downstream_address_last_time))
        begin
          $write("%0d ns: tiger_burst_1_downstream_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_1_downstream_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_downstream_burstcount_last_time <= 0;
      else 
        tiger_burst_1_downstream_burstcount_last_time <= tiger_burst_1_downstream_burstcount;
    end


  //tiger_burst_1_downstream_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_1_downstream_burstcount != tiger_burst_1_downstream_burstcount_last_time))
        begin
          $write("%0d ns: tiger_burst_1_downstream_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_1_downstream_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_downstream_byteenable_last_time <= 0;
      else 
        tiger_burst_1_downstream_byteenable_last_time <= tiger_burst_1_downstream_byteenable;
    end


  //tiger_burst_1_downstream_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_1_downstream_byteenable != tiger_burst_1_downstream_byteenable_last_time))
        begin
          $write("%0d ns: tiger_burst_1_downstream_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_1_downstream_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_downstream_read_last_time <= 0;
      else 
        tiger_burst_1_downstream_read_last_time <= tiger_burst_1_downstream_read;
    end


  //tiger_burst_1_downstream_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_1_downstream_read != tiger_burst_1_downstream_read_last_time))
        begin
          $write("%0d ns: tiger_burst_1_downstream_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_1_downstream_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_downstream_write_last_time <= 0;
      else 
        tiger_burst_1_downstream_write_last_time <= tiger_burst_1_downstream_write;
    end


  //tiger_burst_1_downstream_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_1_downstream_write != tiger_burst_1_downstream_write_last_time))
        begin
          $write("%0d ns: tiger_burst_1_downstream_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_1_downstream_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_1_downstream_writedata_last_time <= 0;
      else 
        tiger_burst_1_downstream_writedata_last_time <= tiger_burst_1_downstream_writedata;
    end


  //tiger_burst_1_downstream_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_1_downstream_writedata != tiger_burst_1_downstream_writedata_last_time) & tiger_burst_1_downstream_write)
        begin
          $write("%0d ns: tiger_burst_1_downstream_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module burstcount_fifo_for_tiger_burst_2_upstream_module (
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
  wire             full_2;
  reg     [  2: 0] how_many_ones;
  wire    [  2: 0] one_count_minus_one;
  wire    [  2: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  2: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  2: 0] p1_stage_1;
  reg     [  2: 0] stage_0;
  reg     [  2: 0] stage_1;
  wire    [  2: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_1;
  assign empty = !full_0;
  assign full_2 = 0;
  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_2_upstream_module (
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
  wire             full_2;
  reg     [  2: 0] how_many_ones;
  wire    [  2: 0] one_count_minus_one;
  wire    [  2: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  reg              stage_0;
  reg              stage_1;
  wire    [  2: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_1;
  assign empty = !full_0;
  assign full_2 = 0;
  //data_1, which is an e_mux
  assign p1_stage_1 = ((full_2 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_burst_2_upstream_arbitrator (
                                           // inputs:
                                            clk,
                                            pipeline_bridge_MEMORY_m1_address_to_slave,
                                            pipeline_bridge_MEMORY_m1_burstcount,
                                            pipeline_bridge_MEMORY_m1_byteenable,
                                            pipeline_bridge_MEMORY_m1_chipselect,
                                            pipeline_bridge_MEMORY_m1_debugaccess,
                                            pipeline_bridge_MEMORY_m1_latency_counter,
                                            pipeline_bridge_MEMORY_m1_read,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_write,
                                            pipeline_bridge_MEMORY_m1_writedata,
                                            reset_n,
                                            tiger_burst_2_upstream_readdata,
                                            tiger_burst_2_upstream_readdatavalid,
                                            tiger_burst_2_upstream_waitrequest,

                                           // outputs:
                                            d1_tiger_burst_2_upstream_end_xfer,
                                            pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream,
                                            pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream,
                                            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register,
                                            pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream,
                                            tiger_burst_2_upstream_address,
                                            tiger_burst_2_upstream_burstcount,
                                            tiger_burst_2_upstream_byteaddress,
                                            tiger_burst_2_upstream_byteenable,
                                            tiger_burst_2_upstream_debugaccess,
                                            tiger_burst_2_upstream_read,
                                            tiger_burst_2_upstream_readdata_from_sa,
                                            tiger_burst_2_upstream_waitrequest_from_sa,
                                            tiger_burst_2_upstream_write,
                                            tiger_burst_2_upstream_writedata
                                         )
;

  output           d1_tiger_burst_2_upstream_end_xfer;
  output           pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream;
  output           pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream;
  output           pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register;
  output           pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream;
  output  [  6: 0] tiger_burst_2_upstream_address;
  output  [  2: 0] tiger_burst_2_upstream_burstcount;
  output  [ 10: 0] tiger_burst_2_upstream_byteaddress;
  output  [ 15: 0] tiger_burst_2_upstream_byteenable;
  output           tiger_burst_2_upstream_debugaccess;
  output           tiger_burst_2_upstream_read;
  output  [127: 0] tiger_burst_2_upstream_readdata_from_sa;
  output           tiger_burst_2_upstream_waitrequest_from_sa;
  output           tiger_burst_2_upstream_write;
  output  [127: 0] tiger_burst_2_upstream_writedata;
  input            clk;
  input   [ 24: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  input   [  2: 0] pipeline_bridge_MEMORY_m1_burstcount;
  input   [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  input            pipeline_bridge_MEMORY_m1_chipselect;
  input            pipeline_bridge_MEMORY_m1_debugaccess;
  input            pipeline_bridge_MEMORY_m1_latency_counter;
  input            pipeline_bridge_MEMORY_m1_read;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register;
  input            pipeline_bridge_MEMORY_m1_write;
  input   [ 31: 0] pipeline_bridge_MEMORY_m1_writedata;
  input            reset_n;
  input   [127: 0] tiger_burst_2_upstream_readdata;
  input            tiger_burst_2_upstream_readdatavalid;
  input            tiger_burst_2_upstream_waitrequest;

  reg              d1_reasons_to_wait;
  reg              d1_tiger_burst_2_upstream_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_tiger_burst_2_upstream;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             p0_tiger_burst_2_upstream_load_fifo;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock;
  wire             pipeline_bridge_MEMORY_m1_arbiterlock2;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream;
  reg     [  1: 0] pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream_reg;
  wire    [ 15: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_continuerequest;
  wire             pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register;
  wire             pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_2_upstream;
  wire    [127: 0] pipeline_bridge_MEMORY_m1_writedata_replicated;
  wire    [  6: 0] tiger_burst_2_upstream_address;
  wire             tiger_burst_2_upstream_allgrants;
  wire             tiger_burst_2_upstream_allow_new_arb_cycle;
  wire             tiger_burst_2_upstream_any_bursting_master_saved_grant;
  wire             tiger_burst_2_upstream_any_continuerequest;
  wire             tiger_burst_2_upstream_arb_counter_enable;
  reg     [  4: 0] tiger_burst_2_upstream_arb_share_counter;
  wire    [  4: 0] tiger_burst_2_upstream_arb_share_counter_next_value;
  wire    [  4: 0] tiger_burst_2_upstream_arb_share_set_values;
  reg     [  1: 0] tiger_burst_2_upstream_bbt_burstcounter;
  wire             tiger_burst_2_upstream_beginbursttransfer_internal;
  wire             tiger_burst_2_upstream_begins_xfer;
  wire    [  2: 0] tiger_burst_2_upstream_burstcount;
  wire             tiger_burst_2_upstream_burstcount_fifo_empty;
  wire    [ 10: 0] tiger_burst_2_upstream_byteaddress;
  wire    [ 15: 0] tiger_burst_2_upstream_byteenable;
  reg     [  2: 0] tiger_burst_2_upstream_current_burst;
  wire    [  2: 0] tiger_burst_2_upstream_current_burst_minus_one;
  wire             tiger_burst_2_upstream_debugaccess;
  wire             tiger_burst_2_upstream_end_xfer;
  wire             tiger_burst_2_upstream_firsttransfer;
  wire             tiger_burst_2_upstream_grant_vector;
  wire             tiger_burst_2_upstream_in_a_read_cycle;
  wire             tiger_burst_2_upstream_in_a_write_cycle;
  reg              tiger_burst_2_upstream_load_fifo;
  wire             tiger_burst_2_upstream_master_qreq_vector;
  wire             tiger_burst_2_upstream_move_on_to_next_transaction;
  wire    [  1: 0] tiger_burst_2_upstream_next_bbt_burstcount;
  wire    [  2: 0] tiger_burst_2_upstream_next_burst_count;
  wire             tiger_burst_2_upstream_non_bursting_master_requests;
  wire             tiger_burst_2_upstream_read;
  wire    [127: 0] tiger_burst_2_upstream_readdata_from_sa;
  wire             tiger_burst_2_upstream_readdatavalid_from_sa;
  reg              tiger_burst_2_upstream_reg_firsttransfer;
  wire    [  2: 0] tiger_burst_2_upstream_selected_burstcount;
  reg              tiger_burst_2_upstream_slavearbiterlockenable;
  wire             tiger_burst_2_upstream_slavearbiterlockenable2;
  wire             tiger_burst_2_upstream_this_cycle_is_the_last_burst;
  wire    [  2: 0] tiger_burst_2_upstream_transaction_burst_count;
  wire             tiger_burst_2_upstream_unreg_firsttransfer;
  wire             tiger_burst_2_upstream_waitrequest_from_sa;
  wire             tiger_burst_2_upstream_waits_for_read;
  wire             tiger_burst_2_upstream_waits_for_write;
  wire             tiger_burst_2_upstream_write;
  wire    [127: 0] tiger_burst_2_upstream_writedata;
  wire             wait_for_tiger_burst_2_upstream_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~tiger_burst_2_upstream_end_xfer;
    end


  assign tiger_burst_2_upstream_begins_xfer = ~d1_reasons_to_wait & ((pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream));
  //assign tiger_burst_2_upstream_readdatavalid_from_sa = tiger_burst_2_upstream_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_2_upstream_readdatavalid_from_sa = tiger_burst_2_upstream_readdatavalid;

  //assign tiger_burst_2_upstream_readdata_from_sa = tiger_burst_2_upstream_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_2_upstream_readdata_from_sa = tiger_burst_2_upstream_readdata;

  assign pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream = ({pipeline_bridge_MEMORY_m1_address_to_slave[24 : 7] , 7'b0} == 25'h1000000) & pipeline_bridge_MEMORY_m1_chipselect;
  //assign tiger_burst_2_upstream_waitrequest_from_sa = tiger_burst_2_upstream_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_2_upstream_waitrequest_from_sa = tiger_burst_2_upstream_waitrequest;

  //tiger_burst_2_upstream_arb_share_counter set values, which is an e_mux
  assign tiger_burst_2_upstream_arb_share_set_values = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream)? ((((pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect)) ? pipeline_bridge_MEMORY_m1_burstcount : 1)) :
    1;

  //tiger_burst_2_upstream_non_bursting_master_requests mux, which is an e_mux
  assign tiger_burst_2_upstream_non_bursting_master_requests = 0;

  //tiger_burst_2_upstream_any_bursting_master_saved_grant mux, which is an e_mux
  assign tiger_burst_2_upstream_any_bursting_master_saved_grant = pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_2_upstream;

  //tiger_burst_2_upstream_arb_share_counter_next_value assignment, which is an e_assign
  assign tiger_burst_2_upstream_arb_share_counter_next_value = tiger_burst_2_upstream_firsttransfer ? (tiger_burst_2_upstream_arb_share_set_values - 1) : |tiger_burst_2_upstream_arb_share_counter ? (tiger_burst_2_upstream_arb_share_counter - 1) : 0;

  //tiger_burst_2_upstream_allgrants all slave grants, which is an e_mux
  assign tiger_burst_2_upstream_allgrants = |tiger_burst_2_upstream_grant_vector;

  //tiger_burst_2_upstream_end_xfer assignment, which is an e_assign
  assign tiger_burst_2_upstream_end_xfer = ~(tiger_burst_2_upstream_waits_for_read | tiger_burst_2_upstream_waits_for_write);

  //end_xfer_arb_share_counter_term_tiger_burst_2_upstream arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_tiger_burst_2_upstream = tiger_burst_2_upstream_end_xfer & (~tiger_burst_2_upstream_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //tiger_burst_2_upstream_arb_share_counter arbitration counter enable, which is an e_assign
  assign tiger_burst_2_upstream_arb_counter_enable = (end_xfer_arb_share_counter_term_tiger_burst_2_upstream & tiger_burst_2_upstream_allgrants) | (end_xfer_arb_share_counter_term_tiger_burst_2_upstream & ~tiger_burst_2_upstream_non_bursting_master_requests);

  //tiger_burst_2_upstream_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_upstream_arb_share_counter <= 0;
      else if (tiger_burst_2_upstream_arb_counter_enable)
          tiger_burst_2_upstream_arb_share_counter <= tiger_burst_2_upstream_arb_share_counter_next_value;
    end


  //tiger_burst_2_upstream_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_upstream_slavearbiterlockenable <= 0;
      else if ((|tiger_burst_2_upstream_master_qreq_vector & end_xfer_arb_share_counter_term_tiger_burst_2_upstream) | (end_xfer_arb_share_counter_term_tiger_burst_2_upstream & ~tiger_burst_2_upstream_non_bursting_master_requests))
          tiger_burst_2_upstream_slavearbiterlockenable <= |tiger_burst_2_upstream_arb_share_counter_next_value;
    end


  //pipeline_bridge_MEMORY/m1 tiger_burst_2/upstream arbiterlock, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock = tiger_burst_2_upstream_slavearbiterlockenable & pipeline_bridge_MEMORY_m1_continuerequest;

  //tiger_burst_2_upstream_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign tiger_burst_2_upstream_slavearbiterlockenable2 = |tiger_burst_2_upstream_arb_share_counter_next_value;

  //pipeline_bridge_MEMORY/m1 tiger_burst_2/upstream arbiterlock2, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_arbiterlock2 = tiger_burst_2_upstream_slavearbiterlockenable2 & pipeline_bridge_MEMORY_m1_continuerequest;

  //tiger_burst_2_upstream_any_continuerequest at least one master continues requesting, which is an e_assign
  assign tiger_burst_2_upstream_any_continuerequest = 1;

  //pipeline_bridge_MEMORY_m1_continuerequest continued request, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_continuerequest = 1;

  assign pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream = pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream & ~(((pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect) & ((pipeline_bridge_MEMORY_m1_latency_counter != 0) | (1 < pipeline_bridge_MEMORY_m1_latency_counter) | (|pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register) | (|pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register))));
  //unique name for tiger_burst_2_upstream_move_on_to_next_transaction, which is an e_assign
  assign tiger_burst_2_upstream_move_on_to_next_transaction = tiger_burst_2_upstream_this_cycle_is_the_last_burst & tiger_burst_2_upstream_load_fifo;

  //the currently selected burstcount for tiger_burst_2_upstream, which is an e_mux
  assign tiger_burst_2_upstream_selected_burstcount = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream)? pipeline_bridge_MEMORY_m1_burstcount :
    1;

  //burstcount_fifo_for_tiger_burst_2_upstream, which is an e_fifo_with_registered_outputs
  burstcount_fifo_for_tiger_burst_2_upstream_module burstcount_fifo_for_tiger_burst_2_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_burst_2_upstream_selected_burstcount),
      .data_out             (tiger_burst_2_upstream_transaction_burst_count),
      .empty                (tiger_burst_2_upstream_burstcount_fifo_empty),
      .fifo_contains_ones_n (),
      .full                 (),
      .read                 (tiger_burst_2_upstream_this_cycle_is_the_last_burst),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_2_upstream_waits_for_read & tiger_burst_2_upstream_load_fifo & ~(tiger_burst_2_upstream_this_cycle_is_the_last_burst & tiger_burst_2_upstream_burstcount_fifo_empty))
    );

  //tiger_burst_2_upstream current burst minus one, which is an e_assign
  assign tiger_burst_2_upstream_current_burst_minus_one = tiger_burst_2_upstream_current_burst - 1;

  //what to load in current_burst, for tiger_burst_2_upstream, which is an e_mux
  assign tiger_burst_2_upstream_next_burst_count = (((in_a_read_cycle & ~tiger_burst_2_upstream_waits_for_read) & ~tiger_burst_2_upstream_load_fifo))? tiger_burst_2_upstream_selected_burstcount :
    ((in_a_read_cycle & ~tiger_burst_2_upstream_waits_for_read & tiger_burst_2_upstream_this_cycle_is_the_last_burst & tiger_burst_2_upstream_burstcount_fifo_empty))? tiger_burst_2_upstream_selected_burstcount :
    (tiger_burst_2_upstream_this_cycle_is_the_last_burst)? tiger_burst_2_upstream_transaction_burst_count :
    tiger_burst_2_upstream_current_burst_minus_one;

  //the current burst count for tiger_burst_2_upstream, to be decremented, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_upstream_current_burst <= 0;
      else if (tiger_burst_2_upstream_readdatavalid_from_sa | (~tiger_burst_2_upstream_load_fifo & (in_a_read_cycle & ~tiger_burst_2_upstream_waits_for_read)))
          tiger_burst_2_upstream_current_burst <= tiger_burst_2_upstream_next_burst_count;
    end


  //a 1 or burstcount fifo empty, to initialize the counter, which is an e_mux
  assign p0_tiger_burst_2_upstream_load_fifo = (~tiger_burst_2_upstream_load_fifo)? 1 :
    (((in_a_read_cycle & ~tiger_burst_2_upstream_waits_for_read) & tiger_burst_2_upstream_load_fifo))? 1 :
    ~tiger_burst_2_upstream_burstcount_fifo_empty;

  //whether to load directly to the counter or to the fifo, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_upstream_load_fifo <= 0;
      else if ((in_a_read_cycle & ~tiger_burst_2_upstream_waits_for_read) & ~tiger_burst_2_upstream_load_fifo | tiger_burst_2_upstream_this_cycle_is_the_last_burst)
          tiger_burst_2_upstream_load_fifo <= p0_tiger_burst_2_upstream_load_fifo;
    end


  //the last cycle in the burst for tiger_burst_2_upstream, which is an e_assign
  assign tiger_burst_2_upstream_this_cycle_is_the_last_burst = ~(|tiger_burst_2_upstream_current_burst_minus_one) & tiger_burst_2_upstream_readdatavalid_from_sa;

  //rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_2_upstream, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_2_upstream_module rdv_fifo_for_pipeline_bridge_MEMORY_m1_to_tiger_burst_2_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream),
      .data_out             (pipeline_bridge_MEMORY_m1_rdv_fifo_output_from_tiger_burst_2_upstream),
      .empty                (),
      .fifo_contains_ones_n (pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_2_upstream),
      .full                 (),
      .read                 (tiger_burst_2_upstream_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_2_upstream_waits_for_read)
    );

  assign pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register = ~pipeline_bridge_MEMORY_m1_rdv_fifo_empty_tiger_burst_2_upstream;
  //local readdatavalid pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream = tiger_burst_2_upstream_readdatavalid_from_sa;

  //replicate narrow data for wide slave
  assign pipeline_bridge_MEMORY_m1_writedata_replicated = {pipeline_bridge_MEMORY_m1_writedata,
    pipeline_bridge_MEMORY_m1_writedata,
    pipeline_bridge_MEMORY_m1_writedata,
    pipeline_bridge_MEMORY_m1_writedata};

  //tiger_burst_2_upstream_writedata mux, which is an e_mux
  assign tiger_burst_2_upstream_writedata = pipeline_bridge_MEMORY_m1_writedata_replicated;

  //byteaddress mux for tiger_burst_2/upstream, which is an e_mux
  assign tiger_burst_2_upstream_byteaddress = pipeline_bridge_MEMORY_m1_address_to_slave;

  //master is always granted when requested
  assign pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream = pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream;

  //pipeline_bridge_MEMORY/m1 saved-grant tiger_burst_2/upstream, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_saved_grant_tiger_burst_2_upstream = pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream;

  //allow new arb cycle for tiger_burst_2/upstream, which is an e_assign
  assign tiger_burst_2_upstream_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign tiger_burst_2_upstream_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign tiger_burst_2_upstream_master_qreq_vector = 1;

  //tiger_burst_2_upstream_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_2_upstream_firsttransfer = tiger_burst_2_upstream_begins_xfer ? tiger_burst_2_upstream_unreg_firsttransfer : tiger_burst_2_upstream_reg_firsttransfer;

  //tiger_burst_2_upstream_unreg_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_2_upstream_unreg_firsttransfer = ~(tiger_burst_2_upstream_slavearbiterlockenable & tiger_burst_2_upstream_any_continuerequest);

  //tiger_burst_2_upstream_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_upstream_reg_firsttransfer <= 1'b1;
      else if (tiger_burst_2_upstream_begins_xfer)
          tiger_burst_2_upstream_reg_firsttransfer <= tiger_burst_2_upstream_unreg_firsttransfer;
    end


  //tiger_burst_2_upstream_next_bbt_burstcount next_bbt_burstcount, which is an e_mux
  assign tiger_burst_2_upstream_next_bbt_burstcount = ((((tiger_burst_2_upstream_write) && (tiger_burst_2_upstream_bbt_burstcounter == 0))))? (tiger_burst_2_upstream_burstcount - 1) :
    ((((tiger_burst_2_upstream_read) && (tiger_burst_2_upstream_bbt_burstcounter == 0))))? 0 :
    (tiger_burst_2_upstream_bbt_burstcounter - 1);

  //tiger_burst_2_upstream_bbt_burstcounter bbt_burstcounter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_upstream_bbt_burstcounter <= 0;
      else if (tiger_burst_2_upstream_begins_xfer)
          tiger_burst_2_upstream_bbt_burstcounter <= tiger_burst_2_upstream_next_bbt_burstcount;
    end


  //tiger_burst_2_upstream_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign tiger_burst_2_upstream_beginbursttransfer_internal = tiger_burst_2_upstream_begins_xfer & (tiger_burst_2_upstream_bbt_burstcounter == 0);

  //tiger_burst_2_upstream_read assignment, which is an e_mux
  assign tiger_burst_2_upstream_read = pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect);

  //tiger_burst_2_upstream_write assignment, which is an e_mux
  assign tiger_burst_2_upstream_write = pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect);

  //tiger_burst_2_upstream_address mux, which is an e_mux
  assign tiger_burst_2_upstream_address = pipeline_bridge_MEMORY_m1_address_to_slave;

  //d1_tiger_burst_2_upstream_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_tiger_burst_2_upstream_end_xfer <= 1;
      else 
        d1_tiger_burst_2_upstream_end_xfer <= tiger_burst_2_upstream_end_xfer;
    end


  //tiger_burst_2_upstream_waits_for_read in a cycle, which is an e_mux
  assign tiger_burst_2_upstream_waits_for_read = tiger_burst_2_upstream_in_a_read_cycle & tiger_burst_2_upstream_waitrequest_from_sa;

  //tiger_burst_2_upstream_in_a_read_cycle assignment, which is an e_assign
  assign tiger_burst_2_upstream_in_a_read_cycle = pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream & (pipeline_bridge_MEMORY_m1_read & pipeline_bridge_MEMORY_m1_chipselect);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = tiger_burst_2_upstream_in_a_read_cycle;

  //tiger_burst_2_upstream_waits_for_write in a cycle, which is an e_mux
  assign tiger_burst_2_upstream_waits_for_write = tiger_burst_2_upstream_in_a_write_cycle & tiger_burst_2_upstream_waitrequest_from_sa;

  //tiger_burst_2_upstream_in_a_write_cycle assignment, which is an e_assign
  assign tiger_burst_2_upstream_in_a_write_cycle = pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream & (pipeline_bridge_MEMORY_m1_write & pipeline_bridge_MEMORY_m1_chipselect);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = tiger_burst_2_upstream_in_a_write_cycle;

  assign wait_for_tiger_burst_2_upstream_counter = 0;
  //tiger_burst_2_upstream_byteenable byte enable port mux, which is an e_mux
  assign tiger_burst_2_upstream_byteenable = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream)? pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_2_upstream :
    -1;

  //be mux control reg for pipeline_bridge_MEMORY/m1 and tiger_burst_2/upstream, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream_reg <= 0;
      else 
        pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream_reg <= tiger_burst_2_upstream_beginbursttransfer_internal & ~tiger_burst_2_upstream_waitrequest_from_sa ? (pipeline_bridge_MEMORY_m1_address_to_slave[3 : 2] + 1) : tiger_burst_2_upstream_beginbursttransfer_internal & tiger_burst_2_upstream_waitrequest_from_sa ? pipeline_bridge_MEMORY_m1_address_to_slave[3 : 2] : ~tiger_burst_2_upstream_waitrequest_from_sa ? (pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream_reg + 1) :  pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream_reg;
    end


  //be mux control for pipeline_bridge_MEMORY/m1 and tiger_burst_2/upstream, which is an e_assign
  assign pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream = tiger_burst_2_upstream_beginbursttransfer_internal ? pipeline_bridge_MEMORY_m1_address_to_slave[3 : 2] : pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream_reg;

  //byte_enable_mux for pipeline_bridge_MEMORY/m1 and tiger_burst_2/upstream, which is an e_mux
  assign pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_2_upstream = (pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream == 0)? pipeline_bridge_MEMORY_m1_byteenable :
    (pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream == 1)? {pipeline_bridge_MEMORY_m1_byteenable, {4'b0}} :
    (pipeline_bridge_MEMORY_m1_be_mux_control_tiger_burst_2_upstream == 2)? {pipeline_bridge_MEMORY_m1_byteenable, {8'b0}} :
    {pipeline_bridge_MEMORY_m1_byteenable, {12'b0}};

  //burstcount mux, which is an e_mux
  assign tiger_burst_2_upstream_burstcount = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream)? pipeline_bridge_MEMORY_m1_burstcount :
    1;

  //debugaccess mux, which is an e_mux
  assign tiger_burst_2_upstream_debugaccess = (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream)? pipeline_bridge_MEMORY_m1_debugaccess :
    0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_2/upstream enable non-zero assertions, which is an e_register
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
      if (pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream && (pipeline_bridge_MEMORY_m1_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: pipeline_bridge_MEMORY/m1 drove 0 on its 'burstcount' port while accessing slave tiger_burst_2/upstream", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_burst_2_downstream_arbitrator (
                                             // inputs:
                                              clk,
                                              d1_data_cache_0_ACCEL_end_xfer,
                                              data_cache_0_ACCEL_readdata_from_sa,
                                              data_cache_0_ACCEL_waitrequest_from_sa,
                                              reset_n,
                                              tiger_burst_2_downstream_address,
                                              tiger_burst_2_downstream_burstcount,
                                              tiger_burst_2_downstream_byteenable,
                                              tiger_burst_2_downstream_granted_data_cache_0_ACCEL,
                                              tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL,
                                              tiger_burst_2_downstream_read,
                                              tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL,
                                              tiger_burst_2_downstream_requests_data_cache_0_ACCEL,
                                              tiger_burst_2_downstream_write,
                                              tiger_burst_2_downstream_writedata,

                                             // outputs:
                                              tiger_burst_2_downstream_address_to_slave,
                                              tiger_burst_2_downstream_latency_counter,
                                              tiger_burst_2_downstream_readdata,
                                              tiger_burst_2_downstream_readdatavalid,
                                              tiger_burst_2_downstream_reset_n,
                                              tiger_burst_2_downstream_waitrequest
                                           )
;

  output  [  6: 0] tiger_burst_2_downstream_address_to_slave;
  output           tiger_burst_2_downstream_latency_counter;
  output  [127: 0] tiger_burst_2_downstream_readdata;
  output           tiger_burst_2_downstream_readdatavalid;
  output           tiger_burst_2_downstream_reset_n;
  output           tiger_burst_2_downstream_waitrequest;
  input            clk;
  input            d1_data_cache_0_ACCEL_end_xfer;
  input   [127: 0] data_cache_0_ACCEL_readdata_from_sa;
  input            data_cache_0_ACCEL_waitrequest_from_sa;
  input            reset_n;
  input   [  6: 0] tiger_burst_2_downstream_address;
  input            tiger_burst_2_downstream_burstcount;
  input   [ 15: 0] tiger_burst_2_downstream_byteenable;
  input            tiger_burst_2_downstream_granted_data_cache_0_ACCEL;
  input            tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL;
  input            tiger_burst_2_downstream_read;
  input            tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL;
  input            tiger_burst_2_downstream_requests_data_cache_0_ACCEL;
  input            tiger_burst_2_downstream_write;
  input   [127: 0] tiger_burst_2_downstream_writedata;

  reg              active_and_waiting_last_time;
  wire             pre_flush_tiger_burst_2_downstream_readdatavalid;
  wire             r_0;
  reg     [  6: 0] tiger_burst_2_downstream_address_last_time;
  wire    [  6: 0] tiger_burst_2_downstream_address_to_slave;
  reg              tiger_burst_2_downstream_burstcount_last_time;
  reg     [ 15: 0] tiger_burst_2_downstream_byteenable_last_time;
  wire             tiger_burst_2_downstream_latency_counter;
  reg              tiger_burst_2_downstream_read_last_time;
  wire    [127: 0] tiger_burst_2_downstream_readdata;
  wire             tiger_burst_2_downstream_readdatavalid;
  wire             tiger_burst_2_downstream_reset_n;
  wire             tiger_burst_2_downstream_run;
  wire             tiger_burst_2_downstream_waitrequest;
  reg              tiger_burst_2_downstream_write_last_time;
  reg     [127: 0] tiger_burst_2_downstream_writedata_last_time;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL | ~tiger_burst_2_downstream_requests_data_cache_0_ACCEL) & ((~tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL | ~(tiger_burst_2_downstream_read | tiger_burst_2_downstream_write) | (1 & ~data_cache_0_ACCEL_waitrequest_from_sa & (tiger_burst_2_downstream_read | tiger_burst_2_downstream_write)))) & ((~tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL | ~(tiger_burst_2_downstream_read | tiger_burst_2_downstream_write) | (1 & ~data_cache_0_ACCEL_waitrequest_from_sa & (tiger_burst_2_downstream_read | tiger_burst_2_downstream_write))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_burst_2_downstream_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_burst_2_downstream_address_to_slave = tiger_burst_2_downstream_address;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_tiger_burst_2_downstream_readdatavalid = 0;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign tiger_burst_2_downstream_readdatavalid = 0 |
    pre_flush_tiger_burst_2_downstream_readdatavalid |
    tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL;

  //tiger_burst_2/downstream readdata mux, which is an e_mux
  assign tiger_burst_2_downstream_readdata = data_cache_0_ACCEL_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_burst_2_downstream_waitrequest = ~tiger_burst_2_downstream_run;

  //latent max counter, which is an e_assign
  assign tiger_burst_2_downstream_latency_counter = 0;

  //tiger_burst_2_downstream_reset_n assignment, which is an e_assign
  assign tiger_burst_2_downstream_reset_n = reset_n;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_2_downstream_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_downstream_address_last_time <= 0;
      else 
        tiger_burst_2_downstream_address_last_time <= tiger_burst_2_downstream_address;
    end


  //tiger_burst_2/downstream waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= tiger_burst_2_downstream_waitrequest & (tiger_burst_2_downstream_read | tiger_burst_2_downstream_write);
    end


  //tiger_burst_2_downstream_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_2_downstream_address != tiger_burst_2_downstream_address_last_time))
        begin
          $write("%0d ns: tiger_burst_2_downstream_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_2_downstream_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_downstream_burstcount_last_time <= 0;
      else 
        tiger_burst_2_downstream_burstcount_last_time <= tiger_burst_2_downstream_burstcount;
    end


  //tiger_burst_2_downstream_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_2_downstream_burstcount != tiger_burst_2_downstream_burstcount_last_time))
        begin
          $write("%0d ns: tiger_burst_2_downstream_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_2_downstream_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_downstream_byteenable_last_time <= 0;
      else 
        tiger_burst_2_downstream_byteenable_last_time <= tiger_burst_2_downstream_byteenable;
    end


  //tiger_burst_2_downstream_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_2_downstream_byteenable != tiger_burst_2_downstream_byteenable_last_time))
        begin
          $write("%0d ns: tiger_burst_2_downstream_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_2_downstream_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_downstream_read_last_time <= 0;
      else 
        tiger_burst_2_downstream_read_last_time <= tiger_burst_2_downstream_read;
    end


  //tiger_burst_2_downstream_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_2_downstream_read != tiger_burst_2_downstream_read_last_time))
        begin
          $write("%0d ns: tiger_burst_2_downstream_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_2_downstream_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_downstream_write_last_time <= 0;
      else 
        tiger_burst_2_downstream_write_last_time <= tiger_burst_2_downstream_write;
    end


  //tiger_burst_2_downstream_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_2_downstream_write != tiger_burst_2_downstream_write_last_time))
        begin
          $write("%0d ns: tiger_burst_2_downstream_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_2_downstream_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_2_downstream_writedata_last_time <= 0;
      else 
        tiger_burst_2_downstream_writedata_last_time <= tiger_burst_2_downstream_writedata;
    end


  //tiger_burst_2_downstream_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_2_downstream_writedata != tiger_burst_2_downstream_writedata_last_time) & tiger_burst_2_downstream_write)
        begin
          $write("%0d ns: tiger_burst_2_downstream_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module burstcount_fifo_for_tiger_burst_3_upstream_module (
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
  reg              full_2;
  wire             full_3;
  reg     [  2: 0] how_many_ones;
  wire    [  2: 0] one_count_minus_one;
  wire    [  2: 0] one_count_plus_one;
  wire             p0_full_0;
  wire    [  2: 0] p0_stage_0;
  wire             p1_full_1;
  wire    [  2: 0] p1_stage_1;
  wire             p2_full_2;
  wire    [  2: 0] p2_stage_2;
  reg     [  2: 0] stage_0;
  reg     [  2: 0] stage_1;
  reg     [  2: 0] stage_2;
  wire    [  2: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_2;
  assign empty = !full_0;
  assign full_3 = 0;
  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module rdv_fifo_for_data_cache_0_dataMaster_to_tiger_burst_3_upstream_module (
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
  wire             full_3;
  reg     [  2: 0] how_many_ones;
  wire    [  2: 0] one_count_minus_one;
  wire    [  2: 0] one_count_plus_one;
  wire             p0_full_0;
  wire             p0_stage_0;
  wire             p1_full_1;
  wire             p1_stage_1;
  wire             p2_full_2;
  wire             p2_stage_2;
  reg              stage_0;
  reg              stage_1;
  reg              stage_2;
  wire    [  2: 0] updated_one_count;
  assign data_out = stage_0;
  assign full = full_2;
  assign empty = !full_0;
  assign full_3 = 0;
  //data_2, which is an e_mux
  assign p2_stage_2 = ((full_3 & ~clear_fifo) == 0)? data_in :
    data_in;

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
    0;

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_burst_3_upstream_arbitrator (
                                           // inputs:
                                            clk,
                                            data_cache_0_dataMaster_address_to_slave,
                                            data_cache_0_dataMaster_burstcount,
                                            data_cache_0_dataMaster_byteenable,
                                            data_cache_0_dataMaster_latency_counter,
                                            data_cache_0_dataMaster_read,
                                            data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register,
                                            data_cache_0_dataMaster_write,
                                            data_cache_0_dataMaster_writedata,
                                            reset_n,
                                            tiger_burst_3_upstream_readdata,
                                            tiger_burst_3_upstream_readdatavalid,
                                            tiger_burst_3_upstream_waitrequest,

                                           // outputs:
                                            d1_tiger_burst_3_upstream_end_xfer,
                                            data_cache_0_dataMaster_granted_tiger_burst_3_upstream,
                                            data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream,
                                            data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream,
                                            data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register,
                                            data_cache_0_dataMaster_requests_tiger_burst_3_upstream,
                                            tiger_burst_3_upstream_address,
                                            tiger_burst_3_upstream_burstcount,
                                            tiger_burst_3_upstream_byteaddress,
                                            tiger_burst_3_upstream_byteenable,
                                            tiger_burst_3_upstream_debugaccess,
                                            tiger_burst_3_upstream_read,
                                            tiger_burst_3_upstream_readdata_from_sa,
                                            tiger_burst_3_upstream_waitrequest_from_sa,
                                            tiger_burst_3_upstream_write,
                                            tiger_burst_3_upstream_writedata
                                         )
;

  output           d1_tiger_burst_3_upstream_end_xfer;
  output           data_cache_0_dataMaster_granted_tiger_burst_3_upstream;
  output           data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream;
  output           data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream;
  output           data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register;
  output           data_cache_0_dataMaster_requests_tiger_burst_3_upstream;
  output  [ 13: 0] tiger_burst_3_upstream_address;
  output  [  2: 0] tiger_burst_3_upstream_burstcount;
  output  [ 15: 0] tiger_burst_3_upstream_byteaddress;
  output  [  3: 0] tiger_burst_3_upstream_byteenable;
  output           tiger_burst_3_upstream_debugaccess;
  output           tiger_burst_3_upstream_read;
  output  [ 31: 0] tiger_burst_3_upstream_readdata_from_sa;
  output           tiger_burst_3_upstream_waitrequest_from_sa;
  output           tiger_burst_3_upstream_write;
  output  [ 31: 0] tiger_burst_3_upstream_writedata;
  input            clk;
  input   [ 31: 0] data_cache_0_dataMaster_address_to_slave;
  input   [  2: 0] data_cache_0_dataMaster_burstcount;
  input   [  3: 0] data_cache_0_dataMaster_byteenable;
  input            data_cache_0_dataMaster_latency_counter;
  input            data_cache_0_dataMaster_read;
  input            data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  input            data_cache_0_dataMaster_write;
  input   [ 31: 0] data_cache_0_dataMaster_writedata;
  input            reset_n;
  input   [ 31: 0] tiger_burst_3_upstream_readdata;
  input            tiger_burst_3_upstream_readdatavalid;
  input            tiger_burst_3_upstream_waitrequest;

  reg              d1_reasons_to_wait;
  reg              d1_tiger_burst_3_upstream_end_xfer;
  wire             data_cache_0_dataMaster_arbiterlock;
  wire             data_cache_0_dataMaster_arbiterlock2;
  wire             data_cache_0_dataMaster_continuerequest;
  wire             data_cache_0_dataMaster_granted_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_rdv_fifo_empty_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_rdv_fifo_output_from_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register;
  wire             data_cache_0_dataMaster_requests_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_saved_grant_tiger_burst_3_upstream;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_tiger_burst_3_upstream;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             p0_tiger_burst_3_upstream_load_fifo;
  wire    [ 13: 0] tiger_burst_3_upstream_address;
  wire             tiger_burst_3_upstream_allgrants;
  wire             tiger_burst_3_upstream_allow_new_arb_cycle;
  wire             tiger_burst_3_upstream_any_bursting_master_saved_grant;
  wire             tiger_burst_3_upstream_any_continuerequest;
  wire             tiger_burst_3_upstream_arb_counter_enable;
  reg     [  2: 0] tiger_burst_3_upstream_arb_share_counter;
  wire    [  2: 0] tiger_burst_3_upstream_arb_share_counter_next_value;
  wire    [  2: 0] tiger_burst_3_upstream_arb_share_set_values;
  reg     [  1: 0] tiger_burst_3_upstream_bbt_burstcounter;
  wire             tiger_burst_3_upstream_beginbursttransfer_internal;
  wire             tiger_burst_3_upstream_begins_xfer;
  wire    [  2: 0] tiger_burst_3_upstream_burstcount;
  wire             tiger_burst_3_upstream_burstcount_fifo_empty;
  wire    [ 15: 0] tiger_burst_3_upstream_byteaddress;
  wire    [  3: 0] tiger_burst_3_upstream_byteenable;
  reg     [  2: 0] tiger_burst_3_upstream_current_burst;
  wire    [  2: 0] tiger_burst_3_upstream_current_burst_minus_one;
  wire             tiger_burst_3_upstream_debugaccess;
  wire             tiger_burst_3_upstream_end_xfer;
  wire             tiger_burst_3_upstream_firsttransfer;
  wire             tiger_burst_3_upstream_grant_vector;
  wire             tiger_burst_3_upstream_in_a_read_cycle;
  wire             tiger_burst_3_upstream_in_a_write_cycle;
  reg              tiger_burst_3_upstream_load_fifo;
  wire             tiger_burst_3_upstream_master_qreq_vector;
  wire             tiger_burst_3_upstream_move_on_to_next_transaction;
  wire    [  1: 0] tiger_burst_3_upstream_next_bbt_burstcount;
  wire    [  2: 0] tiger_burst_3_upstream_next_burst_count;
  wire             tiger_burst_3_upstream_non_bursting_master_requests;
  wire             tiger_burst_3_upstream_read;
  wire    [ 31: 0] tiger_burst_3_upstream_readdata_from_sa;
  wire             tiger_burst_3_upstream_readdatavalid_from_sa;
  reg              tiger_burst_3_upstream_reg_firsttransfer;
  wire    [  2: 0] tiger_burst_3_upstream_selected_burstcount;
  reg              tiger_burst_3_upstream_slavearbiterlockenable;
  wire             tiger_burst_3_upstream_slavearbiterlockenable2;
  wire             tiger_burst_3_upstream_this_cycle_is_the_last_burst;
  wire    [  2: 0] tiger_burst_3_upstream_transaction_burst_count;
  wire             tiger_burst_3_upstream_unreg_firsttransfer;
  wire             tiger_burst_3_upstream_waitrequest_from_sa;
  wire             tiger_burst_3_upstream_waits_for_read;
  wire             tiger_burst_3_upstream_waits_for_write;
  wire             tiger_burst_3_upstream_write;
  wire    [ 31: 0] tiger_burst_3_upstream_writedata;
  wire             wait_for_tiger_burst_3_upstream_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~tiger_burst_3_upstream_end_xfer;
    end


  assign tiger_burst_3_upstream_begins_xfer = ~d1_reasons_to_wait & ((data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream));
  //assign tiger_burst_3_upstream_readdatavalid_from_sa = tiger_burst_3_upstream_readdatavalid so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_3_upstream_readdatavalid_from_sa = tiger_burst_3_upstream_readdatavalid;

  //assign tiger_burst_3_upstream_readdata_from_sa = tiger_burst_3_upstream_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_3_upstream_readdata_from_sa = tiger_burst_3_upstream_readdata;

  assign data_cache_0_dataMaster_requests_tiger_burst_3_upstream = ({data_cache_0_dataMaster_address_to_slave[31 : 14] , 14'b0} == 32'hf0000000) & (data_cache_0_dataMaster_read | data_cache_0_dataMaster_write);
  //assign tiger_burst_3_upstream_waitrequest_from_sa = tiger_burst_3_upstream_waitrequest so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tiger_burst_3_upstream_waitrequest_from_sa = tiger_burst_3_upstream_waitrequest;

  //tiger_burst_3_upstream_arb_share_counter set values, which is an e_mux
  assign tiger_burst_3_upstream_arb_share_set_values = (data_cache_0_dataMaster_granted_tiger_burst_3_upstream)? (((data_cache_0_dataMaster_write) ? data_cache_0_dataMaster_burstcount : 1)) :
    1;

  //tiger_burst_3_upstream_non_bursting_master_requests mux, which is an e_mux
  assign tiger_burst_3_upstream_non_bursting_master_requests = 0;

  //tiger_burst_3_upstream_any_bursting_master_saved_grant mux, which is an e_mux
  assign tiger_burst_3_upstream_any_bursting_master_saved_grant = data_cache_0_dataMaster_saved_grant_tiger_burst_3_upstream;

  //tiger_burst_3_upstream_arb_share_counter_next_value assignment, which is an e_assign
  assign tiger_burst_3_upstream_arb_share_counter_next_value = tiger_burst_3_upstream_firsttransfer ? (tiger_burst_3_upstream_arb_share_set_values - 1) : |tiger_burst_3_upstream_arb_share_counter ? (tiger_burst_3_upstream_arb_share_counter - 1) : 0;

  //tiger_burst_3_upstream_allgrants all slave grants, which is an e_mux
  assign tiger_burst_3_upstream_allgrants = |tiger_burst_3_upstream_grant_vector;

  //tiger_burst_3_upstream_end_xfer assignment, which is an e_assign
  assign tiger_burst_3_upstream_end_xfer = ~(tiger_burst_3_upstream_waits_for_read | tiger_burst_3_upstream_waits_for_write);

  //end_xfer_arb_share_counter_term_tiger_burst_3_upstream arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_tiger_burst_3_upstream = tiger_burst_3_upstream_end_xfer & (~tiger_burst_3_upstream_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //tiger_burst_3_upstream_arb_share_counter arbitration counter enable, which is an e_assign
  assign tiger_burst_3_upstream_arb_counter_enable = (end_xfer_arb_share_counter_term_tiger_burst_3_upstream & tiger_burst_3_upstream_allgrants) | (end_xfer_arb_share_counter_term_tiger_burst_3_upstream & ~tiger_burst_3_upstream_non_bursting_master_requests);

  //tiger_burst_3_upstream_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_upstream_arb_share_counter <= 0;
      else if (tiger_burst_3_upstream_arb_counter_enable)
          tiger_burst_3_upstream_arb_share_counter <= tiger_burst_3_upstream_arb_share_counter_next_value;
    end


  //tiger_burst_3_upstream_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_upstream_slavearbiterlockenable <= 0;
      else if ((|tiger_burst_3_upstream_master_qreq_vector & end_xfer_arb_share_counter_term_tiger_burst_3_upstream) | (end_xfer_arb_share_counter_term_tiger_burst_3_upstream & ~tiger_burst_3_upstream_non_bursting_master_requests))
          tiger_burst_3_upstream_slavearbiterlockenable <= |tiger_burst_3_upstream_arb_share_counter_next_value;
    end


  //data_cache_0/dataMaster tiger_burst_3/upstream arbiterlock, which is an e_assign
  assign data_cache_0_dataMaster_arbiterlock = tiger_burst_3_upstream_slavearbiterlockenable & data_cache_0_dataMaster_continuerequest;

  //tiger_burst_3_upstream_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign tiger_burst_3_upstream_slavearbiterlockenable2 = |tiger_burst_3_upstream_arb_share_counter_next_value;

  //data_cache_0/dataMaster tiger_burst_3/upstream arbiterlock2, which is an e_assign
  assign data_cache_0_dataMaster_arbiterlock2 = tiger_burst_3_upstream_slavearbiterlockenable2 & data_cache_0_dataMaster_continuerequest;

  //tiger_burst_3_upstream_any_continuerequest at least one master continues requesting, which is an e_assign
  assign tiger_burst_3_upstream_any_continuerequest = 1;

  //data_cache_0_dataMaster_continuerequest continued request, which is an e_assign
  assign data_cache_0_dataMaster_continuerequest = 1;

  assign data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream = data_cache_0_dataMaster_requests_tiger_burst_3_upstream & ~((data_cache_0_dataMaster_read & ((data_cache_0_dataMaster_latency_counter != 0) | (1 < data_cache_0_dataMaster_latency_counter) | (|data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register))));
  //unique name for tiger_burst_3_upstream_move_on_to_next_transaction, which is an e_assign
  assign tiger_burst_3_upstream_move_on_to_next_transaction = tiger_burst_3_upstream_this_cycle_is_the_last_burst & tiger_burst_3_upstream_load_fifo;

  //the currently selected burstcount for tiger_burst_3_upstream, which is an e_mux
  assign tiger_burst_3_upstream_selected_burstcount = (data_cache_0_dataMaster_granted_tiger_burst_3_upstream)? data_cache_0_dataMaster_burstcount :
    1;

  //burstcount_fifo_for_tiger_burst_3_upstream, which is an e_fifo_with_registered_outputs
  burstcount_fifo_for_tiger_burst_3_upstream_module burstcount_fifo_for_tiger_burst_3_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (tiger_burst_3_upstream_selected_burstcount),
      .data_out             (tiger_burst_3_upstream_transaction_burst_count),
      .empty                (tiger_burst_3_upstream_burstcount_fifo_empty),
      .fifo_contains_ones_n (),
      .full                 (),
      .read                 (tiger_burst_3_upstream_this_cycle_is_the_last_burst),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_3_upstream_waits_for_read & tiger_burst_3_upstream_load_fifo & ~(tiger_burst_3_upstream_this_cycle_is_the_last_burst & tiger_burst_3_upstream_burstcount_fifo_empty))
    );

  //tiger_burst_3_upstream current burst minus one, which is an e_assign
  assign tiger_burst_3_upstream_current_burst_minus_one = tiger_burst_3_upstream_current_burst - 1;

  //what to load in current_burst, for tiger_burst_3_upstream, which is an e_mux
  assign tiger_burst_3_upstream_next_burst_count = (((in_a_read_cycle & ~tiger_burst_3_upstream_waits_for_read) & ~tiger_burst_3_upstream_load_fifo))? tiger_burst_3_upstream_selected_burstcount :
    ((in_a_read_cycle & ~tiger_burst_3_upstream_waits_for_read & tiger_burst_3_upstream_this_cycle_is_the_last_burst & tiger_burst_3_upstream_burstcount_fifo_empty))? tiger_burst_3_upstream_selected_burstcount :
    (tiger_burst_3_upstream_this_cycle_is_the_last_burst)? tiger_burst_3_upstream_transaction_burst_count :
    tiger_burst_3_upstream_current_burst_minus_one;

  //the current burst count for tiger_burst_3_upstream, to be decremented, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_upstream_current_burst <= 0;
      else if (tiger_burst_3_upstream_readdatavalid_from_sa | (~tiger_burst_3_upstream_load_fifo & (in_a_read_cycle & ~tiger_burst_3_upstream_waits_for_read)))
          tiger_burst_3_upstream_current_burst <= tiger_burst_3_upstream_next_burst_count;
    end


  //a 1 or burstcount fifo empty, to initialize the counter, which is an e_mux
  assign p0_tiger_burst_3_upstream_load_fifo = (~tiger_burst_3_upstream_load_fifo)? 1 :
    (((in_a_read_cycle & ~tiger_burst_3_upstream_waits_for_read) & tiger_burst_3_upstream_load_fifo))? 1 :
    ~tiger_burst_3_upstream_burstcount_fifo_empty;

  //whether to load directly to the counter or to the fifo, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_upstream_load_fifo <= 0;
      else if ((in_a_read_cycle & ~tiger_burst_3_upstream_waits_for_read) & ~tiger_burst_3_upstream_load_fifo | tiger_burst_3_upstream_this_cycle_is_the_last_burst)
          tiger_burst_3_upstream_load_fifo <= p0_tiger_burst_3_upstream_load_fifo;
    end


  //the last cycle in the burst for tiger_burst_3_upstream, which is an e_assign
  assign tiger_burst_3_upstream_this_cycle_is_the_last_burst = ~(|tiger_burst_3_upstream_current_burst_minus_one) & tiger_burst_3_upstream_readdatavalid_from_sa;

  //rdv_fifo_for_data_cache_0_dataMaster_to_tiger_burst_3_upstream, which is an e_fifo_with_registered_outputs
  rdv_fifo_for_data_cache_0_dataMaster_to_tiger_burst_3_upstream_module rdv_fifo_for_data_cache_0_dataMaster_to_tiger_burst_3_upstream
    (
      .clear_fifo           (1'b0),
      .clk                  (clk),
      .data_in              (data_cache_0_dataMaster_granted_tiger_burst_3_upstream),
      .data_out             (data_cache_0_dataMaster_rdv_fifo_output_from_tiger_burst_3_upstream),
      .empty                (),
      .fifo_contains_ones_n (data_cache_0_dataMaster_rdv_fifo_empty_tiger_burst_3_upstream),
      .full                 (),
      .read                 (tiger_burst_3_upstream_move_on_to_next_transaction),
      .reset_n              (reset_n),
      .sync_reset           (1'b0),
      .write                (in_a_read_cycle & ~tiger_burst_3_upstream_waits_for_read)
    );

  assign data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register = ~data_cache_0_dataMaster_rdv_fifo_empty_tiger_burst_3_upstream;
  //local readdatavalid data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream, which is an e_mux
  assign data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream = tiger_burst_3_upstream_readdatavalid_from_sa;

  //tiger_burst_3_upstream_writedata mux, which is an e_mux
  assign tiger_burst_3_upstream_writedata = data_cache_0_dataMaster_writedata;

  //byteaddress mux for tiger_burst_3/upstream, which is an e_mux
  assign tiger_burst_3_upstream_byteaddress = data_cache_0_dataMaster_address_to_slave;

  //master is always granted when requested
  assign data_cache_0_dataMaster_granted_tiger_burst_3_upstream = data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream;

  //data_cache_0/dataMaster saved-grant tiger_burst_3/upstream, which is an e_assign
  assign data_cache_0_dataMaster_saved_grant_tiger_burst_3_upstream = data_cache_0_dataMaster_requests_tiger_burst_3_upstream;

  //allow new arb cycle for tiger_burst_3/upstream, which is an e_assign
  assign tiger_burst_3_upstream_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign tiger_burst_3_upstream_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign tiger_burst_3_upstream_master_qreq_vector = 1;

  //tiger_burst_3_upstream_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_3_upstream_firsttransfer = tiger_burst_3_upstream_begins_xfer ? tiger_burst_3_upstream_unreg_firsttransfer : tiger_burst_3_upstream_reg_firsttransfer;

  //tiger_burst_3_upstream_unreg_firsttransfer first transaction, which is an e_assign
  assign tiger_burst_3_upstream_unreg_firsttransfer = ~(tiger_burst_3_upstream_slavearbiterlockenable & tiger_burst_3_upstream_any_continuerequest);

  //tiger_burst_3_upstream_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_upstream_reg_firsttransfer <= 1'b1;
      else if (tiger_burst_3_upstream_begins_xfer)
          tiger_burst_3_upstream_reg_firsttransfer <= tiger_burst_3_upstream_unreg_firsttransfer;
    end


  //tiger_burst_3_upstream_next_bbt_burstcount next_bbt_burstcount, which is an e_mux
  assign tiger_burst_3_upstream_next_bbt_burstcount = ((((tiger_burst_3_upstream_write) && (tiger_burst_3_upstream_bbt_burstcounter == 0))))? (tiger_burst_3_upstream_burstcount - 1) :
    ((((tiger_burst_3_upstream_read) && (tiger_burst_3_upstream_bbt_burstcounter == 0))))? 0 :
    (tiger_burst_3_upstream_bbt_burstcounter - 1);

  //tiger_burst_3_upstream_bbt_burstcounter bbt_burstcounter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_upstream_bbt_burstcounter <= 0;
      else if (tiger_burst_3_upstream_begins_xfer)
          tiger_burst_3_upstream_bbt_burstcounter <= tiger_burst_3_upstream_next_bbt_burstcount;
    end


  //tiger_burst_3_upstream_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign tiger_burst_3_upstream_beginbursttransfer_internal = tiger_burst_3_upstream_begins_xfer & (tiger_burst_3_upstream_bbt_burstcounter == 0);

  //tiger_burst_3_upstream_read assignment, which is an e_mux
  assign tiger_burst_3_upstream_read = data_cache_0_dataMaster_granted_tiger_burst_3_upstream & data_cache_0_dataMaster_read;

  //tiger_burst_3_upstream_write assignment, which is an e_mux
  assign tiger_burst_3_upstream_write = data_cache_0_dataMaster_granted_tiger_burst_3_upstream & data_cache_0_dataMaster_write;

  //tiger_burst_3_upstream_address mux, which is an e_mux
  assign tiger_burst_3_upstream_address = data_cache_0_dataMaster_address_to_slave;

  //d1_tiger_burst_3_upstream_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_tiger_burst_3_upstream_end_xfer <= 1;
      else 
        d1_tiger_burst_3_upstream_end_xfer <= tiger_burst_3_upstream_end_xfer;
    end


  //tiger_burst_3_upstream_waits_for_read in a cycle, which is an e_mux
  assign tiger_burst_3_upstream_waits_for_read = tiger_burst_3_upstream_in_a_read_cycle & tiger_burst_3_upstream_waitrequest_from_sa;

  //tiger_burst_3_upstream_in_a_read_cycle assignment, which is an e_assign
  assign tiger_burst_3_upstream_in_a_read_cycle = data_cache_0_dataMaster_granted_tiger_burst_3_upstream & data_cache_0_dataMaster_read;

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = tiger_burst_3_upstream_in_a_read_cycle;

  //tiger_burst_3_upstream_waits_for_write in a cycle, which is an e_mux
  assign tiger_burst_3_upstream_waits_for_write = tiger_burst_3_upstream_in_a_write_cycle & tiger_burst_3_upstream_waitrequest_from_sa;

  //tiger_burst_3_upstream_in_a_write_cycle assignment, which is an e_assign
  assign tiger_burst_3_upstream_in_a_write_cycle = data_cache_0_dataMaster_granted_tiger_burst_3_upstream & data_cache_0_dataMaster_write;

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = tiger_burst_3_upstream_in_a_write_cycle;

  assign wait_for_tiger_burst_3_upstream_counter = 0;
  //tiger_burst_3_upstream_byteenable byte enable port mux, which is an e_mux
  assign tiger_burst_3_upstream_byteenable = (data_cache_0_dataMaster_granted_tiger_burst_3_upstream)? data_cache_0_dataMaster_byteenable :
    -1;

  //burstcount mux, which is an e_mux
  assign tiger_burst_3_upstream_burstcount = (data_cache_0_dataMaster_granted_tiger_burst_3_upstream)? data_cache_0_dataMaster_burstcount :
    1;

  //debugaccess mux, which is an e_mux
  assign tiger_burst_3_upstream_debugaccess = 0;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_3/upstream enable non-zero assertions, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          enable_nonzero_assertions <= 0;
      else 
        enable_nonzero_assertions <= 1'b1;
    end


  //data_cache_0/dataMaster non-zero burstcount assertion, which is an e_process
  always @(posedge clk)
    begin
      if (data_cache_0_dataMaster_requests_tiger_burst_3_upstream && (data_cache_0_dataMaster_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: data_cache_0/dataMaster drove 0 on its 'burstcount' port while accessing slave tiger_burst_3/upstream", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_burst_3_downstream_arbitrator (
                                             // inputs:
                                              clk,
                                              d1_pipeline_bridge_PERIPHERALS_s1_end_xfer,
                                              pipeline_bridge_PERIPHERALS_s1_readdata_from_sa,
                                              pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa,
                                              reset_n,
                                              tiger_burst_3_downstream_address,
                                              tiger_burst_3_downstream_burstcount,
                                              tiger_burst_3_downstream_byteenable,
                                              tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1,
                                              tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1,
                                              tiger_burst_3_downstream_read,
                                              tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1,
                                              tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register,
                                              tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1,
                                              tiger_burst_3_downstream_write,
                                              tiger_burst_3_downstream_writedata,

                                             // outputs:
                                              tiger_burst_3_downstream_address_to_slave,
                                              tiger_burst_3_downstream_latency_counter,
                                              tiger_burst_3_downstream_readdata,
                                              tiger_burst_3_downstream_readdatavalid,
                                              tiger_burst_3_downstream_reset_n,
                                              tiger_burst_3_downstream_waitrequest
                                           )
;

  output  [ 13: 0] tiger_burst_3_downstream_address_to_slave;
  output           tiger_burst_3_downstream_latency_counter;
  output  [ 31: 0] tiger_burst_3_downstream_readdata;
  output           tiger_burst_3_downstream_readdatavalid;
  output           tiger_burst_3_downstream_reset_n;
  output           tiger_burst_3_downstream_waitrequest;
  input            clk;
  input            d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;
  input            pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa;
  input            reset_n;
  input   [ 13: 0] tiger_burst_3_downstream_address;
  input            tiger_burst_3_downstream_burstcount;
  input   [  3: 0] tiger_burst_3_downstream_byteenable;
  input            tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_burst_3_downstream_read;
  input            tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  input            tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1;
  input            tiger_burst_3_downstream_write;
  input   [ 31: 0] tiger_burst_3_downstream_writedata;

  reg              active_and_waiting_last_time;
  wire             pre_flush_tiger_burst_3_downstream_readdatavalid;
  wire             r_0;
  reg     [ 13: 0] tiger_burst_3_downstream_address_last_time;
  wire    [ 13: 0] tiger_burst_3_downstream_address_to_slave;
  reg              tiger_burst_3_downstream_burstcount_last_time;
  reg     [  3: 0] tiger_burst_3_downstream_byteenable_last_time;
  wire             tiger_burst_3_downstream_latency_counter;
  reg              tiger_burst_3_downstream_read_last_time;
  wire    [ 31: 0] tiger_burst_3_downstream_readdata;
  wire             tiger_burst_3_downstream_readdatavalid;
  wire             tiger_burst_3_downstream_reset_n;
  wire             tiger_burst_3_downstream_run;
  wire             tiger_burst_3_downstream_waitrequest;
  reg              tiger_burst_3_downstream_write_last_time;
  reg     [ 31: 0] tiger_burst_3_downstream_writedata_last_time;
  //r_0 master_run cascaded wait assignment, which is an e_assign
  assign r_0 = 1 & (tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1 | ~tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1) & ((~tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1 | ~(tiger_burst_3_downstream_read | tiger_burst_3_downstream_write) | (1 & ~pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa & (tiger_burst_3_downstream_read | tiger_burst_3_downstream_write)))) & ((~tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1 | ~(tiger_burst_3_downstream_read | tiger_burst_3_downstream_write) | (1 & ~pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa & (tiger_burst_3_downstream_read | tiger_burst_3_downstream_write))));

  //cascaded wait assignment, which is an e_assign
  assign tiger_burst_3_downstream_run = r_0;

  //optimize select-logic by passing only those address bits which matter.
  assign tiger_burst_3_downstream_address_to_slave = tiger_burst_3_downstream_address;

  //latent slave read data valids which may be flushed, which is an e_mux
  assign pre_flush_tiger_burst_3_downstream_readdatavalid = tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1;

  //latent slave read data valid which is not flushed, which is an e_mux
  assign tiger_burst_3_downstream_readdatavalid = 0 |
    pre_flush_tiger_burst_3_downstream_readdatavalid;

  //tiger_burst_3/downstream readdata mux, which is an e_mux
  assign tiger_burst_3_downstream_readdata = pipeline_bridge_PERIPHERALS_s1_readdata_from_sa;

  //actual waitrequest port, which is an e_assign
  assign tiger_burst_3_downstream_waitrequest = ~tiger_burst_3_downstream_run;

  //latent max counter, which is an e_assign
  assign tiger_burst_3_downstream_latency_counter = 0;

  //tiger_burst_3_downstream_reset_n assignment, which is an e_assign
  assign tiger_burst_3_downstream_reset_n = reset_n;


//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tiger_burst_3_downstream_address check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_downstream_address_last_time <= 0;
      else 
        tiger_burst_3_downstream_address_last_time <= tiger_burst_3_downstream_address;
    end


  //tiger_burst_3/downstream waited last time, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          active_and_waiting_last_time <= 0;
      else 
        active_and_waiting_last_time <= tiger_burst_3_downstream_waitrequest & (tiger_burst_3_downstream_read | tiger_burst_3_downstream_write);
    end


  //tiger_burst_3_downstream_address matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_3_downstream_address != tiger_burst_3_downstream_address_last_time))
        begin
          $write("%0d ns: tiger_burst_3_downstream_address did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_3_downstream_burstcount check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_downstream_burstcount_last_time <= 0;
      else 
        tiger_burst_3_downstream_burstcount_last_time <= tiger_burst_3_downstream_burstcount;
    end


  //tiger_burst_3_downstream_burstcount matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_3_downstream_burstcount != tiger_burst_3_downstream_burstcount_last_time))
        begin
          $write("%0d ns: tiger_burst_3_downstream_burstcount did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_3_downstream_byteenable check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_downstream_byteenable_last_time <= 0;
      else 
        tiger_burst_3_downstream_byteenable_last_time <= tiger_burst_3_downstream_byteenable;
    end


  //tiger_burst_3_downstream_byteenable matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_3_downstream_byteenable != tiger_burst_3_downstream_byteenable_last_time))
        begin
          $write("%0d ns: tiger_burst_3_downstream_byteenable did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_3_downstream_read check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_downstream_read_last_time <= 0;
      else 
        tiger_burst_3_downstream_read_last_time <= tiger_burst_3_downstream_read;
    end


  //tiger_burst_3_downstream_read matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_3_downstream_read != tiger_burst_3_downstream_read_last_time))
        begin
          $write("%0d ns: tiger_burst_3_downstream_read did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_3_downstream_write check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_downstream_write_last_time <= 0;
      else 
        tiger_burst_3_downstream_write_last_time <= tiger_burst_3_downstream_write;
    end


  //tiger_burst_3_downstream_write matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_3_downstream_write != tiger_burst_3_downstream_write_last_time))
        begin
          $write("%0d ns: tiger_burst_3_downstream_write did not heed wait!!!", $time);
          $stop;
        end
    end


  //tiger_burst_3_downstream_writedata check against wait, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tiger_burst_3_downstream_writedata_last_time <= 0;
      else 
        tiger_burst_3_downstream_writedata_last_time <= tiger_burst_3_downstream_writedata;
    end


  //tiger_burst_3_downstream_writedata matches last port_name, which is an e_process
  always @(posedge clk)
    begin
      if (active_and_waiting_last_time & (tiger_burst_3_downstream_writedata != tiger_burst_3_downstream_writedata_last_time) & tiger_burst_3_downstream_write)
        begin
          $write("%0d ns: tiger_burst_3_downstream_writedata did not heed wait!!!", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_top_0_CachetoTiger_arbitrator (
                                             // inputs:
                                              clk,
                                              data_cache_0_CachetoTiger_data,
                                              reset_n,

                                             // outputs:
                                              tiger_top_0_CachetoTiger_data
                                           )
;

  output  [ 39: 0] tiger_top_0_CachetoTiger_data;
  input            clk;
  input   [ 39: 0] data_cache_0_CachetoTiger_data;
  input            reset_n;

  wire    [ 39: 0] tiger_top_0_CachetoTiger_data;
  //mux tiger_top_0_CachetoTiger_data, which is an e_mux
  assign tiger_top_0_CachetoTiger_data = data_cache_0_CachetoTiger_data;


endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_top_0_TigertoCache_arbitrator (
                                             // inputs:
                                              clk,
                                              reset_n,
                                              tiger_top_0_TigertoCache_data,

                                             // outputs:
                                              tiger_top_0_TigertoCache_reset
                                           )
;

  output           tiger_top_0_TigertoCache_reset;
  input            clk;
  input            reset_n;
  input   [ 71: 0] tiger_top_0_TigertoCache_data;

  wire             tiger_top_0_TigertoCache_reset;
  //~tiger_top_0_TigertoCache_reset assignment, which is an e_assign
  assign tiger_top_0_TigertoCache_reset = ~reset_n;


endmodule



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
  input   [  2: 0] tiger_top_0_instructionMaster_burstcount;
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
  reg     [  2: 0] tiger_top_0_instructionMaster_burstcount_last_time;
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
  assign tiger_top_0_instructionMaster_address_to_slave = {7'b0,
    tiger_top_0_instructionMaster_address[24 : 0]};

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



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tigers_jtag_uart_controlSlave_arbitrator (
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
                                                   tigers_jtag_uart_controlSlave_readdata,

                                                  // outputs:
                                                   d1_tigers_jtag_uart_controlSlave_end_xfer,
                                                   pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave,
                                                   pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave,
                                                   pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave,
                                                   pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave,
                                                   tigers_jtag_uart_controlSlave_address,
                                                   tigers_jtag_uart_controlSlave_read,
                                                   tigers_jtag_uart_controlSlave_readdata_from_sa,
                                                   tigers_jtag_uart_controlSlave_reset_n,
                                                   tigers_jtag_uart_controlSlave_write,
                                                   tigers_jtag_uart_controlSlave_writedata
                                                )
;

  output           d1_tigers_jtag_uart_controlSlave_end_xfer;
  output           pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave;
  output           pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave;
  output           pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave;
  output           pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave;
  output           tigers_jtag_uart_controlSlave_address;
  output           tigers_jtag_uart_controlSlave_read;
  output  [ 31: 0] tigers_jtag_uart_controlSlave_readdata_from_sa;
  output           tigers_jtag_uart_controlSlave_reset_n;
  output           tigers_jtag_uart_controlSlave_write;
  output  [ 31: 0] tigers_jtag_uart_controlSlave_writedata;
  input            clk;
  input   [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  input            pipeline_bridge_PERIPHERALS_m1_burstcount;
  input            pipeline_bridge_PERIPHERALS_m1_chipselect;
  input            pipeline_bridge_PERIPHERALS_m1_latency_counter;
  input            pipeline_bridge_PERIPHERALS_m1_read;
  input            pipeline_bridge_PERIPHERALS_m1_write;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata;
  input            reset_n;
  input   [ 31: 0] tigers_jtag_uart_controlSlave_readdata;

  reg              d1_reasons_to_wait;
  reg              d1_tigers_jtag_uart_controlSlave_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_tigers_jtag_uart_controlSlave;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             pipeline_bridge_PERIPHERALS_m1_arbiterlock;
  wire             pipeline_bridge_PERIPHERALS_m1_arbiterlock2;
  wire             pipeline_bridge_PERIPHERALS_m1_continuerequest;
  wire             pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_saved_grant_tigers_jtag_uart_controlSlave;
  wire    [ 13: 0] shifted_address_to_tigers_jtag_uart_controlSlave_from_pipeline_bridge_PERIPHERALS_m1;
  wire             tigers_jtag_uart_controlSlave_address;
  wire             tigers_jtag_uart_controlSlave_allgrants;
  wire             tigers_jtag_uart_controlSlave_allow_new_arb_cycle;
  wire             tigers_jtag_uart_controlSlave_any_bursting_master_saved_grant;
  wire             tigers_jtag_uart_controlSlave_any_continuerequest;
  wire             tigers_jtag_uart_controlSlave_arb_counter_enable;
  reg              tigers_jtag_uart_controlSlave_arb_share_counter;
  wire             tigers_jtag_uart_controlSlave_arb_share_counter_next_value;
  wire             tigers_jtag_uart_controlSlave_arb_share_set_values;
  wire             tigers_jtag_uart_controlSlave_beginbursttransfer_internal;
  wire             tigers_jtag_uart_controlSlave_begins_xfer;
  wire             tigers_jtag_uart_controlSlave_end_xfer;
  wire             tigers_jtag_uart_controlSlave_firsttransfer;
  wire             tigers_jtag_uart_controlSlave_grant_vector;
  wire             tigers_jtag_uart_controlSlave_in_a_read_cycle;
  wire             tigers_jtag_uart_controlSlave_in_a_write_cycle;
  wire             tigers_jtag_uart_controlSlave_master_qreq_vector;
  wire             tigers_jtag_uart_controlSlave_non_bursting_master_requests;
  wire             tigers_jtag_uart_controlSlave_read;
  wire    [ 31: 0] tigers_jtag_uart_controlSlave_readdata_from_sa;
  reg              tigers_jtag_uart_controlSlave_reg_firsttransfer;
  wire             tigers_jtag_uart_controlSlave_reset_n;
  reg              tigers_jtag_uart_controlSlave_slavearbiterlockenable;
  wire             tigers_jtag_uart_controlSlave_slavearbiterlockenable2;
  wire             tigers_jtag_uart_controlSlave_unreg_firsttransfer;
  wire             tigers_jtag_uart_controlSlave_waits_for_read;
  wire             tigers_jtag_uart_controlSlave_waits_for_write;
  wire             tigers_jtag_uart_controlSlave_write;
  wire    [ 31: 0] tigers_jtag_uart_controlSlave_writedata;
  wire             wait_for_tigers_jtag_uart_controlSlave_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~tigers_jtag_uart_controlSlave_end_xfer;
    end


  assign tigers_jtag_uart_controlSlave_begins_xfer = ~d1_reasons_to_wait & ((pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave));
  //assign tigers_jtag_uart_controlSlave_readdata_from_sa = tigers_jtag_uart_controlSlave_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tigers_jtag_uart_controlSlave_readdata_from_sa = tigers_jtag_uart_controlSlave_readdata;

  assign pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave = ({pipeline_bridge_PERIPHERALS_m1_address_to_slave[13 : 3] , 3'b0} == 14'h1000) & pipeline_bridge_PERIPHERALS_m1_chipselect;
  //tigers_jtag_uart_controlSlave_arb_share_counter set values, which is an e_mux
  assign tigers_jtag_uart_controlSlave_arb_share_set_values = 1;

  //tigers_jtag_uart_controlSlave_non_bursting_master_requests mux, which is an e_mux
  assign tigers_jtag_uart_controlSlave_non_bursting_master_requests = pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave;

  //tigers_jtag_uart_controlSlave_any_bursting_master_saved_grant mux, which is an e_mux
  assign tigers_jtag_uart_controlSlave_any_bursting_master_saved_grant = 0;

  //tigers_jtag_uart_controlSlave_arb_share_counter_next_value assignment, which is an e_assign
  assign tigers_jtag_uart_controlSlave_arb_share_counter_next_value = tigers_jtag_uart_controlSlave_firsttransfer ? (tigers_jtag_uart_controlSlave_arb_share_set_values - 1) : |tigers_jtag_uart_controlSlave_arb_share_counter ? (tigers_jtag_uart_controlSlave_arb_share_counter - 1) : 0;

  //tigers_jtag_uart_controlSlave_allgrants all slave grants, which is an e_mux
  assign tigers_jtag_uart_controlSlave_allgrants = |tigers_jtag_uart_controlSlave_grant_vector;

  //tigers_jtag_uart_controlSlave_end_xfer assignment, which is an e_assign
  assign tigers_jtag_uart_controlSlave_end_xfer = ~(tigers_jtag_uart_controlSlave_waits_for_read | tigers_jtag_uart_controlSlave_waits_for_write);

  //end_xfer_arb_share_counter_term_tigers_jtag_uart_controlSlave arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_tigers_jtag_uart_controlSlave = tigers_jtag_uart_controlSlave_end_xfer & (~tigers_jtag_uart_controlSlave_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //tigers_jtag_uart_controlSlave_arb_share_counter arbitration counter enable, which is an e_assign
  assign tigers_jtag_uart_controlSlave_arb_counter_enable = (end_xfer_arb_share_counter_term_tigers_jtag_uart_controlSlave & tigers_jtag_uart_controlSlave_allgrants) | (end_xfer_arb_share_counter_term_tigers_jtag_uart_controlSlave & ~tigers_jtag_uart_controlSlave_non_bursting_master_requests);

  //tigers_jtag_uart_controlSlave_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tigers_jtag_uart_controlSlave_arb_share_counter <= 0;
      else if (tigers_jtag_uart_controlSlave_arb_counter_enable)
          tigers_jtag_uart_controlSlave_arb_share_counter <= tigers_jtag_uart_controlSlave_arb_share_counter_next_value;
    end


  //tigers_jtag_uart_controlSlave_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tigers_jtag_uart_controlSlave_slavearbiterlockenable <= 0;
      else if ((|tigers_jtag_uart_controlSlave_master_qreq_vector & end_xfer_arb_share_counter_term_tigers_jtag_uart_controlSlave) | (end_xfer_arb_share_counter_term_tigers_jtag_uart_controlSlave & ~tigers_jtag_uart_controlSlave_non_bursting_master_requests))
          tigers_jtag_uart_controlSlave_slavearbiterlockenable <= |tigers_jtag_uart_controlSlave_arb_share_counter_next_value;
    end


  //pipeline_bridge_PERIPHERALS/m1 tigers_jtag_uart/controlSlave arbiterlock, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_arbiterlock = tigers_jtag_uart_controlSlave_slavearbiterlockenable & pipeline_bridge_PERIPHERALS_m1_continuerequest;

  //tigers_jtag_uart_controlSlave_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign tigers_jtag_uart_controlSlave_slavearbiterlockenable2 = |tigers_jtag_uart_controlSlave_arb_share_counter_next_value;

  //pipeline_bridge_PERIPHERALS/m1 tigers_jtag_uart/controlSlave arbiterlock2, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_arbiterlock2 = tigers_jtag_uart_controlSlave_slavearbiterlockenable2 & pipeline_bridge_PERIPHERALS_m1_continuerequest;

  //tigers_jtag_uart_controlSlave_any_continuerequest at least one master continues requesting, which is an e_assign
  assign tigers_jtag_uart_controlSlave_any_continuerequest = 1;

  //pipeline_bridge_PERIPHERALS_m1_continuerequest continued request, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_continuerequest = 1;

  assign pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave = pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave & ~(((pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect) & ((pipeline_bridge_PERIPHERALS_m1_latency_counter != 0))));
  //local readdatavalid pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect) & ~tigers_jtag_uart_controlSlave_waits_for_read;

  //tigers_jtag_uart_controlSlave_writedata mux, which is an e_mux
  assign tigers_jtag_uart_controlSlave_writedata = pipeline_bridge_PERIPHERALS_m1_writedata;

  //master is always granted when requested
  assign pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave = pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave;

  //pipeline_bridge_PERIPHERALS/m1 saved-grant tigers_jtag_uart/controlSlave, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_saved_grant_tigers_jtag_uart_controlSlave = pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave;

  //allow new arb cycle for tigers_jtag_uart/controlSlave, which is an e_assign
  assign tigers_jtag_uart_controlSlave_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign tigers_jtag_uart_controlSlave_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign tigers_jtag_uart_controlSlave_master_qreq_vector = 1;

  //tigers_jtag_uart_controlSlave_reset_n assignment, which is an e_assign
  assign tigers_jtag_uart_controlSlave_reset_n = reset_n;

  //tigers_jtag_uart_controlSlave_firsttransfer first transaction, which is an e_assign
  assign tigers_jtag_uart_controlSlave_firsttransfer = tigers_jtag_uart_controlSlave_begins_xfer ? tigers_jtag_uart_controlSlave_unreg_firsttransfer : tigers_jtag_uart_controlSlave_reg_firsttransfer;

  //tigers_jtag_uart_controlSlave_unreg_firsttransfer first transaction, which is an e_assign
  assign tigers_jtag_uart_controlSlave_unreg_firsttransfer = ~(tigers_jtag_uart_controlSlave_slavearbiterlockenable & tigers_jtag_uart_controlSlave_any_continuerequest);

  //tigers_jtag_uart_controlSlave_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tigers_jtag_uart_controlSlave_reg_firsttransfer <= 1'b1;
      else if (tigers_jtag_uart_controlSlave_begins_xfer)
          tigers_jtag_uart_controlSlave_reg_firsttransfer <= tigers_jtag_uart_controlSlave_unreg_firsttransfer;
    end


  //tigers_jtag_uart_controlSlave_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign tigers_jtag_uart_controlSlave_beginbursttransfer_internal = tigers_jtag_uart_controlSlave_begins_xfer;

  //tigers_jtag_uart_controlSlave_read assignment, which is an e_mux
  assign tigers_jtag_uart_controlSlave_read = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //tigers_jtag_uart_controlSlave_write assignment, which is an e_mux
  assign tigers_jtag_uart_controlSlave_write = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave & (pipeline_bridge_PERIPHERALS_m1_write & pipeline_bridge_PERIPHERALS_m1_chipselect);

  assign shifted_address_to_tigers_jtag_uart_controlSlave_from_pipeline_bridge_PERIPHERALS_m1 = pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  //tigers_jtag_uart_controlSlave_address mux, which is an e_mux
  assign tigers_jtag_uart_controlSlave_address = shifted_address_to_tigers_jtag_uart_controlSlave_from_pipeline_bridge_PERIPHERALS_m1 >> 2;

  //d1_tigers_jtag_uart_controlSlave_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_tigers_jtag_uart_controlSlave_end_xfer <= 1;
      else 
        d1_tigers_jtag_uart_controlSlave_end_xfer <= tigers_jtag_uart_controlSlave_end_xfer;
    end


  //tigers_jtag_uart_controlSlave_waits_for_read in a cycle, which is an e_mux
  assign tigers_jtag_uart_controlSlave_waits_for_read = tigers_jtag_uart_controlSlave_in_a_read_cycle & 0;

  //tigers_jtag_uart_controlSlave_in_a_read_cycle assignment, which is an e_assign
  assign tigers_jtag_uart_controlSlave_in_a_read_cycle = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = tigers_jtag_uart_controlSlave_in_a_read_cycle;

  //tigers_jtag_uart_controlSlave_waits_for_write in a cycle, which is an e_mux
  assign tigers_jtag_uart_controlSlave_waits_for_write = tigers_jtag_uart_controlSlave_in_a_write_cycle & 0;

  //tigers_jtag_uart_controlSlave_in_a_write_cycle assignment, which is an e_assign
  assign tigers_jtag_uart_controlSlave_in_a_write_cycle = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave & (pipeline_bridge_PERIPHERALS_m1_write & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = tigers_jtag_uart_controlSlave_in_a_write_cycle;

  assign wait_for_tigers_jtag_uart_controlSlave_counter = 0;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tigers_jtag_uart/controlSlave enable non-zero assertions, which is an e_register
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
      if (pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave && (pipeline_bridge_PERIPHERALS_m1_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS/m1 drove 0 on its 'burstcount' port while accessing slave tigers_jtag_uart/controlSlave", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tigers_jtag_uart_1_controlSlave_arbitrator (
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
                                                     tigers_jtag_uart_1_controlSlave_readdata,

                                                    // outputs:
                                                     d1_tigers_jtag_uart_1_controlSlave_end_xfer,
                                                     pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave,
                                                     pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave,
                                                     pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave,
                                                     pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave,
                                                     tigers_jtag_uart_1_controlSlave_address,
                                                     tigers_jtag_uart_1_controlSlave_read,
                                                     tigers_jtag_uart_1_controlSlave_readdata_from_sa,
                                                     tigers_jtag_uart_1_controlSlave_reset_n,
                                                     tigers_jtag_uart_1_controlSlave_write,
                                                     tigers_jtag_uart_1_controlSlave_writedata
                                                  )
;

  output           d1_tigers_jtag_uart_1_controlSlave_end_xfer;
  output           pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave;
  output           pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave;
  output           pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave;
  output           pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave;
  output           tigers_jtag_uart_1_controlSlave_address;
  output           tigers_jtag_uart_1_controlSlave_read;
  output  [ 31: 0] tigers_jtag_uart_1_controlSlave_readdata_from_sa;
  output           tigers_jtag_uart_1_controlSlave_reset_n;
  output           tigers_jtag_uart_1_controlSlave_write;
  output  [ 31: 0] tigers_jtag_uart_1_controlSlave_writedata;
  input            clk;
  input   [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  input            pipeline_bridge_PERIPHERALS_m1_burstcount;
  input            pipeline_bridge_PERIPHERALS_m1_chipselect;
  input            pipeline_bridge_PERIPHERALS_m1_latency_counter;
  input            pipeline_bridge_PERIPHERALS_m1_read;
  input            pipeline_bridge_PERIPHERALS_m1_write;
  input   [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata;
  input            reset_n;
  input   [ 31: 0] tigers_jtag_uart_1_controlSlave_readdata;

  reg              d1_reasons_to_wait;
  reg              d1_tigers_jtag_uart_1_controlSlave_end_xfer;
  reg              enable_nonzero_assertions;
  wire             end_xfer_arb_share_counter_term_tigers_jtag_uart_1_controlSlave;
  wire             in_a_read_cycle;
  wire             in_a_write_cycle;
  wire             pipeline_bridge_PERIPHERALS_m1_arbiterlock;
  wire             pipeline_bridge_PERIPHERALS_m1_arbiterlock2;
  wire             pipeline_bridge_PERIPHERALS_m1_continuerequest;
  wire             pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_saved_grant_tigers_jtag_uart_1_controlSlave;
  wire    [ 13: 0] shifted_address_to_tigers_jtag_uart_1_controlSlave_from_pipeline_bridge_PERIPHERALS_m1;
  wire             tigers_jtag_uart_1_controlSlave_address;
  wire             tigers_jtag_uart_1_controlSlave_allgrants;
  wire             tigers_jtag_uart_1_controlSlave_allow_new_arb_cycle;
  wire             tigers_jtag_uart_1_controlSlave_any_bursting_master_saved_grant;
  wire             tigers_jtag_uart_1_controlSlave_any_continuerequest;
  wire             tigers_jtag_uart_1_controlSlave_arb_counter_enable;
  reg              tigers_jtag_uart_1_controlSlave_arb_share_counter;
  wire             tigers_jtag_uart_1_controlSlave_arb_share_counter_next_value;
  wire             tigers_jtag_uart_1_controlSlave_arb_share_set_values;
  wire             tigers_jtag_uart_1_controlSlave_beginbursttransfer_internal;
  wire             tigers_jtag_uart_1_controlSlave_begins_xfer;
  wire             tigers_jtag_uart_1_controlSlave_end_xfer;
  wire             tigers_jtag_uart_1_controlSlave_firsttransfer;
  wire             tigers_jtag_uart_1_controlSlave_grant_vector;
  wire             tigers_jtag_uart_1_controlSlave_in_a_read_cycle;
  wire             tigers_jtag_uart_1_controlSlave_in_a_write_cycle;
  wire             tigers_jtag_uart_1_controlSlave_master_qreq_vector;
  wire             tigers_jtag_uart_1_controlSlave_non_bursting_master_requests;
  wire             tigers_jtag_uart_1_controlSlave_read;
  wire    [ 31: 0] tigers_jtag_uart_1_controlSlave_readdata_from_sa;
  reg              tigers_jtag_uart_1_controlSlave_reg_firsttransfer;
  wire             tigers_jtag_uart_1_controlSlave_reset_n;
  reg              tigers_jtag_uart_1_controlSlave_slavearbiterlockenable;
  wire             tigers_jtag_uart_1_controlSlave_slavearbiterlockenable2;
  wire             tigers_jtag_uart_1_controlSlave_unreg_firsttransfer;
  wire             tigers_jtag_uart_1_controlSlave_waits_for_read;
  wire             tigers_jtag_uart_1_controlSlave_waits_for_write;
  wire             tigers_jtag_uart_1_controlSlave_write;
  wire    [ 31: 0] tigers_jtag_uart_1_controlSlave_writedata;
  wire             wait_for_tigers_jtag_uart_1_controlSlave_counter;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_reasons_to_wait <= 0;
      else 
        d1_reasons_to_wait <= ~tigers_jtag_uart_1_controlSlave_end_xfer;
    end


  assign tigers_jtag_uart_1_controlSlave_begins_xfer = ~d1_reasons_to_wait & ((pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave));
  //assign tigers_jtag_uart_1_controlSlave_readdata_from_sa = tigers_jtag_uart_1_controlSlave_readdata so that symbol knows where to group signals which may go to master only, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_readdata_from_sa = tigers_jtag_uart_1_controlSlave_readdata;

  assign pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave = ({pipeline_bridge_PERIPHERALS_m1_address_to_slave[13 : 3] , 3'b0} == 14'h2000) & pipeline_bridge_PERIPHERALS_m1_chipselect;
  //tigers_jtag_uart_1_controlSlave_arb_share_counter set values, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_arb_share_set_values = 1;

  //tigers_jtag_uart_1_controlSlave_non_bursting_master_requests mux, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_non_bursting_master_requests = pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave;

  //tigers_jtag_uart_1_controlSlave_any_bursting_master_saved_grant mux, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_any_bursting_master_saved_grant = 0;

  //tigers_jtag_uart_1_controlSlave_arb_share_counter_next_value assignment, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_arb_share_counter_next_value = tigers_jtag_uart_1_controlSlave_firsttransfer ? (tigers_jtag_uart_1_controlSlave_arb_share_set_values - 1) : |tigers_jtag_uart_1_controlSlave_arb_share_counter ? (tigers_jtag_uart_1_controlSlave_arb_share_counter - 1) : 0;

  //tigers_jtag_uart_1_controlSlave_allgrants all slave grants, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_allgrants = |tigers_jtag_uart_1_controlSlave_grant_vector;

  //tigers_jtag_uart_1_controlSlave_end_xfer assignment, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_end_xfer = ~(tigers_jtag_uart_1_controlSlave_waits_for_read | tigers_jtag_uart_1_controlSlave_waits_for_write);

  //end_xfer_arb_share_counter_term_tigers_jtag_uart_1_controlSlave arb share counter enable term, which is an e_assign
  assign end_xfer_arb_share_counter_term_tigers_jtag_uart_1_controlSlave = tigers_jtag_uart_1_controlSlave_end_xfer & (~tigers_jtag_uart_1_controlSlave_any_bursting_master_saved_grant | in_a_read_cycle | in_a_write_cycle);

  //tigers_jtag_uart_1_controlSlave_arb_share_counter arbitration counter enable, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_arb_counter_enable = (end_xfer_arb_share_counter_term_tigers_jtag_uart_1_controlSlave & tigers_jtag_uart_1_controlSlave_allgrants) | (end_xfer_arb_share_counter_term_tigers_jtag_uart_1_controlSlave & ~tigers_jtag_uart_1_controlSlave_non_bursting_master_requests);

  //tigers_jtag_uart_1_controlSlave_arb_share_counter counter, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tigers_jtag_uart_1_controlSlave_arb_share_counter <= 0;
      else if (tigers_jtag_uart_1_controlSlave_arb_counter_enable)
          tigers_jtag_uart_1_controlSlave_arb_share_counter <= tigers_jtag_uart_1_controlSlave_arb_share_counter_next_value;
    end


  //tigers_jtag_uart_1_controlSlave_slavearbiterlockenable slave enables arbiterlock, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tigers_jtag_uart_1_controlSlave_slavearbiterlockenable <= 0;
      else if ((|tigers_jtag_uart_1_controlSlave_master_qreq_vector & end_xfer_arb_share_counter_term_tigers_jtag_uart_1_controlSlave) | (end_xfer_arb_share_counter_term_tigers_jtag_uart_1_controlSlave & ~tigers_jtag_uart_1_controlSlave_non_bursting_master_requests))
          tigers_jtag_uart_1_controlSlave_slavearbiterlockenable <= |tigers_jtag_uart_1_controlSlave_arb_share_counter_next_value;
    end


  //pipeline_bridge_PERIPHERALS/m1 tigers_jtag_uart_1/controlSlave arbiterlock, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_arbiterlock = tigers_jtag_uart_1_controlSlave_slavearbiterlockenable & pipeline_bridge_PERIPHERALS_m1_continuerequest;

  //tigers_jtag_uart_1_controlSlave_slavearbiterlockenable2 slave enables arbiterlock2, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_slavearbiterlockenable2 = |tigers_jtag_uart_1_controlSlave_arb_share_counter_next_value;

  //pipeline_bridge_PERIPHERALS/m1 tigers_jtag_uart_1/controlSlave arbiterlock2, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_arbiterlock2 = tigers_jtag_uart_1_controlSlave_slavearbiterlockenable2 & pipeline_bridge_PERIPHERALS_m1_continuerequest;

  //tigers_jtag_uart_1_controlSlave_any_continuerequest at least one master continues requesting, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_any_continuerequest = 1;

  //pipeline_bridge_PERIPHERALS_m1_continuerequest continued request, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_continuerequest = 1;

  assign pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave = pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave & ~(((pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect) & ((pipeline_bridge_PERIPHERALS_m1_latency_counter != 0))));
  //local readdatavalid pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave, which is an e_mux
  assign pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect) & ~tigers_jtag_uart_1_controlSlave_waits_for_read;

  //tigers_jtag_uart_1_controlSlave_writedata mux, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_writedata = pipeline_bridge_PERIPHERALS_m1_writedata;

  //master is always granted when requested
  assign pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave = pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave;

  //pipeline_bridge_PERIPHERALS/m1 saved-grant tigers_jtag_uart_1/controlSlave, which is an e_assign
  assign pipeline_bridge_PERIPHERALS_m1_saved_grant_tigers_jtag_uart_1_controlSlave = pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave;

  //allow new arb cycle for tigers_jtag_uart_1/controlSlave, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_allow_new_arb_cycle = 1;

  //placeholder chosen master
  assign tigers_jtag_uart_1_controlSlave_grant_vector = 1;

  //placeholder vector of master qualified-requests
  assign tigers_jtag_uart_1_controlSlave_master_qreq_vector = 1;

  //tigers_jtag_uart_1_controlSlave_reset_n assignment, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_reset_n = reset_n;

  //tigers_jtag_uart_1_controlSlave_firsttransfer first transaction, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_firsttransfer = tigers_jtag_uart_1_controlSlave_begins_xfer ? tigers_jtag_uart_1_controlSlave_unreg_firsttransfer : tigers_jtag_uart_1_controlSlave_reg_firsttransfer;

  //tigers_jtag_uart_1_controlSlave_unreg_firsttransfer first transaction, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_unreg_firsttransfer = ~(tigers_jtag_uart_1_controlSlave_slavearbiterlockenable & tigers_jtag_uart_1_controlSlave_any_continuerequest);

  //tigers_jtag_uart_1_controlSlave_reg_firsttransfer first transaction, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          tigers_jtag_uart_1_controlSlave_reg_firsttransfer <= 1'b1;
      else if (tigers_jtag_uart_1_controlSlave_begins_xfer)
          tigers_jtag_uart_1_controlSlave_reg_firsttransfer <= tigers_jtag_uart_1_controlSlave_unreg_firsttransfer;
    end


  //tigers_jtag_uart_1_controlSlave_beginbursttransfer_internal begin burst transfer, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_beginbursttransfer_internal = tigers_jtag_uart_1_controlSlave_begins_xfer;

  //tigers_jtag_uart_1_controlSlave_read assignment, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_read = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //tigers_jtag_uart_1_controlSlave_write assignment, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_write = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave & (pipeline_bridge_PERIPHERALS_m1_write & pipeline_bridge_PERIPHERALS_m1_chipselect);

  assign shifted_address_to_tigers_jtag_uart_1_controlSlave_from_pipeline_bridge_PERIPHERALS_m1 = pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  //tigers_jtag_uart_1_controlSlave_address mux, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_address = shifted_address_to_tigers_jtag_uart_1_controlSlave_from_pipeline_bridge_PERIPHERALS_m1 >> 2;

  //d1_tigers_jtag_uart_1_controlSlave_end_xfer register, which is an e_register
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          d1_tigers_jtag_uart_1_controlSlave_end_xfer <= 1;
      else 
        d1_tigers_jtag_uart_1_controlSlave_end_xfer <= tigers_jtag_uart_1_controlSlave_end_xfer;
    end


  //tigers_jtag_uart_1_controlSlave_waits_for_read in a cycle, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_waits_for_read = tigers_jtag_uart_1_controlSlave_in_a_read_cycle & 0;

  //tigers_jtag_uart_1_controlSlave_in_a_read_cycle assignment, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_in_a_read_cycle = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave & (pipeline_bridge_PERIPHERALS_m1_read & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //in_a_read_cycle assignment, which is an e_mux
  assign in_a_read_cycle = tigers_jtag_uart_1_controlSlave_in_a_read_cycle;

  //tigers_jtag_uart_1_controlSlave_waits_for_write in a cycle, which is an e_mux
  assign tigers_jtag_uart_1_controlSlave_waits_for_write = tigers_jtag_uart_1_controlSlave_in_a_write_cycle & 0;

  //tigers_jtag_uart_1_controlSlave_in_a_write_cycle assignment, which is an e_assign
  assign tigers_jtag_uart_1_controlSlave_in_a_write_cycle = pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave & (pipeline_bridge_PERIPHERALS_m1_write & pipeline_bridge_PERIPHERALS_m1_chipselect);

  //in_a_write_cycle assignment, which is an e_mux
  assign in_a_write_cycle = tigers_jtag_uart_1_controlSlave_in_a_write_cycle;

  assign wait_for_tigers_jtag_uart_1_controlSlave_counter = 0;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  //tigers_jtag_uart_1/controlSlave enable non-zero assertions, which is an e_register
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
      if (pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave && (pipeline_bridge_PERIPHERALS_m1_burstcount == 0) && enable_nonzero_assertions)
        begin
          $write("%0d ns: pipeline_bridge_PERIPHERALS/m1 drove 0 on its 'burstcount' port while accessing slave tigers_jtag_uart_1/controlSlave", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule



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
  input   [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
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
  wire    [ 13: 0] shifted_address_to_uart_0_s1_from_pipeline_bridge_PERIPHERALS_m1;
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

  assign pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1 = ({pipeline_bridge_PERIPHERALS_m1_address_to_slave[13 : 5] , 5'b0} == 14'h880) & pipeline_bridge_PERIPHERALS_m1_chipselect;
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

               // the_uart_0
                rxd_to_the_uart_0,
                txd_from_the_uart_0
             )
;

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
  input            reset_n;
  input            rxd_to_the_uart_0;

  wire             clk_reset_n;
  wire             d1_data_cache_0_ACCEL_end_xfer;
  wire             d1_onchip_mem_s1_end_xfer;
  wire             d1_pipeline_bridge_MEMORY_s1_end_xfer;
  wire             d1_pipeline_bridge_PERIPHERALS_s1_end_xfer;
  wire             d1_sdram_s1_end_xfer;
  wire             d1_tiger_burst_0_upstream_end_xfer;
  wire             d1_tiger_burst_1_upstream_end_xfer;
  wire             d1_tiger_burst_2_upstream_end_xfer;
  wire             d1_tiger_burst_3_upstream_end_xfer;
  wire             d1_tigers_jtag_uart_1_controlSlave_end_xfer;
  wire             d1_tigers_jtag_uart_controlSlave_end_xfer;
  wire             d1_uart_0_s1_end_xfer;
  wire    [  2: 0] data_cache_0_ACCEL_address;
  wire             data_cache_0_ACCEL_begintransfer;
  wire             data_cache_0_ACCEL_read;
  wire    [127: 0] data_cache_0_ACCEL_readdata;
  wire    [127: 0] data_cache_0_ACCEL_readdata_from_sa;
  wire             data_cache_0_ACCEL_waitrequest;
  wire             data_cache_0_ACCEL_waitrequest_from_sa;
  wire             data_cache_0_ACCEL_write;
  wire    [127: 0] data_cache_0_ACCEL_writedata;
  wire    [ 31: 0] data_cache_0_AccelMaster_address;
  wire    [ 31: 0] data_cache_0_AccelMaster_address_to_slave;
  wire             data_cache_0_AccelMaster_beginbursttransfer;
  wire    [  2: 0] data_cache_0_AccelMaster_burstcount;
  wire    [  3: 0] data_cache_0_AccelMaster_byteenable;
  wire             data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_latency_counter;
  wire             data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_read;
  wire             data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire    [ 31: 0] data_cache_0_AccelMaster_readdata;
  wire             data_cache_0_AccelMaster_readdatavalid;
  wire             data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_AccelMaster_waitrequest;
  wire             data_cache_0_AccelMaster_write;
  wire    [ 31: 0] data_cache_0_AccelMaster_writedata;
  wire    [ 39: 0] data_cache_0_CachetoTiger_data;
  wire    [ 71: 0] data_cache_0_TigertoCache_data;
  wire             data_cache_0_TigertoCache_reset_n;
  wire    [ 31: 0] data_cache_0_dataMaster_address;
  wire    [ 31: 0] data_cache_0_dataMaster_address_to_slave;
  wire             data_cache_0_dataMaster_beginbursttransfer;
  wire    [  2: 0] data_cache_0_dataMaster_burstcount;
  wire    [  3: 0] data_cache_0_dataMaster_byteenable;
  wire             data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_granted_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_latency_counter;
  wire             data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_read;
  wire             data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register;
  wire             data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register;
  wire    [ 31: 0] data_cache_0_dataMaster_readdata;
  wire             data_cache_0_dataMaster_readdatavalid;
  wire             data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1;
  wire             data_cache_0_dataMaster_requests_tiger_burst_3_upstream;
  wire             data_cache_0_dataMaster_waitrequest;
  wire             data_cache_0_dataMaster_write;
  wire    [ 31: 0] data_cache_0_dataMaster_writedata;
  wire    [ 10: 0] onchip_mem_s1_address;
  wire    [  3: 0] onchip_mem_s1_byteenable;
  wire             onchip_mem_s1_chipselect;
  wire             onchip_mem_s1_clken;
  wire             onchip_mem_s1_debugaccess;
  wire    [ 31: 0] onchip_mem_s1_readdata;
  wire    [ 31: 0] onchip_mem_s1_readdata_from_sa;
  wire             onchip_mem_s1_reset;
  wire             onchip_mem_s1_write;
  wire    [ 31: 0] onchip_mem_s1_writedata;
  wire    [ 24: 0] pipeline_bridge_MEMORY_m1_address;
  wire    [ 24: 0] pipeline_bridge_MEMORY_m1_address_to_slave;
  wire    [  2: 0] pipeline_bridge_MEMORY_m1_burstcount;
  wire    [  3: 0] pipeline_bridge_MEMORY_m1_byteenable;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_chipselect;
  wire    [  1: 0] pipeline_bridge_MEMORY_m1_dbs_address;
  wire    [ 15: 0] pipeline_bridge_MEMORY_m1_dbs_write_16;
  wire             pipeline_bridge_MEMORY_m1_debugaccess;
  wire             pipeline_bridge_MEMORY_m1_endofpacket;
  wire             pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_latency_counter;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_read;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register;
  wire    [ 31: 0] pipeline_bridge_MEMORY_m1_readdata;
  wire             pipeline_bridge_MEMORY_m1_readdatavalid;
  wire             pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream;
  wire             pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream;
  wire             pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream;
  wire             pipeline_bridge_MEMORY_m1_waitrequest;
  wire             pipeline_bridge_MEMORY_m1_write;
  wire    [ 31: 0] pipeline_bridge_MEMORY_m1_writedata;
  wire    [ 22: 0] pipeline_bridge_MEMORY_s1_address;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock;
  wire             pipeline_bridge_MEMORY_s1_arbiterlock2;
  wire    [  2: 0] pipeline_bridge_MEMORY_s1_burstcount;
  wire    [  3: 0] pipeline_bridge_MEMORY_s1_byteenable;
  wire             pipeline_bridge_MEMORY_s1_chipselect;
  wire             pipeline_bridge_MEMORY_s1_debugaccess;
  wire             pipeline_bridge_MEMORY_s1_endofpacket;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire    [ 22: 0] pipeline_bridge_MEMORY_s1_nativeaddress;
  wire             pipeline_bridge_MEMORY_s1_read;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_readdata;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_readdata_from_sa;
  wire             pipeline_bridge_MEMORY_s1_readdatavalid;
  wire             pipeline_bridge_MEMORY_s1_reset_n;
  wire             pipeline_bridge_MEMORY_s1_waitrequest;
  wire             pipeline_bridge_MEMORY_s1_waitrequest_from_sa;
  wire             pipeline_bridge_MEMORY_s1_write;
  wire    [ 31: 0] pipeline_bridge_MEMORY_s1_writedata;
  wire    [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address;
  wire    [ 13: 0] pipeline_bridge_PERIPHERALS_m1_address_to_slave;
  wire             pipeline_bridge_PERIPHERALS_m1_burstcount;
  wire    [  3: 0] pipeline_bridge_PERIPHERALS_m1_byteenable;
  wire             pipeline_bridge_PERIPHERALS_m1_chipselect;
  wire             pipeline_bridge_PERIPHERALS_m1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_m1_endofpacket;
  wire             pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_latency_counter;
  wire             pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_read;
  wire             pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_m1_readdata;
  wire             pipeline_bridge_PERIPHERALS_m1_readdatavalid;
  wire             pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave;
  wire             pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1;
  wire             pipeline_bridge_PERIPHERALS_m1_waitrequest;
  wire             pipeline_bridge_PERIPHERALS_m1_write;
  wire    [ 31: 0] pipeline_bridge_PERIPHERALS_m1_writedata;
  wire    [ 11: 0] pipeline_bridge_PERIPHERALS_s1_address;
  wire             pipeline_bridge_PERIPHERALS_s1_arbiterlock;
  wire             pipeline_bridge_PERIPHERALS_s1_arbiterlock2;
  wire             pipeline_bridge_PERIPHERALS_s1_burstcount;
  wire    [  3: 0] pipeline_bridge_PERIPHERALS_s1_byteenable;
  wire             pipeline_bridge_PERIPHERALS_s1_chipselect;
  wire             pipeline_bridge_PERIPHERALS_s1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  wire    [ 11: 0] pipeline_bridge_PERIPHERALS_s1_nativeaddress;
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
  wire    [  3: 0] tiger_burst_0_downstream_arbitrationshare;
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
  wire    [  2: 0] tiger_burst_0_upstream_burstcount;
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
  wire    [ 12: 0] tiger_burst_1_downstream_address;
  wire    [ 12: 0] tiger_burst_1_downstream_address_to_slave;
  wire    [  2: 0] tiger_burst_1_downstream_arbitrationshare;
  wire             tiger_burst_1_downstream_burstcount;
  wire    [  3: 0] tiger_burst_1_downstream_byteenable;
  wire             tiger_burst_1_downstream_debugaccess;
  wire             tiger_burst_1_downstream_granted_onchip_mem_s1;
  wire             tiger_burst_1_downstream_latency_counter;
  wire    [ 12: 0] tiger_burst_1_downstream_nativeaddress;
  wire             tiger_burst_1_downstream_qualified_request_onchip_mem_s1;
  wire             tiger_burst_1_downstream_read;
  wire             tiger_burst_1_downstream_read_data_valid_onchip_mem_s1;
  wire    [ 31: 0] tiger_burst_1_downstream_readdata;
  wire             tiger_burst_1_downstream_readdatavalid;
  wire             tiger_burst_1_downstream_requests_onchip_mem_s1;
  wire             tiger_burst_1_downstream_reset_n;
  wire             tiger_burst_1_downstream_waitrequest;
  wire             tiger_burst_1_downstream_write;
  wire    [ 31: 0] tiger_burst_1_downstream_writedata;
  wire    [ 12: 0] tiger_burst_1_upstream_address;
  wire    [  2: 0] tiger_burst_1_upstream_burstcount;
  wire    [ 14: 0] tiger_burst_1_upstream_byteaddress;
  wire    [  3: 0] tiger_burst_1_upstream_byteenable;
  wire             tiger_burst_1_upstream_debugaccess;
  wire             tiger_burst_1_upstream_read;
  wire    [ 31: 0] tiger_burst_1_upstream_readdata;
  wire    [ 31: 0] tiger_burst_1_upstream_readdata_from_sa;
  wire             tiger_burst_1_upstream_readdatavalid;
  wire             tiger_burst_1_upstream_waitrequest;
  wire             tiger_burst_1_upstream_waitrequest_from_sa;
  wire             tiger_burst_1_upstream_write;
  wire    [ 31: 0] tiger_burst_1_upstream_writedata;
  wire    [  6: 0] tiger_burst_2_downstream_address;
  wire    [  6: 0] tiger_burst_2_downstream_address_to_slave;
  wire    [  2: 0] tiger_burst_2_downstream_arbitrationshare;
  wire             tiger_burst_2_downstream_burstcount;
  wire    [ 15: 0] tiger_burst_2_downstream_byteenable;
  wire             tiger_burst_2_downstream_debugaccess;
  wire             tiger_burst_2_downstream_granted_data_cache_0_ACCEL;
  wire             tiger_burst_2_downstream_latency_counter;
  wire    [  6: 0] tiger_burst_2_downstream_nativeaddress;
  wire             tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL;
  wire             tiger_burst_2_downstream_read;
  wire             tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL;
  wire    [127: 0] tiger_burst_2_downstream_readdata;
  wire             tiger_burst_2_downstream_readdatavalid;
  wire             tiger_burst_2_downstream_requests_data_cache_0_ACCEL;
  wire             tiger_burst_2_downstream_reset_n;
  wire             tiger_burst_2_downstream_waitrequest;
  wire             tiger_burst_2_downstream_write;
  wire    [127: 0] tiger_burst_2_downstream_writedata;
  wire    [  6: 0] tiger_burst_2_upstream_address;
  wire    [  2: 0] tiger_burst_2_upstream_burstcount;
  wire    [ 10: 0] tiger_burst_2_upstream_byteaddress;
  wire    [ 15: 0] tiger_burst_2_upstream_byteenable;
  wire             tiger_burst_2_upstream_debugaccess;
  wire             tiger_burst_2_upstream_read;
  wire    [127: 0] tiger_burst_2_upstream_readdata;
  wire    [127: 0] tiger_burst_2_upstream_readdata_from_sa;
  wire             tiger_burst_2_upstream_readdatavalid;
  wire             tiger_burst_2_upstream_waitrequest;
  wire             tiger_burst_2_upstream_waitrequest_from_sa;
  wire             tiger_burst_2_upstream_write;
  wire    [127: 0] tiger_burst_2_upstream_writedata;
  wire    [ 13: 0] tiger_burst_3_downstream_address;
  wire    [ 13: 0] tiger_burst_3_downstream_address_to_slave;
  wire    [  2: 0] tiger_burst_3_downstream_arbitrationshare;
  wire             tiger_burst_3_downstream_burstcount;
  wire    [  3: 0] tiger_burst_3_downstream_byteenable;
  wire             tiger_burst_3_downstream_debugaccess;
  wire             tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_latency_counter;
  wire    [ 13: 0] tiger_burst_3_downstream_nativeaddress;
  wire             tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_read;
  wire             tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register;
  wire    [ 31: 0] tiger_burst_3_downstream_readdata;
  wire             tiger_burst_3_downstream_readdatavalid;
  wire             tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1;
  wire             tiger_burst_3_downstream_reset_n;
  wire             tiger_burst_3_downstream_waitrequest;
  wire             tiger_burst_3_downstream_write;
  wire    [ 31: 0] tiger_burst_3_downstream_writedata;
  wire    [ 13: 0] tiger_burst_3_upstream_address;
  wire    [  2: 0] tiger_burst_3_upstream_burstcount;
  wire    [ 15: 0] tiger_burst_3_upstream_byteaddress;
  wire    [  3: 0] tiger_burst_3_upstream_byteenable;
  wire             tiger_burst_3_upstream_debugaccess;
  wire             tiger_burst_3_upstream_read;
  wire    [ 31: 0] tiger_burst_3_upstream_readdata;
  wire    [ 31: 0] tiger_burst_3_upstream_readdata_from_sa;
  wire             tiger_burst_3_upstream_readdatavalid;
  wire             tiger_burst_3_upstream_waitrequest;
  wire             tiger_burst_3_upstream_waitrequest_from_sa;
  wire             tiger_burst_3_upstream_write;
  wire    [ 31: 0] tiger_burst_3_upstream_writedata;
  wire    [ 39: 0] tiger_top_0_CachetoTiger_data;
  wire    [ 71: 0] tiger_top_0_TigertoCache_data;
  wire             tiger_top_0_TigertoCache_reset;
  wire    [ 31: 0] tiger_top_0_instructionMaster_address;
  wire    [ 31: 0] tiger_top_0_instructionMaster_address_to_slave;
  wire             tiger_top_0_instructionMaster_beginbursttransfer;
  wire    [  2: 0] tiger_top_0_instructionMaster_burstcount;
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
  wire             tigers_jtag_uart_1_controlSlave_address;
  wire             tigers_jtag_uart_1_controlSlave_read;
  wire    [ 31: 0] tigers_jtag_uart_1_controlSlave_readdata;
  wire    [ 31: 0] tigers_jtag_uart_1_controlSlave_readdata_from_sa;
  wire             tigers_jtag_uart_1_controlSlave_reset_n;
  wire             tigers_jtag_uart_1_controlSlave_write;
  wire    [ 31: 0] tigers_jtag_uart_1_controlSlave_writedata;
  wire             tigers_jtag_uart_controlSlave_address;
  wire             tigers_jtag_uart_controlSlave_read;
  wire    [ 31: 0] tigers_jtag_uart_controlSlave_readdata;
  wire    [ 31: 0] tigers_jtag_uart_controlSlave_readdata_from_sa;
  wire             tigers_jtag_uart_controlSlave_reset_n;
  wire             tigers_jtag_uart_controlSlave_write;
  wire    [ 31: 0] tigers_jtag_uart_controlSlave_writedata;
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
  data_cache_0_ACCEL_arbitrator the_data_cache_0_ACCEL
    (
      .clk                                                           (clk),
      .d1_data_cache_0_ACCEL_end_xfer                                (d1_data_cache_0_ACCEL_end_xfer),
      .data_cache_0_ACCEL_address                                    (data_cache_0_ACCEL_address),
      .data_cache_0_ACCEL_begintransfer                              (data_cache_0_ACCEL_begintransfer),
      .data_cache_0_ACCEL_read                                       (data_cache_0_ACCEL_read),
      .data_cache_0_ACCEL_readdata                                   (data_cache_0_ACCEL_readdata),
      .data_cache_0_ACCEL_readdata_from_sa                           (data_cache_0_ACCEL_readdata_from_sa),
      .data_cache_0_ACCEL_waitrequest                                (data_cache_0_ACCEL_waitrequest),
      .data_cache_0_ACCEL_waitrequest_from_sa                        (data_cache_0_ACCEL_waitrequest_from_sa),
      .data_cache_0_ACCEL_write                                      (data_cache_0_ACCEL_write),
      .data_cache_0_ACCEL_writedata                                  (data_cache_0_ACCEL_writedata),
      .reset_n                                                       (clk_reset_n),
      .tiger_burst_2_downstream_address_to_slave                     (tiger_burst_2_downstream_address_to_slave),
      .tiger_burst_2_downstream_arbitrationshare                     (tiger_burst_2_downstream_arbitrationshare),
      .tiger_burst_2_downstream_burstcount                           (tiger_burst_2_downstream_burstcount),
      .tiger_burst_2_downstream_granted_data_cache_0_ACCEL           (tiger_burst_2_downstream_granted_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_latency_counter                      (tiger_burst_2_downstream_latency_counter),
      .tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL (tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_read                                 (tiger_burst_2_downstream_read),
      .tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL   (tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_requests_data_cache_0_ACCEL          (tiger_burst_2_downstream_requests_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_write                                (tiger_burst_2_downstream_write),
      .tiger_burst_2_downstream_writedata                            (tiger_burst_2_downstream_writedata)
    );

  data_cache_0_TigertoCache_arbitrator the_data_cache_0_TigertoCache
    (
      .clk                               (clk),
      .data_cache_0_TigertoCache_data    (data_cache_0_TigertoCache_data),
      .data_cache_0_TigertoCache_reset_n (data_cache_0_TigertoCache_reset_n),
      .reset_n                           (clk_reset_n),
      .tiger_top_0_TigertoCache_data     (tiger_top_0_TigertoCache_data)
    );

  data_cache_0_AccelMaster_arbitrator the_data_cache_0_AccelMaster
    (
      .clk                                                                               (clk),
      .d1_pipeline_bridge_MEMORY_s1_end_xfer                                             (d1_pipeline_bridge_MEMORY_s1_end_xfer),
      .data_cache_0_AccelMaster_address                                                  (data_cache_0_AccelMaster_address),
      .data_cache_0_AccelMaster_address_to_slave                                         (data_cache_0_AccelMaster_address_to_slave),
      .data_cache_0_AccelMaster_burstcount                                               (data_cache_0_AccelMaster_burstcount),
      .data_cache_0_AccelMaster_byteenable                                               (data_cache_0_AccelMaster_byteenable),
      .data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1                        (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_latency_counter                                          (data_cache_0_AccelMaster_latency_counter),
      .data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1              (data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_read                                                     (data_cache_0_AccelMaster_read),
      .data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1                (data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register (data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .data_cache_0_AccelMaster_readdata                                                 (data_cache_0_AccelMaster_readdata),
      .data_cache_0_AccelMaster_readdatavalid                                            (data_cache_0_AccelMaster_readdatavalid),
      .data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1                       (data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_waitrequest                                              (data_cache_0_AccelMaster_waitrequest),
      .data_cache_0_AccelMaster_write                                                    (data_cache_0_AccelMaster_write),
      .data_cache_0_AccelMaster_writedata                                                (data_cache_0_AccelMaster_writedata),
      .pipeline_bridge_MEMORY_s1_readdata_from_sa                                        (pipeline_bridge_MEMORY_s1_readdata_from_sa),
      .pipeline_bridge_MEMORY_s1_waitrequest_from_sa                                     (pipeline_bridge_MEMORY_s1_waitrequest_from_sa),
      .reset_n                                                                           (clk_reset_n)
    );

  data_cache_0_CachetoTiger_arbitrator the_data_cache_0_CachetoTiger
    (
      .clk                            (clk),
      .data_cache_0_CachetoTiger_data (data_cache_0_CachetoTiger_data),
      .reset_n                        (clk_reset_n)
    );

  data_cache_0_dataMaster_arbitrator the_data_cache_0_dataMaster
    (
      .clk                                                                              (clk),
      .d1_pipeline_bridge_MEMORY_s1_end_xfer                                            (d1_pipeline_bridge_MEMORY_s1_end_xfer),
      .d1_tiger_burst_3_upstream_end_xfer                                               (d1_tiger_burst_3_upstream_end_xfer),
      .data_cache_0_dataMaster_address                                                  (data_cache_0_dataMaster_address),
      .data_cache_0_dataMaster_address_to_slave                                         (data_cache_0_dataMaster_address_to_slave),
      .data_cache_0_dataMaster_burstcount                                               (data_cache_0_dataMaster_burstcount),
      .data_cache_0_dataMaster_byteenable                                               (data_cache_0_dataMaster_byteenable),
      .data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1                        (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_granted_tiger_burst_3_upstream                           (data_cache_0_dataMaster_granted_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_latency_counter                                          (data_cache_0_dataMaster_latency_counter),
      .data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1              (data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream                 (data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_read                                                     (data_cache_0_dataMaster_read),
      .data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1                (data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register (data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream                   (data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register    (data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register),
      .data_cache_0_dataMaster_readdata                                                 (data_cache_0_dataMaster_readdata),
      .data_cache_0_dataMaster_readdatavalid                                            (data_cache_0_dataMaster_readdatavalid),
      .data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1                       (data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_requests_tiger_burst_3_upstream                          (data_cache_0_dataMaster_requests_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_waitrequest                                              (data_cache_0_dataMaster_waitrequest),
      .data_cache_0_dataMaster_write                                                    (data_cache_0_dataMaster_write),
      .data_cache_0_dataMaster_writedata                                                (data_cache_0_dataMaster_writedata),
      .pipeline_bridge_MEMORY_s1_readdata_from_sa                                       (pipeline_bridge_MEMORY_s1_readdata_from_sa),
      .pipeline_bridge_MEMORY_s1_waitrequest_from_sa                                    (pipeline_bridge_MEMORY_s1_waitrequest_from_sa),
      .reset_n                                                                          (clk_reset_n),
      .tiger_burst_3_upstream_readdata_from_sa                                          (tiger_burst_3_upstream_readdata_from_sa),
      .tiger_burst_3_upstream_waitrequest_from_sa                                       (tiger_burst_3_upstream_waitrequest_from_sa)
    );

  data_cache_0 the_data_cache_0
    (
      .asi_TigertoCache_data              (data_cache_0_TigertoCache_data),
      .aso_CachetoTiger_data              (data_cache_0_CachetoTiger_data),
      .avm_AccelMaster_address            (data_cache_0_AccelMaster_address),
      .avm_AccelMaster_beginbursttransfer (data_cache_0_AccelMaster_beginbursttransfer),
      .avm_AccelMaster_burstcount         (data_cache_0_AccelMaster_burstcount),
      .avm_AccelMaster_byteenable         (data_cache_0_AccelMaster_byteenable),
      .avm_AccelMaster_read               (data_cache_0_AccelMaster_read),
      .avm_AccelMaster_readdata           (data_cache_0_AccelMaster_readdata),
      .avm_AccelMaster_readdatavalid      (data_cache_0_AccelMaster_readdatavalid),
      .avm_AccelMaster_waitrequest        (data_cache_0_AccelMaster_waitrequest),
      .avm_AccelMaster_write              (data_cache_0_AccelMaster_write),
      .avm_AccelMaster_writedata          (data_cache_0_AccelMaster_writedata),
      .avm_dataMaster_address             (data_cache_0_dataMaster_address),
      .avm_dataMaster_beginbursttransfer  (data_cache_0_dataMaster_beginbursttransfer),
      .avm_dataMaster_burstcount          (data_cache_0_dataMaster_burstcount),
      .avm_dataMaster_byteenable          (data_cache_0_dataMaster_byteenable),
      .avm_dataMaster_read                (data_cache_0_dataMaster_read),
      .avm_dataMaster_readdata            (data_cache_0_dataMaster_readdata),
      .avm_dataMaster_readdatavalid       (data_cache_0_dataMaster_readdatavalid),
      .avm_dataMaster_waitrequest         (data_cache_0_dataMaster_waitrequest),
      .avm_dataMaster_write               (data_cache_0_dataMaster_write),
      .avm_dataMaster_writedata           (data_cache_0_dataMaster_writedata),
      .avs_ACCEL_address                  (data_cache_0_ACCEL_address),
      .avs_ACCEL_begintransfer            (data_cache_0_ACCEL_begintransfer),
      .avs_ACCEL_read                     (data_cache_0_ACCEL_read),
      .avs_ACCEL_readdata                 (data_cache_0_ACCEL_readdata),
      .avs_ACCEL_waitrequest              (data_cache_0_ACCEL_waitrequest),
      .avs_ACCEL_write                    (data_cache_0_ACCEL_write),
      .avs_ACCEL_writedata                (data_cache_0_ACCEL_writedata),
      .csi_clockreset_clk                 (clk),
      .csi_clockreset_reset_n             (data_cache_0_TigertoCache_reset_n)
    );

  onchip_mem_s1_arbitrator the_onchip_mem_s1
    (
      .clk                                                      (clk),
      .d1_onchip_mem_s1_end_xfer                                (d1_onchip_mem_s1_end_xfer),
      .onchip_mem_s1_address                                    (onchip_mem_s1_address),
      .onchip_mem_s1_byteenable                                 (onchip_mem_s1_byteenable),
      .onchip_mem_s1_chipselect                                 (onchip_mem_s1_chipselect),
      .onchip_mem_s1_clken                                      (onchip_mem_s1_clken),
      .onchip_mem_s1_debugaccess                                (onchip_mem_s1_debugaccess),
      .onchip_mem_s1_readdata                                   (onchip_mem_s1_readdata),
      .onchip_mem_s1_readdata_from_sa                           (onchip_mem_s1_readdata_from_sa),
      .onchip_mem_s1_reset                                      (onchip_mem_s1_reset),
      .onchip_mem_s1_write                                      (onchip_mem_s1_write),
      .onchip_mem_s1_writedata                                  (onchip_mem_s1_writedata),
      .reset_n                                                  (clk_reset_n),
      .tiger_burst_1_downstream_address_to_slave                (tiger_burst_1_downstream_address_to_slave),
      .tiger_burst_1_downstream_arbitrationshare                (tiger_burst_1_downstream_arbitrationshare),
      .tiger_burst_1_downstream_burstcount                      (tiger_burst_1_downstream_burstcount),
      .tiger_burst_1_downstream_byteenable                      (tiger_burst_1_downstream_byteenable),
      .tiger_burst_1_downstream_debugaccess                     (tiger_burst_1_downstream_debugaccess),
      .tiger_burst_1_downstream_granted_onchip_mem_s1           (tiger_burst_1_downstream_granted_onchip_mem_s1),
      .tiger_burst_1_downstream_latency_counter                 (tiger_burst_1_downstream_latency_counter),
      .tiger_burst_1_downstream_qualified_request_onchip_mem_s1 (tiger_burst_1_downstream_qualified_request_onchip_mem_s1),
      .tiger_burst_1_downstream_read                            (tiger_burst_1_downstream_read),
      .tiger_burst_1_downstream_read_data_valid_onchip_mem_s1   (tiger_burst_1_downstream_read_data_valid_onchip_mem_s1),
      .tiger_burst_1_downstream_requests_onchip_mem_s1          (tiger_burst_1_downstream_requests_onchip_mem_s1),
      .tiger_burst_1_downstream_write                           (tiger_burst_1_downstream_write),
      .tiger_burst_1_downstream_writedata                       (tiger_burst_1_downstream_writedata)
    );

  onchip_mem the_onchip_mem
    (
      .address     (onchip_mem_s1_address),
      .byteenable  (onchip_mem_s1_byteenable),
      .chipselect  (onchip_mem_s1_chipselect),
      .clk         (clk),
      .clken       (onchip_mem_s1_clken),
      .debugaccess (onchip_mem_s1_debugaccess),
      .readdata    (onchip_mem_s1_readdata),
      .reset       (onchip_mem_s1_reset),
      .write       (onchip_mem_s1_write),
      .writedata   (onchip_mem_s1_writedata)
    );

  pipeline_bridge_MEMORY_s1_arbitrator the_pipeline_bridge_MEMORY_s1
    (
      .clk                                                                                    (clk),
      .d1_pipeline_bridge_MEMORY_s1_end_xfer                                                  (d1_pipeline_bridge_MEMORY_s1_end_xfer),
      .data_cache_0_AccelMaster_address_to_slave                                              (data_cache_0_AccelMaster_address_to_slave),
      .data_cache_0_AccelMaster_burstcount                                                    (data_cache_0_AccelMaster_burstcount),
      .data_cache_0_AccelMaster_byteenable                                                    (data_cache_0_AccelMaster_byteenable),
      .data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1                             (data_cache_0_AccelMaster_granted_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_latency_counter                                               (data_cache_0_AccelMaster_latency_counter),
      .data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1                   (data_cache_0_AccelMaster_qualified_request_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_read                                                          (data_cache_0_AccelMaster_read),
      .data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1                     (data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register      (data_cache_0_AccelMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1                            (data_cache_0_AccelMaster_requests_pipeline_bridge_MEMORY_s1),
      .data_cache_0_AccelMaster_write                                                         (data_cache_0_AccelMaster_write),
      .data_cache_0_AccelMaster_writedata                                                     (data_cache_0_AccelMaster_writedata),
      .data_cache_0_dataMaster_address_to_slave                                               (data_cache_0_dataMaster_address_to_slave),
      .data_cache_0_dataMaster_burstcount                                                     (data_cache_0_dataMaster_burstcount),
      .data_cache_0_dataMaster_byteenable                                                     (data_cache_0_dataMaster_byteenable),
      .data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1                              (data_cache_0_dataMaster_granted_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_latency_counter                                                (data_cache_0_dataMaster_latency_counter),
      .data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1                    (data_cache_0_dataMaster_qualified_request_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_read                                                           (data_cache_0_dataMaster_read),
      .data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1                      (data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register       (data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register          (data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register),
      .data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1                             (data_cache_0_dataMaster_requests_pipeline_bridge_MEMORY_s1),
      .data_cache_0_dataMaster_write                                                          (data_cache_0_dataMaster_write),
      .data_cache_0_dataMaster_writedata                                                      (data_cache_0_dataMaster_writedata),
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
      .d1_tiger_burst_1_upstream_end_xfer                                              (d1_tiger_burst_1_upstream_end_xfer),
      .d1_tiger_burst_2_upstream_end_xfer                                              (d1_tiger_burst_2_upstream_end_xfer),
      .pipeline_bridge_MEMORY_m1_address                                               (pipeline_bridge_MEMORY_m1_address),
      .pipeline_bridge_MEMORY_m1_address_to_slave                                      (pipeline_bridge_MEMORY_m1_address_to_slave),
      .pipeline_bridge_MEMORY_m1_burstcount                                            (pipeline_bridge_MEMORY_m1_burstcount),
      .pipeline_bridge_MEMORY_m1_byteenable                                            (pipeline_bridge_MEMORY_m1_byteenable),
      .pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream                     (pipeline_bridge_MEMORY_m1_byteenable_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_chipselect                                            (pipeline_bridge_MEMORY_m1_chipselect),
      .pipeline_bridge_MEMORY_m1_dbs_address                                           (pipeline_bridge_MEMORY_m1_dbs_address),
      .pipeline_bridge_MEMORY_m1_dbs_write_16                                          (pipeline_bridge_MEMORY_m1_dbs_write_16),
      .pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream                        (pipeline_bridge_MEMORY_m1_granted_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream                        (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream                        (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_latency_counter                                       (pipeline_bridge_MEMORY_m1_latency_counter),
      .pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream              (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream              (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream              (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_read                                                  (pipeline_bridge_MEMORY_m1_read),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream                (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream                (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream                (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_readdata                                              (pipeline_bridge_MEMORY_m1_readdata),
      .pipeline_bridge_MEMORY_m1_readdatavalid                                         (pipeline_bridge_MEMORY_m1_readdatavalid),
      .pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream                       (pipeline_bridge_MEMORY_m1_requests_tiger_burst_0_upstream),
      .pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream                       (pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream                       (pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_waitrequest                                           (pipeline_bridge_MEMORY_m1_waitrequest),
      .pipeline_bridge_MEMORY_m1_write                                                 (pipeline_bridge_MEMORY_m1_write),
      .pipeline_bridge_MEMORY_m1_writedata                                             (pipeline_bridge_MEMORY_m1_writedata),
      .reset_n                                                                         (clk_reset_n),
      .tiger_burst_0_upstream_readdata_from_sa                                         (tiger_burst_0_upstream_readdata_from_sa),
      .tiger_burst_0_upstream_waitrequest_from_sa                                      (tiger_burst_0_upstream_waitrequest_from_sa),
      .tiger_burst_1_upstream_readdata_from_sa                                         (tiger_burst_1_upstream_readdata_from_sa),
      .tiger_burst_1_upstream_waitrequest_from_sa                                      (tiger_burst_1_upstream_waitrequest_from_sa),
      .tiger_burst_2_upstream_readdata_from_sa                                         (tiger_burst_2_upstream_readdata_from_sa),
      .tiger_burst_2_upstream_waitrequest_from_sa                                      (tiger_burst_2_upstream_waitrequest_from_sa)
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
      .clk                                                                                    (clk),
      .d1_pipeline_bridge_PERIPHERALS_s1_end_xfer                                             (d1_pipeline_bridge_PERIPHERALS_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_s1_address                                                 (pipeline_bridge_PERIPHERALS_s1_address),
      .pipeline_bridge_PERIPHERALS_s1_arbiterlock                                             (pipeline_bridge_PERIPHERALS_s1_arbiterlock),
      .pipeline_bridge_PERIPHERALS_s1_arbiterlock2                                            (pipeline_bridge_PERIPHERALS_s1_arbiterlock2),
      .pipeline_bridge_PERIPHERALS_s1_burstcount                                              (pipeline_bridge_PERIPHERALS_s1_burstcount),
      .pipeline_bridge_PERIPHERALS_s1_byteenable                                              (pipeline_bridge_PERIPHERALS_s1_byteenable),
      .pipeline_bridge_PERIPHERALS_s1_chipselect                                              (pipeline_bridge_PERIPHERALS_s1_chipselect),
      .pipeline_bridge_PERIPHERALS_s1_debugaccess                                             (pipeline_bridge_PERIPHERALS_s1_debugaccess),
      .pipeline_bridge_PERIPHERALS_s1_endofpacket                                             (pipeline_bridge_PERIPHERALS_s1_endofpacket),
      .pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa                                     (pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_nativeaddress                                           (pipeline_bridge_PERIPHERALS_s1_nativeaddress),
      .pipeline_bridge_PERIPHERALS_s1_read                                                    (pipeline_bridge_PERIPHERALS_s1_read),
      .pipeline_bridge_PERIPHERALS_s1_readdata                                                (pipeline_bridge_PERIPHERALS_s1_readdata),
      .pipeline_bridge_PERIPHERALS_s1_readdata_from_sa                                        (pipeline_bridge_PERIPHERALS_s1_readdata_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_readdatavalid                                           (pipeline_bridge_PERIPHERALS_s1_readdatavalid),
      .pipeline_bridge_PERIPHERALS_s1_reset_n                                                 (pipeline_bridge_PERIPHERALS_s1_reset_n),
      .pipeline_bridge_PERIPHERALS_s1_waitrequest                                             (pipeline_bridge_PERIPHERALS_s1_waitrequest),
      .pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa                                     (pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_write                                                   (pipeline_bridge_PERIPHERALS_s1_write),
      .pipeline_bridge_PERIPHERALS_s1_writedata                                               (pipeline_bridge_PERIPHERALS_s1_writedata),
      .reset_n                                                                                (clk_reset_n),
      .tiger_burst_3_downstream_address_to_slave                                              (tiger_burst_3_downstream_address_to_slave),
      .tiger_burst_3_downstream_arbitrationshare                                              (tiger_burst_3_downstream_arbitrationshare),
      .tiger_burst_3_downstream_burstcount                                                    (tiger_burst_3_downstream_burstcount),
      .tiger_burst_3_downstream_byteenable                                                    (tiger_burst_3_downstream_byteenable),
      .tiger_burst_3_downstream_debugaccess                                                   (tiger_burst_3_downstream_debugaccess),
      .tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1                        (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_latency_counter                                               (tiger_burst_3_downstream_latency_counter),
      .tiger_burst_3_downstream_nativeaddress                                                 (tiger_burst_3_downstream_nativeaddress),
      .tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1              (tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_read                                                          (tiger_burst_3_downstream_read),
      .tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1                (tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register (tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register),
      .tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1                       (tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_write                                                         (tiger_burst_3_downstream_write),
      .tiger_burst_3_downstream_writedata                                                     (tiger_burst_3_downstream_writedata)
    );

  pipeline_bridge_PERIPHERALS_m1_arbitrator the_pipeline_bridge_PERIPHERALS_m1
    (
      .clk                                                                              (clk),
      .d1_tigers_jtag_uart_1_controlSlave_end_xfer                                      (d1_tigers_jtag_uart_1_controlSlave_end_xfer),
      .d1_tigers_jtag_uart_controlSlave_end_xfer                                        (d1_tigers_jtag_uart_controlSlave_end_xfer),
      .d1_uart_0_s1_end_xfer                                                            (d1_uart_0_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_m1_address                                           (pipeline_bridge_PERIPHERALS_m1_address),
      .pipeline_bridge_PERIPHERALS_m1_address_to_slave                                  (pipeline_bridge_PERIPHERALS_m1_address_to_slave),
      .pipeline_bridge_PERIPHERALS_m1_burstcount                                        (pipeline_bridge_PERIPHERALS_m1_burstcount),
      .pipeline_bridge_PERIPHERALS_m1_byteenable                                        (pipeline_bridge_PERIPHERALS_m1_byteenable),
      .pipeline_bridge_PERIPHERALS_m1_chipselect                                        (pipeline_bridge_PERIPHERALS_m1_chipselect),
      .pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave           (pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave             (pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1                                 (pipeline_bridge_PERIPHERALS_m1_granted_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_latency_counter                                   (pipeline_bridge_PERIPHERALS_m1_latency_counter),
      .pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave (pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave   (pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1                       (pipeline_bridge_PERIPHERALS_m1_qualified_request_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_read                                              (pipeline_bridge_PERIPHERALS_m1_read),
      .pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave   (pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave     (pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1                         (pipeline_bridge_PERIPHERALS_m1_read_data_valid_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_readdata                                          (pipeline_bridge_PERIPHERALS_m1_readdata),
      .pipeline_bridge_PERIPHERALS_m1_readdatavalid                                     (pipeline_bridge_PERIPHERALS_m1_readdatavalid),
      .pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave          (pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave            (pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1                                (pipeline_bridge_PERIPHERALS_m1_requests_uart_0_s1),
      .pipeline_bridge_PERIPHERALS_m1_waitrequest                                       (pipeline_bridge_PERIPHERALS_m1_waitrequest),
      .pipeline_bridge_PERIPHERALS_m1_write                                             (pipeline_bridge_PERIPHERALS_m1_write),
      .pipeline_bridge_PERIPHERALS_m1_writedata                                         (pipeline_bridge_PERIPHERALS_m1_writedata),
      .reset_n                                                                          (clk_reset_n),
      .tigers_jtag_uart_1_controlSlave_readdata_from_sa                                 (tigers_jtag_uart_1_controlSlave_readdata_from_sa),
      .tigers_jtag_uart_controlSlave_readdata_from_sa                                   (tigers_jtag_uart_controlSlave_readdata_from_sa),
      .uart_0_s1_readdata_from_sa                                                       (uart_0_s1_readdata_from_sa)
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
      .clk                                                              (clk),
      .d1_sdram_s1_end_xfer                                             (d1_sdram_s1_end_xfer),
      .reset_n                                                          (clk_reset_n),
      .sdram_s1_address                                                 (sdram_s1_address),
      .sdram_s1_byteenable_n                                            (sdram_s1_byteenable_n),
      .sdram_s1_chipselect                                              (sdram_s1_chipselect),
      .sdram_s1_read_n                                                  (sdram_s1_read_n),
      .sdram_s1_readdata                                                (sdram_s1_readdata),
      .sdram_s1_readdata_from_sa                                        (sdram_s1_readdata_from_sa),
      .sdram_s1_readdatavalid                                           (sdram_s1_readdatavalid),
      .sdram_s1_reset_n                                                 (sdram_s1_reset_n),
      .sdram_s1_waitrequest                                             (sdram_s1_waitrequest),
      .sdram_s1_waitrequest_from_sa                                     (sdram_s1_waitrequest_from_sa),
      .sdram_s1_write_n                                                 (sdram_s1_write_n),
      .sdram_s1_writedata                                               (sdram_s1_writedata),
      .tiger_burst_0_downstream_address_to_slave                        (tiger_burst_0_downstream_address_to_slave),
      .tiger_burst_0_downstream_arbitrationshare                        (tiger_burst_0_downstream_arbitrationshare),
      .tiger_burst_0_downstream_burstcount                              (tiger_burst_0_downstream_burstcount),
      .tiger_burst_0_downstream_byteenable                              (tiger_burst_0_downstream_byteenable),
      .tiger_burst_0_downstream_granted_sdram_s1                        (tiger_burst_0_downstream_granted_sdram_s1),
      .tiger_burst_0_downstream_latency_counter                         (tiger_burst_0_downstream_latency_counter),
      .tiger_burst_0_downstream_qualified_request_sdram_s1              (tiger_burst_0_downstream_qualified_request_sdram_s1),
      .tiger_burst_0_downstream_read                                    (tiger_burst_0_downstream_read),
      .tiger_burst_0_downstream_read_data_valid_sdram_s1                (tiger_burst_0_downstream_read_data_valid_sdram_s1),
      .tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register (tiger_burst_0_downstream_read_data_valid_sdram_s1_shift_register),
      .tiger_burst_0_downstream_requests_sdram_s1                       (tiger_burst_0_downstream_requests_sdram_s1),
      .tiger_burst_0_downstream_write                                   (tiger_burst_0_downstream_write),
      .tiger_burst_0_downstream_writedata                               (tiger_burst_0_downstream_writedata)
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
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register),
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

  tiger_burst_1_upstream_arbitrator the_tiger_burst_1_upstream
    (
      .clk                                                                             (clk),
      .d1_tiger_burst_1_upstream_end_xfer                                              (d1_tiger_burst_1_upstream_end_xfer),
      .pipeline_bridge_MEMORY_m1_address_to_slave                                      (pipeline_bridge_MEMORY_m1_address_to_slave),
      .pipeline_bridge_MEMORY_m1_burstcount                                            (pipeline_bridge_MEMORY_m1_burstcount),
      .pipeline_bridge_MEMORY_m1_byteenable                                            (pipeline_bridge_MEMORY_m1_byteenable),
      .pipeline_bridge_MEMORY_m1_chipselect                                            (pipeline_bridge_MEMORY_m1_chipselect),
      .pipeline_bridge_MEMORY_m1_debugaccess                                           (pipeline_bridge_MEMORY_m1_debugaccess),
      .pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream                        (pipeline_bridge_MEMORY_m1_granted_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_latency_counter                                       (pipeline_bridge_MEMORY_m1_latency_counter),
      .pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream              (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_read                                                  (pipeline_bridge_MEMORY_m1_read),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream                (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream                       (pipeline_bridge_MEMORY_m1_requests_tiger_burst_1_upstream),
      .pipeline_bridge_MEMORY_m1_write                                                 (pipeline_bridge_MEMORY_m1_write),
      .pipeline_bridge_MEMORY_m1_writedata                                             (pipeline_bridge_MEMORY_m1_writedata),
      .reset_n                                                                         (clk_reset_n),
      .tiger_burst_1_upstream_address                                                  (tiger_burst_1_upstream_address),
      .tiger_burst_1_upstream_burstcount                                               (tiger_burst_1_upstream_burstcount),
      .tiger_burst_1_upstream_byteaddress                                              (tiger_burst_1_upstream_byteaddress),
      .tiger_burst_1_upstream_byteenable                                               (tiger_burst_1_upstream_byteenable),
      .tiger_burst_1_upstream_debugaccess                                              (tiger_burst_1_upstream_debugaccess),
      .tiger_burst_1_upstream_read                                                     (tiger_burst_1_upstream_read),
      .tiger_burst_1_upstream_readdata                                                 (tiger_burst_1_upstream_readdata),
      .tiger_burst_1_upstream_readdata_from_sa                                         (tiger_burst_1_upstream_readdata_from_sa),
      .tiger_burst_1_upstream_readdatavalid                                            (tiger_burst_1_upstream_readdatavalid),
      .tiger_burst_1_upstream_waitrequest                                              (tiger_burst_1_upstream_waitrequest),
      .tiger_burst_1_upstream_waitrequest_from_sa                                      (tiger_burst_1_upstream_waitrequest_from_sa),
      .tiger_burst_1_upstream_write                                                    (tiger_burst_1_upstream_write),
      .tiger_burst_1_upstream_writedata                                                (tiger_burst_1_upstream_writedata)
    );

  tiger_burst_1_downstream_arbitrator the_tiger_burst_1_downstream
    (
      .clk                                                      (clk),
      .d1_onchip_mem_s1_end_xfer                                (d1_onchip_mem_s1_end_xfer),
      .onchip_mem_s1_readdata_from_sa                           (onchip_mem_s1_readdata_from_sa),
      .reset_n                                                  (clk_reset_n),
      .tiger_burst_1_downstream_address                         (tiger_burst_1_downstream_address),
      .tiger_burst_1_downstream_address_to_slave                (tiger_burst_1_downstream_address_to_slave),
      .tiger_burst_1_downstream_burstcount                      (tiger_burst_1_downstream_burstcount),
      .tiger_burst_1_downstream_byteenable                      (tiger_burst_1_downstream_byteenable),
      .tiger_burst_1_downstream_granted_onchip_mem_s1           (tiger_burst_1_downstream_granted_onchip_mem_s1),
      .tiger_burst_1_downstream_latency_counter                 (tiger_burst_1_downstream_latency_counter),
      .tiger_burst_1_downstream_qualified_request_onchip_mem_s1 (tiger_burst_1_downstream_qualified_request_onchip_mem_s1),
      .tiger_burst_1_downstream_read                            (tiger_burst_1_downstream_read),
      .tiger_burst_1_downstream_read_data_valid_onchip_mem_s1   (tiger_burst_1_downstream_read_data_valid_onchip_mem_s1),
      .tiger_burst_1_downstream_readdata                        (tiger_burst_1_downstream_readdata),
      .tiger_burst_1_downstream_readdatavalid                   (tiger_burst_1_downstream_readdatavalid),
      .tiger_burst_1_downstream_requests_onchip_mem_s1          (tiger_burst_1_downstream_requests_onchip_mem_s1),
      .tiger_burst_1_downstream_reset_n                         (tiger_burst_1_downstream_reset_n),
      .tiger_burst_1_downstream_waitrequest                     (tiger_burst_1_downstream_waitrequest),
      .tiger_burst_1_downstream_write                           (tiger_burst_1_downstream_write),
      .tiger_burst_1_downstream_writedata                       (tiger_burst_1_downstream_writedata)
    );

  tiger_burst_1 the_tiger_burst_1
    (
      .clk                         (clk),
      .downstream_address          (tiger_burst_1_downstream_address),
      .downstream_arbitrationshare (tiger_burst_1_downstream_arbitrationshare),
      .downstream_burstcount       (tiger_burst_1_downstream_burstcount),
      .downstream_byteenable       (tiger_burst_1_downstream_byteenable),
      .downstream_debugaccess      (tiger_burst_1_downstream_debugaccess),
      .downstream_nativeaddress    (tiger_burst_1_downstream_nativeaddress),
      .downstream_read             (tiger_burst_1_downstream_read),
      .downstream_readdata         (tiger_burst_1_downstream_readdata),
      .downstream_readdatavalid    (tiger_burst_1_downstream_readdatavalid),
      .downstream_waitrequest      (tiger_burst_1_downstream_waitrequest),
      .downstream_write            (tiger_burst_1_downstream_write),
      .downstream_writedata        (tiger_burst_1_downstream_writedata),
      .reset_n                     (tiger_burst_1_downstream_reset_n),
      .upstream_address            (tiger_burst_1_upstream_byteaddress),
      .upstream_burstcount         (tiger_burst_1_upstream_burstcount),
      .upstream_byteenable         (tiger_burst_1_upstream_byteenable),
      .upstream_debugaccess        (tiger_burst_1_upstream_debugaccess),
      .upstream_nativeaddress      (tiger_burst_1_upstream_address),
      .upstream_read               (tiger_burst_1_upstream_read),
      .upstream_readdata           (tiger_burst_1_upstream_readdata),
      .upstream_readdatavalid      (tiger_burst_1_upstream_readdatavalid),
      .upstream_waitrequest        (tiger_burst_1_upstream_waitrequest),
      .upstream_write              (tiger_burst_1_upstream_write),
      .upstream_writedata          (tiger_burst_1_upstream_writedata)
    );

  tiger_burst_2_upstream_arbitrator the_tiger_burst_2_upstream
    (
      .clk                                                                             (clk),
      .d1_tiger_burst_2_upstream_end_xfer                                              (d1_tiger_burst_2_upstream_end_xfer),
      .pipeline_bridge_MEMORY_m1_address_to_slave                                      (pipeline_bridge_MEMORY_m1_address_to_slave),
      .pipeline_bridge_MEMORY_m1_burstcount                                            (pipeline_bridge_MEMORY_m1_burstcount),
      .pipeline_bridge_MEMORY_m1_byteenable                                            (pipeline_bridge_MEMORY_m1_byteenable),
      .pipeline_bridge_MEMORY_m1_chipselect                                            (pipeline_bridge_MEMORY_m1_chipselect),
      .pipeline_bridge_MEMORY_m1_debugaccess                                           (pipeline_bridge_MEMORY_m1_debugaccess),
      .pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream                        (pipeline_bridge_MEMORY_m1_granted_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_latency_counter                                       (pipeline_bridge_MEMORY_m1_latency_counter),
      .pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream              (pipeline_bridge_MEMORY_m1_qualified_request_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_read                                                  (pipeline_bridge_MEMORY_m1_read),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_0_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_1_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream                (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register (pipeline_bridge_MEMORY_m1_read_data_valid_tiger_burst_2_upstream_shift_register),
      .pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream                       (pipeline_bridge_MEMORY_m1_requests_tiger_burst_2_upstream),
      .pipeline_bridge_MEMORY_m1_write                                                 (pipeline_bridge_MEMORY_m1_write),
      .pipeline_bridge_MEMORY_m1_writedata                                             (pipeline_bridge_MEMORY_m1_writedata),
      .reset_n                                                                         (clk_reset_n),
      .tiger_burst_2_upstream_address                                                  (tiger_burst_2_upstream_address),
      .tiger_burst_2_upstream_burstcount                                               (tiger_burst_2_upstream_burstcount),
      .tiger_burst_2_upstream_byteaddress                                              (tiger_burst_2_upstream_byteaddress),
      .tiger_burst_2_upstream_byteenable                                               (tiger_burst_2_upstream_byteenable),
      .tiger_burst_2_upstream_debugaccess                                              (tiger_burst_2_upstream_debugaccess),
      .tiger_burst_2_upstream_read                                                     (tiger_burst_2_upstream_read),
      .tiger_burst_2_upstream_readdata                                                 (tiger_burst_2_upstream_readdata),
      .tiger_burst_2_upstream_readdata_from_sa                                         (tiger_burst_2_upstream_readdata_from_sa),
      .tiger_burst_2_upstream_readdatavalid                                            (tiger_burst_2_upstream_readdatavalid),
      .tiger_burst_2_upstream_waitrequest                                              (tiger_burst_2_upstream_waitrequest),
      .tiger_burst_2_upstream_waitrequest_from_sa                                      (tiger_burst_2_upstream_waitrequest_from_sa),
      .tiger_burst_2_upstream_write                                                    (tiger_burst_2_upstream_write),
      .tiger_burst_2_upstream_writedata                                                (tiger_burst_2_upstream_writedata)
    );

  tiger_burst_2_downstream_arbitrator the_tiger_burst_2_downstream
    (
      .clk                                                           (clk),
      .d1_data_cache_0_ACCEL_end_xfer                                (d1_data_cache_0_ACCEL_end_xfer),
      .data_cache_0_ACCEL_readdata_from_sa                           (data_cache_0_ACCEL_readdata_from_sa),
      .data_cache_0_ACCEL_waitrequest_from_sa                        (data_cache_0_ACCEL_waitrequest_from_sa),
      .reset_n                                                       (clk_reset_n),
      .tiger_burst_2_downstream_address                              (tiger_burst_2_downstream_address),
      .tiger_burst_2_downstream_address_to_slave                     (tiger_burst_2_downstream_address_to_slave),
      .tiger_burst_2_downstream_burstcount                           (tiger_burst_2_downstream_burstcount),
      .tiger_burst_2_downstream_byteenable                           (tiger_burst_2_downstream_byteenable),
      .tiger_burst_2_downstream_granted_data_cache_0_ACCEL           (tiger_burst_2_downstream_granted_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_latency_counter                      (tiger_burst_2_downstream_latency_counter),
      .tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL (tiger_burst_2_downstream_qualified_request_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_read                                 (tiger_burst_2_downstream_read),
      .tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL   (tiger_burst_2_downstream_read_data_valid_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_readdata                             (tiger_burst_2_downstream_readdata),
      .tiger_burst_2_downstream_readdatavalid                        (tiger_burst_2_downstream_readdatavalid),
      .tiger_burst_2_downstream_requests_data_cache_0_ACCEL          (tiger_burst_2_downstream_requests_data_cache_0_ACCEL),
      .tiger_burst_2_downstream_reset_n                              (tiger_burst_2_downstream_reset_n),
      .tiger_burst_2_downstream_waitrequest                          (tiger_burst_2_downstream_waitrequest),
      .tiger_burst_2_downstream_write                                (tiger_burst_2_downstream_write),
      .tiger_burst_2_downstream_writedata                            (tiger_burst_2_downstream_writedata)
    );

  tiger_burst_2 the_tiger_burst_2
    (
      .clk                         (clk),
      .downstream_address          (tiger_burst_2_downstream_address),
      .downstream_arbitrationshare (tiger_burst_2_downstream_arbitrationshare),
      .downstream_burstcount       (tiger_burst_2_downstream_burstcount),
      .downstream_byteenable       (tiger_burst_2_downstream_byteenable),
      .downstream_debugaccess      (tiger_burst_2_downstream_debugaccess),
      .downstream_nativeaddress    (tiger_burst_2_downstream_nativeaddress),
      .downstream_read             (tiger_burst_2_downstream_read),
      .downstream_readdata         (tiger_burst_2_downstream_readdata),
      .downstream_readdatavalid    (tiger_burst_2_downstream_readdatavalid),
      .downstream_waitrequest      (tiger_burst_2_downstream_waitrequest),
      .downstream_write            (tiger_burst_2_downstream_write),
      .downstream_writedata        (tiger_burst_2_downstream_writedata),
      .reset_n                     (tiger_burst_2_downstream_reset_n),
      .upstream_address            (tiger_burst_2_upstream_byteaddress),
      .upstream_burstcount         (tiger_burst_2_upstream_burstcount),
      .upstream_byteenable         (tiger_burst_2_upstream_byteenable),
      .upstream_debugaccess        (tiger_burst_2_upstream_debugaccess),
      .upstream_nativeaddress      (tiger_burst_2_upstream_address),
      .upstream_read               (tiger_burst_2_upstream_read),
      .upstream_readdata           (tiger_burst_2_upstream_readdata),
      .upstream_readdatavalid      (tiger_burst_2_upstream_readdatavalid),
      .upstream_waitrequest        (tiger_burst_2_upstream_waitrequest),
      .upstream_write              (tiger_burst_2_upstream_write),
      .upstream_writedata          (tiger_burst_2_upstream_writedata)
    );

  tiger_burst_3_upstream_arbitrator the_tiger_burst_3_upstream
    (
      .clk                                                                              (clk),
      .d1_tiger_burst_3_upstream_end_xfer                                               (d1_tiger_burst_3_upstream_end_xfer),
      .data_cache_0_dataMaster_address_to_slave                                         (data_cache_0_dataMaster_address_to_slave),
      .data_cache_0_dataMaster_burstcount                                               (data_cache_0_dataMaster_burstcount),
      .data_cache_0_dataMaster_byteenable                                               (data_cache_0_dataMaster_byteenable),
      .data_cache_0_dataMaster_granted_tiger_burst_3_upstream                           (data_cache_0_dataMaster_granted_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_latency_counter                                          (data_cache_0_dataMaster_latency_counter),
      .data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream                 (data_cache_0_dataMaster_qualified_request_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_read                                                     (data_cache_0_dataMaster_read),
      .data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register (data_cache_0_dataMaster_read_data_valid_pipeline_bridge_MEMORY_s1_shift_register),
      .data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream                   (data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register    (data_cache_0_dataMaster_read_data_valid_tiger_burst_3_upstream_shift_register),
      .data_cache_0_dataMaster_requests_tiger_burst_3_upstream                          (data_cache_0_dataMaster_requests_tiger_burst_3_upstream),
      .data_cache_0_dataMaster_write                                                    (data_cache_0_dataMaster_write),
      .data_cache_0_dataMaster_writedata                                                (data_cache_0_dataMaster_writedata),
      .reset_n                                                                          (clk_reset_n),
      .tiger_burst_3_upstream_address                                                   (tiger_burst_3_upstream_address),
      .tiger_burst_3_upstream_burstcount                                                (tiger_burst_3_upstream_burstcount),
      .tiger_burst_3_upstream_byteaddress                                               (tiger_burst_3_upstream_byteaddress),
      .tiger_burst_3_upstream_byteenable                                                (tiger_burst_3_upstream_byteenable),
      .tiger_burst_3_upstream_debugaccess                                               (tiger_burst_3_upstream_debugaccess),
      .tiger_burst_3_upstream_read                                                      (tiger_burst_3_upstream_read),
      .tiger_burst_3_upstream_readdata                                                  (tiger_burst_3_upstream_readdata),
      .tiger_burst_3_upstream_readdata_from_sa                                          (tiger_burst_3_upstream_readdata_from_sa),
      .tiger_burst_3_upstream_readdatavalid                                             (tiger_burst_3_upstream_readdatavalid),
      .tiger_burst_3_upstream_waitrequest                                               (tiger_burst_3_upstream_waitrequest),
      .tiger_burst_3_upstream_waitrequest_from_sa                                       (tiger_burst_3_upstream_waitrequest_from_sa),
      .tiger_burst_3_upstream_write                                                     (tiger_burst_3_upstream_write),
      .tiger_burst_3_upstream_writedata                                                 (tiger_burst_3_upstream_writedata)
    );

  tiger_burst_3_downstream_arbitrator the_tiger_burst_3_downstream
    (
      .clk                                                                                    (clk),
      .d1_pipeline_bridge_PERIPHERALS_s1_end_xfer                                             (d1_pipeline_bridge_PERIPHERALS_s1_end_xfer),
      .pipeline_bridge_PERIPHERALS_s1_readdata_from_sa                                        (pipeline_bridge_PERIPHERALS_s1_readdata_from_sa),
      .pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa                                     (pipeline_bridge_PERIPHERALS_s1_waitrequest_from_sa),
      .reset_n                                                                                (clk_reset_n),
      .tiger_burst_3_downstream_address                                                       (tiger_burst_3_downstream_address),
      .tiger_burst_3_downstream_address_to_slave                                              (tiger_burst_3_downstream_address_to_slave),
      .tiger_burst_3_downstream_burstcount                                                    (tiger_burst_3_downstream_burstcount),
      .tiger_burst_3_downstream_byteenable                                                    (tiger_burst_3_downstream_byteenable),
      .tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1                        (tiger_burst_3_downstream_granted_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_latency_counter                                               (tiger_burst_3_downstream_latency_counter),
      .tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1              (tiger_burst_3_downstream_qualified_request_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_read                                                          (tiger_burst_3_downstream_read),
      .tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1                (tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register (tiger_burst_3_downstream_read_data_valid_pipeline_bridge_PERIPHERALS_s1_shift_register),
      .tiger_burst_3_downstream_readdata                                                      (tiger_burst_3_downstream_readdata),
      .tiger_burst_3_downstream_readdatavalid                                                 (tiger_burst_3_downstream_readdatavalid),
      .tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1                       (tiger_burst_3_downstream_requests_pipeline_bridge_PERIPHERALS_s1),
      .tiger_burst_3_downstream_reset_n                                                       (tiger_burst_3_downstream_reset_n),
      .tiger_burst_3_downstream_waitrequest                                                   (tiger_burst_3_downstream_waitrequest),
      .tiger_burst_3_downstream_write                                                         (tiger_burst_3_downstream_write),
      .tiger_burst_3_downstream_writedata                                                     (tiger_burst_3_downstream_writedata)
    );

  tiger_burst_3 the_tiger_burst_3
    (
      .clk                         (clk),
      .downstream_address          (tiger_burst_3_downstream_address),
      .downstream_arbitrationshare (tiger_burst_3_downstream_arbitrationshare),
      .downstream_burstcount       (tiger_burst_3_downstream_burstcount),
      .downstream_byteenable       (tiger_burst_3_downstream_byteenable),
      .downstream_debugaccess      (tiger_burst_3_downstream_debugaccess),
      .downstream_nativeaddress    (tiger_burst_3_downstream_nativeaddress),
      .downstream_read             (tiger_burst_3_downstream_read),
      .downstream_readdata         (tiger_burst_3_downstream_readdata),
      .downstream_readdatavalid    (tiger_burst_3_downstream_readdatavalid),
      .downstream_waitrequest      (tiger_burst_3_downstream_waitrequest),
      .downstream_write            (tiger_burst_3_downstream_write),
      .downstream_writedata        (tiger_burst_3_downstream_writedata),
      .reset_n                     (tiger_burst_3_downstream_reset_n),
      .upstream_address            (tiger_burst_3_upstream_byteaddress),
      .upstream_burstcount         (tiger_burst_3_upstream_burstcount),
      .upstream_byteenable         (tiger_burst_3_upstream_byteenable),
      .upstream_debugaccess        (tiger_burst_3_upstream_debugaccess),
      .upstream_nativeaddress      (tiger_burst_3_upstream_address),
      .upstream_read               (tiger_burst_3_upstream_read),
      .upstream_readdata           (tiger_burst_3_upstream_readdata),
      .upstream_readdatavalid      (tiger_burst_3_upstream_readdatavalid),
      .upstream_waitrequest        (tiger_burst_3_upstream_waitrequest),
      .upstream_write              (tiger_burst_3_upstream_write),
      .upstream_writedata          (tiger_burst_3_upstream_writedata)
    );

  tiger_top_0_CachetoTiger_arbitrator the_tiger_top_0_CachetoTiger
    (
      .clk                            (clk),
      .data_cache_0_CachetoTiger_data (data_cache_0_CachetoTiger_data),
      .reset_n                        (clk_reset_n),
      .tiger_top_0_CachetoTiger_data  (tiger_top_0_CachetoTiger_data)
    );

  tiger_top_0_TigertoCache_arbitrator the_tiger_top_0_TigertoCache
    (
      .clk                            (clk),
      .reset_n                        (clk_reset_n),
      .tiger_top_0_TigertoCache_data  (tiger_top_0_TigertoCache_data),
      .tiger_top_0_TigertoCache_reset (tiger_top_0_TigertoCache_reset)
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

  tiger_top_0 the_tiger_top_0
    (
      .asi_CachetoTiger_data                    (tiger_top_0_CachetoTiger_data),
      .aso_TigertoCache_data                    (tiger_top_0_TigertoCache_data),
      .avm_instructionMaster_address            (tiger_top_0_instructionMaster_address),
      .avm_instructionMaster_beginbursttransfer (tiger_top_0_instructionMaster_beginbursttransfer),
      .avm_instructionMaster_burstcount         (tiger_top_0_instructionMaster_burstcount),
      .avm_instructionMaster_read               (tiger_top_0_instructionMaster_read),
      .avm_instructionMaster_readdata           (tiger_top_0_instructionMaster_readdata),
      .avm_instructionMaster_readdatavalid      (tiger_top_0_instructionMaster_readdatavalid),
      .avm_instructionMaster_waitrequest        (tiger_top_0_instructionMaster_waitrequest),
      .clk                                      (clk),
      .reset                                    (tiger_top_0_TigertoCache_reset)
    );

  tigers_jtag_uart_controlSlave_arbitrator the_tigers_jtag_uart_controlSlave
    (
      .clk                                                                            (clk),
      .d1_tigers_jtag_uart_controlSlave_end_xfer                                      (d1_tigers_jtag_uart_controlSlave_end_xfer),
      .pipeline_bridge_PERIPHERALS_m1_address_to_slave                                (pipeline_bridge_PERIPHERALS_m1_address_to_slave),
      .pipeline_bridge_PERIPHERALS_m1_burstcount                                      (pipeline_bridge_PERIPHERALS_m1_burstcount),
      .pipeline_bridge_PERIPHERALS_m1_chipselect                                      (pipeline_bridge_PERIPHERALS_m1_chipselect),
      .pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave           (pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_latency_counter                                 (pipeline_bridge_PERIPHERALS_m1_latency_counter),
      .pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave (pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_read                                            (pipeline_bridge_PERIPHERALS_m1_read),
      .pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave   (pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave          (pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_write                                           (pipeline_bridge_PERIPHERALS_m1_write),
      .pipeline_bridge_PERIPHERALS_m1_writedata                                       (pipeline_bridge_PERIPHERALS_m1_writedata),
      .reset_n                                                                        (clk_reset_n),
      .tigers_jtag_uart_controlSlave_address                                          (tigers_jtag_uart_controlSlave_address),
      .tigers_jtag_uart_controlSlave_read                                             (tigers_jtag_uart_controlSlave_read),
      .tigers_jtag_uart_controlSlave_readdata                                         (tigers_jtag_uart_controlSlave_readdata),
      .tigers_jtag_uart_controlSlave_readdata_from_sa                                 (tigers_jtag_uart_controlSlave_readdata_from_sa),
      .tigers_jtag_uart_controlSlave_reset_n                                          (tigers_jtag_uart_controlSlave_reset_n),
      .tigers_jtag_uart_controlSlave_write                                            (tigers_jtag_uart_controlSlave_write),
      .tigers_jtag_uart_controlSlave_writedata                                        (tigers_jtag_uart_controlSlave_writedata)
    );

  tigers_jtag_uart the_tigers_jtag_uart
    (
      .avs_controlSlave_address   (tigers_jtag_uart_controlSlave_address),
      .avs_controlSlave_read      (tigers_jtag_uart_controlSlave_read),
      .avs_controlSlave_readdata  (tigers_jtag_uart_controlSlave_readdata),
      .avs_controlSlave_write     (tigers_jtag_uart_controlSlave_write),
      .avs_controlSlave_writedata (tigers_jtag_uart_controlSlave_writedata),
      .clk                        (clk),
      .reset_n                    (tigers_jtag_uart_controlSlave_reset_n)
    );

  tigers_jtag_uart_1_controlSlave_arbitrator the_tigers_jtag_uart_1_controlSlave
    (
      .clk                                                                              (clk),
      .d1_tigers_jtag_uart_1_controlSlave_end_xfer                                      (d1_tigers_jtag_uart_1_controlSlave_end_xfer),
      .pipeline_bridge_PERIPHERALS_m1_address_to_slave                                  (pipeline_bridge_PERIPHERALS_m1_address_to_slave),
      .pipeline_bridge_PERIPHERALS_m1_burstcount                                        (pipeline_bridge_PERIPHERALS_m1_burstcount),
      .pipeline_bridge_PERIPHERALS_m1_chipselect                                        (pipeline_bridge_PERIPHERALS_m1_chipselect),
      .pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave           (pipeline_bridge_PERIPHERALS_m1_granted_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_latency_counter                                   (pipeline_bridge_PERIPHERALS_m1_latency_counter),
      .pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave (pipeline_bridge_PERIPHERALS_m1_qualified_request_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_read                                              (pipeline_bridge_PERIPHERALS_m1_read),
      .pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave   (pipeline_bridge_PERIPHERALS_m1_read_data_valid_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave          (pipeline_bridge_PERIPHERALS_m1_requests_tigers_jtag_uart_1_controlSlave),
      .pipeline_bridge_PERIPHERALS_m1_write                                             (pipeline_bridge_PERIPHERALS_m1_write),
      .pipeline_bridge_PERIPHERALS_m1_writedata                                         (pipeline_bridge_PERIPHERALS_m1_writedata),
      .reset_n                                                                          (clk_reset_n),
      .tigers_jtag_uart_1_controlSlave_address                                          (tigers_jtag_uart_1_controlSlave_address),
      .tigers_jtag_uart_1_controlSlave_read                                             (tigers_jtag_uart_1_controlSlave_read),
      .tigers_jtag_uart_1_controlSlave_readdata                                         (tigers_jtag_uart_1_controlSlave_readdata),
      .tigers_jtag_uart_1_controlSlave_readdata_from_sa                                 (tigers_jtag_uart_1_controlSlave_readdata_from_sa),
      .tigers_jtag_uart_1_controlSlave_reset_n                                          (tigers_jtag_uart_1_controlSlave_reset_n),
      .tigers_jtag_uart_1_controlSlave_write                                            (tigers_jtag_uart_1_controlSlave_write),
      .tigers_jtag_uart_1_controlSlave_writedata                                        (tigers_jtag_uart_1_controlSlave_writedata)
    );

  tigers_jtag_uart_1 the_tigers_jtag_uart_1
    (
      .avs_controlSlave_address   (tigers_jtag_uart_1_controlSlave_address),
      .avs_controlSlave_read      (tigers_jtag_uart_1_controlSlave_read),
      .avs_controlSlave_readdata  (tigers_jtag_uart_1_controlSlave_readdata),
      .avs_controlSlave_write     (tigers_jtag_uart_1_controlSlave_write),
      .avs_controlSlave_writedata (tigers_jtag_uart_1_controlSlave_writedata),
      .clk                        (clk),
      .reset_n                    (tigers_jtag_uart_1_controlSlave_reset_n)
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
    0);

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

`include "../altera_libs/altera_mf.v"
`include "../altera_libs/220model.v"
`include "../altera_libs/sgate.v"
`include "tiger_top.v"
`include "tiger_top_0.v"
`include "tigers_jtag_uart_1.v"
`include "data_cache.v"
`include "data_cache_0.v"
`include "tigers_jtag_uart.v"
`include "tiger_burst_0.v"
`include "pipeline_bridge_MEMORY.v"
`include "sdram.v"
`include "sdram_test_component.v"
`include "tiger_burst_3.v"
`include "tiger_burst_1.v"
`include "uart_0.v"
`include "onchip_mem.v"
`include "pipeline_bridge_PERIPHERALS.v"
`include "tiger_burst_2.v"

`timescale 1ns / 1ps

module test_bench 
;


  reg              clk;
  wire             data_cache_0_AccelMaster_beginbursttransfer;
  wire             data_cache_0_dataMaster_beginbursttransfer;
  wire             pipeline_bridge_MEMORY_m1_endofpacket;
  wire             pipeline_bridge_MEMORY_s1_endofpacket_from_sa;
  wire             pipeline_bridge_PERIPHERALS_m1_debugaccess;
  wire             pipeline_bridge_PERIPHERALS_m1_endofpacket;
  wire             pipeline_bridge_PERIPHERALS_s1_endofpacket_from_sa;
  reg              reset_n;
  wire             rxd_to_the_uart_0;
  wire             tiger_burst_0_downstream_debugaccess;
  wire    [ 22: 0] tiger_burst_0_downstream_nativeaddress;
  wire    [ 12: 0] tiger_burst_1_downstream_nativeaddress;
  wire             tiger_burst_2_downstream_debugaccess;
  wire    [  6: 0] tiger_burst_2_downstream_nativeaddress;
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
      .clk                         (clk),
      .reset_n                     (reset_n),
      .rxd_to_the_uart_0           (rxd_to_the_uart_0),
      .txd_from_the_uart_0         (txd_from_the_uart_0),
      .zs_addr_from_the_sdram      (zs_addr_from_the_sdram),
      .zs_ba_from_the_sdram        (zs_ba_from_the_sdram),
      .zs_cas_n_from_the_sdram     (zs_cas_n_from_the_sdram),
      .zs_cke_from_the_sdram       (zs_cke_from_the_sdram),
      .zs_cs_n_from_the_sdram      (zs_cs_n_from_the_sdram),
      .zs_dq_to_and_from_the_sdram (zs_dq_to_and_from_the_sdram),
      .zs_dqm_from_the_sdram       (zs_dqm_from_the_sdram),
      .zs_ras_n_from_the_sdram     (zs_ras_n_from_the_sdram),
      .zs_we_n_from_the_sdram      (zs_we_n_from_the_sdram)
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
    #6 clk <= ~clk;
     else 
    #7 clk <= ~clk;
  
  initial 
    begin
      reset_n <= 0;
      #125 reset_n <= 1;
    end

endmodule


//synthesis translate_on
