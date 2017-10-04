
module sim_model_mipsI #(
///////////////////////////////////////////////////////////////////////////////
//                                Parameters                                 //
///////////////////////////////////////////////////////////////////////////////
parameter CAPTURE_TRANSACTIONS	= 0,

parameter RF_ADDRESS_FILENAME		= "rf_addresses.dat",
parameter RF_DATA_FILENAME			= "rf_data.dat"

) (
///////////////////////////////////////////////////////////////////////////////
//                                  Ports                                    //
///////////////////////////////////////////////////////////////////////////////
// Inputs
input					clk,
input					reset,
	
input		[31: 0]	avm_im_readdata,
input					avm_im_readdatavalid,
input					avm_im_waitrequest,
	
input		[31: 0]	avm_dm_readdata,
input					avm_dm_readdatavalid,
input					avm_dm_waitrequest,

// Bidirectionals

// Outputs
output	[31: 0]	avm_im_address,
output				avm_im_read,

output	[31: 0]	avm_dm_address,
output	[ 3: 0]	avm_dm_byteenable,
output				avm_dm_read,
output				avm_dm_write,
output	[31: 0]	avm_dm_writedata
);


///////////////////////////////////////////////////////////////////////////////
//                              Global Signals                               //
///////////////////////////////////////////////////////////////////////////////
reg		[31: 0]	reg_file [31:0];
reg		[31: 0]	LO;
reg		[31: 0]	HI;

integer i;


///////////////////////////////////////////////////////////////////////////////
//                               Fetch Stage                                 //
///////////////////////////////////////////////////////////////////////////////
// Instruction register signals
wire		[31: 0]	ir;
wire					ir_valid;
wire					instr_done;

// Control flow signals
wire		[31: 0]	prefetched_address;
wire		[31: 0]	prefetched_ir;
wire					prefetched_valid;

reg		[31: 0]	pc;
reg		[31: 0]	nextpc;
wire		[31: 0]	branchpc;
wire					loadpc;
wire					loadedpc;

SM_MIPSI_Fetch_Unit Fetch_Unit (
	// Inputs
	.clk					(clk),
	.reset				(reset),

	.i_readdata			(avm_im_readdata),
	.i_readdatavalid	(avm_im_readdatavalid),
	.i_waitrequest		(avm_im_waitrequest),

	.branch_address	(branchpc),
	.branch_enable		(loadpc & ir_valid),
	
	.read_ir				(instr_done | ((prefetched_address != pc) & prefetched_valid)),

	// Bidirectionals

	// Outputs
	.i_address			(avm_im_address),
	.i_read				(avm_im_read),

	.branched			(loadedpc),

	.pc					(prefetched_address),
	.ir					(prefetched_ir),
	.ir_valid			(prefetched_valid)
);

initial										pc	<= 32'h0;
always @(posedge clk)
begin
	if (reset)								pc <= 32'h0;
	else if (instr_done & ir_valid)	pc <= nextpc;
end

initial										nextpc <= 32'h4;
always @(posedge clk)
begin
	if (reset)								nextpc <= 32'h4;
	else if (instr_done & loadedpc)	nextpc <= branchpc;
	else if (instr_done & ir_valid)	nextpc <= nextpc + 32'd4;
end

assign ir			= prefetched_ir;
assign ir_valid	= (prefetched_address == pc) ? prefetched_valid : 1'b0;

///////////////////////////////////////////////////////////////////////////////
//                               Decode Stage                                //
///////////////////////////////////////////////////////////////////////////////
// IR data
wire		[ 5: 0]	op 	= ir[31:26];
wire		[ 5: 0]	opx	= ir[ 5: 0];
wire		[ 4: 0]	rs		= ir[25:21];
wire		[ 4: 0]	rt		= ir[20:16];
wire		[ 4: 0]	rd		= ir[15:11];
wire		[ 4: 0]	sa		= ir[10: 6];
wire		[ 4: 0]	sav	= reg_file[rs];
wire		[15: 0]	imm16	= ir[15: 0];
wire		[25: 0]	imm26	= ir[25: 0];

// Instructions
wire					rtype		= (op == 6'h00);
wire					ADD		= rtype & (opx == 6'h20);
wire					ADDI		= (op == 6'h08);
wire					ADDIU		= (op == 6'h09);
wire					ADDU		= rtype & (opx == 6'h21);
wire					AND		= rtype & (opx == 6'h24);
wire					ANDI		= (op == 6'h0C);
wire					BEQ		= (op == 6'h04);
wire					BGEZ		= (op == 6'h01) & (rt == 5'h01);
wire					BGEZAL	= (op == 6'h01) & (rt == 5'h11);
wire					BGTZ		= (op == 6'h07);
wire					BLEZ		= (op == 6'h06);
wire					BLTZ		= (op == 6'h01) & (rt == 5'h00);
wire					BLTZAL	= (op == 6'h01) & (rt == 5'h10);
wire					BNE		= (op == 6'h05);
wire					BREAK		= rtype & (opx == 6'h0D);
wire					COP0		= (op == 6'h10);
wire					COP1		= (op == 6'h11);
wire					COP2		= (op == 6'h12);
wire					COP3		= (op == 6'h13);
wire					DIV		= rtype & (opx == 6'h1A);
wire					DIVU		= rtype & (opx == 6'h1B);
wire					J			= (op == 6'h02);
wire					JAL		= (op == 6'h03);
wire					JALR		= rtype & (opx == 6'h09);
wire					JR			= rtype & (opx == 6'h08);
wire					LB			= (op == 6'h20);
wire					LBU		= (op == 6'h24);
wire					LH			= (op == 6'h21);
wire					LHU		= (op == 6'h25);
wire					LUI		= (op == 6'h0F);
wire					LW			= (op == 6'h23);
wire					LWC1		= (op == 6'h31);
wire					LWC2		= (op == 6'h32);
wire					LWC3		= (op == 6'h33);
wire					LWL		= (op == 6'h22);
wire					LWR		= (op == 6'h26);
wire					MFHI		= rtype & (opx == 6'h10);
wire					MFLO		= rtype & (opx == 6'h12);
wire					MTHI		= rtype & (opx == 6'h11);
wire					MTLO		= rtype & (opx == 6'h13);
wire					MULT		= rtype & (opx == 6'h18);
wire					MULTU		= rtype & (opx == 6'h19);
wire					NOR		= rtype & (opx == 6'h27);
wire					OR			= rtype & (opx == 6'h25);
wire					ORI		= (op == 6'h0D);
wire					SB			= (op == 6'h28);
wire					SH			= (op == 6'h29);
wire					SLL		= rtype & (opx == 6'h00);
wire					SLLV		= rtype & (opx == 6'h04);
wire					SLT		= rtype & (opx == 6'h2A);
wire					SLTI		= (op == 6'h0A);
wire					SLTIU		= (op == 6'h0B);
wire					SLTU		= rtype & (opx == 6'h2B);
wire					SRA		= rtype & (opx == 6'h03);
wire					SRAV		= rtype & (opx == 6'h07);
wire					SRL		= rtype & (opx == 6'h02);
wire					SRLV		= rtype & (opx == 6'h06);
wire					SUB		= rtype & (opx == 6'h22);
wire					SUBU		= rtype & (opx == 6'h23);
wire					SW			= (op == 6'h2B);
wire					SWC1		= (op == 6'h39);
wire					SWC2		= (op == 6'h3A);
wire					SWC3		= (op == 6'h3B);
wire					SWL		= (op == 6'h2A);
wire					SWR		= (op == 6'h2E);
wire					SYSCALL	= rtype & (opx == 6'h0C);
wire					XOR		= rtype & (opx == 6'h26);
wire					XORI		= (op == 6'h0E);
wire					alu		= ADD | ADDI | ADDIU | ADDU | AND | ANDI | DIV | DIVU | LUI | MULT | MULTU | 
										NOR | OR | ORI | SLL | SLLV | SLT | SLTI | SLTIU | SLTU | SRA | 
										SRAV | SRL | SRLV | SUB | SUBU | XOR | XORI;
wire					load		= LB | LBU | LH | LHU | LW | LWC1 | LWC2 | LWC3 | LWL | LWR;
wire					store		= SB | SH | SW | SWC1 | SWC2 | SWC3 | SWL | SWR;
wire					mem		= load | store;
wire					none		= ~(ADD | ADDI | ADDIU | ADDU | AND | ANDI | BEQ | BGEZ | BGEZAL |
										BGTZ | BLEZ | BLTZ | BLTZAL | BNE | BREAK | COP0 | COP1 | COP2 | COP3 |
										DIV | DIVU | J | JAL | JALR | JR | LUI | mem | MFHI | MFLO |
										MTHI | MTLO | MULT | MULTU | NOR | OR | ORI | SLL | SLLV | 
										SLT | SLTI | SLTIU | SLTU | SRA | SRAV | SRL | SRLV | SUB | SUBU |
										SYSCALL | XOR | XORI
									);

///////////////////////////////////////////////////////////////////////////////
//                              Operand Stage                                //
///////////////////////////////////////////////////////////////////////////////
wire		[31: 0]	opA = (BGEZAL | BLTZAL | JAL | JALR) ? pc : 
								(LUI)	?									{imm16, 16'd0} :
								(SLL | SRA | SRL) ?					sa :
								(SLLV | SRAV | SRLV) ?				sav :
																			reg_file[rs];
wire		[31: 0]	opB = (BGEZAL | BLTZAL | JAL | JALR) ? 32'd8 : 
								(ADDI | ADDIU | SLTI) ?				{{16{imm16[15]}}, imm16} : 
								(ANDI | ORI | SLTIU | XORI) ?		{16'd0, imm16} :
																			reg_file[rt];


///////////////////////////////////////////////////////////////////////////////
//                               Branch Stage                                //
///////////////////////////////////////////////////////////////////////////////
wire					ctrl_flow_al_taken =	(BGEZAL & (~reg_file[rs][31])) ? 1'b1 :
													(BLTZAL & (reg_file[rs][31])) ? 1'b1 :
													JAL | JALR;

wire   rs_is_zero = (reg_file[rs] == 32'd0) ? 1'b1 : 1'b0;

assign branchpc	=	(BEQ & ($signed(reg_file[rs]) == $signed(reg_file[rt]))) ?	nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
					 		(BGEZ & (~reg_file[rs][31])) ?										nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
					 		(BGEZAL & (~reg_file[rs][31])) ?										nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
						 	(BGTZ & (~reg_file[rs][31] | ~rs_is_zero)) ?						nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
						 	(BLEZ & (reg_file[rs][31] | rs_is_zero)) ?						nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
					 		(BLTZ & (reg_file[rs][31])) ?											nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
						 	(BLTZAL & (reg_file[rs][31])) ?										nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
						 	(BNE & ($signed(reg_file[rs]) != $signed(reg_file[rt]))) ?	nextpc + {{14{imm16[15]}}, imm16, 2'h0} :
						 	(J | JAL) ?																	{nextpc[31:28], imm26, 2'h0} :
						 	(JR | JALR) ?																reg_file[rs] :
																									 		nextpc + 32'd4;

assign loadpc = 	(BEQ & ($signed(reg_file[rs]) == $signed(reg_file[rt]))) ?	1'b1 :
					 	(BGEZ & (~reg_file[rs][31])) ?										1'b1 :
					 	(BGEZAL & (~reg_file[rs][31])) ?										1'b1 :
					 	(BGTZ & (~reg_file[rs][31] & ~rs_is_zero)) ?						1'b1 :
					 	(BLEZ & (reg_file[rs][31] | rs_is_zero)) ?						1'b1 :
					 	(BLTZ & (reg_file[rs][31])) ?											1'b1 :
					 	(BLTZAL & (reg_file[rs][31])) ?										1'b1 :
					 	(BNE & ($signed(reg_file[rs]) != $signed(reg_file[rt]))) ?	1'b1 :
					 	(J | JAL) ?																	1'b1 :
					 	(JR | JALR) ?																1'b1 :
															 											1'b0;


///////////////////////////////////////////////////////////////////////////////
//                              Execute Stage                                //
///////////////////////////////////////////////////////////////////////////////
wire		[31: 0]	add_result	= opA + opB;
wire		[31: 0]	and_result	= opA & opB;
wire		[31: 0]	lui_result	= opA;
wire		[31: 0]	mfhi_result	= HI;
wire		[31: 0]	mflo_result	= LO;
wire		[31: 0]	nor_result	= ~(opA | opB);
wire		[31: 0]	or_result	= opA | opB;
wire		[31: 0]	sll_result	= opB << opA;
wire		[31: 0]	slt_result	= ($signed(opA) < $signed(opB)) ? 32'd1 : 32'd0;
wire		[31: 0]	sltu_result	= ($unsigned(opA) < $unsigned(opB)) ? 32'd1 : 32'd0;
wire		[31: 0]	sra_result	= {{32{opB[31]}}, opB} >> opA;
wire		[31: 0]	srl_result	= {32'd0, opB} >> opA;
wire		[31: 0]	sub_result	= opA - opB;
wire		[31: 0]	xor_result	= opA ^ opB;

wire		[31: 0]	exe_result	= 	(AND | ANDI) ?		and_result :
											(LUI) ?				lui_result :
											(MFHI) ?				mfhi_result :
											(MFLO) ?				mflo_result :
											(NOR) ?				nor_result :
											(OR | ORI) ?		or_result :
											(SLL | SLLV) ?		sll_result :
											(SLT | SLTI) ?		slt_result :
											(SLTU | SLTIU) ?	sltu_result :
											(SRA | SRAV) ?		sra_result :
											(SRL | SRLV) ?		srl_result :
											(SUB | SUBU) ?		sub_result :
											(XOR | XORI) ?		xor_result :
																	add_result;

///////////////////////////////////////////////////////////////////////////////
//                               Memory Stage                                //
///////////////////////////////////////////////////////////////////////////////
reg					load_outstanding;
//reg					load_was_lb;
//reg					load_was_lbu;
//reg					load_was_lw;
//reg		[ 4: 0]	reg_to_load;

//wire					load_done	= avm_dm_readdatavalid & (avm_dm_read | load_outstanding);
wire		[31: 0]	load_data	= 	
		LB ?
			(avm_dm_byteenable[3:0] == 4'h1) ?	{{24{avm_dm_readdata[ 7]}}, avm_dm_readdata[ 7: 0]} :
			(avm_dm_byteenable[3:0] == 4'h2) ?	{{24{avm_dm_readdata[15]}}, avm_dm_readdata[15: 8]} :
			(avm_dm_byteenable[3:0] == 4'h4) ?	{{24{avm_dm_readdata[23]}}, avm_dm_readdata[23:16]} :
															{{24{avm_dm_readdata[31]}}, avm_dm_readdata[31:24]} :
		LBU ?
			(avm_dm_byteenable[3:0] == 4'h1) ?	{24'd0, avm_dm_readdata[ 7: 0]} :
			(avm_dm_byteenable[3:0] == 4'h2) ?	{24'd0, avm_dm_readdata[15: 8]} :
			(avm_dm_byteenable[3:0] == 4'h4) ?	{24'd0, avm_dm_readdata[23:16]} :
															{24'd0, avm_dm_readdata[31:24]} :
		LH ?
			(avm_dm_byteenable[3:0] == 4'h3) ?	{{16{avm_dm_readdata[15]}}, avm_dm_readdata[15: 0]} :
															{{16{avm_dm_readdata[31]}}, avm_dm_readdata[31:16]} :
		LHU ?
			(avm_dm_byteenable[3:0] == 4'h3) ?	{16'd0, avm_dm_readdata[15: 0]} :
															{16'd0, avm_dm_readdata[31:16]} :
			avm_dm_readdata;

assign avm_dm_address		= {{16{imm16[15]}}, imm16} + reg_file[rs];
assign avm_dm_writedata		= (SW) ? reg_file[rt] : ((SH) ? {2{reg_file[rt][15:0]}} : {4{reg_file[rt][7:0]}});
assign avm_dm_byteenable	= (SW | LW) ? 4'hF : ((SH | LH | LHU) ? (avm_dm_address[1] == 1'b0 ? 4'h3 : 4'hC) : (avm_dm_address[1:0] == 2'h0 ? 4'h1 : (avm_dm_address[1:0] == 2'h1 ? 4'h2 : (avm_dm_address[1:0] == 2'h2 ? 4'h4 : 4'h8))));
assign avm_dm_read			= ir_valid & load & ~load_outstanding;
assign avm_dm_write			= ir_valid & store & ~load_outstanding;

always @(posedge clk)
begin
	if (reset)
		load_outstanding <= 1'b0;
	else if (load_outstanding & avm_dm_readdatavalid)
		load_outstanding <= 1'b0;
	else if (load & ir_valid & ~avm_dm_waitrequest & ~avm_dm_readdatavalid)
		load_outstanding <= 1'b1;
end


///////////////////////////////////////////////////////////////////////////////
//                             Writeback Stage                               //
///////////////////////////////////////////////////////////////////////////////
wire					wr_to_ra		=	BGEZAL | BLTZAL | JAL;
wire					wr_to_rt		=	ADDI | ADDIU | ANDI | load | LUI | ORI | SLTI | SLTIU | XORI;

wire		[ 4: 0]	rf_wr_addr	= wr_to_ra ? 5'h1F : wr_to_rt ? rt : rd;
wire		[31: 0]	rf_wr_data	= load ? load_data : exe_result;
wire					rf_wr_en		= ir_valid & instr_done & (rf_wr_addr != 5'h00) & (load | 
											(alu & ~DIV & ~DIVU & ~MULT & ~MULTU) | 
											MFHI | MFLO | ctrl_flow_al_taken);

always @(posedge clk)
begin
	if (reset)
	begin
		for (i = 0; i < 32; i=i+1)
		begin
			reg_file[i] <= 32'd0;
		end
	end
	else if (rf_wr_en)
	begin
		reg_file[rf_wr_addr]	<= rf_wr_data;
	end
end

always @(posedge clk)
begin
	if (reset)
	begin
		LO <= 32'd0;
		HI <= 32'd0;
	end
	else if (ir_valid)
	begin
		if (DIV | DIVU)
		begin
			LO <= reg_file[rs] / reg_file[rt];
			HI <= reg_file[rs] % reg_file[rt];
		end
		else if (MTHI)
			HI <= reg_file[rs];
		else if (MTLO)
			LO <= reg_file[rs];
		else if (MULT)
			{HI, LO} <= reg_file[rs] * reg_file[rt];
		else if (MULTU)
			{HI, LO} <= {{32{1'b0}} , reg_file[rs]} * {{32{1'b0}}, reg_file[rt]};
	end
end


///////////////////////////////////////////////////////////////////////////////
//                              Control Logic                                //
///////////////////////////////////////////////////////////////////////////////
assign instr_done	= (~store | ~avm_dm_waitrequest) & (~load | avm_dm_readdatavalid) & (~loadpc | loadedpc);


///////////////////////////////////////////////////////////////////////////////
//                             Error Checking                                //
///////////////////////////////////////////////////////////////////////////////
// Checking register file writes
reg		[31: 0]	rf_address_handle;
reg		[31: 0]	rf_data_handle;
reg		[31: 0]	rf_writes;
reg		[31: 0]	rf_addresses	[5000000:0];
reg		[31: 0]	rf_data 			[5000000:0];
initial
begin
	if (CAPTURE_TRANSACTIONS == 1)
	begin
		rf_address_handle	= $fopen (RF_ADDRESS_FILENAME);
		rf_data_handle		= $fopen (RF_DATA_FILENAME);
	end
	else
	begin
//		$readmemh(RF_ADDRESS_FILENAME,	rf_addresses);
//		$readmemh(RF_DATA_FILENAME,		rf_data);
		$readmemh("new_rf_addresses.dat",	rf_addresses);
		$readmemh("new_rf_data.dat",			rf_data);
	end

	rf_writes <= 32'h0;
end
always @(posedge clk)
begin
	if (CAPTURE_TRANSACTIONS == 1)
	begin
		if (rf_wr_en)
		begin
			$fwrite (rf_address_handle, "%x\n", rf_wr_addr);
			$fwrite (rf_data_handle, "%x\n", rf_wr_data);
			$fflush (rf_address_handle);
			$fflush (rf_data_handle);
			rf_writes <= rf_writes + 1;
		end
	end
	else
	begin
		if (rf_wr_en)
		begin
			if ((rf_wr_addr != rf_addresses[rf_writes]) || (rf_wr_data != rf_data[rf_writes]))
			begin
				if ((rf_wr_addr == rf_addresses[rf_writes]) && (rf_wr_data == 32'h00000060))
				begin
					if (rf_data[rf_writes + 1] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 2;
					end
					else if (rf_data[rf_writes + 2] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 3;
					end
					else if (rf_data[rf_writes + 3] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 4;
					end
					else if (rf_data[rf_writes + 4] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 5;
					end
					else if (rf_data[rf_writes + 5] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 6;
					end
					else if (rf_data[rf_writes + 6] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 7;
					end
					else if (rf_data[rf_writes + 7] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 8;
					end
					else if (rf_data[rf_writes + 8] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 9;
					end
					else if (rf_data[rf_writes + 9] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 10;
					end
					else if (rf_data[rf_writes + 10] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 11;
					end
					else if (rf_data[rf_writes + 11] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 12;
					end
					else if (rf_data[rf_writes + 12] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 13;
					end
					else if (rf_data[rf_writes + 13] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 14;
					end
					else if (rf_data[rf_writes + 14] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 15;
					end
					else if (rf_data[rf_writes + 15] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 16;
					end
					else if (rf_data[rf_writes + 16] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 17;
					end
					else if (rf_data[rf_writes + 17] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 18;
					end
					else if (rf_data[rf_writes + 18] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 19;
					end
					else if (rf_data[rf_writes + 19] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 20;
					end
					else if (rf_data[rf_writes + 20] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 21;
					end
					else if (rf_data[rf_writes + 21] == 32'h00000060)
					begin
						rf_writes <= rf_writes + 22;
					end
					else
					begin
						rf_writes <= rf_writes + 23;
					end
				end
				else if ((rf_wr_addr == rf_addresses[rf_writes]) && (rf_wr_data == 32'h00000040))
				begin
					rf_writes <= rf_writes;
				end
				else
				begin
					$display ("Register file write %d has an error!\n", rf_writes + 1);
					$display ("Expected address: %02x", rf_addresses[rf_writes]);
					$display ("Actual address:   %02x", rf_wr_addr);
					$display ("Expected data:    %08x", rf_data[rf_writes]);
					$display ("Actual data:      %08x", rf_wr_data);
					$stop;
				end
			end
			else
				rf_writes <= rf_writes + 1;
		end
	end
end

// Check for unimplemented/unknown instructions
always @(posedge clk)
begin
	if (ir_valid & none)
	begin
		$displayh("Found unimplemented instruction!");
		$displayh("PC:", pc);
		$displayh("IR:", ir);
		$displayh("OP:", op);
		$displayh("OPX:", opx);
		$displayh("RS:", rs);
		$displayh("RT:", rt);
		$displayh("RD:", rd);
		$displayh("IMM16:", imm16);
		$stop;
	end
	else if (ir_valid & BREAK)
	begin
		$displayh("Hit break instruction!");
		$displayh("PC:", pc);
		$displayh("IR:", ir);
		$stop;
	end
	else if (ir_valid & SYSCALL)
	begin
		$displayh("Hit syscall instruction!");
		$displayh("PC:", pc);
		$displayh("IR:", ir);
		$stop;
	end
	else if (ir_valid & (COP0 | COP1 | COP2 | COP3))
	begin
		$displayh("Hit coprocessor instruction!");
		$displayh("PC:", pc);
		$displayh("IR:", ir);
		$stop;
	end
	else if (ir_valid & (LWC1 | LWC2 | LWC3 | SWC1 | SWC2 | SWC3))
	begin
		$displayh("Hit coprocessor mem instruction!");
		$displayh("PC:", pc);
		$displayh("IR:", ir);
		$stop;
	end
	else if (ir_valid & mem & ~SW & ~LW & ~SH & ~ LH & ~ LHU & ~LB & ~LBU & ~SB)
	begin
		$displayh("Hit memory instruction!");
		$displayh("PC:", pc);
		$displayh("IR:", ir);
		$displayh("OP:", op);
		$displayh("OPX:", opx);
		$displayh("RS:", rs);
		$displayh("RT:", rt);
		$displayh("RD:", rd);
		$displayh("IMM16:", imm16);
		$stop;
	end
end

endmodule




module SM_MIPSI_Fetch_Unit #(
///////////////////////////////////////////////////////////////////////////////
//                                Parameters                                 //
///////////////////////////////////////////////////////////////////////////////
parameter RESET_ADDRESS	 	= 32'h00000000,
parameter PREFETCH_DEPTH	= 4

) (
///////////////////////////////////////////////////////////////////////////////
//                                  Ports                                    //
///////////////////////////////////////////////////////////////////////////////
// Inputs
input 					clk,
input 					reset,

input			[31: 0]	i_readdata,
input						i_readdatavalid,
input						i_waitrequest,

input			[31: 0]	branch_address,
input						branch_enable,

input						read_ir,

// Bidirectionals

// Outputs
output reg	[31: 0]	i_address,
output reg				i_read,

output					branched,

output		[31: 0]	pc,
output		[31: 0]	ir,
output					ir_valid
);

///////////////////////////////////////////////////////////////////////////////
//                                Constants                                  //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                             Internal Signals                              //
///////////////////////////////////////////////////////////////////////////////
// Internal Wires
reg						load_address	[PREFETCH_DEPTH:1];
reg						load_data		[PREFETCH_DEPTH:1];
reg						shift				[PREFETCH_DEPTH:1];

// Internal Registers
reg			[31: 0]	prefetched_address			[PREFETCH_DEPTH + 1:1];
reg						prefetched_address_valid	[PREFETCH_DEPTH + 1:0];
reg			[31: 0]	prefetched_data				[PREFETCH_DEPTH + 1:1];
reg						prefetched_data_valid		[PREFETCH_DEPTH + 1:0];

// State Machine Registers

// Internal Variables
integer					i;

///////////////////////////////////////////////////////////////////////////////
//                         Finite State Machine(s)                           //
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//                             Sequential logic                              //
///////////////////////////////////////////////////////////////////////////////
// Output Registers
initial											i_address 	<= RESET_ADDRESS;
always @(posedge clk)
begin
	if (reset)									i_address 	<= RESET_ADDRESS;
	else if (branched)						i_address	<= branch_address;
	else if (i_read & ~i_waitrequest)	i_address	<= i_address + 32'd4;
end

initial											i_read		<= 1'b0;
always @(posedge clk)
begin
	if (reset)									i_read		<= 1'b0;
	else if (i_read & ~i_waitrequest)	i_read		<= ~prefetched_address_valid[PREFETCH_DEPTH - 1];
	else if (~i_read)							i_read		<= ~prefetched_address_valid[PREFETCH_DEPTH];
end

// Internal Registers
always @(posedge clk)
begin
	if (reset)
	begin
		for ( i = 1; i <= PREFETCH_DEPTH + 1; i = i + 1 )
		begin
			prefetched_address[i]			<= 32'h0;
			prefetched_address_valid[i]	<=  1'b0;
			prefetched_data[i]				<= 32'h0;
			prefetched_data_valid[i]		<=  1'b0;
		end
		prefetched_address_valid[0]		<=  1'b1;
		prefetched_data_valid[0]			<=  1'b1;
	end
	else
	begin
		for ( i = 1; i <= PREFETCH_DEPTH; i = i + 1 )
		begin
			if (shift[i] & load_address[i + 1])
			begin
				prefetched_address[i]			<= i_address;
				prefetched_address_valid[i]	<= 1'b1;
			end
			else if (shift[i])
			begin
				prefetched_address[i]			<= prefetched_address[i + 1];
				prefetched_address_valid[i]	<= prefetched_address_valid[i + 1];
			end
			else if (load_address[i])
			begin
				prefetched_address[i]			<= i_address;
				prefetched_address_valid[i]	<= 1'b1;
			end

			if (shift[i] & load_data[i + 1])
			begin
				prefetched_data[i]			<= i_readdata;
				prefetched_data_valid[i]	<= i_readdatavalid & prefetched_address_valid[i + 1]; // Add 2nd arg only to correct bad bus transactions: waitrequest get low as readdatavalid goes high
			end
			else if (shift[i])
			begin
				prefetched_data[i]			<= prefetched_data[i + 1];
				prefetched_data_valid[i]	<= prefetched_data_valid[i + 1];
			end
			else if (load_data[i])
			begin
				prefetched_data[i]			<= i_readdata;
				prefetched_data_valid[i]	<= i_readdatavalid & prefetched_address_valid[i]; // Add 2nd arg only to correct bad bus transactions: waitrequest get low as readdatavalid goes high
			end
		end
	end
end

///////////////////////////////////////////////////////////////////////////////
//                           Combinational logic                             //
///////////////////////////////////////////////////////////////////////////////
// Output Assignments
assign branched 	= branch_enable & ((i_read & ~i_waitrequest) | ~i_read);

assign pc			= prefetched_address[1];
assign ir			= prefetched_data[1];
assign ir_valid	= prefetched_data_valid[1];

// Internal Assignments
always @(*)
begin
	for ( i = 1; i <= PREFETCH_DEPTH; i = i + 1 ) 
	begin
		shift[i]				= read_ir & prefetched_data_valid[1];
		load_data[i]		= i_readdatavalid & ~prefetched_data_valid[i] & prefetched_data_valid[i-1];
		load_address[i]	= i_read & ~i_waitrequest & ~prefetched_address_valid[i] & prefetched_address_valid[i-1];
	end
end

///////////////////////////////////////////////////////////////////////////////
//                             Internal Modules                              //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                         Simulation Error Checking                         //
///////////////////////////////////////////////////////////////////////////////

endmodule


