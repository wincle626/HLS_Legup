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

//
//Burst adapter parameters:
//adapter is mastered by: data_cache_0/dataMaster
//adapter masters: uart_0/s1
//asp_debug: 0
//byteaddr_width: 5
//ceil_data_width: 16
//data_width: 16
//dbs_shift: 1
//dbs_upstream_burstcount_width: 4
//downstream_addr_shift: 1
//downstream_burstcount_width: 1
//downstream_max_burstcount: 1
//downstream_pipeline: 0
//dynamic_slave: 0
//master_always_burst_max_burst: 0
//master_burst_on_burst_boundaries_only: 0
//master_data_width: 32
//master_interleave: 0
//master_linewrap_bursts: 0
//nativeaddr_width: 4
//slave_always_burst_max_burst: 0
//slave_burst_on_burst_boundaries_only: 0
//slave_interleave: 0
//slave_linewrap_bursts: 0
//upstream_burstcount: upstream_burstcount
//upstream_burstcount_width: 3
//upstream_max_burstcount: 4
//zero_address_width: 0


module tiger_burst_5 (
                       // inputs:
                        clk,
                        downstream_readdata,
                        downstream_readdatavalid,
                        downstream_waitrequest,
                        reset_n,
                        upstream_address,
                        upstream_burstcount,
                        upstream_byteenable,
                        upstream_debugaccess,
                        upstream_nativeaddress,
                        upstream_read,
                        upstream_write,
                        upstream_writedata,

                       // outputs:
                        downstream_address,
                        downstream_arbitrationshare,
                        downstream_burstcount,
                        downstream_byteenable,
                        downstream_debugaccess,
                        downstream_nativeaddress,
                        downstream_read,
                        downstream_write,
                        downstream_writedata,
                        upstream_readdata,
                        upstream_readdatavalid,
                        upstream_waitrequest
                     )
;

  output  [  3: 0] downstream_address;
  output  [  3: 0] downstream_arbitrationshare;
  output           downstream_burstcount;
  output  [  1: 0] downstream_byteenable;
  output           downstream_debugaccess;
  output  [  3: 0] downstream_nativeaddress;
  output           downstream_read;
  output           downstream_write;
  output  [ 15: 0] downstream_writedata;
  output  [ 15: 0] upstream_readdata;
  output           upstream_readdatavalid;
  output           upstream_waitrequest;
  input            clk;
  input   [ 15: 0] downstream_readdata;
  input            downstream_readdatavalid;
  input            downstream_waitrequest;
  input            reset_n;
  input   [  4: 0] upstream_address;
  input   [  2: 0] upstream_burstcount;
  input   [  1: 0] upstream_byteenable;
  input            upstream_debugaccess;
  input   [  3: 0] upstream_nativeaddress;
  input            upstream_read;
  input            upstream_write;
  input   [ 15: 0] upstream_writedata;

  wire    [  4: 0] current_upstream_address;
  wire    [  3: 0] downstream_address;
  wire    [  3: 0] downstream_arbitrationshare;
  wire             downstream_burstcount;
  wire    [  1: 0] downstream_byteenable;
  wire             downstream_debugaccess;
  wire    [  3: 0] downstream_nativeaddress;
  wire             downstream_read;
  wire             downstream_write;
  wire    [ 15: 0] downstream_writedata;
  reg     [  4: 0] registered_upstream_address;
  wire             sync_nativeaddress;
  reg     [  1: 0] transactions_remaining;
  reg     [ 15: 0] upstream_readdata;
  reg              upstream_readdatavalid;
  wire             upstream_waitrequest;
  assign sync_nativeaddress = |upstream_nativeaddress;
  //downstream, which is an e_avalon_master
  //upstream, which is an e_avalon_slave
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          registered_upstream_address <= 0;
      else if (~|transactions_remaining)
          registered_upstream_address <= upstream_address;
    end


  assign current_upstream_address = ~|transactions_remaining ? upstream_address : registered_upstream_address;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          transactions_remaining <= 0;
      else 
        transactions_remaining <= (upstream_read & ~upstream_waitrequest) ? (upstream_burstcount - 1) : (downstream_read & ~downstream_waitrequest & (|transactions_remaining)) ? (transactions_remaining - downstream_burstcount) : transactions_remaining;
    end


  assign downstream_burstcount = 1;
  assign downstream_arbitrationshare = upstream_burstcount;
  assign downstream_address = current_upstream_address;
  assign downstream_nativeaddress = upstream_nativeaddress;
  assign downstream_read = upstream_read | (|transactions_remaining);
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          upstream_readdatavalid <= 0;
      else 
        upstream_readdatavalid <= downstream_readdatavalid;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          upstream_readdata <= 0;
      else 
        upstream_readdata <= downstream_readdata;
    end


  assign downstream_write = upstream_write & !downstream_read;
  assign downstream_byteenable = upstream_byteenable;
  assign downstream_writedata = upstream_writedata;
  assign upstream_waitrequest = downstream_waitrequest | (|transactions_remaining);
  assign downstream_debugaccess = upstream_debugaccess;

endmodule

