// (C) 2001-2011 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.

module s4gx_qsys_pcie_ddr3(
  input            OSC_50_BANK2,
  input            PCIE_PREST_n,
  input            PCIE_REFCLK_p,
  input            GCLKIN,
  input    [ 3: 0] PCIE_RX_p,
  output   [ 3: 0] PCIE_TX_p,

  output   [13: 0] M1_DDR2_addr,
  output   [ 2: 0] M1_DDR2_ba,
  output   [ 0: 0] M1_DDR2_cas_n,
  output   [ 1: 0] M1_DDR2_cke,
  output   [ 1: 0] M1_DDR2_clk_n,
  output   [ 1: 0] M1_DDR2_clk,
  output   [ 1: 0] M1_DDR2_cs_n,
  output   [ 7: 0] M1_DDR2_dm,
  inout    [63: 0] M1_DDR2_dq,
  inout    [ 7: 0] M1_DDR2_dqs,
  inout    [ 7: 0] M1_DDR2_dqsn,
  output   [ 1: 0] M1_DDR2_odt,
  output   [ 0: 0] M1_DDR2_ras_n,
  output   [ 0: 0] M1_DDR2_we_n,
  input            oct_rup_pad,
  input            oct_rdn_pad,

  input    [ 0: 0] BUTTON,
  output   [ 7: 0] LED
);

    wire   [16: 0] reconfig_fromgxb;
    wire   [ 3: 0] reconfig_togxb;
    wire           busy;


    reg            L0_led;
    reg    [25: 0] alive_cnt;
    reg            alive_led;
    reg            comp_led;
    reg    [ 3: 0] lane_active_led;
    wire   [63: 0] test_out_icm;
    wire   [39: 0] test_in;

    wire clk50;
    wire clk125;
    wire pll_lock;
    wire reset_n;

    assign test_in[39 : 12] = 0;
    assign test_in[11: 8] = 4'b0010;
    assign test_in[7 : 6] = 2'b10;
    assign test_in[5] = 1'b0;
    assign test_in[4 : 0] = 5'b01000;

    assign LED = {lane_active_led, 1'b0, comp_led, alive_led, L0_led};
    assign reset_n = BUTTON[0];

    reconfig_pll reconfig_pll(
        .inclk0(OSC_50_BANK2),
        .c0(clk50),
        .c1(clk125),
        .locked(pll_lock)
    );


    //The ALTGXB Reconfig block is necessary for Stratix IV GX transceiver offset cancellation.
    //Currently, its instantiated outside of Qsys.
    //In future Quartus release, this block will become a standard Qsys component.

    // Please see offset cancelation_reset requirement issue
    // http://www.altera.com/support/kdb/solutions/rd02092011_274.html
    // http://www.altera.com/support/kdb/solutions/rd12172009_309.html
    // offset_cancellation_reset is given by     qmegawiz -silent -wiz_override="offset_cancellation_reset"   <altgx_reconfig filename.v>
    altgx_reconfig altgx_reconfig (
            .offset_cancellation_reset(!pll_lock),
            .reconfig_clk (clk50),
            .reconfig_fromgxb (reconfig_fromgxb),
            .busy (busy),
            .reconfig_togxb (reconfig_togxb)
        );


    q_sys q_sys_inst(
        .reset_n                                           (reset_n),
        .clkin_100                                         (GCLKIN),
        .clkin_50_clk                                      (clk50),
        .clkin_50_clk_in_reset_reset_n                     (reset_n),
 
        .pcie_hard_ip_0_pcie_rstn_export                   (PCIE_PREST_n),
        .pcie_hard_ip_0_reconfig_togxb_data                (reconfig_togxb),
        .pcie_hard_ip_0_reconfig_fromgxb_0_data            (reconfig_fromgxb),
        .pcie_hard_ip_0_fixedclk_clk                       (clk125),
        .pcie_hard_ip_0_rx_in_rx_datain_0                  (PCIE_RX_p[0]),
        .pcie_hard_ip_0_rx_in_rx_datain_1                  (PCIE_RX_p[1]),
        .pcie_hard_ip_0_rx_in_rx_datain_2                  (PCIE_RX_p[2]),
        .pcie_hard_ip_0_rx_in_rx_datain_3                  (PCIE_RX_p[3]),
        .pcie_hard_ip_0_tx_out_tx_dataout_0                (PCIE_TX_p[0]),
        .pcie_hard_ip_0_tx_out_tx_dataout_1                (PCIE_TX_p[1]),
        .pcie_hard_ip_0_tx_out_tx_dataout_2                (PCIE_TX_p[2]),
        .pcie_hard_ip_0_tx_out_tx_dataout_3                (PCIE_TX_p[3]),
        .pcie_hard_ip_0_reconfig_busy_busy_altgxb_reconfig (busy),
        .pcie_hard_ip_0_pipe_ext_pll_powerdown             (~PCIE_PREST_n),
        .pcie_hard_ip_0_pipe_ext_gxb_powerdown             (~PCIE_PREST_n),
        .pcie_hard_ip_0_refclk_export                      (PCIE_REFCLK_p),
        .pcie_hard_ip_0_test_out_test_out                  (test_out_icm),
        .pcie_hard_ip_0_test_in_test_in                    (test_in),
        .memory_mem_dqs_n                                  (M1_DDR2_dqsn),
        .memory_mem_ras_n                                  (M1_DDR2_ras_n),
        .memory_mem_dq                                     (M1_DDR2_dq),
        .memory_mem_dm                                     (M1_DDR2_dm),
        .memory_mem_dqs                                    (M1_DDR2_dqs),
        .memory_mem_cke                                    (M1_DDR2_cke),
        .memory_mem_ck_n                                   (M1_DDR2_clk_n),
        .memory_mem_odt                                    (M1_DDR2_odt),
        .memory_mem_ba                                     (M1_DDR2_ba),
        .memory_mem_cs_n                                   (M1_DDR2_cs_n),
        .memory_mem_cas_n                                  (M1_DDR2_cas_n),
        .memory_mem_we_n                                   (M1_DDR2_we_n),
        .memory_mem_a                                      (M1_DDR2_addr),
        .memory_mem_ck                                     (M1_DDR2_clk),
        .oct_rdn                                           (oct_rup_pad),
        .oct_rup                                           (oct_rdn_pad)
    );

  //LED logic
  always @(posedge clk125 or negedge reset_n)
    begin
      if (reset_n == 0)
        begin
          alive_cnt <= 0;
          alive_led <= 0;
          comp_led <= 0;
          L0_led <= 0;
          lane_active_led <= 0;
        end
      else 
        begin
          alive_cnt <= alive_cnt +1;
          alive_led <= alive_cnt[25];
          comp_led <= ~(test_out_icm[4 : 0] == 5'b00011);
          L0_led <= ~(test_out_icm[4 : 0] == 5'b01111);
          lane_active_led[3 : 0] <= ~(test_out_icm[28 : 25]);
        end
    end

endmodule
