module ztiger_datapath(
	input clock, 
	input stall_fetch, stall_decode, reset_wb,
		
	input [31:0] r0, r1, r2, r3, r4, r5, r6, r7,
	input [31:0] control_ex,
	input [31:0] control_wb,

	input [31:0] instruction,
	output[31:0] rs, rt,

	input [7:0]  data_mux_sel,
	input [31:0] add_result,
	output[31:0] dataa, datab	
);
	
	wire [31:0] result;
	genvar i;
	generate
		for( i=0 ; i<32 ; i = i+1)
		begin: result_mux
			assign result[i] = (r0[i] & control_wb[8]) | (r1[i] & control_wb[9]) | (r2[i] & control_wb[10]) | (r3[i] & control_wb[11]) | 
			                   (r4[i] & control_wb[12]) | (r5[i] & control_wb[13]) | (r6[i] & control_wb[14]) | (r7[i] & control_wb[15]);
		end
	endgenerate

	/********** 	variables 			**********/
	reg [31:0] instr;
	reg [4:0]  rdaddr_rs_de, rdaddr_rt_de;
	
	/********** 	wb stage 			**********/
	wire       wren      = ~reset_wb & control_wb[4:0] != 5'd0;
	wire [4:0] wraddr    = control_wb[4:0];	
	
	// register file
	wire [31:0] rs_q, rt_q;
	ztiger_reg_file rs_file(
		.clock          (clock),
		.wraddress      (wraddr),
		.wren           (wren),
		.data           (result),
		.rdaddress      (instruction[25:21]),
		.rd_addressstall(stall_fetch),
		.q              (rs_q)
	);

	ztiger_reg_file rt_file(
		.clock          (clock),
		.wraddress      (wraddr),
		.wren           (wren),
		.data           (result),
		.rdaddress      (instruction[20:16]),
		.rd_addressstall(stall_fetch),
		.q              (rt_q)
	);

	// data forwarding
	reg [31:0] data_rs,        data_rt;
	reg [4:0]  wraddr_buff_rs, wraddr_buff_rt;
	reg        wren_buff_rs,   wren_buff_rt;
	
	wire rs_sel    = (wraddr_buff_rs == instr[25:21]) & wren_buff_rs & (control_wb[4:0] != instr[25:21]);
	wire rt_sel    = (wraddr_buff_rt == instr[20:16]) & wren_buff_rt & (control_wb[4:0] != instr[20:16]);	
	wire rs_sel_de = (wraddr_buff_rs == rdaddr_rs_de) & wren_buff_rs;
	wire rt_sel_de = (wraddr_buff_rt == rdaddr_rt_de) & wren_buff_rt;	
	
	always @ (posedge clock)
	begin
		if((stall_fetch & rs_sel) | stall_decode)
		begin
			// stall
		end
		else begin
			data_rs        <= result;
			wraddr_buff_rs <= wraddr;
			wren_buff_rs   <= wren;
		end
	end		
	
	always @ (posedge clock)
	begin
		if((stall_fetch & rt_sel) | stall_decode)
		begin
			// stall
		end
		else begin
			data_rt        <= result;
			wraddr_buff_rt <= wraddr;
			wren_buff_rt   <= wren;
		end
	end
	
	/********** 	fetch stage 		**********/
	always @ (posedge clock)
	begin
		if(stall_fetch)
			;// stall
		else
			instr <= instruction;		
	end

	/**********		decode stage 		**********/		
	assign rs = rs_sel ? data_rs : rs_q;
	assign rt = rt_sel ? data_rt : rt_q;
	
	reg  [31:0] rs_buff, rt_buff;
	reg  [15:0] imm;	
	always @ (posedge clock)
	begin
		if(stall_decode)
		begin
			// stall
		end
		else begin
			rs_buff <= rs;
			rt_buff <= rt;	
			imm <= instr[15:0];	
		end
	end		
		
	// for data forwarding	
	always @ (posedge clock)
	begin
		if(stall_decode)
		begin
			// stall
		end
		else begin
			rdaddr_rs_de <= instr[25:21];
			rdaddr_rt_de <= instr[20:16];
		end
	end			
		
	/********** 	execution stage	**********/
	wire [31:0] rs_ex = rs_sel_de ? data_rs : rs_buff;
	wire [31:0] rt_ex = rt_sel_de ? data_rt : rt_buff;	
	
	wire [31:0] shamt = { 27'd0, imm[10:6] };
	generate
		for( i=0 ; i<32 ; i = i+1)
		begin: dataa_mux
			assign dataa[i] = (rs_ex[i] & data_mux_sel[7]) | (add_result[i] & data_mux_sel[6]) | (shamt[i] & data_mux_sel[5]) | (result[i] & data_mux_sel[4]);
		end
	endgenerate

	wire [31:0] ZeroImm = {16'd0, imm[15:0]};
	wire [31:0] SignImm = {{16{imm[15]}}, imm[15:0]};
	generate
		for( i=0 ; i<32 ; i = i+1)
		begin: datab_mux
			assign datab[i] = (rt_ex[i] & data_mux_sel[3]) | (ZeroImm[i] & data_mux_sel[2]) | (SignImm[i] & data_mux_sel[1]) | (result[i] & data_mux_sel[0]);
		end
	endgenerate
	
endmodule 
