module pcie_tutorial(
  input OSC_50_BANK2,
  input PCIE_PREST_n,
  input PCIE_REFCLK_p,
  input [3:0] PCIE_RX_p,
  output [3:0] PCIE_TX_p,
  output [15:0] M1_DDR2_addr,
	output [2:0]  M1_DDR2_ba,
	output [1:0]  M1_DDR2_clk,
	output [1:0]  M1_DDR2_clk_n,
	output [1:0]  M1_DDR2_cke,
	output [1:0]  M1_DDR2_cs_n,
	output [7:0]  M1_DDR2_dm,
	output [0:0]  M1_DDR2_ras_n,
	output [0:0]  M1_DDR2_cas_n,
	output [0:0]  M1_DDR2_we_n,
	inout [63:0]  M1_DDR2_dq,
	inout [7:0]   M1_DDR2_dqs,
	inout [7:0]   M1_DDR2_dqsn,
	output [1:0]  M1_DDR2_odt,
	input rdn,
	input rup
);

wire clk50, clk125;
my_pll pll_inst(
  .inclk0(OSC_50_BANK2),
  .c0(clk50),
  .c1(clk125)
);

qsys_system system_inst(
  .clk_clk(clk50),
  .pcie_ip_refclk_export(PCIE_REFCLK_p),
  .pcie_ip_fixedclk_clk(clk125),
  .reset_reset_n(1'b1),
  .pcie_ip_pcie_rstn_export(PCIE_PREST_n),
  .pcie_ip_rx_in_rx_datain_0(PCIE_RX_p[0]),
  .pcie_ip_rx_in_rx_datain_1(PCIE_RX_p[1]),
  .pcie_ip_rx_in_rx_datain_2(PCIE_RX_p[2]),
  .pcie_ip_rx_in_rx_datain_3(PCIE_RX_p[3]),
  .pcie_ip_tx_out_tx_dataout_0(PCIE_TX_p[0]),
  .pcie_ip_tx_out_tx_dataout_1(PCIE_TX_p[1]),
  .pcie_ip_tx_out_tx_dataout_2(PCIE_TX_p[2]),
  .pcie_ip_tx_out_tx_dataout_3(PCIE_TX_p[3]),
  .memory_mem_a(M1_DDR2_addr),
				.memory_mem_ba(M1_DDR2_ba),
				.memory_mem_ck(M1_DDR2_clk),
				.memory_mem_ck_n(M1_DDR2_clk_n),
				.memory_mem_cke(M1_DDR2_cke),
				.memory_mem_cs_n(M1_DDR2_cs_n),
				.memory_mem_dm(M1_DDR2_dm),
				.memory_mem_ras_n(M1_DDR2_ras_n),
				.memory_mem_cas_n(M1_DDR2_cas_n),
				.memory_mem_we_n(M1_DDR2_we_n),
				.memory_mem_dq(M1_DDR2_dq),
				.memory_mem_dqs(M1_DDR2_dqs),
				.memory_mem_dqs_n(M1_DDR2_dqsn),
				.memory_mem_odt(M1_DDR2_odt),
				.oct_rdn(rdn),
				.oct_rup(rup)
);

endmodule
