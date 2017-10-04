int board_id = DE4; //DE4 by default
struct board_parameters boards[NUM_SUPPORTED_BOARDS];

void acl_init_de4_parameters(void) {
  boards[DE4].board_name = "de4";
  boards[DE4].acl_pci_global_mem_bar = 0;
  boards[DE4].acl_pci_cra_bar = 2;
  boards[DE4].acl_pci_cra_size = 0x4000;
  boards[DE4].acl_kernel_csr_bar = 2;
  boards[DE4].acl_kernel_csr_offset = 0x4000;
  boards[DE4].acl_kernel_option0_csr_size = 0x10;
  boards[DE4].acl_configuration_storage_bar = 2;
  boards[DE4].acl_configuration_storage_offset = 0x5000;
  boards[DE4].acl_pcie_dma_bar = 2;
  boards[DE4].acl_pcie_dma_offset = 0x20006000;
  boards[DE4].acl_pcie_dma_descriptor_bar = 2;
  boards[DE4].acl_pcie_dma_descriptor_offset = 0x20006020;
  boards[DE4].acl_pcie_pio_in_bar = 2;
  boards[DE4].acl_pcie_pio_in_offset = 0x20006080;
  boards[DE4].acl_pcie_pio_out_bar = 2;
  boards[DE4].acl_pcie_pio_out_offset = 0x20006100;
  boards[DE4].acl_pcie_tx_port = 0x80000000;
  boards[DE4].acl_pcie_memwindow_bar = 2;
  boards[DE4].acl_pcie_memwindow_cra = 0x20006030;
  boards[DE4].acl_pcie_memwindow_base = 0x00000000;
  boards[DE4].acl_pcie_memwindow_size = 0x20000000;
  boards[DE4].acl_pcie_kernwindow_bar = 2;
  boards[DE4].acl_pcie_kernwindow_cra = 0x20006038;
  boards[DE4].acl_pcie_kernwindow_base = 0x00000000;
  boards[DE4].acl_pcie_kernwindow_size = 0x1000;
  boards[DE4].acl_pcie_em_bar = 2;
  boards[DE4].acl_pcie_em_offset = 0x20008000;
  boards[DE4].acl_pcie_em2_bar = 2;
  boards[DE4].acl_pcie_em2_offset = 0x2000c000;
  //byte offsets
  boards[DE4].kernel_offset_csr = 0x00;
  boards[DE4].kernel_offset_printf_buffer_size = 0x04;
  boards[DE4].kernel_offset_invocation_image = 0x0c;
  boards[DE4].kernel_csr_go = 0;
  boards[DE4].kernel_csr_done = 1;
  boards[DE4].kernel_csr_stalled = 3;
  boards[DE4].kernel_csr_unstall = 4;
  boards[DE4].kernel_csr_last_status_bit = boards[DE4].kernel_csr_unstall;
  boards[DE4].kernel_csr_status_bits_mask =(unsigned) (( 1<< (boards[DE4].kernel_csr_last_status_bit+1) )-1);
  boards[DE4].kernel_csr_lmem_invalid_bank = 11;
  boards[DE4].kernel_csr_lsu_active = 12;
  boards[DE4].kernel_csr_wr_active = 13;
  boards[DE4].kernel_csr_valid_in = 14;
  boards[DE4].kernel_csr_started = 15;
  boards[DE4].kernel_csr_first_version_bit = 16;
  boards[DE4].kernel_csr_last_version_bit = 31;
  boards[DE4].pcie_cra_irq_status = 0x0040;
  boards[DE4].pcie_cra_irq_enable = 0x0050;
  boards[DE4].pcie_cra_addr_trans = 0x1000;
  boards[DE4].acl_pcie_kernel_irq_vec = 0;
  boards[DE4].acl_pcie_dma_irq_vec = 1;
  boards[DE4].use_kernelpll_reconfig = 0;
  boards[DE4].acl_pcie_kernelpll_reconfig_bar = 0;
  boards[DE4].acl_pcie_kernelpll_reconfig_offset = 0;
  boards[DE4].acl_pcie_kernelpll_rom_bar = 0;
  boards[DE4].acl_pcie_kernelpll_rom_offset = 0;
  boards[DE4].acl_pcie_counter_bar = 0;
  boards[DE4].acl_pcie_counter_offset = 0;
#ifndef QSYS_IFACE
  boards[DE4].pcie_cra_irq_rxmirq = 7;
  boards[DE4].pcie_cra_avl_irq_vec_lo = 8;
  boards[DE4].pcie_cra_avl_irq_vec_hi = 13;
#endif
  boards[DE4].dma_alignment_bytes = 32;
  boards[DE4].dma_dc_transfer_complete_irq_mask = 14;
  boards[DE4].dma_dc_early_done_enable = 24;
  boards[DE4].dma_dc_go = 31;
  boards[DE4].dma_csr_status = 0x00;
  boards[DE4].dma_csr_control = 0x04;
  boards[DE4].dma_status_busy = 0;
  boards[DE4].dma_status_descriptor_empty = 1;
  boards[DE4].dma_status_resetting = 6;
  boards[DE4].dma_status_irq = 9;
  boards[DE4].dma_status_count_lo = 16;
  boards[DE4].dma_status_count_hi = 31;
  boards[DE4].dma_ctrl_stop = 0;
  boards[DE4].dma_ctrl_reset = 1;
  boards[DE4].dma_ctrl_irq_enable = 4;
  boards[DE4].perfmon_status_reg = 0x00*4;
  boards[DE4].perfmon_build_number = 0x01*4;
  boards[DE4].perfmon_perf_registers = 0x02*4;
  boards[DE4].perfmon_log_start_reg = 0x03*4;
  boards[DE4].perfmon_log_end_reg = 0x04*4;
  boards[DE4].perfmon_log_registers = 0x05*4;
  boards[DE4].perfmon_trigger_fifo_register = 0x06*4;
  boards[DE4].perfmon_enable_monitoring = 0x01;
  boards[DE4].perfmon_enable_logging = 0x02;
  boards[DE4].perfmon_buffer_is_empty = 0x04;
  boards[DE4].perfmon_reset = 0x10;
  boards[DE4].perfmon_trigger_log_on_fifo_activity = 0x20;
  boards[DE4].pio_data = 0*4;
  boards[DE4].pio_set = 4*4;
  boards[DE4].pio_clr = 5*4;
  boards[DE4].pio_out_swreset = 31;
  boards[DE4].pio_out_pllreset = 30;
  boards[DE4].pio_out_interleave_mode = 8;
  boards[DE4].pio_in_kpll_locked = 8;
  boards[DE4].pio_in_a_init_done = 0;
  boards[DE4].pio_in_a_cal_success = 1;
  boards[DE4].pio_in_b_init_done = 2;
  boards[DE4].pio_in_b_cal_success = 3;
  boards[DE4].has_temp_sensor = 0;
  boards[DE4].temp_sensor_addr = 0x0000;  // No sensor
}

void acl_init_de4_expcard_parameters(void) {
  boards[DE4_EXPCARD] = boards[DE4];
  boards[DE4_EXPCARD].board_name = "ede4";
  boards[DE4_EXPCARD].acl_pcie_dma_offset = 0x4006000;
  boards[DE4_EXPCARD].acl_pcie_dma_descriptor_offset = 0x4006020;
  boards[DE4_EXPCARD].acl_pcie_pio_in_offset = 0x4006080;
  boards[DE4_EXPCARD].acl_pcie_pio_out_offset = 0x4006100;
  boards[DE4_EXPCARD].acl_pcie_memwindow_cra = 0x4006030;
  boards[DE4_EXPCARD].acl_pcie_memwindow_size = 0x4000000;
  boards[DE4_EXPCARD].acl_pcie_kernwindow_cra = 0x4006038;
  boards[DE4_EXPCARD].acl_pcie_em_offset = 0x4008000;
  boards[DE4_EXPCARD].acl_pcie_em2_bar = 2;
  boards[DE4_EXPCARD].acl_pcie_em2_offset = 0x400c000;
}

void acl_init_pcie385_parameters(void) {
  boards[PCIE385] = boards[DE4];
  boards[PCIE385].board_name = "e385";
  boards[PCIE385].acl_pcie_pio_in_offset = 0x4006100; 
  boards[PCIE385].acl_pcie_pio_out_offset = 0x4006200; 
  boards[PCIE385].use_kernelpll_reconfig = 1;
  boards[PCIE385].acl_pcie_kernelpll_reconfig_bar = 2;
  boards[PCIE385].acl_pcie_kernelpll_reconfig_offset = 0x4006800;
  boards[PCIE385].acl_pcie_kernelpll_rom_bar = 2;
  boards[PCIE385].acl_pcie_kernelpll_rom_offset = 0x4006400;
  boards[PCIE385].acl_pcie_counter_bar = 2;
  boards[PCIE385].acl_pcie_counter_offset = 0x4006080;
  boards[PCIE385].dma_alignment_bytes = 64;
  boards[PCIE385].acl_pcie_dma_offset = 0x4006000;
  boards[PCIE385].acl_pcie_dma_descriptor_offset = 0x4006020;
  boards[PCIE385].acl_pcie_memwindow_cra = 0x4006030;
  boards[PCIE385].acl_pcie_memwindow_size = 0x4000000;
  boards[PCIE385].acl_pcie_kernwindow_cra = 0x4006038;
  boards[PCIE385].acl_pcie_em_offset = 0x4008000;
  boards[PCIE385].acl_pcie_em2_bar = 2;
  boards[PCIE385].acl_pcie_em2_offset = 0x400c000;
  boards[PCIE385].has_temp_sensor = 1;
  boards[PCIE385].temp_sensor_addr = 0x6040;
}

void acl_init_bsp_parameters(void) {
  boards[BSP] = boards[PCIE385];
  boards[BSP].board_name = "bsp";
  boards[BSP].acl_pci_global_mem_bar = 0;
  boards[BSP].acl_pci_cra_offset = 0;
  boards[BSP].acl_pci_cra_bar = 0;
  boards[BSP].acl_pci_cra_size = 0x4000;
  boards[BSP].acl_kernel_csr_bar = 0;
  boards[BSP].acl_kernel_csr_offset = 0x4000;
  boards[BSP].acl_configuration_storage_bar = 0;
  boards[BSP].acl_configuration_storage_offset = 0x6000;
  boards[BSP].acl_pcie_dma_bar = 0;
  boards[BSP].acl_pcie_dma_offset = 0x0c800;
  boards[BSP].acl_pcie_dma_descriptor_bar = 0;
  boards[BSP].acl_pcie_dma_descriptor_offset = 0x0c820;
  boards[BSP].acl_pcie_pio_in_bar = 0;
  boards[BSP].acl_pcie_pio_in_offset = 0x20006080;
  boards[BSP].acl_pcie_pio_out_bar = 0;
  boards[BSP].acl_pcie_pio_out_offset = 0x20006100;
  boards[BSP].acl_pcie_tx_port = 0x80000000;
  boards[BSP].acl_pcie_memwindow_bar = 0;
  boards[BSP].acl_pcie_memwindow_cra = 0x0c870;
  boards[BSP].acl_pcie_memwindow_base = 0x10000;
  boards[BSP].acl_pcie_memwindow_size = 0x10000;
  boards[BSP].acl_pcie_kernwindow_bar = 0;
  boards[BSP].acl_pcie_kernwindow_cra = 0x4100;
  boards[BSP].acl_pcie_kernwindow_base = 0x4000;
  boards[BSP].acl_pcie_kernwindow_size = 0x1000;
  boards[BSP].acl_pcie_em_bar = 0;
  boards[BSP].acl_pcie_em_offset = 0x20000;
  boards[BSP].acl_pcie_em2_bar = 0;
  boards[BSP].acl_pcie_em2_offset = 0x24000;
  boards[BSP].acl_pcie_kernel_irq_vec = 0;
  boards[BSP].acl_pcie_dma_irq_vec = 1;
  boards[BSP].use_kernelpll_reconfig = 1;
  boards[BSP].acl_pcie_kernelpll_reconfig_bar = 0;
  boards[BSP].acl_pcie_kernelpll_reconfig_offset = 0x0c000;
  boards[BSP].acl_pcie_kernelpll_rom_bar = 0;
  boards[BSP].acl_pcie_kernelpll_rom_offset = 0x0c400;
  boards[BSP].acl_pcie_counter_bar = 0;
  boards[BSP].acl_pcie_counter_offset = 0x0c100;
  boards[BSP].dma_alignment_bytes = 64;
  boards[BSP].pio_data = 0*4;
  boards[BSP].pio_set = 4*4;
  boards[BSP].pio_clr = 5*4;
  boards[BSP].pio_out_swreset = 31;
  boards[BSP].pio_out_pllreset = 30;
  boards[BSP].pio_out_interleave_mode = 8;
  boards[BSP].pio_in_kpll_locked = 8;
  boards[BSP].pio_in_a_init_done = 0;
  boards[BSP].pio_in_a_cal_success = 1;
  boards[BSP].pio_in_b_init_done = 0;
  boards[BSP].pio_in_b_cal_success = 3;
  boards[BSP].has_temp_sensor = 0;
  boards[BSP].temp_sensor_addr = 0x0000;  // No sensor
}

void acl_init_c5dk_parameters(void) {
  boards[C5DK] = boards[PCIE385];
  boards[C5DK].board_name = "c5dk";
  
  boards[DE4].acl_pci_global_mem_bar = 1;
  boards[DE4].acl_pci_cra_bar = 1;
  boards[DE4].acl_pci_cra_size = 0x4000;
  boards[DE4].acl_kernel_csr_bar = 1;
  boards[DE4].acl_kernel_csr_offset = 0x4000;
  
  boards[C5DK].acl_pcie_pio_in_offset = 0x4006100; 
  boards[C5DK].acl_pcie_pio_out_offset = 0x4006200; 
  boards[C5DK].use_kernelpll_reconfig = 1;
  boards[C5DK].acl_pcie_kernelpll_reconfig_bar = 0;
  boards[C5DK].acl_pcie_kernelpll_reconfig_offset = 0x4006800;
  boards[C5DK].acl_pcie_kernelpll_rom_bar = 0;
  boards[C5DK].acl_pcie_kernelpll_rom_offset = 0x4006400;
  boards[C5DK].acl_pcie_counter_bar = 0;
  boards[C5DK].acl_pcie_counter_offset = 0x4006080;
  boards[C5DK].dma_alignment_bytes = 64;
  boards[C5DK].acl_pcie_dma_offset = 0x4006000;
  boards[C5DK].acl_pcie_dma_descriptor_offset = 0x4006020;
  boards[C5DK].acl_pcie_memwindow_cra = 0x4006030;
  boards[C5DK].acl_pcie_memwindow_size = 0x4000000;
  boards[C5DK].acl_pcie_kernwindow_cra = 0x4006038;
  boards[C5DK].acl_pcie_em_offset = 0x4008000;
  boards[C5DK].acl_pcie_em2_bar = 0;
  boards[C5DK].acl_pcie_em2_offset = 0x400c000;
  boards[C5DK].has_temp_sensor = 0;
  boards[C5DK].temp_sensor_addr = 0x0;
}


void acl_init_board_parameters(void) {
  acl_init_de4_parameters();
  acl_init_de4_expcard_parameters();
  acl_init_pcie385_parameters();
  acl_init_bsp_parameters();
  acl_init_c5dk_parameters();
}


void acl_set_board_id_based_on_device_id(int device_id) {
  if (device_id == ACL_PCI_DE4_DEVICE_ID) {
    board_id = DE4;
  } else if (device_id == ACL_PCI_DE4_EXPCARD_DEVICE_ID) {
    board_id = DE4_EXPCARD;
  } else if (device_id == ACL_PCI_PCIE385_DEVICE_ID) {
    board_id = PCIE385;
  } else if (device_id == ACL_PCI_BSP_DEVICE_ID) {
    board_id = BSP;
  } else if (device_id == ACL_SOC_C5_DK) {
    board_id = C5DK;
  } else {
    board_id = -1;
  }
}
