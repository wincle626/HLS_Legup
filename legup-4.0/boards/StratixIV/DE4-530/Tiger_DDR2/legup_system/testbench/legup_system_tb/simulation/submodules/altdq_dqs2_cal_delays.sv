// (C) 2001-2013 Altera Corporation. All rights reserved.
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


`timescale 1 ps / 1 ps


`ifdef ARRIAIIGX
`define DQS_CONFIG arriaiigx_dqs_config
`define IO_CONFIG arriaiigx_io_config
`endif



`define DQS_CONFIG stratixiv_dqs_config
`define IO_CONFIG stratixiv_io_config



module altdq_dqs2_cal_delays #(

        parameter CLOCK_FREQ = "",
        parameter PIN_WIDTH = 1,
        parameter EXTRA_OUTPUT_WIDTH = 1,
	parameter DEGREES_PER_PHASE_TAP = "",
	parameter DELAY_WIDTH = 32,
	parameter DLL_USE_2X_CLK = 0
		    
) (
	input wire config_data_in,
	input wire config_update,
	input wire config_dqs_ena,
	input wire [PIN_WIDTH-1:0] config_io_ena,
	input wire [EXTRA_OUTPUT_WIDTH-1:0] config_extra_io_ena,
	input wire config_dqs_io_ena,
	input wire config_clock_in,

	output wire [DELAY_WIDTH-1:0] opa_clock_delay,
	output wire [DELAY_WIDTH-1:0] dqs_in_busout_delay,
	output wire [DELAY_WIDTH-1:0] dqs_in_enable_on_delay,
	output wire [DELAY_WIDTH-1:0] dqs_in_enable_off_delay,
	output wire [DELAY_WIDTH-1:0] dqs_out_ptap_delay,
	output wire [DELAY_WIDTH-1:0] dqs_out_dtap_delay,
	output wire [DELAY_WIDTH-1:0] dq_out_ptap_delay,
	output wire [(PIN_WIDTH*DELAY_WIDTH)-1:0] dq_out_dtap_delay,
	output wire [(PIN_WIDTH*DELAY_WIDTH)-1:0] dq_in_dtap_delay,
	output wire [(PIN_WIDTH*DELAY_WIDTH)-1:0] extra_out_dtap_delay

);


localparam DELAYED_CLOCK_TAP = 2;

localparam EXTRA_OUTPUT_WIDTH_LOCAL = EXTRA_OUTPUT_WIDTH > 0 ? EXTRA_OUTPUT_WIDTH : 1;



function integer phase_to_ps;
	input integer clk_rate;
	input integer deg;
	phase_to_ps = deg * (1000000) / clk_rate / (360);
endfunction

function integer phasetap_to_ps;
	input integer clk_rate;
	input integer tap;
	begin
		phasetap_to_ps = phase_to_ps(clk_rate, tap * DEGREES_PER_PHASE_TAP);
	end
endfunction

function integer opa_to_ps;
	input integer clk_rate;
	input integer phase_offset;
	input integer cycle_delay;
	input integer phase_transfer;
	input integer phase_invert;
	begin
		integer phase;
		integer src;
		integer dst1;
		integer dst2;
		integer ps;
		src = ((DELAYED_CLOCK_TAP*DEGREES_PER_PHASE_TAP) + (phase_transfer*180)) % 360;
		dst1 = ((phase_offset*DEGREES_PER_PHASE_TAP) + (phase_invert*180)) % 360;
		if (src >= dst1)
			dst2 = dst1 + 360;
		else
			dst2 = dst1;
		phase = dst2 + (cycle_delay*360) - (DELAYED_CLOCK_TAP*DEGREES_PER_PHASE_TAP);
		ps = phase_to_ps(clk_rate, phase);
		opa_to_ps = ps;
	end
endfunction

function integer ena_to_ps;
	input integer clk_rate;
	input integer phase_offset;
	input integer phase_transfer;
	input integer phase_invert;
	begin
		integer src1;
		integer src2;
		integer dst1;
		integer dst2;
		integer phase;
		integer ps;
		integer extra_cycles;

		extra_cycles = 0;
		src1 = 0;	  
		dst1 = ((phase_offset*DEGREES_PER_PHASE_TAP) + (phase_invert*180) + (phase_transfer*180)) % 360; 
		if (phase_transfer)
		begin
			src2 = dst1;
			if (src1 >= dst1) 
				extra_cycles = extra_cycles + 1;
		end
		else
		begin
			src2 = src1;
		end
		dst2 = ((phase_offset*DEGREES_PER_PHASE_TAP) + (phase_invert*180)) % 360;
		if (src2 >= dst2)
			extra_cycles = extra_cycles + 1;
		phase = dst2 + extra_cycles*360;
		ps = phase_to_ps(clk_rate, phase);
		ena_to_ps = ps;
	end
endfunction

localparam DTAP_DELAY = 50;

function integer dtap_to_ps;
	input integer dtap;
	begin
		dtap_to_ps = dtap * DTAP_DELAY;
	end
endfunction


      
wire dqoutputphaseinvert;
wire [3:0] dqoutputphasesetting;
wire [3:0] dqsbusoutdelaysetting;
wire [3:0] dqsoutputphasesetting;
wire [3:0] resyncinputphasesetting;
wire [2:0] dqsenabledelaysetting;
wire [2:0] dqsinputphasesetting;
wire [3:0] dqsenablectrlphasesetting;
wire dqsbusoutfinedelaysetting;
wire dqsenablectrlphaseinvert;
wire dqsenablefinedelaysetting;
wire dqsoutputphaseinvert;
wire enadataoutbypass;
wire enadqsenablephasetransferreg;
wire enaoctphasetransferreg;
wire enaoutputphasetransferreg;
wire enaoctcycledelaysetting;	
wire enaoutputcycledelaysetting; 

wire [3:0] dqs_outputdelaysetting1;
wire [2:0] dqs_outputdelaysetting2;

wire [3:0] dq_outputdelaysetting1[PIN_WIDTH];
wire [2:0] dq_outputdelaysetting2[PIN_WIDTH];
wire [3:0] dq_inputdelaysetting[PIN_WIDTH];

wire [3:0] extra_outputdelaysetting1[EXTRA_OUTPUT_WIDTH_LOCAL];
wire [2:0] extra_outputdelaysetting2[EXTRA_OUTPUT_WIDTH_LOCAL];
wire [3:0] extra_inputdelaysetting[EXTRA_OUTPUT_WIDTH_LOCAL];


assign opa_clock_delay = phasetap_to_ps(CLOCK_FREQ, DELAYED_CLOCK_TAP);

`IO_CONFIG dqs_io_config_inst (
	.datain(config_data_in),  
	.clk(config_clock_in),
	.ena(config_dqs_io_ena),
	.update(config_update),  
	.outputdelaysetting1(dqs_outputdelaysetting1),
	.outputdelaysetting2(dqs_outputdelaysetting2)
	);

assign dqs_out_dtap_delay = dtap_to_ps(dqs_outputdelaysetting1 + dqs_outputdelaysetting2);


`DQS_CONFIG dqs_config_inst ( 
	.clk(config_clock_in),
	.datain(config_data_in),
	.dataout(),
	.ena(config_dqs_ena),
	.update(config_update),

	.dqoutputphaseinvert(dqoutputphaseinvert), 
	.dqoutputphasesetting(dqoutputphasesetting), 
 	.dqsbusoutdelaysetting(dqsbusoutdelaysetting), 
	.dqsenablectrlphaseinvert(dqsenablectrlphaseinvert), 
	.dqsenablectrlphasesetting(dqsenablectrlphasesetting), 
	.dqsenabledelaysetting(dqsenabledelaysetting), 
	.dqsinputphasesetting(dqsinputphasesetting), 
	.dqsoutputphaseinvert(dqsoutputphaseinvert), 
	.dqsoutputphasesetting(dqsoutputphasesetting), 
	.enadqsenablephasetransferreg(enadqsenablephasetransferreg), 
	.enaoctcycledelaysetting(enaoctcycledelaysetting), 
	.enaoctphasetransferreg(enaoctphasetransferreg),
	.enaoutputcycledelaysetting(enaoutputcycledelaysetting),
	.enaoutputphasetransferreg(enaoutputphasetransferreg)
);


assign dqs_in_busout_delay = dtap_to_ps(dqsbusoutdelaysetting) + phase_to_ps(CLOCK_FREQ, 90);
assign dqs_in_enable_off_delay =  ena_to_ps(CLOCK_FREQ, dqsenablectrlphasesetting, 
					enadqsenablephasetransferreg, dqsenablectrlphaseinvert) +
			      dtap_to_ps(dqsenabledelaysetting);
assign dqs_in_enable_on_delay = dqs_in_enable_off_delay + phase_to_ps(CLOCK_FREQ, 180);
assign dqs_out_ptap_delay = opa_to_ps(CLOCK_FREQ, dqsoutputphasesetting, enaoctcycledelaysetting, 
				  enaoctphasetransferreg, dqsoutputphaseinvert);
assign dq_out_ptap_delay = opa_to_ps(CLOCK_FREQ, dqoutputphasesetting, enaoutputcycledelaysetting, 
				 enaoutputphasetransferreg, dqoutputphaseinvert);



generate
genvar pin;
for (pin = 0; pin < PIN_WIDTH; pin++)
begin: data_settings
	`IO_CONFIG dq_config (
		.datain(config_data_in),          
		.clk(config_clock_in),
		.ena(config_io_ena[pin]),
		.update(config_update),       
		.outputdelaysetting1(dq_outputdelaysetting1[pin]),
		.outputdelaysetting2(dq_outputdelaysetting2[pin]),
		.padtoinputregisterdelaysetting(dq_inputdelaysetting[pin])
	);

	assign dq_out_dtap_delay[((pin+1)*DELAY_WIDTH)-1:pin*DELAY_WIDTH] = dtap_to_ps(dq_outputdelaysetting1[pin] + dq_outputdelaysetting2[pin]);
	assign dq_in_dtap_delay[((pin+1)*DELAY_WIDTH)-1:pin*DELAY_WIDTH] = dtap_to_ps(dq_inputdelaysetting[pin]);
end
	
for (pin = 0; pin < EXTRA_OUTPUT_WIDTH; pin++)
begin: extra_settings
	`IO_CONFIG extra_config (
		.datain(config_data_in),          
		.clk(config_clock_in),
		.ena(config_extra_io_ena[pin]),
		.update(config_update),       
		.outputdelaysetting1(extra_outputdelaysetting1[pin]),
		.outputdelaysetting2(extra_outputdelaysetting2[pin]),
		.padtoinputregisterdelaysetting(extra_inputdelaysetting[pin])
	);

	assign extra_out_dtap_delay[((pin+1)*DELAY_WIDTH)-1:pin*DELAY_WIDTH] = dtap_to_ps(extra_outputdelaysetting1[pin] + extra_outputdelaysetting2[pin]);
end
endgenerate


endmodule


