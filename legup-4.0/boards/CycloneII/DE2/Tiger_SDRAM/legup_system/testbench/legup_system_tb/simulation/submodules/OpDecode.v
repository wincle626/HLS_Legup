// synthesis translate_off
`timescale 1ps / 1ps
// synthesis translate_on

module OpDecode (
	input clk,
	input reset,

	input [31:0] instr_in_od,
	input insValid_in_od,
	input [31:0] pc_in_od,
	input cnt_inc_in_od,
	
	output reg [31:0] pc_od_ah,
	output reg call_od_ah,
	output reg retn_od_ah,
	output reg cnt_inc_od_ah
);
	
	reg [31:0] pc_r;
	reg call_r, call_rr;
	reg retn_r, retn_rr;
	reg cnt_inc_r;
	reg pc_within_8;
	
	wire jalr = insValid_in_od & (instr_in_od[31:26] == 6'b0 & instr_in_od[20:16] == 5'b0 & instr_in_od[10:0] == 11'b000_0000_1001);
	wire jal  = insValid_in_od & (instr_in_od[31:26] == 6'b00_0011);
	wire jr31 = insValid_in_od & (instr_in_od[31: 0] == 32'b0000_0011_1110_0000_0000_0000_0000_1000);	// $s = $ra = 31
	
	always @ (posedge clk)
	if (reset) begin
	// 1st pipeline
		cnt_inc_r  <= 1'b0;
		pc_r       <= 32'b0;
		pc_within_8<= 1'b1;	// (pc_within_8==0) ==> pc_r contains the jumped pc (target pc)
							//	Since there is always a delay slot after a call/retn instruction,
							//		if pc_within_8, it means the jumping is not yet perform.		
		call_r     <= 1'b0;
		call_rr    <= 1'b0;
		retn_rr    <= 1'b0;
		retn_r     <= 1'b0;

	// 2nd pipeline - output
		cnt_inc_od_ah<= 1'b0;
		pc_od_ah     <= 32'b0;
		call_od_ah   <= 1'b0;
		retn_od_ah   <= 1'b0;
	end
	else begin
	// 1st pipeline
		cnt_inc_r  <= cnt_inc_in_od;
		pc_r       <= pc_in_od;
		pc_within_8<= (call_rr|retn_rr) ? (pc_in_od == pc_r) | (pc_in_od == pc_r+32'h4 ) : 1'b1;
			// (pc_within_8==0) ==> pc_r contains the jumped(target) pc; pc_within_8 is assigned to 1 if no jalr|jal|jr31 instruction is detected

		call_r     <= (jalr|jal);
		retn_r     <= jr31;
		call_rr    <= (call_rr & pc_within_8) ? 1'b1 : call_r;	// call_rr stays at 1 if target pc is not yet detected
		retn_rr    <= (retn_rr & pc_within_8) ? 1'b1 : retn_r;	// retn_rr stays at 1 if target pc is not yet detected

	// 2nd pipeline - output
		cnt_inc_od_ah<= cnt_inc_r;
		pc_od_ah     <= pc_r;
		call_od_ah   <= call_rr & ~pc_within_8;
		retn_od_ah   <= retn_rr & ~pc_within_8;
	end
	
	// synthesis translate off
	reg [31:0] instr_r, instr_diff;
	always @ (posedge clk) begin
		instr_r <= instr_in_od;                // aligned with call_r | retn_r
		instr_diff <= instr_r ^ instr_in_od;   // aligned with call_r | retn_r ===> if (instr_diff==1) means instr_r contains a new instruction
		if (pc_within_8 & (call_rr | retn_rr)  & (call_r | retn_r) & instr_diff) begin    // instr_diff helps to ignore the case that the same jumping instruction lasts for multiple cycles
			$display ("Warning: A jumping instruction is detected when the previous jumping is not yet performed @ %d", $time);
		//	$stop();
		end
	end
	// synthesis translate on
endmodule
