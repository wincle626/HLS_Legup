  //Example instantiation for system 'tiger'
  tiger tiger_inst
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

