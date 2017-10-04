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

module tiger_burst_8_fifo_module_fifo_ram_module (
                                                   // inputs:
                                                    clk,
                                                    data,
                                                    rdaddress,
                                                    rdclken,
                                                    reset_n,
                                                    wraddress,
                                                    wrclock,
                                                    wren,

                                                   // outputs:
                                                    q
                                                 )
;

  output  [127: 0] q;
  input            clk;
  input   [127: 0] data;
  input            rdaddress;
  input            rdclken;
  input            reset_n;
  input            wraddress;
  input            wrclock;
  input            wren;

  reg     [127: 0] mem_array [  1: 0];
  wire    [127: 0] q;
  reg              read_address;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          read_address <= 0;
      else if (rdclken)
          read_address <= rdaddress;
    end


  // Data read is synchronized through latent_rdaddress.
  assign q = mem_array[read_address];

  always @(posedge wrclock)
    begin
      // Write data
      if (wren)
          mem_array[wraddress] <= data;
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on
//synthesis read_comments_as_HDL on
//  always @(rdaddress)
//    begin
//      read_address = rdaddress;
//    end
//
//
//  lpm_ram_dp lpm_ram_dp_component
//    (
//      .data (data),
//      .q (q),
//      .rdaddress (read_address),
//      .rdclken (rdclken),
//      .rdclock (clk),
//      .wraddress (wraddress),
//      .wrclock (wrclock),
//      .wren (wren)
//    );
//
//  defparam lpm_ram_dp_component.lpm_file = "UNUSED",
//           lpm_ram_dp_component.lpm_hint = "USE_EAB=OFF",
//           lpm_ram_dp_component.lpm_indata = "REGISTERED",
//           lpm_ram_dp_component.lpm_outdata = "UNREGISTERED",
//           lpm_ram_dp_component.lpm_rdaddress_control = "REGISTERED",
//           lpm_ram_dp_component.lpm_width = 128,
//           lpm_ram_dp_component.lpm_widthad = 1,
//           lpm_ram_dp_component.lpm_wraddress_control = "REGISTERED",
//           lpm_ram_dp_component.suppress_memory_conversion_warnings = "ON";
//
//synthesis read_comments_as_HDL off

endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

module tiger_burst_8_fifo_module (
                                   // inputs:
                                    clk,
                                    clk_en,
                                    fifo_read,
                                    fifo_wr_data,
                                    fifo_write,
                                    flush_fifo,
                                    inc_pending_data,
                                    reset_n,

                                   // outputs:
                                    fifo_datavalid,
                                    fifo_empty,
                                    fifo_full,
                                    fifo_rd_data,
                                    p1_fifo_empty
                                 )
;

  output           fifo_datavalid;
  output           fifo_empty;
  output           fifo_full;
  output  [127: 0] fifo_rd_data;
  output           p1_fifo_empty;
  input            clk;
  input            clk_en;
  input            fifo_read;
  input   [127: 0] fifo_wr_data;
  input            fifo_write;
  input            flush_fifo;
  input            inc_pending_data;
  input            reset_n;

  wire             estimated_rdaddress;
  reg              estimated_wraddress;
  wire             fifo_datavalid;
  wire             fifo_dec;
  reg              fifo_empty;
  reg              fifo_full;
  wire             fifo_inc;
  wire    [127: 0] fifo_ram_q;
  wire    [127: 0] fifo_rd_data;
  reg              last_write_collision;
  reg     [127: 0] last_write_data;
  wire             p1_estimated_wraddress;
  wire             p1_fifo_empty;
  wire             p1_fifo_full;
  wire             p1_wraddress;
  wire             rdaddress;
  reg              rdaddress_reg;
  reg              wraddress;
  wire             write_collision;
  assign p1_wraddress = (fifo_write)? wraddress - 1 :
    wraddress;

  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          wraddress <= 0;
      else if (clk_en)
          if (flush_fifo)
              wraddress <= 0;
          else 
            wraddress <= p1_wraddress;
    end


  assign rdaddress = flush_fifo ? 0 : fifo_read ? (rdaddress_reg - 1) : rdaddress_reg;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          rdaddress_reg <= 0;
      else 
        rdaddress_reg <= rdaddress;
    end


  assign fifo_datavalid = ~fifo_empty;
  assign fifo_inc = fifo_write & ~fifo_read;
  assign fifo_dec = fifo_read & ~fifo_write;
  assign estimated_rdaddress = rdaddress_reg - 1;
  assign p1_estimated_wraddress = (inc_pending_data)? estimated_wraddress - 1 :
    estimated_wraddress;

  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          estimated_wraddress <= {1 {1'b1}};
      else if (clk_en)
          if (flush_fifo)
              estimated_wraddress <= {1 {1'b1}};
          else 
            estimated_wraddress <= p1_estimated_wraddress;
    end


  assign p1_fifo_empty = flush_fifo  | ((~fifo_inc & fifo_empty) | (fifo_dec & (wraddress == estimated_rdaddress)));
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_empty <= 1;
      else if (clk_en)
          fifo_empty <= p1_fifo_empty;
    end


  assign p1_fifo_full = ~flush_fifo & ((~fifo_dec & fifo_full)  | (inc_pending_data & (estimated_wraddress == rdaddress)));
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          fifo_full <= 0;
      else if (clk_en)
          fifo_full <= p1_fifo_full;
    end


  assign write_collision = fifo_write && (wraddress == rdaddress);
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_write_data <= 0;
      else if (write_collision)
          last_write_data <= fifo_wr_data;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          last_write_collision <= 0;
      else if (write_collision)
          last_write_collision <= -1;
      else if (fifo_read)
          last_write_collision <= 0;
    end


  assign fifo_rd_data = last_write_collision ? last_write_data : fifo_ram_q;
  //tiger_burst_8_fifo_module_fifo_ram, which is an e_ram
  tiger_burst_8_fifo_module_fifo_ram_module tiger_burst_8_fifo_module_fifo_ram
    (
      .clk       (clk),
      .data      (fifo_wr_data),
      .q         (fifo_ram_q),
      .rdaddress (rdaddress),
      .rdclken   (1'b1),
      .reset_n   (reset_n),
      .wraddress (wraddress),
      .wrclock   (clk),
      .wren      (fifo_write)
    );


endmodule



// turn off superfluous verilog processor warnings 
// altera message_level Level1 
// altera message_off 10034 10035 10036 10037 10230 10240 10030 

//
//Burst adapter parameters:
//adapter is mastered by: data_cache_0/AccelMaster
//adapter masters: data_cache_0/ACCEL
//asp_debug: 0
//byteaddr_width: 11
//ceil_data_width: 128
//data_width: 128
//dbs_shift: -2
//dbs_upstream_burstcount_width: 3
//downstream_addr_shift: 4
//downstream_burstcount_width: 1
//downstream_max_burstcount: 1
//downstream_pipeline: 0
//dynamic_slave: 1
//master_always_burst_max_burst: 0
//master_burst_on_burst_boundaries_only: 0
//master_data_width: 32
//master_interleave: 0
//master_linewrap_bursts: 0
//nativeaddr_width: 7
//slave_always_burst_max_burst: 0
//slave_burst_on_burst_boundaries_only: 0
//slave_interleave: 0
//slave_linewrap_bursts: 0
//upstream_burstcount: upstream_burstcount
//upstream_burstcount_width: 3
//upstream_max_burstcount: 4
//zero_address_width: 0


module tiger_burst_8 (
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

  output  [  6: 0] downstream_address;
  output  [  2: 0] downstream_arbitrationshare;
  output           downstream_burstcount;
  output  [ 15: 0] downstream_byteenable;
  output           downstream_debugaccess;
  output  [  6: 0] downstream_nativeaddress;
  output           downstream_read;
  output           downstream_write;
  output  [127: 0] downstream_writedata;
  output  [127: 0] upstream_readdata;
  output           upstream_readdatavalid;
  output           upstream_waitrequest;
  input            clk;
  input   [127: 0] downstream_readdata;
  input            downstream_readdatavalid;
  input            downstream_waitrequest;
  input            reset_n;
  input   [ 10: 0] upstream_address;
  input   [  2: 0] upstream_burstcount;
  input   [ 15: 0] upstream_byteenable;
  input            upstream_debugaccess;
  input   [  6: 0] upstream_nativeaddress;
  input            upstream_read;
  input            upstream_write;
  input   [127: 0] upstream_writedata;

  wire    [  1: 0] address_offset;
  reg              atomic_counter;
  wire    [ 10: 0] current_upstream_address;
  wire    [  2: 0] current_upstream_burstcount;
  wire             current_upstream_read;
  wire             current_upstream_write;
  reg     [  2: 0] data_counter;
  wire    [  2: 0] dbs_adjusted_upstream_burstcount;
  wire    [  6: 0] downstream_address;
  wire    [ 10: 0] downstream_address_base;
  wire    [  2: 0] downstream_arbitrationshare;
  wire             downstream_burstcount;
  wire             downstream_burstdone;
  wire    [ 15: 0] downstream_byteenable;
  wire             downstream_debugaccess;
  wire    [  6: 0] downstream_nativeaddress;
  reg              downstream_read;
  wire             downstream_write;
  wire    [127: 0] downstream_writedata;
  wire             enable_state_change;
  wire             fifo_datavalid;
  wire             fifo_empty;
  wire             fifo_full;
  wire    [127: 0] fifo_rd_data;
  wire             fifo_read;
  wire    [127: 0] fifo_wr_data;
  wire             fifo_write;
  wire             flush_fifo;
  reg     [  2: 0] full_width_rdv_counter;
  wire             max_burst_size;
  reg     [  1: 0] negative_dbs_rdv_counter;
  wire    [  2: 0] negative_dbs_read_expression;
  wire             p1_atomic_counter;
  wire             p1_fifo_empty;
  wire             p1_state_busy;
  wire             p1_state_idle;
  wire             pending_register_enable;
  wire             pending_upstream_read;
  reg              pending_upstream_read_reg;
  wire             pending_upstream_write;
  reg              pending_upstream_write_reg;
  wire    [ 10: 0] quantized_burst_base;
  wire    [ 10: 0] quantized_burst_limit;
  reg     [  1: 0] read_address_offset;
  wire             read_update_count;
  wire    [  2: 0] read_write_dbs_adjusted_upstream_burstcount;
  reg     [  2: 0] registered_read_write_dbs_adjusted_upstream_burstcount;
  reg     [ 10: 0] registered_upstream_address;
  reg     [  2: 0] registered_upstream_burstcount;
  reg     [  6: 0] registered_upstream_nativeaddress;
  reg              registered_upstream_read;
  reg              registered_upstream_write;
  reg              state_busy;
  reg              state_idle;
  wire             sync_nativeaddress;
  wire    [  2: 0] transactions_remaining;
  reg     [  2: 0] transactions_remaining_reg;
  wire             update_count;
  wire             upstream_burstdone;
  wire             upstream_read_run;
  wire    [127: 0] upstream_readdata;
  wire             upstream_readdatavalid;
  wire             upstream_waitrequest;
  wire             upstream_write_run;
  reg     [  1: 0] write_address_offset;
  wire             write_update_count;
  assign sync_nativeaddress = |upstream_nativeaddress;
  //downstream, which is an e_avalon_master
  //upstream, which is an e_avalon_slave
  assign upstream_burstdone = current_upstream_read ? (transactions_remaining == downstream_burstcount) & downstream_read & ~downstream_waitrequest : (transactions_remaining == (atomic_counter + 1)) & downstream_write & ~downstream_waitrequest;
  assign p1_atomic_counter = atomic_counter + (downstream_read ? downstream_burstcount : 1);
  assign downstream_burstdone = (downstream_read | downstream_write) & ~downstream_waitrequest & (p1_atomic_counter == downstream_burstcount);
  assign quantized_burst_base = upstream_address & ~15;
  assign quantized_burst_limit = (((upstream_address & ~3) + {upstream_burstcount,
    2'b00} - 1) | 15) + 1;

  assign negative_dbs_read_expression = (quantized_burst_limit - quantized_burst_base) >> 4;
  assign dbs_adjusted_upstream_burstcount = pending_register_enable ? read_write_dbs_adjusted_upstream_burstcount : registered_read_write_dbs_adjusted_upstream_burstcount;
  assign read_write_dbs_adjusted_upstream_burstcount = upstream_read ? negative_dbs_read_expression : upstream_burstcount;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          registered_read_write_dbs_adjusted_upstream_burstcount <= 0;
      else if (pending_register_enable)
          registered_read_write_dbs_adjusted_upstream_burstcount <= read_write_dbs_adjusted_upstream_burstcount;
    end


  assign p1_state_idle = state_idle & ~upstream_read & ~upstream_write | state_busy & (data_counter == 0) & p1_fifo_empty & ~pending_upstream_read & ~pending_upstream_write;
  assign p1_state_busy = state_idle & (upstream_read | upstream_write) | state_busy & (~(data_counter == 0) | ~p1_fifo_empty | pending_upstream_read | pending_upstream_write);
  assign enable_state_change = ~(downstream_read | downstream_write) | ~downstream_waitrequest;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pending_upstream_read_reg <= 0;
      else if (upstream_read & state_idle)
          pending_upstream_read_reg <= -1;
      else if (downstream_readdatavalid)
          pending_upstream_read_reg <= 0;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          pending_upstream_write_reg <= 0;
      else if (upstream_burstdone)
          pending_upstream_write_reg <= 0;
      else if (upstream_write & (state_idle | ~upstream_waitrequest))
          pending_upstream_write_reg <= -1;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          state_idle <= 1;
      else if (enable_state_change)
          state_idle <= p1_state_idle;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          state_busy <= 0;
      else if (enable_state_change)
          state_busy <= p1_state_busy;
    end


  assign pending_upstream_read = pending_upstream_read_reg;
  assign pending_upstream_write = pending_upstream_write_reg & ~upstream_burstdone;
  assign pending_register_enable = state_idle | ((upstream_read | upstream_write) & ~upstream_waitrequest);
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          registered_upstream_read <= 0;
      else if (pending_register_enable)
          registered_upstream_read <= upstream_read;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          registered_upstream_write <= 0;
      else if (pending_register_enable)
          registered_upstream_write <= upstream_write;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          registered_upstream_burstcount <= 0;
      else if (pending_register_enable)
          registered_upstream_burstcount <= upstream_burstcount;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          registered_upstream_address <= 0;
      else if (pending_register_enable)
          registered_upstream_address <= upstream_address;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          registered_upstream_nativeaddress <= 0;
      else if (pending_register_enable)
          registered_upstream_nativeaddress <= upstream_nativeaddress;
    end


  assign current_upstream_read = registered_upstream_read & !downstream_write;
  assign current_upstream_write = registered_upstream_write;
  assign current_upstream_address = registered_upstream_address;
  assign current_upstream_burstcount = pending_register_enable ? upstream_burstcount : registered_upstream_burstcount;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          atomic_counter <= 0;
      else if ((downstream_read | downstream_write) & ~downstream_waitrequest)
          atomic_counter <= downstream_burstdone ? 0 : p1_atomic_counter;
    end


  assign read_update_count = current_upstream_read & ~downstream_waitrequest;
  assign write_update_count = current_upstream_write & downstream_write & downstream_burstdone;
  assign update_count = read_update_count | write_update_count;
  assign transactions_remaining = (state_idle & (upstream_read | upstream_write)) ? dbs_adjusted_upstream_burstcount : transactions_remaining_reg;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          transactions_remaining_reg <= 0;
      else 
        transactions_remaining_reg <= (state_idle & (upstream_read | upstream_write)) ? dbs_adjusted_upstream_burstcount : update_count ? transactions_remaining_reg - downstream_burstcount : transactions_remaining_reg;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          data_counter <= 0;
      else 
        data_counter <= state_idle & upstream_read & ~upstream_waitrequest ?  dbs_adjusted_upstream_burstcount : downstream_readdatavalid ? data_counter - 1 : data_counter;
    end


  assign max_burst_size = 1;
  assign downstream_burstcount = current_upstream_read ? (((transactions_remaining > max_burst_size) ? max_burst_size : transactions_remaining)) : 1;
  assign downstream_arbitrationshare = current_upstream_read ? (dbs_adjusted_upstream_burstcount) : dbs_adjusted_upstream_burstcount;
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          write_address_offset <= 0;
      else 
        write_address_offset <= state_idle & upstream_write ? 0 : ((downstream_write & ~downstream_waitrequest & downstream_burstdone & |downstream_byteenable[15 : 12])) ? write_address_offset + downstream_burstcount : write_address_offset;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          read_address_offset <= 0;
      else 
        read_address_offset <= state_idle & upstream_read ? 0 : (downstream_read & ~downstream_waitrequest) ? read_address_offset + downstream_burstcount : read_address_offset;
    end


  assign downstream_nativeaddress = registered_upstream_nativeaddress >> 2;
  assign address_offset = current_upstream_read ? read_address_offset : write_address_offset;
  assign downstream_address_base = current_upstream_address;
  assign downstream_address = downstream_address_base + {address_offset, 4'b0000};
  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          downstream_read <= 0;
      else if (~downstream_read | ~downstream_waitrequest)
          downstream_read <= state_idle & upstream_read ? 1 : (transactions_remaining == downstream_burstcount) ? 0 : downstream_read;
    end


  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          negative_dbs_rdv_counter <= 0;
      else 
        negative_dbs_rdv_counter <= (state_idle & upstream_read & ~upstream_waitrequest) ? upstream_address[3 : 2] : fifo_datavalid ? negative_dbs_rdv_counter + 1 : negative_dbs_rdv_counter;
    end


  assign fifo_read = ~fifo_empty && ((negative_dbs_rdv_counter == 3) || ((full_width_rdv_counter + 1) == current_upstream_burstcount));
  assign fifo_write = downstream_readdatavalid;
  assign fifo_wr_data = downstream_readdata;
  assign flush_fifo = 1'b0;
  tiger_burst_8_fifo_module the_tiger_burst_8_fifo_module
    (
      .clk              (clk),
      .clk_en           (1'b1),
      .fifo_datavalid   (fifo_datavalid),
      .fifo_empty       (fifo_empty),
      .fifo_full        (fifo_full),
      .fifo_rd_data     (fifo_rd_data),
      .fifo_read        (fifo_read),
      .fifo_wr_data     (fifo_wr_data),
      .fifo_write       (fifo_write),
      .flush_fifo       (flush_fifo),
      .inc_pending_data (fifo_write),
      .p1_fifo_empty    (p1_fifo_empty),
      .reset_n          (reset_n)
    );

  always @(posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
          full_width_rdv_counter <= 0;
      else 
        full_width_rdv_counter <= (state_idle & upstream_read & ~upstream_waitrequest) ? 0 : upstream_readdatavalid ? full_width_rdv_counter + 1 : full_width_rdv_counter;
    end


  assign upstream_readdatavalid = fifo_datavalid;
  assign upstream_readdata = ((0 == negative_dbs_rdv_counter))? {fifo_rd_data[31 : 0], fifo_rd_data[31 : 0], fifo_rd_data[31 : 0], fifo_rd_data[31 : 0]} :
    ((1 == negative_dbs_rdv_counter))? {fifo_rd_data[63 : 32], fifo_rd_data[63 : 32], fifo_rd_data[63 : 32], fifo_rd_data[63 : 32]} :
    ((2 == negative_dbs_rdv_counter))? {fifo_rd_data[95 : 64], fifo_rd_data[95 : 64], fifo_rd_data[95 : 64], fifo_rd_data[95 : 64]} :
    {fifo_rd_data[127 : 96], fifo_rd_data[127 : 96], fifo_rd_data[127 : 96], fifo_rd_data[127 : 96]};

  assign downstream_byteenable = upstream_byteenable;
  assign downstream_write = upstream_write & state_busy & !pending_upstream_read & fifo_empty;
  assign downstream_writedata = upstream_writedata;
  assign upstream_read_run = state_idle & upstream_read;
  assign upstream_write_run = state_busy & upstream_write & ~downstream_waitrequest & !downstream_read;
  assign upstream_waitrequest = (upstream_read | current_upstream_read) ? ~upstream_read_run : current_upstream_write ? ~upstream_write_run : 1;
  assign downstream_debugaccess = upstream_debugaccess;

//synthesis translate_off
//////////////// SIMULATION-ONLY CONTENTS
  always @(posedge clk)
    begin
      if (fifo_full && fifo_write)
        begin
          $write("%0d ns: simulation assertion failed: tiger_burst_8: illegal write into full fifo.", $time);
          $stop;
        end
    end



//////////////// END SIMULATION-ONLY CONTENTS

//synthesis translate_on

endmodule

